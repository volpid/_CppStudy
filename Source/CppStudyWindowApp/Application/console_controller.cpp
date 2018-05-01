
#include "console_controller.h"

#include "../Resource/resource.h"

#include <dinput.h>
#include <iostream>
#include <joystickapi.h>
#include <Xinput.h>

/* order dependant */
#include <WinInet.h>
#include <ShlObj.h>

/* import library */
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Xinput.lib")

//-----------------------------------------------------------------------------------------
// Local
//-----------------------------------------------------------------------------------------
namespace
{
    struct JoyStickCallbackData
    {
        IDirectInputDevice8W*   device;
        int                     objectCount;
        int                     axisCount;
        int                     sliderCount;
        int                     buttonCount;
        int                     povCount;
    };


    BOOL CALLBACK JoypadDeviceProc(const DIDEVICEINSTANCE* di, void* user)
    {
        ControllerApp* pControllerApp = dynamic_cast<ControllerApp*> (CoreWindowApp::GetInstance());
        if (pControllerApp == nullptr)
        {
            return false;
        }

        return pControllerApp->JoypadDeviceCallback(di, user);
    }

    BOOL CALLBACK JoyObjectProc(const DIDEVICEOBJECTINSTANCE* pInst, void* pUserData)
    {
        ControllerApp* pControllerApp = dynamic_cast<ControllerApp*> (CoreWindowApp::GetInstance());
        if (pControllerApp == nullptr)
        {
            return false;
        }

        return pControllerApp->JoyObjectCallback(pInst, pUserData);
    }
}

ControllerApp thisApp;

//-----------------------------------------------------------------------------------------
// ControllerApp
//-----------------------------------------------------------------------------------------
LRESULT ControllerApp::MsgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    const int MS_WM_XBUTTONDOWN	= 0x020B;
    const int MS_WM_XBUTTONUP = 0x020C;

	switch (message)
	{
        case WM_CREATE:
        {
            CreateWindowEx(NULL, "BUTTON", "XBox", WS_TABSTOP|WS_VISIBLE| WS_CHILD|BS_DEFPUSHBUTTON, 
                10, 10, 100, 24,
                hWnd, (HMENU)IDC_BUTTON_XBOX, hAppInst_, NULL);

            CreateWindowEx(NULL, "BUTTON", "Winmm", WS_TABSTOP|WS_VISIBLE| WS_CHILD|BS_DEFPUSHBUTTON, 
                10, 40, 100, 24,
                hWnd, (HMENU)IDC_BUTTON_WINMM, hAppInst_, NULL);

            CreateWindowEx(NULL, "BUTTON", "DInput", WS_TABSTOP|WS_VISIBLE| WS_CHILD|BS_DEFPUSHBUTTON, 
                10, 70, 100, 24,
                hWnd, (HMENU)IDC_BUTTON_DINPUT, hAppInst_, NULL);
        }
        break;
        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            int wmEvent = HIWORD(wParam);
            switch (wmId)
            {
                case IDC_BUTTON_XBOX:
                {
                    SetControllerType(ControllerType::Xbox);
                    return 0;
                }
                break;
                case IDC_BUTTON_WINMM:
                {
                    SetControllerType(ControllerType::Winmm);
                    return 0;
                }
                break;
                case IDC_BUTTON_DINPUT:
                {
                    SetControllerType(ControllerType::DInput);
                    return 0;
                }
                break;
            }
        }
        break;
        //@note Device changed
        case WM_DEVICECHANGE:
        {
            std::cout << "devcie changed!" << std::endl;
        }
        break;
        case MS_WM_XBUTTONDOWN:
        case MS_WM_XBUTTONUP:
        //case MS_MK_BUTTON4:
        //case MS_MK_BUTTON5:
        {
            std::cout << "ms_WM changed!" << std::endl;
        }
        break;
	}
    return CoreWindowApp::MsgProc(hWnd, message, wParam, lParam);
}

bool ControllerApp::InitializeApp(void)
{
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        return false;
    }

    TestCOMObject();
    InitializeController();
    return true;
}

void ControllerApp::Update(void)
{
    switch (controllerType_)
    {
        case ControllerType::Xbox:
        {
            ProcessXBoxController();
        }
        break;
        case ControllerType::Winmm:
        {
            ProcessJoyStickController();
        }
        break;        
        case ControllerType::DInput:
        {
            ProcessDInputController();
        }
        break;
    }
}

void ControllerApp::CleanUp(void)
{
    CoUninitialize();
}

BOOL ControllerApp::JoypadDeviceCallback(const DIDEVICEINSTANCE* di, void* user)
{
    DIDEVCAPS deviceCap;
    DIPROPDWORD dipd;

    HRESULT hr = pDirectInput8_->CreateDevice(di->guidInstance, &pDirectInputDevice8_, nullptr);
    if (!SUCCEEDED(hr))
    {
        std::cout << "s_pDirectInput8->CreateDevice fail" << std::endl;
        return DIENUM_STOP;
    }

    hr = pDirectInputDevice8_->SetDataFormat(&c_dfDIJoystick2);
    if (!SUCCEEDED(hr))
    {
        std::cout << "SetDataFormat fail" << std::endl;
        return DIENUM_STOP;
    }

    memset(&deviceCap, 0, sizeof(deviceCap));
    deviceCap.dwSize = sizeof(deviceCap);    
    hr = pDirectInputDevice8_->GetCapabilities(&deviceCap);
    if (!SUCCEEDED(hr))
    {
        std::cout << "GetCapabilities fail" << std::endl;
        return DIENUM_STOP;
    }

    memset(&dipd, 0, sizeof(dipd));
    dipd.diph.dwSize = sizeof(dipd);
    dipd.diph.dwHeaderSize = sizeof(dipd.diph);
    dipd.diph.dwHow = DIPH_DEVICE;
    dipd.dwData = DIPROPAXISMODE_ABS;
    hr = pDirectInputDevice8_->SetProperty(DIPROP_AXISMODE, &dipd.diph);
    if (!SUCCEEDED(hr))
    {
        std::cout << "DIPROP_AXISMODE fail" << std::endl;
        return DIENUM_STOP;
    }

    JoyStickCallbackData data;
    memset(&data, 0, sizeof(data));
    //hr = pDirectInputDevice8_->EnumObjects(JoyObjectProc, &data, DIDFT_AXIS | DIDFT_BUTTON | DIDFT_POV);
    hr = pDirectInputDevice8_->EnumObjects(JoyObjectProc, &data, DIDFT_ALIAS);
    if (!SUCCEEDED(hr))
    {
        std::cout << "JoyObjectCallback fail" << std::endl;
        return DIENUM_STOP;
    }

    std::cout << "udata obj :" << data.objectCount << std::endl;
    std::cout << "udata axis :" << data.axisCount << std::endl;
    std::cout << "udata button :" << data.buttonCount << std::endl;
    std::cout << "udata pov :" << data.povCount << std::endl;

    return DIENUM_CONTINUE;
}

BOOL ControllerApp::JoyObjectCallback(const DIDEVICEOBJECTINSTANCE* pInst, void* pUserData)
{
    JoyStickCallbackData* pData = (JoyStickCallbackData*) pUserData;

    if (DIDFT_GETTYPE(pInst->dwType) & DIDFT_AXIS)
    {
        ++pData->axisCount;
    }
    if (DIDFT_GETTYPE(pInst->dwType) & DIDFT_BUTTON)
    {
        ++pData->buttonCount;
    }
    if (DIDFT_GETTYPE(pInst->dwType) & DIDFT_POV)
    {
        ++pData->povCount;
    }

    ++pData->objectCount;
    return TRUE;
}

void ControllerApp::InitializeController(void)
{
    MMRESULT mmr;
    int numJoysticks = joyGetNumDevs();
    joyInfoExPrev_.resize(numJoysticks);

    for (int idx = 0; idx < numJoysticks; ++idx)
    {
        JOYINFOEX& joyInfoEx = joyInfoExPrev_[idx];

        memset(&joyInfoEx, 0, sizeof(joyInfoEx));
        joyInfoEx.dwSize = sizeof(joyInfoEx);
        joyInfoEx.dwFlags = JOY_RETURNALL;

        mmr = joyGetPosEx(idx, &joyInfoEx);
        if (mmr == JOYERR_NOERROR)
        {
            std::cout << "joypad - idx " << idx << " is inited" << std::endl;
        }
    }
   
    if (joySetCapture(hMainWnd_, 0, 0, false))
    {
        std::cout << "joy captture success" << std::endl;
    }

    HRESULT hr = DirectInput8Create(hAppInst_, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**) &pDirectInput8_, nullptr);
    if (!SUCCEEDED(hr))
    {
        std::cout << "dx joystick fail" << std::endl;
    }

    hr = pDirectInput8_->EnumDevices(DI8DEVCLASS_GAMECTRL, JoypadDeviceProc, nullptr, DIEDFL_ALLDEVICES);
    if (!SUCCEEDED(hr))
    {   
        std::cout << "dx joystick enum fail" << std::endl;
    }
}

void ControllerApp::ProcessXBoxController(void)
{
    /*
        @Note : XBox controller keys
        #define XINPUT_GAMEPAD_DPAD_UP          0x0001
        #define XINPUT_GAMEPAD_DPAD_DOWN        0x0002
        #define XINPUT_GAMEPAD_DPAD_LEFT        0x0004
        #define XINPUT_GAMEPAD_DPAD_RIGHT       0x0008
        #define XINPUT_GAMEPAD_START            0x0010
        #define XINPUT_GAMEPAD_BACK             0x0020
        #define XINPUT_GAMEPAD_LEFT_THUMB       0x0040
        #define XINPUT_GAMEPAD_RIGHT_THUMB      0x0080
        #define XINPUT_GAMEPAD_LEFT_SHOULDER    0x0100
        #define XINPUT_GAMEPAD_RIGHT_SHOULDER   0x0200
        #define XINPUT_GAMEPAD_A                0x1000
        #define XINPUT_GAMEPAD_B                0x2000
        #define XINPUT_GAMEPAD_X                0x4000
        #define XINPUT_GAMEPAD_Y                0x8000
    */

    DWORD dwResult;
    for (DWORD idx = 0; idx < XUSER_MAX_COUNT; ++idx)
    {
        XINPUT_STATE state;
        ZeroMemory(&state, sizeof(state));
        
        dwResult = XInputGetState(idx, &state);
        if (dwResult == ERROR_SUCCESS)
        {   
            if (state.Gamepad.wButtons ^ xboxStatePrev_[idx].Gamepad.wButtons)
            {
                // test vibration with buttn A
                if (state.Gamepad.wButtons & XINPUT_GAMEPAD_A)
                {      
                    XINPUT_VIBRATION vibration;
                    vibration.wLeftMotorSpeed = 21156;
                    vibration.wRightMotorSpeed = 256;
                    XInputSetState(idx, &vibration);
                }
                else 
                {
                    XINPUT_VIBRATION vibration;
                    vibration.wLeftMotorSpeed = 0;
                    vibration.wRightMotorSpeed = 0;
                    XInputSetState(idx, &vibration);
                }

                std::cout << "button state has chagned." << (int) state.Gamepad.wButtons << std::endl;                
            }

            if (state.Gamepad.bLeftTrigger ^ xboxStatePrev_[idx].Gamepad.bLeftTrigger)
            {
                std::cout << "bLeftTrigger state has chagned." << (int) state.Gamepad.bLeftTrigger << std::endl;
            }

            if (state.Gamepad.bRightTrigger ^ xboxStatePrev_[idx].Gamepad.bRightTrigger)
            {
                std::cout << "bRightTrigger state has chagned." << (int) state.Gamepad.bRightTrigger << std::endl;                
            }

            memcpy(&xboxStatePrev_[idx], &state, sizeof(xboxStatePrev_[idx]));
        }
    }
}

void ControllerApp::ProcessJoyStickController(void)
{
    //mmr = joyGetPosEx(0, &m_JoyInfoEx);
    //mmr = joyGetPosEx(1, &m_JoyInfoEx);
    //mmr = joyGetPosEx(2, &m_JoyInfoEx);
    //mmr = joyGetPosEx(3, &m_JoyInfoEx);
    //mmr = joyGetPosEx(4, &m_JoyInfoEx);
    //mmr = joyGetPosEx(5, &m_JoyInfoEx);
    //mmr = joyGetPosEx(6, &m_JoyInfoEx);
    //mmr = joyGetPosEx(7, &m_JoyInfoEx);
    //mmr = joyGetPosEx(8, &m_JoyInfoEx);
    //mmr = joyGetPosEx(9, &m_JoyInfoEx);
    //mmr = joyGetPosEx(10, &m_JoyInfoEx);
    //mmr = joyGetPosEx(11, &m_JoyInfoEx);
    //mmr = joyGetPosEx(12, &m_JoyInfoEx);
    //mmr = joyGetPosEx(13, &m_JoyInfoEx);
    //mmr = joyGetPosEx(14, &m_JoyInfoEx);
    //mmr = joyGetPosEx(15, &m_JoyInfoEx);
    JOYINFOEX joyInfoEx;
    ZeroMemory(&joyInfoEx, sizeof(joyInfoEx));
    joyInfoEx.dwSize = sizeof(joyInfoEx);
    joyInfoEx.dwFlags = JOY_RETURNALL;
    
    MMRESULT mmr = joyGetPosEx(0, &joyInfoEx);
    if (mmr == JOYERR_NOERROR)
    {
        if (joyInfoEx.dwButtons ^ joyInfoExPrev_[0].dwButtons)
        {
            std::cout << "button state has chagned." << (int) joyInfoEx.dwButtons << std::endl;
            joyInfoExPrev_[0].dwButtons = joyInfoEx.dwButtons;
        }

        if (abs(static_cast<long long> (joyInfoEx.dwXpos - joyInfoExPrev_[0].dwXpos)) > 10000)
        {
            std::cout << "button dwXpos chagned." << joyInfoEx.dwXpos << (int) joyInfoEx.dwButtons << std::endl;
            joyInfoExPrev_[0].dwXpos = joyInfoEx.dwXpos;
        }
        if (abs(static_cast<long long> (joyInfoEx.dwYpos - joyInfoExPrev_[0].dwYpos)) > 10000)
        {
            std::cout << "button dwYpos chagned." << joyInfoEx.dwYpos << (int) joyInfoEx.dwButtons << std::endl;
            joyInfoExPrev_[0].dwYpos = joyInfoEx.dwYpos;
        }
        if (abs(static_cast<long long> (joyInfoEx.dwZpos - joyInfoExPrev_[0].dwZpos)) > 10000)
        {
            std::cout << "button dwZpos chagned." << joyInfoEx.dwZpos << (int) joyInfoEx.dwButtons << std::endl;
            joyInfoExPrev_[0].dwZpos = joyInfoEx.dwZpos;
        }
        if (abs(static_cast<long long> (joyInfoEx.dwRpos - joyInfoExPrev_[0].dwRpos)) > 10000)
        {
            std::cout << "button dwRpos chagned." << joyInfoEx.dwRpos << (int) joyInfoEx.dwButtons << std::endl;
            joyInfoExPrev_[0].dwRpos = joyInfoEx.dwRpos;
        }
        if (abs(static_cast<long long> (joyInfoEx.dwUpos - joyInfoExPrev_[0].dwUpos)) > 10000)
        {
            std::cout << "button dwUpos chagned." << joyInfoEx.dwUpos << (int) joyInfoEx.dwButtons << std::endl;
            joyInfoExPrev_[0].dwUpos = joyInfoEx.dwUpos;
        }
        if (abs(static_cast<long long> (joyInfoEx.dwVpos - joyInfoExPrev_[0].dwVpos)) > 10000)
        {
            std::cout << "button dwVpos chagned." << joyInfoEx.dwVpos << (int) joyInfoEx.dwButtons << std::endl;
            joyInfoExPrev_[0].dwVpos = joyInfoEx.dwVpos;
        }
        if (abs(static_cast<long long> (joyInfoEx.dwPOV - joyInfoExPrev_[0].dwPOV)) > 10000)
        {
            std::cout << "button dwPOV chagned." << joyInfoEx.dwPOV << (int) joyInfoEx.dwButtons << std::endl;
            joyInfoExPrev_[0].dwPOV = joyInfoEx.dwPOV;
        }
    }
}

void ControllerApp::ProcessDInputController(void)
{
    if (pDirectInputDevice8_ != nullptr)
    {
        pDirectInputDevice8_->Poll();
        
        DIJOYSTATE2 joyState;
        HRESULT hr = pDirectInputDevice8_->GetDeviceState(sizeof(joyState), &joyState);
        if (FAILED(hr))
        {
            pDirectInputDevice8_->Acquire();
            pDirectInputDevice8_->Poll();
            hr = pDirectInputDevice8_->GetDeviceState(sizeof(joyState), &joyState);
            if (FAILED(hr))
            {
                std::cout << "ProcessDInputController failed" << std::endl;
                return;
            }
        }

        for (int idx = 0; idx < sizeof(joyState.rgbButtons) / sizeof(joyState.rgbButtons[0]); ++idx)
        {
            if (joyState.rgbButtons[idx] != joyState_.rgbButtons[idx])
            {
                std::cout << "DInput btn changed : " << idx << std::endl;
            }
        }

        memcpy(&joyState_, &joyState, sizeof(joyState_));
    }
}

void ControllerApp::TestCOMObject(void)
{
    /*test COM Object*/
    IActiveDesktop* pIAD = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER, IID_IActiveDesktop, (void**) &pIAD);
    if (FAILED(hr))
    {
        return;
    }

    WCHAR wallpaperFile[MAX_PATH];
    hr = pIAD->GetWallpaper(wallpaperFile, MAX_PATH, 0);
    std::wcout << L"cout" << wallpaperFile << std::endl;
}

#include "console_controller.h"

#include <dinput.h>
#include <iostream>
#include <joystickapi.h>
#include <Xinput.h>

/*order dependant*/
#include <WinInet.h>
#include <ShlObj.h>

Controller::ProcessType s_processType = Controller::Xbox;

JOYINFOEX s_joyInfoEx;
JOYINFOEX s_oldState;

IDirectInput8* s_pDirectInput8;
IDirectInputDevice8* s_pDirectInputDevice8;

DIJOYSTATE2 s_joyState;

void Controller::SetProcessType(int type)
{
    s_processType = static_cast<Controller::ProcessType> (type);
}

void Controller::IninitializeComLibrary(void)
{
    HRESULT hr = CoInitialize(NULL);

    if (SUCCEEDED(hr))
    {
        /*test COM Object*/
        IActiveDesktop* pIAD = nullptr;
        CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER, IID_IActiveDesktop, (void**) &pIAD);
        if (SUCCEEDED(hr))
        {
            WCHAR wallpaperFile[MAX_PATH];
            hr = pIAD->GetWallpaper(wallpaperFile, MAX_PATH, 0);
            std::wcout << L"cout" << wallpaperFile << std::endl;
        }
    }
}

void Controller::UninitializeComLibrary(void)
{
    CoUninitialize();
}

struct JoyStickCallbackData
{
    IDirectInputDevice8W*   device;
    int                     objectCount;
    int                     axisCount;
    int                     sliderCount;
    int                     buttonCount;
    int                     povCount;
};

BOOL CALLBACK JoyObjectCallback(const DIDEVICEOBJECTINSTANCE* pInst, void* pUserData)
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

BOOL CALLBACK joypadDeviceCallback(const DIDEVICEINSTANCE* di, void* user)
{
    int joy = 0;
    DIDEVCAPS deviceCap;
    DIPROPDWORD dipd;

    HRESULT hr = s_pDirectInput8->CreateDevice(di->guidInstance, &s_pDirectInputDevice8, nullptr);
    if (!SUCCEEDED(hr))
    {
        std::cout << "s_pDirectInput8->CreateDevice fail" << std::endl;
    }

    hr = s_pDirectInputDevice8->SetDataFormat(&c_dfDIJoystick2);
    if (!SUCCEEDED(hr))
    {
        std::cout << "SetDataFormat fail" << std::endl;
    }

    memset(&deviceCap, 0, sizeof(deviceCap));
    deviceCap.dwSize = sizeof(deviceCap);
    
    hr = s_pDirectInputDevice8->GetCapabilities(&deviceCap);
    if (!SUCCEEDED(hr))
    {
        std::cout << "GetCapabilities fail" << std::endl;
    }

    memset(&dipd, 0, sizeof(dipd));
    dipd.diph.dwSize = sizeof(dipd);
    dipd.diph.dwHeaderSize = sizeof(dipd.diph);
    dipd.diph.dwHow = DIPH_DEVICE;
    dipd.dwData = DIPROPAXISMODE_ABS;

    hr = s_pDirectInputDevice8->SetProperty(DIPROP_AXISMODE, &dipd.diph);
    if (!SUCCEEDED(hr))
    {
        std::cout << "DIPROP_AXISMODE fail" << std::endl;
    }

    JoyStickCallbackData data;
    memset(&data, 0, sizeof(data));

    //hr = s_pDirectInputDevice8->EnumObjects(JoyObjectCallback, &data, DIDFT_AXIS | DIDFT_BUTTON | DIDFT_POV);
    hr = s_pDirectInputDevice8->EnumObjects(JoyObjectCallback, &data, DIDFT_ALIAS);
    if (!SUCCEEDED(hr))
    {
        std::cout << "JoyObjectCallback fail" << std::endl;
    }

    std::cout << "udata obj :" << data.objectCount << std::endl;
    std::cout << "udata axis :" << data.axisCount << std::endl;
    std::cout << "udata button :" << data.buttonCount << std::endl;
    std::cout << "udata pov :" << data.povCount << std::endl;

    return DIENUM_CONTINUE;
}

void Controller::InitInstanceHook(HINSTANCE hInstance, HWND hWnd)
{
    int numJoysticks = joyGetNumDevs();
    MMRESULT mmr;

    for (int idx = 0; idx < numJoysticks; ++idx)
    {
        memset(&s_joyInfoEx, 0, sizeof(s_joyInfoEx));
        s_joyInfoEx.dwSize = sizeof(s_joyInfoEx);
        s_joyInfoEx.dwFlags = JOY_RETURNALL;

        mmr = joyGetPosEx(idx, &s_joyInfoEx);
        if (mmr == JOYERR_NOERROR)
        {
            std::cout << "idx " << idx << "is inited" << std::endl;
        }
    }
   
    memset(&s_oldState, 0, sizeof(s_oldState));
    if (joySetCapture(hWnd, 0, NULL, false))
    {
        std::cout << "joy captture success" << std::endl;
    }

    HRESULT hr = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**) &s_pDirectInput8, nullptr);
    if (!SUCCEEDED(hr))
    {
        std::cout << "dx joystick fail" << std::endl;
    }

    hr = s_pDirectInput8->EnumDevices(DI8DEVCLASS_GAMECTRL, joypadDeviceCallback, nullptr, DIEDFL_ALLDEVICES);
    if (!SUCCEEDED(hr))
    {   
        std::cout << "dx joystick enum fail" << std::endl;
    }
}


void ProcessXBoxController(void)
{   
    static XINPUT_STATE prevState;
    DWORD dwResult;
    for (DWORD idx = 0; idx < XUSER_MAX_COUNT; ++idx)
    {
        XINPUT_STATE state;
        ZeroMemory(&state, sizeof(state));
        
        dwResult = XInputGetState(idx, &state);
        if (dwResult == ERROR_SUCCESS)
        {
            if (state.Gamepad.wButtons ^ prevState.Gamepad.wButtons)
            {
                /*
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
                prevState.Gamepad.wButtons = state.Gamepad.wButtons;
            }

            if (state.Gamepad.bLeftTrigger ^ prevState.Gamepad.bLeftTrigger)
            {
                std::cout << "bLeftTrigger state has chagned." << (int) state.Gamepad.bLeftTrigger << std::endl;
                prevState.Gamepad.bLeftTrigger = state.Gamepad.bLeftTrigger;
            }

            if (state.Gamepad.bRightTrigger ^ prevState.Gamepad.bRightTrigger)
            {
                std::cout << "bRightTrigger state has chagned." << (int) state.Gamepad.bRightTrigger << std::endl;
                prevState.Gamepad.bRightTrigger = state.Gamepad.bRightTrigger;
            }
        }
    }
}

void ProcessJoySticCotroller(void)
{
    //JoystickInfo_t &info = m_pJoystickInfo[nJoystick];
    //JOYINFOEX& ji = info.m_JoyInfoEx;   

    MMRESULT mmr = joyGetPosEx(0, &s_joyInfoEx);

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

    if (mmr == JOYERR_NOERROR)
    {
        if (s_joyInfoEx.dwButtons ^ s_oldState.dwButtons)
        {
            std::cout << "button state has chagned." << (int) s_joyInfoEx.dwButtons << std::endl;
            s_oldState.dwButtons = s_joyInfoEx.dwButtons;
        }

        if (abs(static_cast<long long> (s_joyInfoEx.dwXpos - s_oldState.dwXpos)) > 10000)
        {
            std::cout << "button dwXpos chagned." << s_joyInfoEx.dwXpos<< (int) s_joyInfoEx.dwButtons << std::endl;
            s_oldState.dwXpos = s_joyInfoEx.dwXpos;
        }

        if (abs(static_cast<long long> (s_joyInfoEx.dwYpos - s_oldState.dwYpos)) > 10000)
        {
            std::cout << "button dwYpos chagned." << s_joyInfoEx.dwYpos<< (int) s_joyInfoEx.dwButtons << std::endl;
            s_oldState.dwYpos = s_joyInfoEx.dwYpos;
        }
        if (abs(static_cast<long long> (s_joyInfoEx.dwZpos - s_oldState.dwZpos)) > 10000)
        {
            std::cout << "button dwZpos chagned." << s_joyInfoEx.dwZpos<< (int) s_joyInfoEx.dwButtons << std::endl;
            s_oldState.dwZpos = s_joyInfoEx.dwZpos;
        }
        if (abs(static_cast<long long> (s_joyInfoEx.dwRpos - s_oldState.dwRpos)) > 10000)
        {
            std::cout << "button dwRpos chagned." << s_joyInfoEx.dwRpos<< (int) s_joyInfoEx.dwButtons << std::endl;
            s_oldState.dwRpos = s_joyInfoEx.dwRpos;
        }
        if (abs(static_cast<long long> (s_joyInfoEx.dwUpos - s_oldState.dwUpos)) > 10000)
        {
            std::cout << "button dwUpos chagned." << s_joyInfoEx.dwUpos<< (int) s_joyInfoEx.dwButtons << std::endl;
            s_oldState.dwUpos = s_joyInfoEx.dwUpos;
        }
        if (abs(static_cast<long long> (s_joyInfoEx.dwVpos - s_oldState.dwVpos)) > 10000)
        {
            std::cout << "button dwVpos chagned." << s_joyInfoEx.dwVpos<< (int) s_joyInfoEx.dwButtons << std::endl;
            s_oldState.dwVpos = s_joyInfoEx.dwVpos;
        }
        if (abs(static_cast<long long> (s_joyInfoEx.dwPOV - s_oldState.dwPOV)) > 10000)
        {
            std::cout << "button dwPOV chagned." << s_joyInfoEx.dwPOV<< (int) s_joyInfoEx.dwButtons << std::endl;
            s_oldState.dwPOV = s_joyInfoEx.dwPOV;
        }
    }
}

void ProcessDInputController(void)
{
    if(s_pDirectInputDevice8 != nullptr)
    {
        s_pDirectInputDevice8->Poll();

        DIJOYSTATE2 joyState;
        HRESULT hr = s_pDirectInputDevice8->GetDeviceState(sizeof(joyState), &joyState);
        if (FAILED(hr))
        {
            s_pDirectInputDevice8->Acquire();
            s_pDirectInputDevice8->Poll();
            hr = s_pDirectInputDevice8->GetDeviceState(sizeof(joyState), &joyState);
            if (FAILED(hr))
            {
                std::cout << "ProcessDInputController failed" << std::endl;
                return;
            }
        }

        for (int idx = 0; idx < sizeof(joyState.rgbButtons) / sizeof(joyState.rgbButtons[0]); ++idx)
        {
            if (joyState.rgbButtons[idx] != s_joyState.rgbButtons[idx])
            {
                std::cout << "DInput btn changed : " << idx << std::endl;
            }
        }

        memcpy(&s_joyState, &joyState, sizeof(s_joyState));
    }
} 

void Controller::ProcessController(void)
{
    switch(s_processType)
    {
        case Controller::Xbox:
        {
            ProcessXBoxController();
        }
        break;
        case Controller::Winmm:
        {
            ProcessJoySticCotroller();
        }
        break;
        case Controller::DInput:
        {
            ProcessDInputController();
        }
        break;
    }
}

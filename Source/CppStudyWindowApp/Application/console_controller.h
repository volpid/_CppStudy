
#ifndef __CONSOLE_CONTROLLER__H__
#define __CONSOLE_CONTROLLER__H__

#include "../Common/core_window.h"

#include <dinput.h>
#include <vector>
#include <Xinput.h>

//-----------------------------------------------------------------------------------------
// ControllerApp
//-----------------------------------------------------------------------------------------
class ControllerApp : public CoreWindowApp
{
public:
    enum ControllerType
    {
        Xbox,
        Winmm,
        DInput,
    };

public:
    ControllerApp(void) = default;
    virtual ~ControllerApp(void) = default;

    virtual LRESULT MsgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual bool InitializeApp(void);
    virtual void Update(void);
    virtual void CleanUp(void);
    
    BOOL JoypadDeviceCallback(const DIDEVICEINSTANCE* di, void* user);
    BOOL JoyObjectCallback(const DIDEVICEOBJECTINSTANCE* pInst, void* pUserData);

private:
    void InitializeController(void);

    void ProcessXBoxController(void);
    void ProcessJoyStickController(void);
    void ProcessDInputController(void);

    void TestCOMObject(void);

    inline void SetControllerType(ControllerType);

private:
    ControllerType controllerType_ = ControllerType::Xbox;

    XINPUT_STATE xboxStatePrev_[XUSER_MAX_COUNT];
    
    std::vector<JOYINFOEX> joyInfoExPrev_;
    
    DIJOYSTATE2 joyState_;    
    IDirectInput8* pDirectInput8_ = nullptr;
    IDirectInputDevice8* pDirectInputDevice8_ = nullptr;    
};

//-----------------------------------------------------------------------------------------
// ControllerApp
//-----------------------------------------------------------------------------------------
inline void ControllerApp::SetControllerType(ControllerType type)
{
    controllerType_ = type;
}

#endif /*__CONSOLE_CONTROLLER__H__*/
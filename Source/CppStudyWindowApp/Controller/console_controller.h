
#ifndef __CONSOLE_CONTROLLER__H__
#define __CONSOLE_CONTROLLER__H__

#include <Windows.h>

namespace Controller
{
    enum ProcessType
    {
        Xbox,
        Winmm,
        DInput,
    };

    void SetProcessType(int type);

    void IninitializeComLibrary(void);
    void UninitializeComLibrary(void);

    void InitInstanceHook(HINSTANCE hInstance, HWND hWnd);
    void ProcessController(void);
}

#endif /*__CONSOLE_CONTROLLER__H__*/
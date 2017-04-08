
#include <Windows.h>

#include "Frame\dll_main_proxy.h"

#include <iostream>

BOOL WINAPI DllMain(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved)
{
    UNREFERENCED_PARAMETER(hInstDll);
    UNREFERENCED_PARAMETER(lpvReserved);

    switch (fwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            DllMainProxy::DllProcessAttach(hInstDll, fwReason, lpvReserved);
        }
        break;
        case DLL_THREAD_ATTACH: 
        {
            DllMainProxy::DllThreadAttach(hInstDll, fwReason, lpvReserved);
        }
        break;
        case DLL_THREAD_DETACH: 
        {
            DllMainProxy::DllThreadDetach(hInstDll, fwReason, lpvReserved);
        }
        break;
        case DLL_PROCESS_DETACH: 
        {
            DllMainProxy::DllProcessDetach(hInstDll, fwReason, lpvReserved);
        }
        break;
        default:
        {}
        break;
    }

    return TRUE;
}
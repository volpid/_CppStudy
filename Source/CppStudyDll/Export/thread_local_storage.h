
#ifndef __THREAD_LOCAL_STORAGE__H__
#define __THREAD_LOCAL_STORAGE__H__

#include <Windows.h>

#if defined(EXPORT_DLL)
    #define DllDeclspec __declspec(dllexport)
#else
    #define DllDeclspec __declspec(dllimport)
#endif /**/

#if defined(__cplusplus)
extern "C"
{
#endif /**/

    DllDeclspec BOOL WINAPI WINAPI_StoreStaticTSL(DWORD dw);
    DllDeclspec BOOL GetStaticTSL(DWORD* dw);
    DllDeclspec BOOL WINAPI WINAPI_StoreDynamicTSL(DWORD dw);
    DllDeclspec BOOL GetDynamicTSL(DWORD* dw);

#if defined(__cplusplus)
}
#endif /**/



#endif /*__THREAD_LOCAL_STORAGE__H__*/
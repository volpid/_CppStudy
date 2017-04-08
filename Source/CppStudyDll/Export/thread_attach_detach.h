
#ifndef __THREAD_ATTACH_DETACH__H__
#define __THREAD_ATTACH_DETACH__H__

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

    DllDeclspec void StopThreadLogging(void);

#if defined(__cplusplus)
}
#endif /**/

#endif /*__THREAD_ATTACH_DETACH__H__*/
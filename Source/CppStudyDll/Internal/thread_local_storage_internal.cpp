
#define EXPORT_DLL
#include <dynamiclib/thread_local_storage.h>

#include <iostream>>

DWORD dwTlsIndex = -1;

__declspec(thread) DWORD g_staticTls;

//-----------------------------------------------------------------------------------
/*export*/

BOOL WINAPI WINAPI_StoreStaticTSL(DWORD dw)
{
    g_staticTls = dw;
    return TRUE;
}

BOOL GetStaticTSL(DWORD* dw)
{
    (*dw) = g_staticTls;
    return TRUE;
}

BOOL WINAPI WINAPI_StoreDynamicTSL(DWORD dw)
{
    LPVOID lpvData;
    DWORD* pData;

    lpvData = TlsGetValue(dwTlsIndex);
    if(lpvData == NULL)
    {
        lpvData = (LPVOID) LocalAlloc(LPTR, 256);
        if(lpvData == NULL)
        {
            return FALSE;
        }
        if(TlsSetValue(dwTlsIndex, lpvData))
        {
            return FALSE;
        }
    }

    std::cout << std::hex;
    std::cout << "set dynamic data :" << lpvData << std::endl;
    std::cout << std::dec;

    pData = (DWORD*) lpvData;
    *(pData) = dw;

    return TRUE;
}

BOOL GetDynamicTSL(DWORD* dw)
{
    LPVOID lpvData;
    DWORD* pData;

    lpvData = TlsGetValue(dwTlsIndex);
    if(lpvData == NULL)
    {
        return FALSE;
    }

    std::cout << std::hex;
    std::cout << "get dynamic data :" << lpvData << std::endl;
    std::cout << std::dec;

    pData = (DWORD*) lpvData;
    (*dw) = (*pData);

    return TRUE;
}
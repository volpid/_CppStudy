
#include "dll_main_proxy.h"

#include <Windows.h>

extern DWORD dwTlsIndex;

//-----------------------------------------------------------------------------------
// DllMainThreadLocalStorage
//-----------------------------------------------------------------------------------
class DllMainThreadLocalStorage : public DllMainProxy
{
public:
    DllMainThreadLocalStorage(void) = default;
    virtual ~DllMainThreadLocalStorage(void) = default;

    virtual void DllProcessAttachInternal(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved);
    virtual void DllThreadAttachInternal(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved);
    virtual void DllThreadDetachInternal(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved);
    virtual void DllProcessDetachInternal(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved);
    
private:
    DllMainThreadLocalStorage(const DllMainThreadLocalStorage&) = delete;
    DllMainThreadLocalStorage& operator=(const DllMainThreadLocalStorage&) = delete;
    DllMainThreadLocalStorage(DllMainThreadLocalStorage&&) = delete;
    DllMainThreadLocalStorage& operator=(DllMainThreadLocalStorage&&) = delete;
};

RegisterDllMainProxy(DllMainThreadLocalStorage);

//-----------------------------------------------------------------------------------

void DllMainThreadLocalStorage::DllProcessAttachInternal(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved)
{
    dwTlsIndex = TlsAlloc();
    if(dwTlsIndex == TLS_OUT_OF_INDEXES)
    {   
        exit(-1);
        return;
    }
}

void DllMainThreadLocalStorage::DllThreadAttachInternal(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved)
{
    LPVOID lpvData;
    BOOL fIgnore;
    lpvData = (LPVOID) LocalAlloc(LPTR, 256);
    if(lpvData != NULL)
    {
        fIgnore = TlsSetValue(dwTlsIndex, lpvData);
    }
}

void DllMainThreadLocalStorage::DllThreadDetachInternal(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved)
{
    LPVOID lpvData;
    BOOL fIgnore;
    lpvData = TlsGetValue(dwTlsIndex);
    if(lpvData != NULL)
    {
        LocalFree(lpvData);
    }
}

void DllMainThreadLocalStorage::DllProcessDetachInternal(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved)
{
    LPVOID lpvData;
    lpvData = TlsGetValue(dwTlsIndex);
    if(lpvData != NULL)
    {
        LocalFree(lpvData);
    }

    TlsFree(dwTlsIndex);
}

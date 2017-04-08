
#ifndef __DLL_MAIN_PROXY__H__
#define __DLL_MAIN_PROXY__H__

#include <vector>
#include <Windows.h>

//-----------------------------------------------------------------------------------
// DllMainProxy
//-----------------------------------------------------------------------------------
class DllMainProxy
{
public:
    static std::vector<DllMainProxy*>& MainProxyLinkIntacne(void);
    static void DllProcessAttach(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved);
    static void DllThreadAttach(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved);
    static void DllThreadDetach(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved);
    static void DllProcessDetach(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved);

public:
    DllMainProxy(void);
    virtual ~DllMainProxy(void);

    virtual void DllProcessAttachInternal(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved) = 0;
    virtual void DllThreadAttachInternal(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved) = 0;
    virtual void DllThreadDetachInternal(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved) = 0;
    virtual void DllProcessDetachInternal(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved) = 0;

private:
    DllMainProxy(const DllMainProxy&) = delete;
    DllMainProxy& operator=(const DllMainProxy&) = delete;
    DllMainProxy(DllMainProxy&&) = delete;
    DllMainProxy& operator=(DllMainProxy&&) = delete;
};

#define RegisterDllMainProxy(proxyClassName) \
    static proxyClassName s_proxy##proxyClassName

#endif /*__DLL_MAIN_PROXY__H__*/
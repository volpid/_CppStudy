
#include "dll_main_proxy.h"

#include <algorithm>

//-----------------------------------------------------------------------------------
// DllMainProxy 
//-----------------------------------------------------------------------------------
std::vector<DllMainProxy*>& DllMainProxy::MainProxyLinkIntacne(void)
{
    static std::vector<DllMainProxy*> s_dllMainProxies;
    return s_dllMainProxies;
}

void DllMainProxy::DllProcessAttach(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved)
{
    for (DllMainProxy* pMain : MainProxyLinkIntacne())
    {
        pMain->DllProcessAttachInternal(hInstDll, fwReason, lpvReserved);
    }
}

void DllMainProxy::DllThreadAttach(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved)
{
    for (DllMainProxy* pMain : MainProxyLinkIntacne())
    {
        pMain->DllThreadAttachInternal(hInstDll, fwReason, lpvReserved);
    }
}

void DllMainProxy::DllThreadDetach(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved)
{
    for (DllMainProxy* pMain : MainProxyLinkIntacne())
    {
        pMain->DllThreadDetachInternal(hInstDll, fwReason, lpvReserved);
    }
}

void DllMainProxy::DllProcessDetach(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved)
{
    for (DllMainProxy* pMain : MainProxyLinkIntacne())
    {
        pMain->DllProcessDetachInternal(hInstDll, fwReason, lpvReserved);
    }
}

//-----------------------------------------------------------------------------------
DllMainProxy::DllMainProxy(void)
{
    MainProxyLinkIntacne().push_back(this);
}

DllMainProxy::~DllMainProxy(void)
{
    MainProxyLinkIntacne().erase(
        std::remove(MainProxyLinkIntacne().begin(), MainProxyLinkIntacne().end(), this),
        MainProxyLinkIntacne().end()
    );
}

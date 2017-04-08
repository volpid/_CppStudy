
#include "dll_main_proxy.h"

#include <Windows.h>

//-----------------------------------------------------------------------------------
// DllMainProxyTemplate
//-----------------------------------------------------------------------------------
class DllMainTemplate : public DllMainProxy
{
public:
    DllMainTemplate(void) = default;
    virtual ~DllMainTemplate(void) = default;

    virtual void DllProcessAttachInternal(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved);
    virtual void DllThreadAttachInternal(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved);
    virtual void DllThreadDetachInternal(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved);
    virtual void DllProcessDetachInternal(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved);

private:
    DllMainTemplate(const DllMainTemplate&) = delete;
    DllMainTemplate& operator=(const DllMainTemplate&) = delete;
    DllMainTemplate(DllMainTemplate&&) = delete;
    DllMainTemplate& operator=(DllMainTemplate&&) = delete;
};

RegisterDllMainProxy(DllMainTemplate);

//-----------------------------------------------------------------------------------

void DllMainTemplate::DllProcessAttachInternal(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved)
{
}

void DllMainTemplate::DllThreadAttachInternal(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved)
{
}

void DllMainTemplate::DllThreadDetachInternal(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved)
{
}

void DllMainTemplate::DllProcessDetachInternal(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved)
{
}

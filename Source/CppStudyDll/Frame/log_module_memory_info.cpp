
#include "dll_main_proxy.h"

#include "../Internal/module_memory_info_internal.h"

#include <Windows.h>

//-----------------------------------------------------------------------------------
// DllMainProxyTemplate
//-----------------------------------------------------------------------------------
class DllMainLogMemoryInfo : public DllMainProxy
{
public:
    DllMainLogMemoryInfo(void) = default;
    virtual ~DllMainLogMemoryInfo(void) = default;

    virtual void DllProcessAttachInternal(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved);
    virtual void DllThreadAttachInternal(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved);
    virtual void DllThreadDetachInternal(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved);
    virtual void DllProcessDetachInternal(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved);

private:
    DllMainLogMemoryInfo(const DllMainLogMemoryInfo&) = delete;
    DllMainLogMemoryInfo& operator=(const DllMainLogMemoryInfo&) = delete;
    DllMainLogMemoryInfo(DllMainLogMemoryInfo&&) = delete;
    DllMainLogMemoryInfo& operator=(DllMainLogMemoryInfo&&) = delete;
};

RegisterDllMainProxy(DllMainLogMemoryInfo);

//-----------------------------------------------------------------------------------

void DllMainLogMemoryInfo::DllProcessAttachInternal(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved)
{
    char szBuff[MAX_PATH] = {0,};
    PBYTE pb = NULL;
    MEMORY_BASIC_INFORMATION mbi;
    while(VirtualQuery(pb, &mbi, sizeof(mbi)) == sizeof(mbi))
    {
        int nLen;
        char szModuleName[MAX_PATH] = {0,};
        if(mbi.State == MEM_FREE)
        {
            mbi.AllocationBase = mbi.BaseAddress;
        }

        if(mbi.AllocationBase == hInstDll
            || mbi.AllocationBase != mbi.BaseAddress
            || mbi.AllocationBase == NULL)
        {
            nLen = 0;
        }
        else
        {
            nLen = GetModuleFileNameA((HINSTANCE) mbi.AllocationBase,
                szModuleName,
                _countof(szModuleName));
        }

        if(nLen > 0)
        {
            wsprintf(szBuff, "%p-%s", mbi.AllocationBase, szModuleName);
            LogModuleMemoryInfo(szBuff);
        }

        pb += mbi.RegionSize;
    }
}

void DllMainLogMemoryInfo::DllThreadAttachInternal(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved)
{
}

void DllMainLogMemoryInfo::DllThreadDetachInternal(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved)
{
}

void DllMainLogMemoryInfo::DllProcessDetachInternal(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved)
{
}

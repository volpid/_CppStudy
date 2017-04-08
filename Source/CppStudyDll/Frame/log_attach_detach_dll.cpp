
#include "dll_main_proxy.h"

#include "../Internal/thread_attach_detach_internal.h"

#include <chrono>
#include <ctime>
#include <iostream>
#include <mutex>
#include <Windows.h>
#include <thread>

namespace 
{   
    std::mutex s_lock;

    void PrintWithLock(HINSTANCE hInstDll, const char* msg)
    {
        if (g_loggingAttachDetach == true)
        {
            time_t now_time;
            char buf[256] = {0,};
            time(&now_time); 
            ctime_s(buf,sizeof(buf),&now_time);

            std::lock_guard<std::mutex> guard(s_lock);            
            std::cout << buf << std::hex;
            std::cout << hInstDll << " " << msg << std::dec << std::endl;
        }
    }
}


//-----------------------------------------------------------------------------------
// DllMainLogAttachDetach
//-----------------------------------------------------------------------------------
class DllMainLogAttachDetach : public DllMainProxy
{
public:
    DllMainLogAttachDetach(void) = default;
    virtual ~DllMainLogAttachDetach(void) = default;

    virtual void DllProcessAttachInternal(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved);
    virtual void DllThreadAttachInternal(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved);
    virtual void DllThreadDetachInternal(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved);
    virtual void DllProcessDetachInternal(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved);

private:
    DllMainLogAttachDetach(const DllMainLogAttachDetach&) = delete;
    DllMainLogAttachDetach& operator=(const DllMainLogAttachDetach&) = delete;
    DllMainLogAttachDetach(DllMainLogAttachDetach&&) = delete;
    DllMainLogAttachDetach& operator=(DllMainLogAttachDetach&&) = delete;
};

RegisterDllMainProxy(DllMainLogAttachDetach);

//-----------------------------------------------------------------------------------
void DllMainLogAttachDetach::DllProcessAttachInternal(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved)
{
    PrintWithLock(hInstDll, "DllMain DLL_PROCESS_ATTACH");
}

void DllMainLogAttachDetach::DllThreadAttachInternal(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved)
{
    PrintWithLock(hInstDll, "DllMain DLL_THREAD_ATTACH");
}

void DllMainLogAttachDetach::DllThreadDetachInternal(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved)
{
    PrintWithLock(hInstDll, "DllMain DLL_THREAD_DETACH");
}

void DllMainLogAttachDetach::DllProcessDetachInternal(HINSTANCE hInstDll, DWORD fwReason, LPVOID lpvReserved)
{
    PrintWithLock(hInstDll, "DllMain DLL_PROCESS_DETACH");
}

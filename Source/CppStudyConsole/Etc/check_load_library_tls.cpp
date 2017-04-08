
#include <iostream>
#include <functional>
#include <mutex>
#include <random>
#include <thread>

#include <Windows.h>

std::default_random_engine generator;
std::uniform_int_distribution<int> distribution(1, 100);
auto rng = bind(distribution, generator);

using SetDataTls = BOOL (WINAPI *)(DWORD);
using GetDataTls = BOOL (*)(DWORD*);
using StopLogging = void (*)(void);

SetDataTls StoreStaticTSL = nullptr;
GetDataTls GetStaticTSL = nullptr;
SetDataTls StoreDynamicTSL = nullptr;
GetDataTls GetDynamicTSL = nullptr;

static std::mutex s_lock;
void PrintOut(int id, const char* msg, DWORD number)
{
    
    std::cout << id << " " << msg << " " << number << std::endl;
}

void ReadWriteStaticTls(int count, int id)
{
    for (int n = 0; n < count; ++n)
    {   
        std::lock_guard<std::mutex> guard(s_lock);
        DWORD random = rng();        
        StoreStaticTSL(random);
        PrintOut(id, "write static", random);
        
        DWORD read;
        GetStaticTSL(&read);
        PrintOut(id, "read static", random);
    }
}

void ReadWriteDynamicTls(int count, int id)
{
    for (int n = 0; n < count; ++n)
    {
        std::lock_guard<std::mutex> guard(s_lock);
        DWORD random = rng();        
        StoreDynamicTSL(random);
        PrintOut(id, "write dynamic", random);
        
        DWORD read;
        GetDynamicTSL(&read);
        PrintOut(id, "read dynamic", random);
    }
}

static int _main(int argc, char** argv)
{
    HMODULE hModule = LoadLibrary("./dynamic_library.dll");

    StoreStaticTSL = reinterpret_cast<SetDataTls> (GetProcAddress(hModule, "_WINAPI_StoreStaticTSL@4"));
    GetStaticTSL = reinterpret_cast<GetDataTls> (GetProcAddress(hModule, "GetStaticTSL"));
    StoreDynamicTSL = reinterpret_cast<SetDataTls> (GetProcAddress(hModule, "_WINAPI_StoreDynamicTSL@4"));
    GetDynamicTSL = reinterpret_cast<GetDataTls> (GetProcAddress(hModule, "GetDynamicTSL"));
    StopLogging stopLoggingFunc = reinterpret_cast<StopLogging> (GetProcAddress(hModule, "StopThreadLogging"));

    if ((StoreStaticTSL == nullptr)
        || (GetStaticTSL == nullptr)
        || (StoreDynamicTSL == nullptr)
        || (GetDynamicTSL == nullptr))
    {
        std::cout << "dll load faild" << std::endl;
        exit(1);
    }

    stopLoggingFunc();

    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++ i)
    {
        const int dynamicIndexOffset = 100;
        threads.push_back(std::thread(ReadWriteStaticTls, 2, i));
        threads.push_back(std::thread(ReadWriteDynamicTls, 2, i +  dynamicIndexOffset));
    }

    for (auto& t : threads)
    {
        t.join();
    }

    FreeLibrary(hModule);    
    return 0;
}
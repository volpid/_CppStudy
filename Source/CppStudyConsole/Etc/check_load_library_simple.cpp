
#include <iostream>
#include <thread>
#include <vector>

#include <Windows.h>

static int _main(int argc, char** argv)
{
    HMODULE hModule = LoadLibrary("./dynamic_library.dll");

    auto lambdaEmptyFunc = [](int id) 
    {};

    std::vector<std::thread> threads;

    for (int i = 0; i < 5; ++ i)
    {
        threads.push_back(std::thread(lambdaEmptyFunc, i));
    }

    for (auto& t : threads)
    {
        t.join();
    }

    using PrintModuleMemInfo = void (*)(void);
    PrintModuleMemInfo printModuleMemInfo = reinterpret_cast<PrintModuleMemInfo> (GetProcAddress(hModule, "PrintModuleMemoryInfo"));

    if (printModuleMemInfo != nullptr)
    {
        printModuleMemInfo();
    }

    FreeLibrary(hModule);    
    return 0;
}
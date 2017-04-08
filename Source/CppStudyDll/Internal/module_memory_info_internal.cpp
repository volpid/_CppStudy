
#define EXPORT_DLL
#include <dynamiclib/module_memory_info.h>
#include "module_memory_info_internal.h"

#include <iostream>
#include <string>
#include <vector>

namespace
{
    std::vector<std::string> s_moduleLogs;
}

void LogModuleMemoryInfo(const char* log)
{
    s_moduleLogs.emplace_back(log);
}

//-----------------------------------------------------------------------------------
/*export*/
void PrintModuleMemoryInfo(void)
{
    std::cout << std::endl;
    std::cout << "PrintModuleMemoryInfo" << std::endl;
    for (std::string& log : s_moduleLogs)
    {
        std::cout << log.c_str() << std::endl;
    }
    std::cout << std::endl;
}


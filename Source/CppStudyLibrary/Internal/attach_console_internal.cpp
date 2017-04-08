
#define _CRT_SECURE_NO_WARNINGS
#include <io.h>
#include <stdio.h>
#include <Windows.h>

bool CreateConsole(void)
{
    try
    {
        AllocConsole();        
        freopen("CONOUT$", "w", stdout);
        return true;
    }
    catch(...)
    {
    }

    return false;
}

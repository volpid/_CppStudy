
#include "Application/console_controller.h" 
#include "Common/core_window.h"
#include "Resource/resource.h"

#include <iostream>

#include <staticlib/attach_console.h>

/*import library*/
#pragma comment(lib, "static_library.lib")

#define MAX_LOADSTRING 100

INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY WinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow)
{
    CreateConsole();

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

    char szTitle[MAX_LOADSTRING];
    char szWindowClass[MAX_LOADSTRING];
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_WINDOWSAPPLICATION, szWindowClass, MAX_LOADSTRING);

    CoreWindowApp* pApp = CoreWindowApp::GetInstance();
    if (pApp == nullptr || pApp->Initialize(hInstance, szWindowClass, szTitle) == false)
    {
        return -1;
    }

    return pApp->Run();
}


#include "Controller/console_controller.h" 
#include "Resource/resource.h"

#include <iostream>
#include <staticlib/attach_console.h>
#include <Windows.h>

/*import library*/
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "static_library.lib")
#pragma comment(lib, "Xinput.lib")
#pragma comment(lib, "winmm.lib")

#define MAX_LOADSTRING 100

HINSTANCE hInst;
char szTitle[MAX_LOADSTRING];
char szWindowClass[MAX_LOADSTRING];

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);


int APIENTRY WinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow)
{
    CreateConsole();

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_WINDOWSAPPLICATION, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

    Controller::IninitializeComLibrary();

    MSG msg;
	HACCEL hAccelTable;
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSAPPLICATION));

    bool idDone = false;
    while (idDone == false)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) == true)
        {
            if (msg.message == WM_QUIT)
            {
                idDone = true;
                continue;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            Controller::ProcessController();
        }
    }

    Controller::UninitializeComLibrary();

	return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSAPPLICATION));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_WINDOWSAPPLICATION);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance;

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 300, 300, NULL, NULL, hInstance, NULL);
      
   if (!hWnd)
   {
      return FALSE;
   }

   Controller::InitInstanceHook(hInstance, hWnd);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    const int MS_WM_XBUTTONDOWN	= 0x020B;
    const int MS_WM_XBUTTONUP = 0x020C;

	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
        case WM_CREATE:
        {
            CreateWindowEx(NULL, "BUTTON", "XBox", WS_TABSTOP|WS_VISIBLE| WS_CHILD|BS_DEFPUSHBUTTON, 
                10, 10, 100, 24,
                hWnd, (HMENU)IDC_BUTTON_XBOX, hInst, NULL);

            CreateWindowEx(NULL, "BUTTON", "Winmm", WS_TABSTOP|WS_VISIBLE| WS_CHILD|BS_DEFPUSHBUTTON, 
                10, 40, 100, 24,
                hWnd, (HMENU)IDC_BUTTON_WINMM, hInst, NULL);

            CreateWindowEx(NULL, "BUTTON", "DInput", WS_TABSTOP|WS_VISIBLE| WS_CHILD|BS_DEFPUSHBUTTON, 
                10, 70, 100, 24,
                hWnd, (HMENU)IDC_BUTTON_DINPUT, hInst, NULL);
        }
        break;
        case WM_COMMAND:
        {
            wmId    = LOWORD(wParam);
            wmEvent = HIWORD(wParam);
            switch (wmId)
            {
                case IDM_ABOUT:
                {
                    DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                }
                break;
                case IDM_EXIT:
                {
                    DestroyWindow(hWnd);
                }
                break;

                case IDC_BUTTON_XBOX:
                {
                    Controller::SetProcessType(Controller::Xbox);
                }
                break;

                case IDC_BUTTON_WINMM:
                {
                    Controller::SetProcessType(Controller::Winmm);
                }
                break;

                case IDC_BUTTON_DINPUT:
                {
                    Controller::SetProcessType(Controller::DInput);
                }
                break;

                default:
                {
                    return DefWindowProc(hWnd, message, wParam, lParam);
                }
            }
        }
        break;
        case WM_PAINT:
        {
            hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;
        case WM_DESTROY:
        {   
            PostQuitMessage(0);
        }
        break;
        //@note Device changed
        case WM_DEVICECHANGE:
        {
            std::cout << "devcie changed!" << std::endl;
        }
        break;

        case MS_WM_XBUTTONDOWN:
        case MS_WM_XBUTTONUP:
        //case MS_MK_BUTTON4:
        //case MS_MK_BUTTON5:
        {
            std::cout << "ms_WM changed!" << std::endl;
        }
        break;
        default:
        {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
	}
	return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	    case WM_INITDIALOG:
        {
            return (INT_PTR)TRUE;
        }
	    case WM_COMMAND:
        {
		    if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		    {
			    EndDialog(hDlg, LOWORD(wParam));
			    return (INT_PTR)TRUE;
		    }
        }
		break;
	}
	return (INT_PTR)FALSE;
}


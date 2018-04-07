
#include "core_window.h"

#include "../Resource/resource.h"

//-----------------------------------------------------------------------------------------
// Local
//-----------------------------------------------------------------------------------------
namespace
{
    LRESULT CALLBACK MainMsgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        return CoreWindowApp::GetInstance()->MsgProc(hWnd, message, wParam, lParam);
    }

    INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
    {
	    UNREFERENCED_PARAMETER(lParam);
	    switch (message)
	    {
	        case WM_INITDIALOG:
            {
                return static_cast<INT_PTR> (TRUE);                
            }
	        case WM_COMMAND:
            {
		        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		        {
			        EndDialog(hDlg, LOWORD(wParam));
                    return static_cast<INT_PTR> (TRUE);
		        }
            }
		    break;
	    }

        return static_cast<INT_PTR> (FALSE);
    }
}

//-----------------------------------------------------------------------------------------
// CoreWindowApp
//-----------------------------------------------------------------------------------------
CoreWindowApp* CoreWindowApp::GetInstance(void)
{
    return thisApp;
}

CoreWindowApp* CoreWindowApp::thisApp = nullptr;

//-----------------------------------------------------------------------------------------
CoreWindowApp::CoreWindowApp(void)
{
    thisApp = this;
}

CoreWindowApp::~CoreWindowApp(void)
{
}

LRESULT CoreWindowApp::MsgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            int wmEvent = HIWORD(wParam);
            switch (wmId)
            {
                case IDM_ABOUT:
                {
                    DialogBox(hAppInst_, MAKEINTRESOURCE(IDD_ABOUTBOX), hMainWnd_, About);
                    return 0;
                }
                break;
                case IDM_EXIT:
                {
                    DestroyWindow(hWnd);
                    return 0;
                }
                break;
            }
        }
        break;
        case WM_PAINT:
        {
            HDC hdc;
            PAINTSTRUCT ps;
            hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            return 0;
        }
        break;
        case WM_DESTROY:
        {   
            PostQuitMessage(0);
            return 0;
        }
        break;
	}

    return DefWindowProc(hWnd, message, wParam, lParam);
}

bool CoreWindowApp::InitializeApp(void)
{
    return true;
}

void CoreWindowApp::Update(void)
{
}

void CoreWindowApp::CleanUp(void)
{
}

bool CoreWindowApp::Initialize(HINSTANCE hInstance, const char* wndClass, const char* wndCaption)
{
    hAppInst_ = hInstance;
    mainWndClass_ = wndClass;
    mainWndCaption_ = wndCaption;

    if (InitializeWindow() == false)
    {
        return false;
    }

    return InitializeApp();
}

int CoreWindowApp::Run(void)
{
    MSG msg = {0};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE) == true)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            Update();
        }
    }

    CleanUp();
    return static_cast<int> (msg.wParam);
}

bool CoreWindowApp::InitializeWindow(void)
{
    const char* szWindowClass = "CoreWindowApp";
    WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = MainMsgProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra	= 0;
	wcex.hInstance = hAppInst_;
	wcex.hIcon = LoadIcon(hAppInst_, MAKEINTRESOURCE(IDI_WINDOWSAPPLICATION));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_WINDOWSAPPLICATION);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    if (RegisterClassEx(&wcex) == false)
    {
        MessageBox(0, "Register Class Failed.", nullptr, MB_OK);
        return false;
    }

    RECT rect = {0, 0, clientWidth_, clientHeight_};
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    
    hMainWnd_ = CreateWindow(szWindowClass, 
        mainWndCaption_.c_str(), 
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 
        CW_USEDEFAULT, 
        width, 
        height, 
        nullptr, 
        nullptr, 
        hAppInst_, 
        nullptr);
      
   if (hMainWnd_ == nullptr)
   {
      return false;
   }

   ShowWindow(hMainWnd_, SW_SHOW);
   UpdateWindow(hMainWnd_);
   
   return true;
}
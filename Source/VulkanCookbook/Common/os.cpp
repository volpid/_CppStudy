
#include "os.h"

#include "sample_framework.h"

#if defined(VK_USE_PLATFORM_WIN32_KHR)

//----------------------------------------------------------------
// Local
//----------------------------------------------------------------
namespace
{
    enum UserMessage
    {
        msgResize = WM_USER + 1,
        msgQuit,
        msgMouseClick,
        msgMouseMove,
        msgMouseWheel,
    };

    const char* wndClassName = "vulkanCookbook";

    LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
            case WM_LBUTTONDOWN:
            {
                PostMessage(hWnd, UserMessage::msgMouseClick, 0, 1);
                break;
            }
            break;
            case WM_LBUTTONUP:
            {
                PostMessage(hWnd, UserMessage::msgMouseClick, 0, 0);
                break;
            }
            break;
            case WM_RBUTTONDOWN:
            {
                PostMessage(hWnd, UserMessage::msgMouseClick, 1, 1);
                break;
            }
            break;
            case WM_RBUTTONUP:
            {
                PostMessage(hWnd, UserMessage::msgMouseClick, 1, 0);
                break;
            }
            break;
            case WM_MOUSEMOVE:
            {
                PostMessage(hWnd, UserMessage::msgMouseMove, LOWORD(lParam), HIWORD(lParam));
            }
            break;
            case WM_MOUSEWHEEL:
            {
                PostMessage(hWnd, UserMessage::msgMouseWheel, HIWORD(wParam), 0);
            }
            break;
            case WM_SIZE:
            case WM_EXITSIZEMOVE:
            {
                PostMessage(hWnd, UserMessage::msgResize, wParam, lParam);
            }
            break;
            case WM_KEYDOWN:
            {
                if (wParam == VK_ESCAPE)
                {
                    PostMessage(hWnd, UserMessage::msgQuit, wParam, lParam);
                }
            }
            break;
            case WM_CLOSE:
            {
                PostMessage(hWnd, UserMessage::msgQuit, wParam, lParam);
            }
            break;
            default:
            {
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        }

        return 0;
    }
}

//----------------------------------------------------------------
// WindowFramework
//----------------------------------------------------------------
WindowFramework::WindowFramework(const char* title, int x, int y, int width, int height, VulkanSampleBase& sample)
    : windowParams_(), sample_(sample), created_(false)
{   
    windowParams_.hIntance = GetModuleHandle(nullptr);

    WNDCLASSEX windowClass = 
    {
        sizeof(WNDCLASSEX),
        CS_HREDRAW | CS_VREDRAW,
        WindowProcedure,
        0, 
        0,
        windowParams_.hIntance,
        nullptr,
        LoadCursor(nullptr, IDC_ARROW),
        (HBRUSH) (COLOR_WINDOW + 1),
        nullptr,
        wndClassName,
        nullptr
    };

    if (RegisterClassEx(&windowClass) == false)
    {
        return;
    }

    windowParams_.hWnd = CreateWindow(wndClassName,
        title,
        WS_OVERLAPPEDWINDOW, 
        x, 
        y, 
        width,
        height,
        nullptr,
        nullptr,
        windowParams_.hIntance,
        nullptr);

    created_ = true;
}

WindowFramework::~WindowFramework(void)
{
    if (windowParams_.hWnd != nullptr)
    {
        DestroyWindow(windowParams_.hWnd);
    }

    if (windowParams_.hIntance != nullptr)
    {
        UnregisterClass(wndClassName, windowParams_.hIntance);
    }
}

void WindowFramework::Render(void)
{   
    if (created_ == true && sample_.Initialize(windowParams_) == true)
    {
        ShowWindow(windowParams_.hWnd, SW_SHOWNORMAL);
        UpdateWindow(windowParams_.hWnd);

        MSG message;
        bool loop = true;
        while (loop == true)
        {
            if (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
            {
                switch (message.message)
                {
                    case UserMessage::msgMouseClick:
                    {
                        sample_.MouseClick(static_cast<size_t> (message.wParam), message.lParam > 0);
                    }
                    break;
                    case UserMessage::msgMouseMove:
                    {
                        sample_.MouseMove(static_cast<int> (message.wParam), static_cast<int> (message.lParam));
                    }
                    break;
                    case UserMessage::msgMouseWheel:
                    {
                        sample_.MouseWheel(static_cast<short> (message.wParam) * 0.002f);
                    }
                    break;
                    case UserMessage::msgResize:
                    {
                        if (sample_.Resize() == false)
                        {
                            loop = false;
                        }
                    }
                    break;
                    case UserMessage::msgQuit:
                    {
                        loop = false;
                    }
                    break;
                }

                TranslateMessage(&message);
                DispatchMessage(&message);
            }
            else
            {
                if (sample_.IsReady() == true)
                {
                    sample_.UpdateTime();
                    sample_.Draw();
                    sample_.MouseReset();
                }
            }
        }
    }

    sample_.Deinitialize();
}

#endif /*VK_USE_PLATFORM_WIN32_KHR*/
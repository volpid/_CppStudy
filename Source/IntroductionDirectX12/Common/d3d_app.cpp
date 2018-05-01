
#include "d3d_app.h"

#include <assert.h>
#include <stdlib.h>
#include <vector>

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

//----------------------------------------------------------------
// D3DApp
//----------------------------------------------------------------
D3DApp* D3DApp::GetApp(void)
{
    return d3dApp;
}

D3DApp* D3DApp::d3dApp = nullptr;

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return D3DApp::GetApp()->MsgProc(hwnd, msg, wParam, lParam);
}

//----------------------------------------------------------------
D3DApp::D3DApp(void)
{
    assert(d3dApp == nullptr);
    d3dApp = this;
}

D3DApp::~D3DApp(void)
{
    if (d3dDevice_ != nullptr)
    {
        FlushCommandQueue();
    }
}

HINSTANCE D3DApp::AppInst(void) const
{
    return hAppInst_;
}

HWND D3DApp::MainWnd(void) const
{
    return hMainWnd_;
}

float D3DApp::AspectRatio(void) const
{
    return static_cast<float> (clientWidth_) / clientHeight_;
}

bool D3DApp::Get4xMsaaState(void) const
{
    return msaaState4x_;
}

void D3DApp::Set4xMsaaState(bool value)
{
    if (msaaState4x_ != value)
    {
        msaaState4x_ = value;

        CreateSwapChain();
        OnResize();
    }
}

int D3DApp::Run(void)
{
    MSG msg = {0};

    timer_.Reset();

    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            timer_.Tick();

            if (appPaused_ == true)
            {
                Sleep(100);
            }
            else
            {
                CalculateFrameStats();
                Update(timer_);
                Draw(timer_);
            }
        }
    }

    return static_cast<int> (msg.wParam);
}

bool D3DApp::Initialize(HINSTANCE hInstance)
{
    hAppInst_ = hInstance;

    if (InitMainWindow() == false)
    {
        return false;
    }

    if (InitDirect3D() == false)
    {
        return false;
    }

    OnResize();

    return true;
}

LRESULT D3DApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_ACTIVATE:
        {
            if (LOWORD(wParam) == WA_INACTIVE)
            {
                appPaused_ = true;
                timer_.Stop();
            }
            else
            {
                appPaused_ = false;
                timer_.Start();
            }
        }
        break;
        case WM_SIZE:
        {
            clientWidth_ = LOWORD(lParam);
            clientHeight_ = HIWORD(lParam);
            if (d3dDevice_ != nullptr)
            {
                if (wParam == SIZE_MINIMIZED)
                {
                    appPaused_ = true;
                    mininized_ = true;
                    maximized_ = false;
                }
                else if (wParam == SIZE_MAXIMIZED)
                {
                    appPaused_ = false;
                    mininized_ = false;
                    maximized_ = true;
                    OnResize();
                }
                else if (wParam == SIZE_RESTORED)
                {
                    if (mininized_ == true)
                    {
                        appPaused_ = false;
                        mininized_ = false;
                        OnResize();
                    }
                    else if (maximized_ == true)
                    {
                        appPaused_ = false;
                        maximized_ = false;
                        OnResize();
                    }
                    else if (resizing_ == true)
                    {
                    }
                    else
                    {
                        OnResize();
                    }
                }
            }
            return 0;
        }
        break;
        case WM_ENTERSIZEMOVE:
        {
            appPaused_ = true;
            resizing_ = true;
            timer_.Stop();
            return 0;
        }
        break;
        case WM_EXITSIZEMOVE:
        {
            appPaused_ = false;
            resizing_ = false;
            timer_.Start();
            OnResize();
            return 0;
        }
        break;
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
        break;
        case WM_MENUCHAR:
        {
            return MAKELRESULT(0, MNC_CLOSE);
        }
        break;
        case WM_GETMINMAXINFO:
        {
            ((MINMAXINFO*) lParam)->ptMinTrackSize.x = 200;
            ((MINMAXINFO*) lParam)->ptMinTrackSize.y = 200;
            return 0;
        }
        break;
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        {
            OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        }
        break;
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
        {
            OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        }
        break;
        case WM_MOUSEMOVE:
        {
            OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        }
        break;
        case WM_KEYUP:
        {
            if (wParam == VK_ESCAPE)
            {
                PostQuitMessage(0);
            }
            else if (wParam == VK_F2)
            {
                Set4xMsaaState(!msaaState4x_);
            }
        }
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void D3DApp::CreateRtvAndDsvDescriptorHeaps(void)
{
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
    rtvHeapDesc.NumDescriptors = swapChainbufferCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;

    ThrowIfFailed(d3dDevice_->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(rtvHeap_.GetAddressOf())));

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;
    
    ThrowIfFailed(d3dDevice_->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(dsvHeap_.GetAddressOf())));
}

void D3DApp::OnResize(void)
{
    assert(d3dDevice_ != nullptr);
    assert(swapChain_ != nullptr);
    assert(commandListAlloc_ != nullptr);
    
    FlushCommandQueue();
    
    ThrowIfFailed(commandList_->Reset(commandListAlloc_.Get(), nullptr));

    for (int i = 0; i < swapChainbufferCount; ++i)
    {
        swapChainBuffer_[i].Reset();
    }
    depthStencilBuffer_.Reset();

    ThrowIfFailed(swapChain_->ResizeBuffers(swapChainbufferCount, 
        clientWidth_,
        clientHeight_,
        backBufferFormat_,
        DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

    currBackBuffer_ = 0;

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(rtvHeap_->GetCPUDescriptorHandleForHeapStart());
    for (UINT i = 0; i < swapChainbufferCount; ++i)
    {
        ThrowIfFailed(swapChain_->GetBuffer(i, IID_PPV_ARGS(&swapChainBuffer_[i])));
        d3dDevice_->CreateRenderTargetView(swapChainBuffer_[i].Get(), nullptr, rtvHeapHandle);
        rtvHeapHandle.Offset(1, rtvDescriptorSize_);
    }

    D3D12_RESOURCE_DESC depthStencilDesc;
    depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDesc.Alignment = 0;
    depthStencilDesc.Width = clientWidth_;
    depthStencilDesc.Height = clientHeight_;
    depthStencilDesc.DepthOrArraySize = 1;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    depthStencilDesc.SampleDesc.Count = msaaState4x_ ? 4 : 1;
    depthStencilDesc.SampleDesc.Quality = msaaState4x_ ? (msaaQuality4x_ - 1) : 0;
    depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE optClear;
    optClear.Format = depthStencilFormat_;
    optClear.DepthStencil.Depth = 1.0f;
    optClear.DepthStencil.Stencil = 0;
    ThrowIfFailed(d3dDevice_->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &depthStencilDesc,
        D3D12_RESOURCE_STATE_COMMON,
        &optClear,
        IID_PPV_ARGS(depthStencilBuffer_.GetAddressOf())));

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format = depthStencilFormat_;
    dsvDesc.Texture2D.MipSlice = 0;
    d3dDevice_->CreateDepthStencilView(depthStencilBuffer_.Get(), &dsvDesc, DepthStencilView());

    commandList_->ResourceBarrier(1, 
        &CD3DX12_RESOURCE_BARRIER::Transition(depthStencilBuffer_.Get(),
            D3D12_RESOURCE_STATE_COMMON,
            D3D12_RESOURCE_STATE_DEPTH_WRITE));

    ThrowIfFailed(commandList_->Close());
    ID3D12CommandList* cmdsLists[] = {commandList_.Get()};
    commandQueue_->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    FlushCommandQueue();

    screenViewport_.TopLeftX = 0;
    screenViewport_.TopLeftY = 0;
    screenViewport_.Width = static_cast<float> (clientWidth_);
    screenViewport_.Height = static_cast<float> (clientHeight_);
    screenViewport_.MinDepth = 0.0f;
    screenViewport_.MaxDepth = 1.0f;

    scissorRect_ = {0, 0, clientWidth_, clientHeight_};
}

void D3DApp::OnMouseDown(WPARAM btnState, int x, int y)
{
}

void D3DApp::OnMouseUp(WPARAM btnState, int x, int y)
{
}

void D3DApp::OnMouseMove(WPARAM btnState, int x, int y)
{
}

bool D3DApp::InitMainWindow(void)
{
    WNDCLASS wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hAppInst_;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) GetStockObject(NULL_BRUSH);
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = "MainWindow";

    if (RegisterClass(&wc) == false)
    {
        MessageBox(0, "Register Class Failed.", nullptr, MB_OK);
        return false;
    }

    RECT rect = {0, 0, clientWidth_, clientHeight_};
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    hMainWnd_ = CreateWindow("MainWindow", 
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

    ShowWindow(hMainWnd_, SW_SHOW);
    UpdateWindow(hMainWnd_);

    return true;
}

bool D3DApp::InitDirect3D(void)
{
#if defined(DEBUG) || defined(_DEBUG)
    Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
    ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
    debugController->EnableDebugLayer();
#endif /**/

    ThrowIfFailed(CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_)));

    HRESULT hardwareResult = D3D12CreateDevice(nullptr, 
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&d3dDevice_));

     if (FAILED(hardwareResult))
     {
        Microsoft::WRL::ComPtr<IDXGIAdapter> pWrapAdapter;
        ThrowIfFailed(dxgiFactory_->EnumWarpAdapter(IID_PPV_ARGS(&pWrapAdapter)));

        ThrowIfFailed(D3D12CreateDevice(pWrapAdapter.Get(), 
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&d3dDevice_)));
     }

     ThrowIfFailed(d3dDevice_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_)));

     rtvDescriptorSize_ = d3dDevice_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
     dsvDescriptorSize_ = d3dDevice_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
     cbvSrvDescriptorSize_ = d3dDevice_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
     
     D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualiyLevels;
     msQualiyLevels.Format = backBufferFormat_;
     msQualiyLevels.SampleCount = 4;
     msQualiyLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
     msQualiyLevels.NumQualityLevels = 0;
     ThrowIfFailed(d3dDevice_->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
        &msQualiyLevels,
        sizeof(msQualiyLevels)));

    msaaQuality4x_ = msQualiyLevels.NumQualityLevels;
    assert(msaaQuality4x_ > 0 && "Unexpected MSAA quality level.");

#if defined(DEBUG) || defined(_DEBUG)
    LogAdapters();
#endif /**/

    CreateCommandObjects();
    CreateSwapChain();
    CreateRtvAndDsvDescriptorHeaps();

    return true;
}

void D3DApp::CreateCommandObjects(void)
{
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
       
    ThrowIfFailed(d3dDevice_->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue_)));

    ThrowIfFailed(d3dDevice_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(commandListAlloc_.GetAddressOf())));

    ThrowIfFailed(d3dDevice_->CreateCommandList(0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        commandListAlloc_.Get(),
        nullptr,
        IID_PPV_ARGS(commandList_.GetAddressOf())));

    commandList_->Close();
}

void D3DApp::CreateSwapChain(void)
{
    swapChain_.Reset();

    DXGI_SWAP_CHAIN_DESC sd;
    sd.BufferDesc.Width = clientWidth_;
    sd.BufferDesc.Height = clientHeight_;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.Format = backBufferFormat_;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.SampleDesc.Count = msaaState4x_ ? 4 : 1;
    sd.SampleDesc.Quality = msaaState4x_ ? (msaaQuality4x_ - 1) : 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = swapChainbufferCount;
    sd.OutputWindow = hMainWnd_;
    sd.Windowed = true;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    ThrowIfFailed(dxgiFactory_->CreateSwapChain(commandQueue_.Get(),
        &sd,
        swapChain_.GetAddressOf()));
}

void D3DApp::FlushCommandQueue(void)
{
    ++currentFence_;
    ThrowIfFailed(commandQueue_->Signal(fence_.Get(), currentFence_));

    if (fence_->GetCompletedValue() < currentFence_)
    {
        HANDLE eventHandle = CreateEventEx(nullptr,
            false, 
            false, 
            EVENT_ALL_ACCESS);
        
        ThrowIfFailed(fence_->SetEventOnCompletion(currentFence_, eventHandle));
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }
}

ID3D12Resource* D3DApp::CurrentBackBuffer(void) const
{
    return swapChainBuffer_[currBackBuffer_].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE D3DApp::CurrentBackBufferView(void) const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeap_->GetCPUDescriptorHandleForHeapStart(),
        currBackBuffer_,
        rtvDescriptorSize_);
}

D3D12_CPU_DESCRIPTOR_HANDLE D3DApp::DepthStencilView(void) const
{
    return dsvHeap_->GetCPUDescriptorHandleForHeapStart();
}

void D3DApp::CalculateFrameStats(void)
{
    static int frameCount = 0;
    static float timeElapsed = 0.0f;

    ++frameCount;

    if ((timer_.TotalTime() - timeElapsed) >= 1.0f)
    {
        float fps = (float) frameCount;
        float mspf = 1000.0f / fps;

        std::string fpsStr = std::to_string(fps);
        std::string mspfStr = std::to_string(mspf);
        std::string windowText = mainWndCaption_
            + " fps : " + fpsStr
            + " mspf : " + mspfStr;

        SetWindowText(hMainWnd_, windowText.c_str());

        frameCount = 0;
        timeElapsed += 1.0f;
    }
}

void D3DApp::LogAdapters(void)
{
    UINT i = 0;
    IDXGIAdapter* adapter = nullptr;
    std::vector<IDXGIAdapter*> adapterList;
    while (dxgiFactory_->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_ADAPTER_DESC desc;
        adapter->GetDesc(&desc);

        std::string text = "***Adapter : ";
        std::wstring msg = desc.Description;
        text.append(msg.begin(), msg.end());
        text += "\n";

        OutputDebugString(text.c_str());
        adapterList.push_back(adapter);
        ++i;
    }

    for (size_t i = 0; i < adapterList.size(); ++i)
    {
        LogAdapterOutpujts(adapterList[i]);
        ReleaseCom(adapterList[i]);
    }
}

void D3DApp::LogAdapterOutpujts(IDXGIAdapter* adapter)
{
    UINT i = 0;
    IDXGIOutput* output = nullptr;
    while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_OUTPUT_DESC desc;
        output->GetDesc(&desc);

        std::string text = "***Output: ";
        std::wstring msg = desc.DeviceName;
        text.append(msg.begin(), msg.end());
        text += "\n";

        OutputDebugString(text.c_str());

        LogOutputDisplayModes(output, DXGI_FORMAT_B8G8R8A8_UNORM);
        ReleaseCom(output);
        ++i;
    }
}

void D3DApp::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
{
    UINT count = 0;
    UINT flags = 0;

    output->GetDisplayModeList(format, flags, &count, nullptr);
    std::vector<DXGI_MODE_DESC> modeList(count);
    output->GetDisplayModeList(format, flags, &count, &modeList[0]);

    for (auto& mode : modeList)
    {
        UINT n = mode.RefreshRate.Numerator;
        UINT d = mode.RefreshRate.Denominator;
        std::string text = "width = " + std::to_string(mode.Width) + " "
            + "height = " + std::to_string(mode.Height) + " "
            + "refresh = " + std::to_string(n) + "/" + std::to_string(d);
            + "\n";

        OutputDebugString(text.c_str());
    }
}
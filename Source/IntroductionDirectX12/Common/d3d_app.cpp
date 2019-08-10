
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
    if (_d3dDevice != nullptr)
    {
        FlushCommandQueue();
    }
}

HINSTANCE D3DApp::AppInst(void) const
{
    return _hAppInst;
}

HWND D3DApp::MainWnd(void) const
{
    return _hMainWnd;
}

float D3DApp::AspectRatio(void) const
{
    return static_cast<float> (_clientWidth) / _clientHeight;
}

bool D3DApp::Get4xMsaaState(void) const
{
    return _msaaState4x;
}

void D3DApp::Set4xMsaaState(bool value)
{
    if (_msaaState4x != value)
    {
        _msaaState4x = value;

        CreateSwapChain();
        OnResize();
    }
}

int D3DApp::Run(void)
{
    MSG msg = {0};

    _timer.Reset();

    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            _timer.Tick();

            if (_appPaused == true)
            {
                Sleep(100);
            }
            else
            {
                CalculateFrameStats();
                Update(_timer);
                Draw(_timer);
            }
        }
    }

    return static_cast<int> (msg.wParam);
}

bool D3DApp::Initialize(HINSTANCE hInstance)
{
    _hAppInst = hInstance;

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
                _appPaused = true;
                _timer.Stop();
            }
            else
            {
                _appPaused = false;
                _timer.Start();
            }
        }
        break;
        case WM_SIZE:
        {
            _clientWidth = LOWORD(lParam);
            _clientHeight = HIWORD(lParam);
            if (_d3dDevice != nullptr)
            {
                if (wParam == SIZE_MINIMIZED)
                {
                    _appPaused = true;
                    _mininized = true;
                    _maximized = false;
                }
                else if (wParam == SIZE_MAXIMIZED)
                {
                    _appPaused = false;
                    _mininized = false;
                    _maximized = true;
                    OnResize();
                }
                else if (wParam == SIZE_RESTORED)
                {
                    if (_mininized == true)
                    {
                        _appPaused = false;
                        _mininized = false;
                        OnResize();
                    }
                    else if (_maximized == true)
                    {
                        _appPaused = false;
                        _maximized = false;
                        OnResize();
                    }
                    else if (_resizing == true)
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
            _appPaused = true;
            _resizing = true;
            _timer.Stop();
            return 0;
        }
        break;
        case WM_EXITSIZEMOVE:
        {
            _appPaused = false;
            _resizing = false;
            _timer.Start();
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
                Set4xMsaaState(!_msaaState4x);
            }
        }
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void D3DApp::CreateRtvAndDsvDescriptorHeaps(void)
{
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
    rtvHeapDesc.NumDescriptors = SwapChainbufferCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;

    ThrowIfFailed(_d3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(_rtvHeap.GetAddressOf())));

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;
    
    ThrowIfFailed(_d3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(_dsvHeap.GetAddressOf())));
}

void D3DApp::OnResize(void)
{
    assert(_d3dDevice != nullptr);
    assert(_swapChain != nullptr);
    assert(_commandListAlloc != nullptr);
    
    FlushCommandQueue();
    
    ThrowIfFailed(_commandList->Reset(_commandListAlloc.Get(), nullptr));

    for (int i = 0; i < SwapChainbufferCount; ++i)
    {
        _swapChainBuffer[i].Reset();
    }
    _depthStencilBuffer.Reset();

    ThrowIfFailed(_swapChain->ResizeBuffers(SwapChainbufferCount, 
        _clientWidth,
        _clientHeight,
        _backBufferFormat,
        DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

    currBackBuffer_ = 0;

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(_rtvHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT i = 0; i < SwapChainbufferCount; ++i)
    {
        ThrowIfFailed(_swapChain->GetBuffer(i, IID_PPV_ARGS(&_swapChainBuffer[i])));
        _d3dDevice->CreateRenderTargetView(_swapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
        rtvHeapHandle.Offset(1, _rtvDescriptorSize);
    }

    D3D12_RESOURCE_DESC depthStencilDesc;
    depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDesc.Alignment = 0;
    depthStencilDesc.Width = _clientWidth;
    depthStencilDesc.Height = _clientHeight;
    depthStencilDesc.DepthOrArraySize = 1;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    depthStencilDesc.SampleDesc.Count = _msaaState4x ? 4 : 1;
    depthStencilDesc.SampleDesc.Quality = _msaaState4x ? (_msaaQuality4x - 1) : 0;
    depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE optClear;
    optClear.Format = _depthStencilFormat;
    optClear.DepthStencil.Depth = 1.0f;
    optClear.DepthStencil.Stencil = 0;

    CD3DX12_HEAP_PROPERTIES heapProptertyDeafult = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    ThrowIfFailed(_d3dDevice->CreateCommittedResource(
        &heapProptertyDeafult,
        D3D12_HEAP_FLAG_NONE,
        &depthStencilDesc,
        D3D12_RESOURCE_STATE_COMMON,
        &optClear,
        IID_PPV_ARGS(_depthStencilBuffer.GetAddressOf())));

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format = _depthStencilFormat;
    dsvDesc.Texture2D.MipSlice = 0;
    _d3dDevice->CreateDepthStencilView(_depthStencilBuffer.Get(), &dsvDesc, DepthStencilView());

    CD3DX12_RESOURCE_BARRIER barrierCommonToDepthWrite = CD3DX12_RESOURCE_BARRIER::Transition(_depthStencilBuffer.Get(),
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_RESOURCE_STATE_DEPTH_WRITE);
    _commandList->ResourceBarrier(1, &barrierCommonToDepthWrite);

    ThrowIfFailed(_commandList->Close());
    ID3D12CommandList* cmdsLists[] = {_commandList.Get()};
    _commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    FlushCommandQueue();

    _screenViewport.TopLeftX = 0;
    _screenViewport.TopLeftY = 0;
    _screenViewport.Width = static_cast<float> (_clientWidth);
    _screenViewport.Height = static_cast<float> (_clientHeight);
    _screenViewport.MinDepth = 0.0f;
    _screenViewport.MaxDepth = 1.0f;

    _scissorRect = {0, 0, _clientWidth, _clientHeight};
}

void D3DApp::OnMouseDown(WPARAM btnState, int x, int y)
{
    _Unreferenced_parameter_(btnState);
    _Unreferenced_parameter_(x);
    _Unreferenced_parameter_(y);
}

void D3DApp::OnMouseUp(WPARAM btnState, int x, int y)
{
    _Unreferenced_parameter_(btnState);
    _Unreferenced_parameter_(x);
    _Unreferenced_parameter_(y);
}

void D3DApp::OnMouseMove(WPARAM btnState, int x, int y)
{
    _Unreferenced_parameter_(btnState);
    _Unreferenced_parameter_(x);
    _Unreferenced_parameter_(y);
}

bool D3DApp::InitMainWindow(void)
{
    WNDCLASS wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = _hAppInst;
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

    RECT rect = {0, 0, _clientWidth, _clientHeight};
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    _hMainWnd = CreateWindow("MainWindow", 
        _mainWndCaption.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        width,
        height,
        nullptr, 
        nullptr, 
        _hAppInst,
        nullptr);

    if (!_hMainWnd)
    {
        MessageBox(0, "CreateWindow Failed.", nullptr, MB_OK);
        return false;
    }

    ShowWindow(_hMainWnd, SW_SHOW);
    UpdateWindow(_hMainWnd);

    return true;
}

bool D3DApp::InitDirect3D(void)
{
#if defined(DEBUG) || defined(_DEBUG)
    Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
    ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
    debugController->EnableDebugLayer();
#endif /**/

    ThrowIfFailed(CreateDXGIFactory(IID_PPV_ARGS(&_dxgiFactory)));

    HRESULT hardwareResult = D3D12CreateDevice(nullptr, 
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&_d3dDevice));

     if (FAILED(hardwareResult))
     {
        Microsoft::WRL::ComPtr<IDXGIAdapter> pWrapAdapter;
        ThrowIfFailed(_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWrapAdapter)));

        ThrowIfFailed(D3D12CreateDevice(pWrapAdapter.Get(), 
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&_d3dDevice)));
     }

     ThrowIfFailed(_d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence)));

     _rtvDescriptorSize = _d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
     _dsvDescriptorSize = _d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
     _cbvSrvDescriptorSize = _d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
     
     D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualiyLevels;
     msQualiyLevels.Format = _backBufferFormat;
     msQualiyLevels.SampleCount = 4;
     msQualiyLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
     msQualiyLevels.NumQualityLevels = 0;
     ThrowIfFailed(_d3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
        &msQualiyLevels,
        sizeof(msQualiyLevels)));

    _msaaQuality4x = msQualiyLevels.NumQualityLevels;
    assert(_msaaQuality4x > 0 && "Unexpected MSAA quality level.");

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
       
    ThrowIfFailed(_d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&_commandQueue)));

    ThrowIfFailed(_d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(_commandListAlloc.GetAddressOf())));

    ThrowIfFailed(_d3dDevice->CreateCommandList(0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        _commandListAlloc.Get(),
        nullptr,
        IID_PPV_ARGS(_commandList.GetAddressOf())));

    _commandList->Close();
}

void D3DApp::CreateSwapChain(void)
{
    _swapChain.Reset();

    DXGI_SWAP_CHAIN_DESC sd;
    sd.BufferDesc.Width = _clientWidth;
    sd.BufferDesc.Height = _clientHeight;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.Format = _backBufferFormat;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.SampleDesc.Count = _msaaState4x ? 4 : 1;
    sd.SampleDesc.Quality = _msaaState4x ? (_msaaQuality4x - 1) : 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = SwapChainbufferCount;
    sd.OutputWindow = _hMainWnd;
    sd.Windowed = true;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    ThrowIfFailed(_dxgiFactory->CreateSwapChain(_commandQueue.Get(),
        &sd,
        _swapChain.GetAddressOf()));
}

void D3DApp::FlushCommandQueue(void)
{
    ++_currentFence;
    ThrowIfFailed(_commandQueue->Signal(_fence.Get(), _currentFence));

    if (_fence->GetCompletedValue() < _currentFence)
    {
        HANDLE eventHandle = CreateEventEx(nullptr,
            false, 
            false, 
            EVENT_ALL_ACCESS);
        
        ThrowIfFailed(_fence->SetEventOnCompletion(_currentFence, eventHandle));
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }
}

ID3D12Resource* D3DApp::CurrentBackBuffer(void) const
{
    return _swapChainBuffer[currBackBuffer_].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE D3DApp::CurrentBackBufferView(void) const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
        currBackBuffer_,
        _rtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE D3DApp::DepthStencilView(void) const
{
    return _dsvHeap->GetCPUDescriptorHandleForHeapStart();
}

void D3DApp::CalculateFrameStats(void)
{
    static int frameCount = 0;
    static float timeElapsed = 0.0f;

    ++frameCount;

    if ((_timer.TotalTime() - timeElapsed) >= 1.0f)
    {
        float fps = (float) frameCount;
        float mspf = 1000.0f / fps;

        std::string fpsStr = std::to_string(fps);
        std::string mspfStr = std::to_string(mspf);
        std::string windowText = _mainWndCaption
            + " fps : " + fpsStr
            + " mspf : " + mspfStr;

        SetWindowText(_hMainWnd, windowText.c_str());

        frameCount = 0;
        timeElapsed += 1.0f;
    }
}

void D3DApp::LogAdapters(void)
{
    UINT i = 0;
    IDXGIAdapter* adapter = nullptr;
    std::vector<IDXGIAdapter*> adapterList;
    while (_dxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
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

    for (size_t idx = 0; idx < adapterList.size(); ++idx)
    {
        LogAdapterOutpujts(adapterList[idx]);
        ReleaseCom(adapterList[idx]);
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
            + "refresh = " + std::to_string(n) + "/" + std::to_string(d)
            + "\n";

        OutputDebugString(text.c_str());
    }
}
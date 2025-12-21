
#ifndef __D3D_APP__H__
#define __D3D_APP__H__

#include "d3d_util.h"
#include "gametimer.h"

#if defined(DEBUG) || defined(_DEBUG)
    #define _CRTDBG_MAP_ALLOC
    #include <crtdbg.h>
#endif /**/

#include <Windowsx.h>

//----------------------------------------------------------------
// D3DApp
//----------------------------------------------------------------
class D3DApp
{
public:
    static D3DApp* GetApp(void);

protected:
    static D3DApp* d3dApp;
    static constexpr int SwapChainbufferCount = 2;

protected:
    D3DApp(void);    
    virtual ~D3DApp(void);
        
    D3DApp(const D3DApp& other) = delete;
    D3DApp& operator=(const D3DApp& other) = delete;

public:
    HINSTANCE AppInst(void) const;
    HWND MainWnd(void) const;
    float AspectRatio(void) const;

    bool Get4xMsaaState(void) const;
    void Set4xMsaaState(bool value);

    int Run(void);

    virtual bool Initialize(HINSTANCE hInstance);
    virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
    virtual void CreateRtvAndDsvDescriptorHeaps(void);
    virtual void OnResize(void);
    virtual void Update(const GameTimer& timer) = 0;
    virtual void Draw(const GameTimer& timer) = 0;

    virtual void OnMouseDown(WPARAM btnState, int x, int y);
    virtual void OnMouseUp(WPARAM btnState, int x, int y);
    virtual void OnMouseMove(WPARAM btnState, int x, int y);

protected:
    bool InitMainWindow(void);
    bool InitDirect3D(void);
    void CreateCommandObjects(void);
    void CreateSwapChain(void);

    void FlushCommandQueue(void);

    ID3D12Resource* CurrentBackBuffer(void) const;
    D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView(void) const;
    D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView(void) const;

    void CalculateFrameStats(void);
    void LogAdapters(void);
    void LogAdapterOutpujts(IDXGIAdapter* adapter);
    void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);

protected:
    HINSTANCE _hAppInst = nullptr;
    HWND _hMainWnd = nullptr;

    bool _appPaused = false;
    bool _mininized = false;
    bool _maximized = false;
    bool _resizing = false;
    bool _fullScreenState = false;

    bool _msaaState4x = false;
    UINT _msaaQuality4x = 0;

    GameTimer _timer;

    Microsoft::WRL::ComPtr<IDXGIFactory4> _dxgiFactory;
    Microsoft::WRL::ComPtr<IDXGISwapChain> _swapChain;
    Microsoft::WRL::ComPtr<ID3D12Device> _d3dDevice;

    Microsoft::WRL::ComPtr<ID3D12Fence> _fence;
    UINT64 _currentFence = 0;

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> _commandQueue;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> _commandListAlloc;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList;

    int currBackBuffer_ = 0;

    Microsoft::WRL::ComPtr<ID3D12Resource> _swapChainBuffer[SwapChainbufferCount];
    Microsoft::WRL::ComPtr<ID3D12Resource> _depthStencilBuffer;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _rtvHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _dsvHeap;

    D3D12_VIEWPORT _screenViewport;
    D3D12_RECT _scissorRect;

    UINT _rtvDescriptorSize = 0;
    UINT _dsvDescriptorSize = 0;
    UINT _cbvSrvDescriptorSize = 0;

    std::string _mainWndCaption = "d3d App";
    D3D_DRIVER_TYPE _d3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
    DXGI_FORMAT _backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT _depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    
    int _clientWidth = 800;
    int _clientHeight = 600;
};

#endif /*__D3D_APP__H__*/
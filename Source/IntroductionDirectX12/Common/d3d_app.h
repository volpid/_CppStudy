
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
    static constexpr int swapChainbufferCount = 2;

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
    virtual void Update(const GameTimer& gt) = 0;
    virtual void Draw(const GameTimer& gt) = 0;

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
    HINSTANCE hAppInst_ = nullptr;
    HWND hMainWnd_ = nullptr;

    bool appPaused_ = false;
    bool mininized_ = false;
    bool maximized_ = false;
    bool resizing_ = false;
    bool fullScreenState_ = false;

    bool msaaState4x_ = false;
    UINT msaaQuality4x_ = 0;

    GameTimer timer_;

    Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory_;
    Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain_;
    Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice_;

    Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
    UINT64 currentFence_ = 0;

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandListAlloc_;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;

    int currBackBuffer_ = 0;

    Microsoft::WRL::ComPtr<ID3D12Resource> swapChainBuffer_[swapChainbufferCount];
    Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer_;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap_;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap_;

    D3D12_VIEWPORT screenViewport_;
    D3D12_RECT scissorRect_;

    UINT rtvDescriptorSize_ = 0;
    UINT dsvDescriptorSize_ = 0;
    UINT cbvSrvDescriptorSize_ = 0;

    std::string mainWndCaption_ = "d3d App";
    D3D_DRIVER_TYPE d3dDriverType_ = D3D_DRIVER_TYPE_HARDWARE;
    DXGI_FORMAT backBufferFormat_ = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT depthStencilFormat_ = DXGI_FORMAT_D24_UNORM_S8_UINT;
    
    int clientWidth_ = 800;
    int clientHeight_ = 600;
};

#endif /*__D3D_APP__H__*/
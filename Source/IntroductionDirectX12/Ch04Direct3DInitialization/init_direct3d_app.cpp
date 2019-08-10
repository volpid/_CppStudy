
#include "../Common/d3d_app.h"

//----------------------------------------------------------------
// InitDirect3DApp
//----------------------------------------------------------------
class InitDirect3DApp : public D3DApp
{
public:
    InitDirect3DApp(void) = default;
    virtual ~InitDirect3DApp(void) = default;

    virtual bool Initialize(HINSTANCE hInstance) override;

private:
    virtual void OnResize(void) override;
    virtual void Update(const GameTimer& timer) override;
    virtual void Draw(const GameTimer& timer) override;
};

//InitDirect3DApp appInstance;

//----------------------------------------------------------------
bool InitDirect3DApp::Initialize(HINSTANCE hInstance)
{
    if (D3DApp::Initialize(hInstance) == false)
    {
        return false;
    }
    return true;
}

void InitDirect3DApp::OnResize(void)
{
    D3DApp::OnResize();
}

void InitDirect3DApp::Update(const GameTimer& timer)
{
    _Unreferenced_parameter_(timer);
}

void InitDirect3DApp::Draw(const GameTimer& timer) 
{
    _Unreferenced_parameter_(timer);

    ThrowIfFailed(_commandListAlloc->Reset());
    ThrowIfFailed(_commandList->Reset(_commandListAlloc.Get(), nullptr));

    CD3DX12_RESOURCE_BARRIER barrierPresentToRenderTarget = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    _commandList->ResourceBarrier(1, &barrierPresentToRenderTarget);

    _commandList->RSSetViewports(1, &_screenViewport);
    _commandList->RSSetScissorRects(1, &_scissorRect);

    D3D12_CPU_DESCRIPTOR_HANDLE currentBackBufferView = CurrentBackBufferView();
    D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = DepthStencilView();

    _commandList->ClearRenderTargetView(currentBackBufferView, DirectX::Colors::LightSteelBlue, 0,  nullptr);
    _commandList->ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    _commandList->OMSetRenderTargets(1, &currentBackBufferView, true, &depthStencilView);

    CD3DX12_RESOURCE_BARRIER barrierRenderTargetToPresent = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);
    _commandList->ResourceBarrier(1, &barrierRenderTargetToPresent);
    
    ThrowIfFailed(_commandList->Close());

    ID3D12CommandList* cmdsLists[] = {_commandList.Get()};
    _commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    ThrowIfFailed(_swapChain->Present(0, 0));
    currBackBuffer_ = (currBackBuffer_ + 1) % SwapChainbufferCount;

    FlushCommandQueue();
}

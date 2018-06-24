
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

    ThrowIfFailed(commandListAlloc_->Reset());
    ThrowIfFailed(commandList_->Reset(commandListAlloc_.Get(), nullptr));

    commandList_->ResourceBarrier(1, 
        &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
            D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RENDER_TARGET));

    commandList_->RSSetViewports(1, &screenViewport_);
    commandList_->RSSetScissorRects(1, &scissorRect_);

    commandList_->ClearRenderTargetView(CurrentBackBufferView(),
        DirectX::Colors::LightSteelBlue,
        0, 
        nullptr);
    commandList_->ClearDepthStencilView(DepthStencilView(),
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 
        1.0f, 
        0,
        0,
        nullptr);

    commandList_->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

    commandList_->ResourceBarrier(1, 
        &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PRESENT));
    
    ThrowIfFailed(commandList_->Close());

    ID3D12CommandList* cmdsLists[] = {commandList_.Get()};
    commandQueue_->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    ThrowIfFailed(swapChain_->Present(0, 0));
    currBackBuffer_ = (currBackBuffer_ + 1) % swapChainbufferCount;

    FlushCommandQueue();
}

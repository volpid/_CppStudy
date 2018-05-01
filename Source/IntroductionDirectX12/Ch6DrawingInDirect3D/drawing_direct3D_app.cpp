
#include "../Common/d3d_app.h"
#include "../Common/math_helper.h"
#include "../Common/upload_buffer.h"

#include <array>
#include <memory>
#include <vector>

//----------------------------------------------------------------
// data
//----------------------------------------------------------------
struct Vertex
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT4 color;
};

struct ObjectConstant
{
    DirectX::XMFLOAT4X4 worldViewProj = MathHelper::Indentity4x4();
};

//----------------------------------------------------------------
// BoxApp
//----------------------------------------------------------------
class BoxApp : public D3DApp
{
public:
    BoxApp(void) = default;
    virtual ~BoxApp(void) = default;

    BoxApp(const BoxApp& other) = delete;
    BoxApp& operator=(const BoxApp& other) = delete;

    virtual bool Initialize(HINSTANCE hInstance) override;

    virtual void OnResize(void) override;
    virtual void Update(const GameTimer& gt) override;
    virtual void Draw(const GameTimer& gt) override;

    virtual void OnMouseDown(WPARAM btnState, int x, int y) override;
    virtual void OnMouseUp(WPARAM btnState, int x, int y) override;
    virtual void OnMouseMove(WPARAM btnState, int x, int y) override;

private:
    void BuildDescriptorHeap(void);
    void BuildConstantBuffer(void);
    void BuildRootSignature(void);
    void BuildShaderAndInputLayout(void);
    void BuildBoxGeometry(void);
    void BuildPSO(void);

private:
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> cbvHeap_ = nullptr;

    std::unique_ptr<UplaodBuffer<ObjectConstant>> objectCB_ = nullptr;
    std::unique_ptr<MeshGeometry> boxGeometry_ = nullptr;

    Microsoft::WRL::ComPtr<ID3DBlob> vsByteCode_ = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> psByteCode_ = nullptr;

    std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout_;

    Microsoft::WRL::ComPtr<ID3D12PipelineState> pso_ = nullptr;

    DirectX::XMFLOAT4X4 world_ = MathHelper::Indentity4x4();
    DirectX::XMFLOAT4X4 view_ = MathHelper::Indentity4x4();
    DirectX::XMFLOAT4X4 proj_ = MathHelper::Indentity4x4();

    float theta_ = 1.5f * MathHelper::Pi;
    float phi_ = MathHelper::Pi / 4;
    float radius_ = 5.0f;

    POINT lastMousePos_;
};

BoxApp appInstance;

//----------------------------------------------------------------
bool BoxApp::Initialize(HINSTANCE hInstance)
{
    if (D3DApp::Initialize(hInstance) == false)
    {
        return false;
    }

    ThrowIfFailed(commandList_->Reset(commandListAlloc_.Get(), nullptr));

    BuildDescriptorHeap();
    BuildConstantBuffer();
    BuildRootSignature();
    BuildShaderAndInputLayout();
    BuildBoxGeometry();
    BuildPSO();

    ThrowIfFailed(commandList_->Close());
    ID3D12CommandList* cmdLists[] = {commandList_.Get()};
    commandQueue_->ExecuteCommandLists(_countof(cmdLists), cmdLists);

    FlushCommandQueue();
}

void BoxApp::OnResize(void)
{
    D3DApp::OnResize();

    DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi,
        AspectRatio(),
        1.0f,
        1000.0f);
    DirectX::XMStoreFloat4x4(&proj_, proj);
}

void BoxApp::Update(const GameTimer& gt)
{
    float x = radius_ * sinf(phi_) * cosf(theta_);
    float y = radius_ * sinf(phi_) * sinf(theta_);
    float z = radius_ * cosf(phi_);

    DirectX::XMVECTOR pos = DirectX::XMVectorSet(x, y, z, 1.0f);
    DirectX::XMVECTOR target = DirectX::XMVectorZero();
    DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(pos, target, up);
    DirectX::XMStoreFloat4x4(&view_, view);

    DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&world_);
    DirectX::XMMATRIX proj = DirectX::XMLoadFloat4x4(&proj_);
    DirectX::XMMATRIX worldViewProj = world * view * proj;

    ObjectConstant objectConstant;
    DirectX::XMStoreFloat4x4(&objectConstant.worldViewProj, DirectX::XMMatrixTranspose(worldViewProj));
    objectCB_->CopyData(0, objectConstant);
}

void BoxApp::Draw(const GameTimer& gt)
{
    ThrowIfFailed(commandListAlloc_->Reset());
    ThrowIfFailed(commandList_->Reset(commandListAlloc_.Get(), pso_.Get()));

    commandList_->RSSetViewports(1, &screenViewport_);
    commandList_->RSSetScissorRects(1, &scissorRect_);

    commandList_->ResourceBarrier(1, 
        &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
            D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RENDER_TARGET));

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

    ID3D12DescriptorHeap* descriptorHeap[] = {cbvHeap_.Get()};
    commandList_->SetDescriptorHeaps(_countof(descriptorHeap), descriptorHeap);

    commandList_->SetGraphicsRootSignature(rootSignature_.Get());
    
    commandList_->IASetVertexBuffers(0, 1, &boxGeometry_->VertexBufferView());
    commandList_->IASetIndexBuffer(&boxGeometry_->IndexBufferView());
    commandList_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    commandList_->SetGraphicsRootDescriptorTable(0, cbvHeap_->GetGPUDescriptorHandleForHeapStart());

    commandList_->DrawIndexedInstanced(boxGeometry_->drawArgs["box"].indexCount, 1, 0, 0, 0);

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

void BoxApp::OnMouseDown(WPARAM btnState, int x, int y)
{
    lastMousePos_.x = x;
    lastMousePos_.y = y;

    SetCapture(hMainWnd_);
}

void BoxApp::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void BoxApp::OnMouseMove(WPARAM btnState, int x, int y)
{
    if ((btnState & MK_LBUTTON) == MK_LBUTTON)
    {
        float dx = DirectX::XMConvertToRadians(0.25f * static_cast<float> (x - lastMousePos_.x));
        float dy = DirectX::XMConvertToRadians(0.25f * static_cast<float> (y - lastMousePos_.y));

        theta_ += dx;
        phi_ += dy;

        phi_ = MathHelper::Clamp(phi_, 0.1f, MathHelper::Pi - 0.1f);        
    }
    else if ((btnState & MK_RBUTTON) == MK_RBUTTON)
    {
        float dx = 0.005f * static_cast<float> (x - lastMousePos_.x);
        float dy = 0.005f * static_cast<float> (y - lastMousePos_.y);

        radius_ += (dx - dy);
        radius_ = MathHelper::Clamp(radius_, 3.0f, 15.0f);
    }

    lastMousePos_.x = x;
    lastMousePos_.y = y;
}

void BoxApp::BuildDescriptorHeap(void)
{
    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
    cbvHeapDesc.NumDescriptors = 1;
    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    cbvHeapDesc.NodeMask = 0;

    ThrowIfFailed(d3dDevice_->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&cbvHeap_)));
}

void BoxApp::BuildConstantBuffer(void)
{
    objectCB_ = std::make_unique<UplaodBuffer<ObjectConstant>>(d3dDevice_.Get(), 1, true);

    const UINT objCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstant));

    D3D12_GPU_VIRTUAL_ADDRESS cbAddress = objectCB_->Resource()->GetGPUVirtualAddress();
    int boxCBufIndex = 0;
    cbAddress += boxCBufIndex * objCBByteSize;

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
    cbvDesc.BufferLocation = cbAddress;
    cbvDesc.SizeInBytes = objCBByteSize;

    d3dDevice_->CreateConstantBufferView(&cbvDesc, cbvHeap_->GetCPUDescriptorHandleForHeapStart());
}

void BoxApp::BuildRootSignature(void)
{
    CD3DX12_ROOT_PARAMETER slotRootParameter[1];
    CD3DX12_DESCRIPTOR_RANGE cbvTable;
    cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
    slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc = 
    {
        1, 
        slotRootParameter, 
        0, 
        nullptr, 
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT,
    };

    Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, 
        D3D_ROOT_SIGNATURE_VERSION_1,
        serializedRootSig.GetAddressOf(),
        errorBlob.GetAddressOf());

    if (errorBlob != nullptr)
    {   
        OutputDebugString(static_cast<char*> (errorBlob->GetBufferPointer()));
    }
    
    ThrowIfFailed(hr);
    ThrowIfFailed(d3dDevice_->CreateRootSignature(0,
        serializedRootSig->GetBufferPointer(),
        serializedRootSig->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature_)));
}

void BoxApp::BuildShaderAndInputLayout(void)
{
    HRESULT hr = S_OK;

    vsByteCode_ = D3DUtil::CompileShader("..\\..\\Source\\IntroductionDirectX12\\Shader\\ch06_color_vs.hlsl", nullptr, "vs_main", "vs_5_0");
    psByteCode_ = D3DUtil::CompileShader("..\\..\\Source\\IntroductionDirectX12\\Shader\\ch06_color_ps.hlsl", nullptr, "ps_main", "ps_5_0");

    inputLayout_ = 
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };
}

void BoxApp::BuildBoxGeometry(void)
{
    std::array<Vertex, 8> vertices = 
    {
        Vertex({DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT4(DirectX::Colors::White)}),
        Vertex({DirectX::XMFLOAT3(-1.0f, +1.0f, -1.0f), DirectX::XMFLOAT4(DirectX::Colors::Black)}),
        Vertex({DirectX::XMFLOAT3(+1.0f, +1.0f, -1.0f), DirectX::XMFLOAT4(DirectX::Colors::Red)}),
        Vertex({DirectX::XMFLOAT3(+1.0f, -1.0f, -1.0f), DirectX::XMFLOAT4(DirectX::Colors::Green)}),
        Vertex({DirectX::XMFLOAT3(-1.0f, -1.0f, +1.0f), DirectX::XMFLOAT4(DirectX::Colors::Blue)}),
        Vertex({DirectX::XMFLOAT3(-1.0f, +1.0f, +1.0f), DirectX::XMFLOAT4(DirectX::Colors::Yellow)}),
        Vertex({DirectX::XMFLOAT3(+1.0f, +1.0f, +1.0f), DirectX::XMFLOAT4(DirectX::Colors::Cyan)}),
        Vertex({DirectX::XMFLOAT3(+1.0f, -1.0f, +1.0f), DirectX::XMFLOAT4(DirectX::Colors::Magenta)})
    };

    std::array<std::uint16_t, 36> indices = 
    {
        //front face
        0, 1, 2,
        0, 2, 3,
        //back face
        4, 6, 5,
        4, 7, 6,
        //left face
        4, 5, 1,
        4, 1, 0,
        //right face
        3, 2, 6,
        3, 6, 7,
        //top face
        1, 5, 6,
        1, 6, 2,
        //bottom face
        4, 0, 3,
        4, 3, 7,
    };

    const UINT vbByteSize = (UINT) vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT) indices.size() * sizeof(std::uint16_t);

    boxGeometry_ = std::make_unique<MeshGeometry>();
    boxGeometry_->name = "boxGeometry";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &boxGeometry_->vertexBufferCPU));
    CopyMemory(boxGeometry_->vertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &boxGeometry_->indexBufferCPU));
    CopyMemory(boxGeometry_->indexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    boxGeometry_->vertexBufferGPU = D3DUtil::CreataDefaultBuffer(d3dDevice_.Get(),
        commandList_.Get(), 
        vertices.data(),
        vbByteSize,
        boxGeometry_->vertexBufferUploader);

    boxGeometry_->indexBufferGPU = D3DUtil::CreataDefaultBuffer(d3dDevice_.Get(),
        commandList_.Get(), 
        indices.data(),
        ibByteSize,
        boxGeometry_->indexBufferUploader);

    boxGeometry_->vertexByteStride = sizeof(Vertex);
    boxGeometry_->vertexBufferByteSize = vbByteSize;
    boxGeometry_->indexFormat = DXGI_FORMAT_R16_UINT;
    boxGeometry_->indexBufferByteSize = ibByteSize;

    SubmeshGeometry submesh;
    submesh.indexCount = (UINT) indices.size();
    submesh.startIndexLocation = 0;
    submesh.baseVertexLocation = 0;

    boxGeometry_->drawArgs["box"] = submesh;
}

void BoxApp::BuildPSO(void)
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
    ZeroMemory(&psoDesc, sizeof(psoDesc));
    psoDesc.InputLayout = {inputLayout_.data(), (UINT) inputLayout_.size()};
    psoDesc.pRootSignature = rootSignature_.Get();
    psoDesc.VS = 
    {
        reinterpret_cast<BYTE*> (vsByteCode_->GetBufferPointer()),
        vsByteCode_->GetBufferSize()
    };
    psoDesc.PS = 
    {
        reinterpret_cast<BYTE*> (psByteCode_->GetBufferPointer()),
        psByteCode_->GetBufferSize()
    };
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = backBufferFormat_;
    psoDesc.SampleDesc.Count = (msaaState4x_ == true) ? (4) : (1);
    psoDesc.SampleDesc.Quality = (msaaState4x_ == true) ? (msaaQuality4x_ - 1) : (0);
    psoDesc.DSVFormat = depthStencilFormat_;

    ThrowIfFailed(d3dDevice_->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso_)));
}

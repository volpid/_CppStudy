
#include "ch07_frame_resource.h"

#include "ch07_waves.h"

#include "../Common/d3d_app.h"
#include "../Common/geometry_generator.h"
#include "../Common/math_helper.h"
#include "../Common/upload_buffer.h"

#include <array>
#include <memory>
#include <vector>

//----------------------------------------------------------------
// ShapeApp
//----------------------------------------------------------------
enum class RenderLayer : int
{
    Opaque = 0,
    Count,
};

class LandAndWaveApp : public D3DApp
{
public:
    LandAndWaveApp(void) = default;
    virtual ~LandAndWaveApp(void);

    LandAndWaveApp(const LandAndWaveApp& other) = delete;
    LandAndWaveApp& operator=(const LandAndWaveApp& other) = delete;

    virtual bool Initialize(HINSTANCE hInstance) override;

    virtual void OnResize(void) override;
    virtual void Update(const GameTimer& timer) override;
    virtual void Draw(const GameTimer& timer) override;

    virtual void OnMouseDown(WPARAM btnState, int x, int y) override;
    virtual void OnMouseUp(WPARAM btnState, int x, int y) override;
    virtual void OnMouseMove(WPARAM btnState, int x, int y) override;

    void OnKeyboardInput(const GameTimer& timer);
    void UpdateCamera(const GameTimer& timer);
    void UpdateObjectCBuffer(const GameTimer& timer);
    void UpdateMainPassCBuffer(const GameTimer& timer);
    void UpdateWaves(const GameTimer& timer);

private:
    void BuildRootSignature(void);
    void BuildShaderAndInputLayout(void);
    void BuildLandGeometry(void);
    void BuildWavesGeometry(void);
    void BuildRenderItem(void);
    void BuildFrameResource(void);
    void BuildPSO(void);

    void DrawRenderItem(ID3D12GraphicsCommandList* pCmdList, const std::vector<RenderItemCh07*>& renderItems);

    float GetHillHeight(float x, float z) const;
    
private:
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> cbvHeap_ = nullptr;

    std::vector<std::unique_ptr<FrameResourceCh07LandWave>> frameResources_;
    FrameResourceCh07LandWave* pCurFrameResource_ = nullptr;
    int curFrameResourceIndex_ = 0;

    std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> geometries_;
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>> shaders_;
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> psos_;

    std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout_;

    RenderItemCh07* pWavesRenderItem_ = nullptr;
    std::vector<std::unique_ptr<RenderItemCh07>> allRenderItems_;    
    std::unique_ptr<WavesCh07> waves_;

    std::vector<RenderItemCh07*> renderItemLayer_[static_cast<int> (RenderLayer::Count)];

    PassConstantCh07 mainPassCBuffer_;
    UINT passCbvOffset_ = 0;

    DirectX::XMFLOAT3 eyePosition_;
    DirectX::XMFLOAT4X4 view_ = MathHelper::Indentity4x4();
    DirectX::XMFLOAT4X4 proj_ = MathHelper::Indentity4x4();

    bool isWireFrame_ = false;

    float theta_ = 1.5f * MathHelper::Pi;
    float phi_ = 0.2f * MathHelper::Pi;
    float radius_ = 15.0f;

    POINT lastMousePos_;
};

//LandAndWaveApp appInstance;

//----------------------------------------------------------------
LandAndWaveApp::~LandAndWaveApp(void)
{
    if (_d3dDevice != nullptr)
    {
        FlushCommandQueue();
    }
}

bool LandAndWaveApp::Initialize(HINSTANCE hInstance)
{
    if (D3DApp::Initialize(hInstance) == false)
    {
        return false;
    }

    ThrowIfFailed(_commandList->Reset(_commandListAlloc.Get(), nullptr));
    waves_ = std::make_unique<WavesCh07>(128, 128, 1.0f, 0.03f, 3.0f, 0.2f);
    
    BuildRootSignature();
    BuildShaderAndInputLayout();    
    BuildLandGeometry();
    BuildWavesGeometry();
    BuildRenderItem();
    BuildFrameResource();
    BuildPSO();

    ThrowIfFailed(_commandList->Close());
    ID3D12CommandList* cmdLists[] = {_commandList.Get()};
    _commandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

    FlushCommandQueue();

    return true;
}

void LandAndWaveApp::OnResize(void)
{
    D3DApp::OnResize();

    DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi,
        AspectRatio(),
        1.0f,
        1000.0f);
    DirectX::XMStoreFloat4x4(&proj_, proj);
}

void LandAndWaveApp::Update(const GameTimer& timer)
{
    OnKeyboardInput(timer);
    UpdateCamera(timer);

    curFrameResourceIndex_ = (curFrameResourceIndex_ + 1) % NumFrameResourceCh07;
    pCurFrameResource_ = frameResources_[curFrameResourceIndex_].get();

    if (pCurFrameResource_->fence != 0 && _fence->GetCompletedValue() < pCurFrameResource_->fence)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
        ThrowIfFailed(_fence->SetEventOnCompletion(pCurFrameResource_->fence, eventHandle));
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }

    UpdateObjectCBuffer(timer);
    UpdateMainPassCBuffer(timer);
    UpdateWaves(timer);
}

void LandAndWaveApp::Draw(const GameTimer& timer)
{
    _Unreferenced_parameter_(timer);

    auto cmdListAlloc = pCurFrameResource_->cmdlistAlloc;
    ThrowIfFailed(cmdListAlloc->Reset());

    if (isWireFrame_ == true)
    {
        ThrowIfFailed(_commandList->Reset(cmdListAlloc.Get(), psos_["opaque_wireframe"].Get()));
    }
    else
    {
        ThrowIfFailed(_commandList->Reset(cmdListAlloc.Get(), psos_["opaque"].Get()));
    }

    _commandList->RSSetViewports(1, &_screenViewport);
    _commandList->RSSetScissorRects(1, &_scissorRect);

    CD3DX12_RESOURCE_BARRIER barrierPresentToRenderTarget = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    _commandList->ResourceBarrier(1, &barrierPresentToRenderTarget);

    D3D12_CPU_DESCRIPTOR_HANDLE currentBackBufferView = CurrentBackBufferView();
    D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = DepthStencilView();

    _commandList->ClearRenderTargetView(currentBackBufferView, DirectX::Colors::LightSteelBlue, 0, nullptr);
    _commandList->ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    _commandList->OMSetRenderTargets(1, &currentBackBufferView, true, &depthStencilView);
    _commandList->SetGraphicsRootSignature(rootSignature_.Get());

    auto passCBuffer = pCurFrameResource_->passCBuffer->Resource();
    _commandList->SetGraphicsRootConstantBufferView(1, passCBuffer->GetGPUVirtualAddress());
    
    DrawRenderItem(_commandList.Get(), renderItemLayer_[static_cast<int> (RenderLayer::Opaque)]);

    CD3DX12_RESOURCE_BARRIER barrierRenderTargetToPresent = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);
    _commandList->ResourceBarrier(1, &barrierRenderTargetToPresent);
    
    ThrowIfFailed(_commandList->Close());

    ID3D12CommandList* cmdsLists[] = {_commandList.Get()};
    _commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    ThrowIfFailed(_swapChain->Present(0, 0));
    currBackBuffer_ = (currBackBuffer_ + 1) % SwapChainbufferCount;

    pCurFrameResource_->fence = ++_currentFence;
    _commandQueue->Signal(_fence.Get(), _currentFence);
}

void LandAndWaveApp::OnMouseDown(WPARAM btnState, int x, int y)
{
    _Unreferenced_parameter_(btnState);
    lastMousePos_.x = x;
    lastMousePos_.y = y;

    SetCapture(_hMainWnd);
}

void LandAndWaveApp::OnMouseUp(WPARAM btnState, int x, int y)
{   
    _Unreferenced_parameter_(btnState);
    _Unreferenced_parameter_(x);
    _Unreferenced_parameter_(y);

    ReleaseCapture();
}

void LandAndWaveApp::OnMouseMove(WPARAM btnState, int x, int y)
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
        float dx = 0.05f * static_cast<float> (x - lastMousePos_.x);
        float dy = 0.05f * static_cast<float> (y - lastMousePos_.y);

        radius_ += (dx - dy);
        radius_ = MathHelper::Clamp(radius_, 5.0f, 150.0f);
    }

    lastMousePos_.x = x;
    lastMousePos_.y = y;
}

void LandAndWaveApp::OnKeyboardInput(const GameTimer& timer)
{
    _Unreferenced_parameter_(timer);

    if (GetAsyncKeyState('1') & 0x8000)
    {
        isWireFrame_ = true;
    }
    else
    {
        isWireFrame_ = false;
    }
}

void LandAndWaveApp::UpdateCamera(const GameTimer& timer)
{
     _Unreferenced_parameter_(timer);

    eyePosition_.x = radius_ * sinf(phi_) * cosf(theta_);
    eyePosition_.z = radius_ * sinf(phi_) * sinf(theta_);
    eyePosition_.y = radius_ * cosf(phi_);

    DirectX::XMVECTOR pos = DirectX::XMVectorSet(eyePosition_.x, eyePosition_.y, eyePosition_.z, 1.0f);
    DirectX::XMVECTOR target = DirectX::XMVectorZero();
    DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    
    DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(pos, target, up);
    DirectX::XMStoreFloat4x4(&view_, view);
}

void LandAndWaveApp::UpdateObjectCBuffer(const GameTimer& timer)
{
    _Unreferenced_parameter_(timer);

    auto curObjectCBuffer = pCurFrameResource_->objectCBuffer.get();
    for (auto& e : allRenderItems_)
    {
        if (e->numFrameDirty > 0)
        {
            DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&e->world);

            ObjectConstantCh07 objConstant;
            DirectX::XMStoreFloat4x4(&objConstant.world, DirectX::XMMatrixTranspose(world));

            curObjectCBuffer->CopyData(e->objCBufferIndex, objConstant);
            --e->numFrameDirty;
        }
    }
}

void LandAndWaveApp::UpdateMainPassCBuffer(const GameTimer& timer)
{
    _Unreferenced_parameter_(timer);

    DirectX::XMMATRIX view = DirectX::XMLoadFloat4x4(&view_);
    DirectX::XMMATRIX proj = DirectX::XMLoadFloat4x4(&proj_);
    DirectX::XMMATRIX viewProj = DirectX::XMMatrixMultiply(view, proj);

    DirectX::XMVECTOR viewDeterminant = DirectX::XMMatrixDeterminant(view);
    DirectX::XMVECTOR projDeterminant = DirectX::XMMatrixDeterminant(proj);
    DirectX::XMVECTOR viewProjDeterminant = DirectX::XMMatrixDeterminant(viewProj);

    DirectX::XMMATRIX invView = DirectX::XMMatrixInverse(&viewDeterminant, view);
    DirectX::XMMATRIX invProj = DirectX::XMMatrixInverse(&projDeterminant, proj);
    DirectX::XMMATRIX invViewProj = DirectX::XMMatrixInverse(&viewProjDeterminant, viewProj);

    DirectX::XMStoreFloat4x4(&mainPassCBuffer_.view, DirectX::XMMatrixTranspose(view));
    DirectX::XMStoreFloat4x4(&mainPassCBuffer_.viewInv, DirectX::XMMatrixTranspose(invView));
    DirectX::XMStoreFloat4x4(&mainPassCBuffer_.proj, DirectX::XMMatrixTranspose(proj));
    DirectX::XMStoreFloat4x4(&mainPassCBuffer_.projInv, DirectX::XMMatrixTranspose(invProj));
    DirectX::XMStoreFloat4x4(&mainPassCBuffer_.viewProj, DirectX::XMMatrixTranspose(viewProj));
    DirectX::XMStoreFloat4x4(&mainPassCBuffer_.viewProjInv, DirectX::XMMatrixTranspose(invViewProj));

    mainPassCBuffer_.eyePosWorld = eyePosition_;
    mainPassCBuffer_.renderTargetSize = DirectX::XMFLOAT2((float) _clientWidth, (float) _clientHeight);
    mainPassCBuffer_.renderTargetSizeInv = DirectX::XMFLOAT2( 1.0f / _clientWidth, 1.0f / _clientHeight);
    mainPassCBuffer_.zNear = 1.0f;
    mainPassCBuffer_.zFar = 1000.0f;
    mainPassCBuffer_.totalTime = timer.TotalTime();
    mainPassCBuffer_.deltaTime = timer.DeltaTime();

    auto curPassCBuffer = pCurFrameResource_->passCBuffer.get();
    curPassCBuffer->CopyData(0, mainPassCBuffer_);
}

void LandAndWaveApp::UpdateWaves(const GameTimer& timer)
{
    static float base = 0.0f;
    if ((timer.TotalTime() - base) >= 0.25f)
    {
        base += 0.25f;

        int row = MathHelper::Rand(4, waves_->RowCount() - 5);
        int col = MathHelper::Rand(4, waves_->ColumnCount() - 5);
        float mag = MathHelper::RandF(0.2f, 0.5f);

        waves_->Disturb(row, col, mag);
    }

    waves_->Update(timer.DeltaTime());

    auto curWaveVB = pCurFrameResource_->waveVBuffer.get();
    for (int idx = 0; idx < waves_->VertexCount(); ++idx)
    {
        VertexCh07 vertex;
        vertex.pos = waves_->Position(idx);
        vertex.color = DirectX::XMFLOAT4(DirectX::Colors::Blue);

        curWaveVB->CopyData(idx, vertex);
    }

    pWavesRenderItem_->geometry->vertexBufferGPU = curWaveVB->Resource();
}

void LandAndWaveApp::BuildRootSignature(void)
{
    CD3DX12_ROOT_PARAMETER slotRootParameter[2];
    slotRootParameter[0].InitAsConstantBufferView(0);
    slotRootParameter[1].InitAsConstantBufferView(1);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc = 
    {
        2, 
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
    ThrowIfFailed(_d3dDevice->CreateRootSignature(0,
        serializedRootSig->GetBufferPointer(),
        serializedRootSig->GetBufferSize(),
        IID_PPV_ARGS(rootSignature_.GetAddressOf())));
}

void LandAndWaveApp::BuildShaderAndInputLayout(void)
{
    shaders_["standardVS"] = D3DUtil::CompileShader("..\\..\\Source\\IntroductionDirectX12\\Shader\\ch07_color_vs.hlsl", nullptr, "vs_main", "vs_5_0");
    shaders_["standardPS"] = D3DUtil::CompileShader("..\\..\\Source\\IntroductionDirectX12\\Shader\\ch07_color_ps.hlsl", nullptr, "ps_main", "ps_5_0");

    inputLayout_ = 
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };
}

void LandAndWaveApp::BuildLandGeometry(void)
{
    GeometryGenerator geometryGenerator;
    GeometryGenerator::MeshData grid = geometryGenerator.CreateGrid(160.0f, 160.0f, 50, 50);

    std::vector<VertexCh07> vertices(grid.vertices.size());
    for (size_t idx = 0; idx < grid.vertices.size(); ++idx)
    {
        auto& pos = grid.vertices[idx].position;
        vertices[idx].pos = pos;
        vertices[idx].pos.y = GetHillHeight(pos.x, pos.z);

        if (vertices[idx].pos.y < -10.0f)
        {
            vertices[idx].color = DirectX::XMFLOAT4(1.0f, 0.96f, 0.62f, 1.0f);
        }
        else if (vertices[idx].pos.y < 5.0f)
        {
            vertices[idx].color = DirectX::XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
        }
        else if (vertices[idx].pos.y < 12.0f)
        {
            vertices[idx].color = DirectX::XMFLOAT4(0.1f, 0.48f, 0.19f, 1.0f);
        }
        else if (vertices[idx].pos.y < 20.0f)
        {
            vertices[idx].color = DirectX::XMFLOAT4(0.45f, 0.39f, 0.34f, 1.0f);
        }
        else
        {
            vertices[idx].color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        }
    }

    const UINT vbByteSize = (UINT) vertices.size() * sizeof(VertexCh07);

    std::vector<std::uint16_t> indices = grid.GetIndex16();
    const UINT ibByteSize = (UINT) indices.size() * sizeof(std::uint16_t);

    auto geometry = std::make_unique<MeshGeometry>();
    geometry->name = "landGeo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geometry->vertexBufferCPU));
    CopyMemory(geometry->vertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geometry->indexBufferCPU));
    CopyMemory(geometry->indexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geometry->vertexBufferGPU = D3DUtil::CreataDefaultBuffer(_d3dDevice.Get(), 
        _commandList.Get(),
        vertices.data(),
        vbByteSize,
        geometry->vertexBufferUploader);
    
    geometry->indexBufferGPU = D3DUtil::CreataDefaultBuffer(_d3dDevice.Get(), 
        _commandList.Get(),
        indices.data(),
        ibByteSize,
        geometry->indexBufferUploader);

    geometry->vertexByteStride = sizeof(VertexCh07);
    geometry->vertexBufferByteSize = vbByteSize;
    geometry->indexFormat = DXGI_FORMAT_R16_UINT;
    geometry->indexBufferByteSize = ibByteSize;

    SubmeshGeometry submesh;
    submesh.indexCount = static_cast<UINT> (indices.size());
    submesh.startIndexLocation = 0;
    submesh.baseVertexLocation = 0;

    geometry->drawArgs["grid"] = submesh;
    geometries_[geometry->name] = std::move(geometry);    
}

void LandAndWaveApp::BuildWavesGeometry(void)
{
    std::vector<std::uint16_t> indices(3 * waves_->TriangleCount());
    assert(waves_->VertexCount() < 0x0000ffff);

    int row = waves_->RowCount();
    int col = waves_->ColumnCount();

    int k = 0;
    for (int rowidx = 0; rowidx < row - 1; ++rowidx)
    {
        for (int colidx = 0; colidx < col - 1; ++colidx)
        {
            indices[k] = static_cast<uint16_t>(rowidx * col + colidx);
            indices[k + 1] = static_cast<uint16_t>(rowidx * col + colidx + 1);
            indices[k + 2] = static_cast<uint16_t>((rowidx + 1) * col + colidx);

            indices[k + 3] = static_cast<uint16_t>((rowidx + 1) * col + colidx);
            indices[k + 4] = static_cast<uint16_t>(rowidx * col + colidx + 1);
            indices[k + 5] = static_cast<uint16_t>((rowidx + 1) * col + colidx + 1);
            
            k += 6;
        }
    }

    UINT vbByteSize = waves_->VertexCount() * sizeof(VertexCh07);
    UINT ibByteSize = static_cast<UINT> (indices.size() * sizeof(std::uint16_t));

    auto geometry = std::make_unique<MeshGeometry>();
    geometry->name = "waterGeo";

    geometry->vertexBufferCPU = nullptr;
    geometry->vertexBufferGPU = nullptr;
    
    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geometry->indexBufferCPU));
    CopyMemory(geometry->indexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geometry->indexBufferGPU = D3DUtil::CreataDefaultBuffer(_d3dDevice.Get(), 
        _commandList.Get(),
        indices.data(),
        ibByteSize,
        geometry->indexBufferUploader);

    geometry->vertexByteStride = sizeof(VertexCh07);
    geometry->vertexBufferByteSize = vbByteSize;
    geometry->indexFormat = DXGI_FORMAT_R16_UINT;
    geometry->indexBufferByteSize = ibByteSize;

    SubmeshGeometry submesh;
    submesh.indexCount = static_cast<UINT> (indices.size());
    submesh.startIndexLocation = 0;
    submesh.baseVertexLocation = 0;

    geometry->drawArgs["grid"] = submesh;
    geometries_[geometry->name] = std::move(geometry);    
}

void LandAndWaveApp::BuildRenderItem(void)
{
    auto waveRenderItem = std::make_unique<RenderItemCh07>();
    waveRenderItem->world = MathHelper::Indentity4x4();
    waveRenderItem->objCBufferIndex = 0;
    waveRenderItem->geometry = geometries_["waterGeo"].get();
    waveRenderItem->primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    waveRenderItem->indexCount = waveRenderItem->geometry->drawArgs["grid"].indexCount;
    waveRenderItem->startIndexLocation = waveRenderItem->geometry->drawArgs["grid"].startIndexLocation;
    waveRenderItem->baseVertexLocation = waveRenderItem->geometry->drawArgs["grid"].baseVertexLocation;

    pWavesRenderItem_ = waveRenderItem.get();
    renderItemLayer_[static_cast<int> (RenderLayer::Opaque)].push_back(waveRenderItem.get());

    auto gridRenderItem = std::make_unique<RenderItemCh07>();
    gridRenderItem->world = MathHelper::Indentity4x4();
    gridRenderItem->objCBufferIndex = 1;
    gridRenderItem->geometry = geometries_["landGeo"].get();
    gridRenderItem->primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    gridRenderItem->indexCount = gridRenderItem->geometry->drawArgs["grid"].indexCount;
    gridRenderItem->startIndexLocation = gridRenderItem->geometry->drawArgs["grid"].startIndexLocation;
    gridRenderItem->baseVertexLocation = gridRenderItem->geometry->drawArgs["grid"].baseVertexLocation;

    renderItemLayer_[static_cast<int> (RenderLayer::Opaque)].push_back(gridRenderItem.get());
    
    allRenderItems_.push_back(std::move(waveRenderItem));
    allRenderItems_.push_back(std::move(gridRenderItem));
}

void LandAndWaveApp::BuildFrameResource(void)
{
    for (int idx = 0; idx < NumFrameResourceCh07; ++idx)
    {
        frameResources_.push_back(
            std::make_unique<FrameResourceCh07LandWave> (_d3dDevice.Get(), 
                1, 
                static_cast<UINT> (allRenderItems_.size()), 
                waves_->VertexCount()));
    }
}

void LandAndWaveApp::BuildPSO(void)
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;
    ZeroMemory(&opaquePsoDesc, sizeof(opaquePsoDesc));
    opaquePsoDesc.InputLayout = {inputLayout_.data(), static_cast<UINT> (inputLayout_.size())};
    opaquePsoDesc.pRootSignature = rootSignature_.Get();
    opaquePsoDesc.VS = 
    {
        reinterpret_cast<BYTE*> (shaders_["standardVS"]->GetBufferPointer()),
        shaders_["standardVS"]->GetBufferSize()
    };
    opaquePsoDesc.PS = 
    {
        reinterpret_cast<BYTE*> (shaders_["standardPS"]->GetBufferPointer()),
        shaders_["standardPS"]->GetBufferSize()
    };
    opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    opaquePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;    
    opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    opaquePsoDesc.SampleMask = UINT_MAX;
    opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    opaquePsoDesc.NumRenderTargets = 1;
    opaquePsoDesc.RTVFormats[0] = _backBufferFormat;
    opaquePsoDesc.SampleDesc.Count = (_msaaState4x == true) ? (4) : (1);
    opaquePsoDesc.SampleDesc.Quality = (_msaaState4x == true) ? (_msaaQuality4x - 1) : (0);
    opaquePsoDesc.DSVFormat = _depthStencilFormat;

    ThrowIfFailed(_d3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&psos_["opaque"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC opaqueWireframePsoDesc = opaquePsoDesc;
    opaqueWireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
    ThrowIfFailed(_d3dDevice->CreateGraphicsPipelineState(&opaqueWireframePsoDesc, IID_PPV_ARGS(&psos_["opaque_wireframe"])));
}

void LandAndWaveApp::DrawRenderItem(ID3D12GraphicsCommandList* pCmdList, const std::vector<RenderItemCh07*>& renderItems)
{
    UINT objCBufferByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstantCh07));
    auto objectCBuffer = pCurFrameResource_->objectCBuffer->Resource();
    for (size_t idx = 0; idx < renderItems.size(); ++idx)
    {
        auto renderItem = renderItems[idx];

        D3D12_VERTEX_BUFFER_VIEW vertexBufferView = renderItem->geometry->VertexBufferView();
        D3D12_INDEX_BUFFER_VIEW indexBufferView = renderItem->geometry->IndexBufferView();

        pCmdList->IASetVertexBuffers(0, 1, &vertexBufferView);
        pCmdList->IASetIndexBuffer(&indexBufferView);
        pCmdList->IASetPrimitiveTopology(renderItem->primitiveType);

        D3D12_GPU_VIRTUAL_ADDRESS objCBufferAddress = objectCBuffer->GetGPUVirtualAddress();
        objCBufferAddress += renderItem->objCBufferIndex * objCBufferByteSize;

        pCmdList->SetGraphicsRootConstantBufferView(0, objCBufferAddress);        
        pCmdList->DrawIndexedInstanced(renderItem->indexCount, 
            1, 
            renderItem->startIndexLocation,
            renderItem->baseVertexLocation,
            0);
    }
}

float LandAndWaveApp::GetHillHeight(float x, float z) const
{
    return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
}
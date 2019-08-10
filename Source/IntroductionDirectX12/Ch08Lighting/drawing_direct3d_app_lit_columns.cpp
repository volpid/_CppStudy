
#include "ch08_frame_resource.h"

#include "../Common/d3d_app.h"
#include "../Common/geometry_generator.h"
#include "../Common/math_helper.h"
#include "../Common/upload_buffer.h"

#include <array>
#include <memory>
#include <vector>

//----------------------------------------------------------------
// LitColumnsApp
//----------------------------------------------------------------
class LitColumnsApp : public D3DApp
{
public:
    LitColumnsApp(void);
    virtual ~LitColumnsApp(void);

    LitColumnsApp(const LitColumnsApp& other) = delete;
    LitColumnsApp& operator=(const LitColumnsApp& other) = delete;

    virtual bool Initialize(HINSTANCE hInstance) override;

//    virtual void OnResize(void) override;
//    virtual void Update(const GameTimer& timer) override;
//    virtual void Draw(const GameTimer& timer) override;
//
//    virtual void OnMouseDown(WPARAM btnState, int x, int y) override;
//    virtual void OnMouseUp(WPARAM btnState, int x, int y) override;
//    virtual void OnMouseMove(WPARAM btnState, int x, int y) override;
//
//    void OnKeyboardInput(const GameTimer& timer);
//    void UpdateCamera(const GameTimer& timer);
//    void UpdateObjectCBuffer(const GameTimer& timer);
//    void UpdateMainPassCBuffer(const GameTimer& timer);
//
private:
    void _BuildRootSignature(void);
    void _BuildShaderAndInputLayout(void);
//    void BuildDescriptorHeap(void);
//    void BuildConstantBufferView(void);
    
    
//    void BuildShapeGeometry(void);
//    void BuildRenderItem(void);
//    void BuildFrameResource(void);
//    void BuildPSO(void);
//
//    void DrawRenderItem(ID3D12GraphicsCommandList* pCmdList, const std::vector<RenderItemCh07*>& renderItems);
//    
private:
    Microsoft::WRL::ComPtr<ID3D12RootSignature> _rootSignature = nullptr;
    //Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _cbvHeap = nullptr;

    //std::vector<std::unique_ptr<FrameResourceCh07Shape>> frameResources_;
    //FrameResourceCh07Shape* pCurFrameResource_ = nullptr;
    //int curFrameResourceIndex_ = 0;

    //std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> geometries_;
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>> _shaders;
    //std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> psos_;

    std::vector<D3D12_INPUT_ELEMENT_DESC> _inputLayout;

    //std::vector<std::unique_ptr<RenderItemCh07>> allRenderItems_;
    //std::vector<RenderItemCh07*> opaqueRenderItems_;

    //PassConstantCh07 mainPassCBuffer_;
    //UINT passCbvOffset_ = 0;

    //DirectX::XMFLOAT3 eyePosition_;
    //DirectX::XMFLOAT4X4 view_ = MathHelper::Indentity4x4();
    //DirectX::XMFLOAT4X4 proj_ = MathHelper::Indentity4x4();

    //bool isWireFrame_ = false;

    //float theta_ = 1.5f * MathHelper::Pi;
    //float phi_ = 0.2f * MathHelper::Pi;
    //float radius_ = 15.0f;

    //POINT lastMousePos_;
};

LitColumnsApp appInstance;

//----------------------------------------------------------------
LitColumnsApp::LitColumnsApp(void)
    : D3DApp()
{
}

LitColumnsApp::~LitColumnsApp(void)
{
}

bool LitColumnsApp::Initialize(HINSTANCE hInstance)
{
    if (D3DApp::Initialize(hInstance) == false)
    {
        return false;
    }

    ThrowIfFailed(_commandList->Reset(_commandListAlloc.Get(), nullptr));

    _cbvSrvDescriptorSize = _d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    _BuildRootSignature();
    _BuildShaderAndInputLayout();
    //BuildShapeGeometry();
    //BuildRenderItem();
    //BuildFrameResource();
    //BuildDescriptorHeap();
    //BuildConstantBufferView();
    //BuildPSO();

    //ThrowIfFailed(commandList_->Close());
    //ID3D12CommandList* cmdLists[] = {commandList_.Get()};
    //commandQueue_->ExecuteCommandLists(_countof(cmdLists), cmdLists);

    //FlushCommandQueue();

    return true;
}

//
//void LitColumnsApp::OnResize(void)
//{
//    D3DApp::OnResize();
//
//    DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi,
//        AspectRatio(),
//        1.0f,
//        1000.0f);
//    DirectX::XMStoreFloat4x4(&proj_, proj);
//}
//
//void LitColumnsApp::Update(const GameTimer& timer)
//{
//    OnKeyboardInput(timer);
//    UpdateCamera(timer);
//
//    curFrameResourceIndex_ = (curFrameResourceIndex_ + 1) % NumFrameResourceCh07;
//    pCurFrameResource_ = frameResources_[curFrameResourceIndex_].get();
//
//    if (pCurFrameResource_->fence != 0 && fence_->GetCompletedValue() < pCurFrameResource_->fence)
//    {
//        HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
//        ThrowIfFailed(fence_->SetEventOnCompletion(pCurFrameResource_->fence, eventHandle));
//        WaitForSingleObject(eventHandle, INFINITE);
//        CloseHandle(eventHandle);
//    }
//
//    UpdateObjectCBuffer(timer);
//    UpdateMainPassCBuffer(timer);
//}
//
//void LitColumnsApp::Draw(const GameTimer& timer)
//{
//    _Unreferenced_parameter_(timer);
//
//    auto cmdListAlloc = pCurFrameResource_->cmdlistAlloc;
//    ThrowIfFailed(cmdListAlloc->Reset());
//
//    if (isWireFrame_ == true)
//    {
//        ThrowIfFailed(commandList_->Reset(cmdListAlloc.Get(), psos_["opaque_wireframe"].Get()));
//    }
//    else
//    {
//        ThrowIfFailed(commandList_->Reset(cmdListAlloc.Get(), psos_["opaque"].Get()));
//    }
//
//    commandList_->RSSetViewports(1, &screenViewport_);
//    commandList_->RSSetScissorRects(1, &scissorRect_);
//
//    CD3DX12_RESOURCE_BARRIER barrierPresentToRenderTarget = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
//        D3D12_RESOURCE_STATE_PRESENT,
//        D3D12_RESOURCE_STATE_RENDER_TARGET);
//    commandList_->ResourceBarrier(1, &barrierPresentToRenderTarget);
//
//    D3D12_CPU_DESCRIPTOR_HANDLE currentBackBufferView = CurrentBackBufferView();
//    D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = DepthStencilView();
//
//    commandList_->ClearRenderTargetView(currentBackBufferView, DirectX::Colors::LightSteelBlue, 0, nullptr);
//    commandList_->ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
//
//    commandList_->OMSetRenderTargets(1, &currentBackBufferView, true, &depthStencilView);
//
//    ID3D12DescriptorHeap* descriptorHeap[] = {cbvHeap_.Get()};
//    commandList_->SetDescriptorHeaps(_countof(descriptorHeap), descriptorHeap);
//
//    commandList_->SetGraphicsRootSignature(rootSignature_.Get());
//    
//    int passCbvIndex = passCbvOffset_ + curFrameResourceIndex_;
//    auto passCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(cbvHeap_->GetGPUDescriptorHandleForHeapStart());
//    passCbvHandle.Offset(passCbvIndex, cbvSrvDescriptorSize_);
//    commandList_->SetGraphicsRootDescriptorTable(1, passCbvHandle);
//
//    DrawRenderItem(commandList_.Get(), opaqueRenderItems_);
//
//    CD3DX12_RESOURCE_BARRIER barrierRenderTargetToPresent = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
//        D3D12_RESOURCE_STATE_RENDER_TARGET,
//        D3D12_RESOURCE_STATE_PRESENT);
//    commandList_->ResourceBarrier(1, &barrierRenderTargetToPresent);
//    
//    ThrowIfFailed(commandList_->Close());
//
//    ID3D12CommandList* cmdsLists[] = {commandList_.Get()};
//    commandQueue_->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
//
//    ThrowIfFailed(swapChain_->Present(0, 0));
//    currBackBuffer_ = (currBackBuffer_ + 1) % swapChainbufferCount;
//
//    pCurFrameResource_->fence = ++currentFence_;
//    commandQueue_->Signal(fence_.Get(), currentFence_);
//}
//
//void LitColumnsApp::OnMouseDown(WPARAM btnState, int x, int y)
//{
//    _Unreferenced_parameter_(btnState);
//    lastMousePos_.x = x;
//    lastMousePos_.y = y;
//
//    SetCapture(hMainWnd_);
//}
//
//void LitColumnsApp::OnMouseUp(WPARAM btnState, int x, int y)
//{   
//    _Unreferenced_parameter_(btnState);
//    _Unreferenced_parameter_(x);
//    _Unreferenced_parameter_(y);
//
//    ReleaseCapture();
//}
//
//void LitColumnsApp::OnMouseMove(WPARAM btnState, int x, int y)
//{
//    if ((btnState & MK_LBUTTON) == MK_LBUTTON)
//    {
//        float dx = DirectX::XMConvertToRadians(0.25f * static_cast<float> (x - lastMousePos_.x));
//        float dy = DirectX::XMConvertToRadians(0.25f * static_cast<float> (y - lastMousePos_.y));
//
//        theta_ += dx;
//        phi_ += dy;
//
//        phi_ = MathHelper::Clamp(phi_, 0.1f, MathHelper::Pi - 0.1f);        
//    }
//    else if ((btnState & MK_RBUTTON) == MK_RBUTTON)
//    {
//        float dx = 0.005f * static_cast<float> (x - lastMousePos_.x);
//        float dy = 0.005f * static_cast<float> (y - lastMousePos_.y);
//
//        radius_ += (dx - dy);
//        radius_ = MathHelper::Clamp(radius_, 5.0f, 150.0f);
//    }
//
//    lastMousePos_.x = x;
//    lastMousePos_.y = y;
//}
//
//void LitColumnsApp::OnKeyboardInput(const GameTimer& timer)
//{
//    _Unreferenced_parameter_(timer);
//
//    if (GetAsyncKeyState('1') & 0x8000)
//    {
//        isWireFrame_ = true;
//    }
//    else
//    {
//        isWireFrame_ = false;
//    }
//}
//
//void LitColumnsApp::UpdateCamera(const GameTimer& timer)
//{
//     _Unreferenced_parameter_(timer);
//
//    eyePosition_.x = radius_ * sinf(phi_) * cosf(theta_);
//    eyePosition_.z = radius_ * sinf(phi_) * sinf(theta_);
//    eyePosition_.y = radius_ * cosf(phi_);
//
//    DirectX::XMVECTOR pos = DirectX::XMVectorSet(eyePosition_.x, eyePosition_.y, eyePosition_.z, 1.0f);
//    DirectX::XMVECTOR target = DirectX::XMVectorZero();
//    DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
//    
//    DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(pos, target, up);
//    DirectX::XMStoreFloat4x4(&view_, view);
//}
//
//void LitColumnsApp::UpdateObjectCBuffer(const GameTimer& timer)
//{
//    _Unreferenced_parameter_(timer);
//
//    auto curObjectCBuffer = pCurFrameResource_->objectCBuffer.get();
//    for (auto& e : allRenderItems_)
//    {
//        if (e->numFrameDirty > 0)
//        {
//            DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&e->world);
//
//            ObjectConstantCh07 objConstant;
//            DirectX::XMStoreFloat4x4(&objConstant.world, DirectX::XMMatrixTranspose(world));
//
//            curObjectCBuffer->CopyData(e->objCBufferIndex, objConstant);
//            --e->numFrameDirty;
//        }
//    }
//}
//
//void LitColumnsApp::UpdateMainPassCBuffer(const GameTimer& timer)
//{
//    _Unreferenced_parameter_(timer);
//
//    DirectX::XMMATRIX view = DirectX::XMLoadFloat4x4(&view_);
//    DirectX::XMMATRIX proj = DirectX::XMLoadFloat4x4(&proj_);
//    DirectX::XMMATRIX viewProj = DirectX::XMMatrixMultiply(view, proj);
//
//    DirectX::XMVECTOR viewDeterminant = DirectX::XMMatrixDeterminant(view);
//    DirectX::XMVECTOR projDeterminant = DirectX::XMMatrixDeterminant(proj);
//    DirectX::XMVECTOR viewProjDeterminant = DirectX::XMMatrixDeterminant(viewProj);
//
//    DirectX::XMMATRIX invView = DirectX::XMMatrixInverse(&viewDeterminant, view);
//    DirectX::XMMATRIX invProj = DirectX::XMMatrixInverse(&projDeterminant, proj);
//    DirectX::XMMATRIX invViewProj = DirectX::XMMatrixInverse(&viewProjDeterminant, viewProj);
//
//    DirectX::XMStoreFloat4x4(&mainPassCBuffer_.view, DirectX::XMMatrixTranspose(view));
//    DirectX::XMStoreFloat4x4(&mainPassCBuffer_.viewInv, DirectX::XMMatrixTranspose(invView));
//    DirectX::XMStoreFloat4x4(&mainPassCBuffer_.proj, DirectX::XMMatrixTranspose(proj));
//    DirectX::XMStoreFloat4x4(&mainPassCBuffer_.projInv, DirectX::XMMatrixTranspose(invProj));
//    DirectX::XMStoreFloat4x4(&mainPassCBuffer_.viewProj, DirectX::XMMatrixTranspose(viewProj));
//    DirectX::XMStoreFloat4x4(&mainPassCBuffer_.viewProjInv, DirectX::XMMatrixTranspose(invViewProj));
//
//    mainPassCBuffer_.eyePosWorld = eyePosition_;
//    mainPassCBuffer_.renderTargetSize = DirectX::XMFLOAT2((float) clientWidth_, (float) clientHeight_);
//    mainPassCBuffer_.renderTargetSizeInv = DirectX::XMFLOAT2( 1.0f / clientWidth_, 1.0f / clientHeight_);
//    mainPassCBuffer_.zNear = 1.0f;
//    mainPassCBuffer_.zFar = 1000.0f;
//    mainPassCBuffer_.totalTime = timer.TotalTime();
//    mainPassCBuffer_.deltaTime = timer.DeltaTime();
//
//    auto curPassCBuffer = pCurFrameResource_->passCBuffer.get();
//    curPassCBuffer->CopyData(0, mainPassCBuffer_);
//}

void LitColumnsApp::_BuildRootSignature(void)
{
    CD3DX12_ROOT_PARAMETER slotRootParameter[3];
    slotRootParameter[0].InitAsConstantBufferView(0);
    slotRootParameter[1].InitAsConstantBufferView(1);
    slotRootParameter[2].InitAsConstantBufferView(2);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc = 
    {
        3, 
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
        IID_PPV_ARGS(_rootSignature.GetAddressOf())));
}


//
//void LitColumnsApp::BuildDescriptorHeap(void)
//{
//    UINT objCount = static_cast<UINT> (opaqueRenderItems_.size());
//    UINT numDescriptor = (objCount + 1) * NumFrameResourceCh07; 
//
//    passCbvOffset_ = objCount * NumFrameResourceCh07;
//
//    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
//    cbvHeapDesc.NumDescriptors = numDescriptor;
//    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
//    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
//    cbvHeapDesc.NodeMask = 0;
//    ThrowIfFailed(d3dDevice_->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&cbvHeap_)));
//}
//
//void LitColumnsApp::BuildConstantBufferView(void)
//{
//    UINT objCBufferByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstantCh07));
//    UINT objCount = static_cast<UINT> (opaqueRenderItems_.size());
//
//    for (int frameIndex = 0; frameIndex < NumFrameResourceCh07; ++frameIndex)
//    {
//        auto objectCBuffer = frameResources_[frameIndex]->objectCBuffer->Resource();
//        for (UINT idx = 0; idx < objCount; ++idx)
//        {
//            D3D12_GPU_VIRTUAL_ADDRESS cbufferAddress = objectCBuffer->GetGPUVirtualAddress();
//            cbufferAddress += (idx * objCBufferByteSize);
//
//            int heapIndex = frameIndex * objCount + idx;
//            auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(cbvHeap_->GetCPUDescriptorHandleForHeapStart());
//            handle.Offset(heapIndex, cbvSrvDescriptorSize_);
//
//            D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
//            cbvDesc.BufferLocation = cbufferAddress;
//            cbvDesc.SizeInBytes = objCBufferByteSize;
//
//            d3dDevice_->CreateConstantBufferView(&cbvDesc, handle);
//        }
//    }
//
//    UINT passCBufferByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(PassConstantCh07));    
//    for (int frameIndex = 0; frameIndex < NumFrameResourceCh07; ++frameIndex)
//    {
//        auto passCBuffer = frameResources_[frameIndex]->passCBuffer->Resource();
//        D3D12_GPU_VIRTUAL_ADDRESS cbufferAddress = passCBuffer->GetGPUVirtualAddress();
//
//        int heapIndex = passCbvOffset_ + frameIndex;
//        auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(cbvHeap_->GetCPUDescriptorHandleForHeapStart());
//        handle.Offset(heapIndex, cbvSrvDescriptorSize_);
//
//        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
//        cbvDesc.BufferLocation = cbufferAddress;
//        cbvDesc.SizeInBytes = passCBufferByteSize;
//
//        d3dDevice_->CreateConstantBufferView(&cbvDesc, handle);
//    }
//}

void LitColumnsApp::_BuildShaderAndInputLayout(void)
{
    _shaders["standardVS"] = D3DUtil::CompileShader("..\\..\\Source\\IntroductionDirectX12\\Shader\\ch08_default_vs.hlsl", nullptr, "vs_main", "vs_5_0");
    _shaders["standardPS"] = D3DUtil::CompileShader("..\\..\\Source\\IntroductionDirectX12\\Shader\\ch08_default_ps.hlsl", nullptr, "ps_main", "ps_5_0");

    _inputLayout = 
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };
}
//
//void LitColumnsApp::BuildShapeGeometry(void)
//{
//    GeometryGenerator geometryGenerator;
//    GeometryGenerator::MeshData box = geometryGenerator.CreateBox(1.5f, 0.5f, 1.5f, 3);
//    GeometryGenerator::MeshData grid = geometryGenerator.CreateGrid(20.0f, 30.0f, 60, 40);
//    GeometryGenerator::MeshData sphere = geometryGenerator.CreateSphere(0.5f, 20, 20);
//    GeometryGenerator::MeshData cylinder = geometryGenerator.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20);
//
//    UINT boxVertexOffset = 0;
//    UINT gridVertexOffset = static_cast<UINT> (box.vertices.size());
//    UINT sphereVertexOffset = gridVertexOffset + static_cast<UINT> (grid.vertices.size());
//    UINT cylinderVertexOffset = sphereVertexOffset + static_cast<UINT> (sphere.vertices.size());
//
//    UINT boxIndexOffset = 0;
//    UINT gridIndexOffset = static_cast<UINT> (box.indices32.size());
//    UINT sphereIndexOffset = gridIndexOffset + static_cast<UINT> (grid.indices32.size());
//    UINT cylinderIndexOffset = sphereIndexOffset + static_cast<UINT> (sphere.indices32.size());
//
//    SubmeshGeometry boxSubmesh;
//    boxSubmesh.indexCount = static_cast<UINT> (box.indices32.size());
//    boxSubmesh.startIndexLocation = boxIndexOffset;
//    boxSubmesh.baseVertexLocation = boxVertexOffset;
//    
//    SubmeshGeometry gridSubmesh;
//    gridSubmesh.indexCount = static_cast<UINT> (grid.indices32.size());
//    gridSubmesh.startIndexLocation = gridIndexOffset;
//    gridSubmesh.baseVertexLocation = gridVertexOffset;
//
//    SubmeshGeometry sphereSubmesh;
//    sphereSubmesh.indexCount = static_cast<UINT> (sphere.indices32.size());
//    sphereSubmesh.startIndexLocation = sphereIndexOffset;
//    sphereSubmesh.baseVertexLocation = sphereVertexOffset;
//
//    SubmeshGeometry cylinderSubmesh;
//    cylinderSubmesh.indexCount = static_cast<UINT> (cylinder.indices32.size());
//    cylinderSubmesh.startIndexLocation = cylinderIndexOffset;
//    cylinderSubmesh.baseVertexLocation = cylinderVertexOffset;
//
//    auto totalVertexCount = box.vertices.size()
//        + grid.vertices.size()
//        + sphere.vertices.size()
//        + cylinder.vertices.size();
//
//    std::vector<VertexCh07> vertices(totalVertexCount);
//
//    UINT vertexIndex = 0;
//    for (size_t idx = 0; idx < box.vertices.size(); ++idx)
//    {
//        vertices[vertexIndex].pos = box.vertices[idx].position;
//        vertices[vertexIndex].color = DirectX::XMFLOAT4(DirectX::Colors::DarkGreen);
//        ++vertexIndex;
//    }
//
//    for (size_t idx = 0; idx < grid.vertices.size(); ++idx)
//    {
//        vertices[vertexIndex].pos = grid.vertices[idx].position;
//        vertices[vertexIndex].color = DirectX::XMFLOAT4(DirectX::Colors::ForestGreen);
//        ++vertexIndex;
//    }
//
//    for (size_t idx = 0; idx < sphere.vertices.size(); ++idx)
//    {
//        vertices[vertexIndex].pos = sphere.vertices[idx].position;
//        vertices[vertexIndex].color = DirectX::XMFLOAT4(DirectX::Colors::Crimson);
//        ++vertexIndex;
//    }
//
//    for (size_t idx = 0; idx < cylinder.vertices.size(); ++idx)
//    {
//        vertices[vertexIndex].pos = cylinder.vertices[idx].position;
//        vertices[vertexIndex].color = DirectX::XMFLOAT4(DirectX::Colors::SteelBlue);
//        ++vertexIndex;
//    }
//
//    std::vector<std::uint16_t> indices;
//    indices.insert(indices.end(), std::begin(box.GetIndex16()), std::end(box.GetIndex16()));
//    indices.insert(indices.end(), std::begin(grid.GetIndex16()), std::end(grid.GetIndex16()));
//    indices.insert(indices.end(), std::begin(sphere.GetIndex16()), std::end(sphere.GetIndex16()));
//    indices.insert(indices.end(), std::begin(cylinder.GetIndex16()), std::end(cylinder.GetIndex16()));
//
//    const UINT vbByteSize = static_cast<UINT> (vertices.size()) * sizeof(VertexCh07);
//    const UINT ibByteSize = static_cast<UINT> (indices.size()) * sizeof(std::uint16_t);
//
//    auto geometry = std::make_unique<MeshGeometry>();
//    geometry->name = "shapeGeometry";
//
//    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geometry->vertexBufferCPU));
//    CopyMemory(geometry->vertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);
//
//    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geometry->indexBufferCPU));
//    CopyMemory(geometry->indexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);
//
//    geometry->vertexBufferGPU = D3DUtil::CreataDefaultBuffer(d3dDevice_.Get(), 
//        commandList_.Get(),
//        vertices.data(),
//        vbByteSize,
//        geometry->vertexBufferUploader);
//    
//    geometry->indexBufferGPU = D3DUtil::CreataDefaultBuffer(d3dDevice_.Get(), 
//        commandList_.Get(),
//        indices.data(),
//        ibByteSize,
//        geometry->indexBufferUploader);
//
//    geometry->vertexByteStride = sizeof(VertexCh07);
//    geometry->vertexBufferByteSize = vbByteSize;
//    geometry->indexFormat = DXGI_FORMAT_R16_UINT;
//    geometry->indexBufferByteSize = ibByteSize;
//
//    geometry->drawArgs["box"] = boxSubmesh;
//    geometry->drawArgs["grid"] = gridSubmesh;
//    geometry->drawArgs["sphere"] = sphereSubmesh;
//    geometry->drawArgs["cylinder"] = cylinderSubmesh;
//
//    geometries_[geometry->name] = std::move(geometry);    
//}
//
//void LitColumnsApp::BuildRenderItem(void)
//{
//    auto boxRenderItem = std::make_unique<RenderItemCh07>();
//    DirectX::XMStoreFloat4x4(&boxRenderItem->world, 
//        DirectX::XMMatrixScaling(2.0f, 2.0f, 2.0f) * DirectX::XMMatrixTranslation(0.0f, 0.5f, 0.0f));
//    boxRenderItem->objCBufferIndex = 0;
//    boxRenderItem->geometry = geometries_["shapeGeometry"].get();
//    boxRenderItem->primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
//    boxRenderItem->indexCount = boxRenderItem->geometry->drawArgs["box"].indexCount;
//    boxRenderItem->startIndexLocation = boxRenderItem->geometry->drawArgs["box"].startIndexLocation;
//    boxRenderItem->baseVertexLocation = boxRenderItem->geometry->drawArgs["box"].baseVertexLocation;
//    allRenderItems_.push_back(std::move(boxRenderItem));
//
//    auto gridRenderItem = std::make_unique<RenderItemCh07>();
//    gridRenderItem->world = MathHelper::Indentity4x4();
//    gridRenderItem->objCBufferIndex = 1;
//    gridRenderItem->geometry = geometries_["shapeGeometry"].get();
//    gridRenderItem->primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
//    gridRenderItem->indexCount = gridRenderItem->geometry->drawArgs["grid"].indexCount;
//    gridRenderItem->startIndexLocation = gridRenderItem->geometry->drawArgs["grid"].startIndexLocation;
//    gridRenderItem->baseVertexLocation = gridRenderItem->geometry->drawArgs["grid"].baseVertexLocation;
//    allRenderItems_.push_back(std::move(gridRenderItem));
//
//    UINT objCBufferIndex = 2;
//    for (int idx = 0; idx < 5; ++idx)
//    {
//        auto leftCylinderItem = std::make_unique<RenderItemCh07>();
//        auto rightCylinderItem = std::make_unique<RenderItemCh07>();
//        auto leftSphereItem = std::make_unique<RenderItemCh07>();
//        auto rightSphereItem = std::make_unique<RenderItemCh07>();
//
//        DirectX::XMMATRIX leftCylinderWorld = DirectX::XMMatrixTranslation(-5.0f, 1.5f, -10.0f + idx * 5.0f);
//        DirectX::XMMATRIX rightCylinderWorld = DirectX::XMMatrixTranslation(+5.0f, 1.5f, -10.0f + idx * 5.0f);
//        DirectX::XMMATRIX leftSphereWorld = DirectX::XMMatrixTranslation(-5.0f, 3.5f, -10.0f + idx * 5.0f);
//        DirectX::XMMATRIX rightSphereWorld = DirectX::XMMatrixTranslation(+5.0f, 3.5f, -10.0f + idx * 5.0f);
//
//        DirectX::XMStoreFloat4x4(&leftCylinderItem->world, leftCylinderWorld);
//        leftCylinderItem->objCBufferIndex = objCBufferIndex++;
//        leftCylinderItem->geometry = geometries_["shapeGeometry"].get();
//        leftCylinderItem->primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
//        leftCylinderItem->indexCount = leftCylinderItem->geometry->drawArgs["cylinder"].indexCount;
//        leftCylinderItem->startIndexLocation = leftCylinderItem->geometry->drawArgs["cylinder"].startIndexLocation;
//        leftCylinderItem->baseVertexLocation = leftCylinderItem->geometry->drawArgs["cylinder"].baseVertexLocation;
//
//        DirectX::XMStoreFloat4x4(&rightCylinderItem->world, rightCylinderWorld);
//        rightCylinderItem->objCBufferIndex = objCBufferIndex++;
//        rightCylinderItem->geometry = geometries_["shapeGeometry"].get();
//        rightCylinderItem->primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
//        rightCylinderItem->indexCount = rightCylinderItem->geometry->drawArgs["cylinder"].indexCount;
//        rightCylinderItem->startIndexLocation = rightCylinderItem->geometry->drawArgs["cylinder"].startIndexLocation;
//        rightCylinderItem->baseVertexLocation = rightCylinderItem->geometry->drawArgs["cylinder"].baseVertexLocation;
//
//        DirectX::XMStoreFloat4x4(&leftSphereItem->world, leftSphereWorld);
//        leftSphereItem->objCBufferIndex = objCBufferIndex++;
//        leftSphereItem->geometry = geometries_["shapeGeometry"].get();
//        leftSphereItem->primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
//        leftSphereItem->indexCount = leftSphereItem->geometry->drawArgs["sphere"].indexCount;
//        leftSphereItem->startIndexLocation = leftSphereItem->geometry->drawArgs["sphere"].startIndexLocation;
//        leftSphereItem->baseVertexLocation = leftSphereItem->geometry->drawArgs["sphere"].baseVertexLocation;
//
//        DirectX::XMStoreFloat4x4(&rightSphereItem->world, rightSphereWorld);
//        rightSphereItem->objCBufferIndex = objCBufferIndex++;
//        rightSphereItem->geometry = geometries_["shapeGeometry"].get();
//        rightSphereItem->primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
//        rightSphereItem->indexCount = rightSphereItem->geometry->drawArgs["sphere"].indexCount;
//        rightSphereItem->startIndexLocation = rightSphereItem->geometry->drawArgs["sphere"].startIndexLocation;
//        rightSphereItem->baseVertexLocation = rightSphereItem->geometry->drawArgs["sphere"].baseVertexLocation;
//
//        allRenderItems_.push_back(std::move(leftCylinderItem));
//        allRenderItems_.push_back(std::move(rightCylinderItem));
//        allRenderItems_.push_back(std::move(leftSphereItem));
//        allRenderItems_.push_back(std::move(rightSphereItem));
//    }
//
//    for (auto& item : allRenderItems_)
//    {
//        opaqueRenderItems_.push_back(item.get());
//    }
//}
//
//void LitColumnsApp::BuildFrameResource(void)
//{
//    for (int idx = 0; idx < NumFrameResourceCh07; ++idx)
//    {
//        frameResources_.push_back(
//            std::make_unique<FrameResourceCh07Shape> (d3dDevice_.Get(), 
//                1, 
//                static_cast<UINT> (allRenderItems_.size())));
//    }
//}
//
//void LitColumnsApp::BuildPSO(void)
//{
//    D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;
//    ZeroMemory(&opaquePsoDesc, sizeof(opaquePsoDesc));
//    opaquePsoDesc.InputLayout = {inputLayout_.data(), static_cast<UINT> (inputLayout_.size())};
//    opaquePsoDesc.pRootSignature = rootSignature_.Get();
//    opaquePsoDesc.VS = 
//    {
//        reinterpret_cast<BYTE*> (shaders_["standardVS"]->GetBufferPointer()),
//        shaders_["standardVS"]->GetBufferSize()
//    };
//    opaquePsoDesc.PS = 
//    {
//        reinterpret_cast<BYTE*> (shaders_["standardPS"]->GetBufferPointer()),
//        shaders_["standardPS"]->GetBufferSize()
//    };
//    opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
//    opaquePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;    
//    opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
//    opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
//    opaquePsoDesc.SampleMask = UINT_MAX;
//    opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
//    opaquePsoDesc.NumRenderTargets = 1;
//    opaquePsoDesc.RTVFormats[0] = backBufferFormat_;
//    opaquePsoDesc.SampleDesc.Count = (msaaState4x_ == true) ? (4) : (1);
//    opaquePsoDesc.SampleDesc.Quality = (msaaState4x_ == true) ? (msaaQuality4x_ - 1) : (0);
//    opaquePsoDesc.DSVFormat = depthStencilFormat_;
//
//    ThrowIfFailed(d3dDevice_->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&psos_["opaque"])));
//
//    D3D12_GRAPHICS_PIPELINE_STATE_DESC opaqueWireframePsoDesc = opaquePsoDesc;
//    opaqueWireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
//    ThrowIfFailed(d3dDevice_->CreateGraphicsPipelineState(&opaqueWireframePsoDesc, IID_PPV_ARGS(&psos_["opaque_wireframe"])));
//}
//
//void LitColumnsApp::DrawRenderItem(ID3D12GraphicsCommandList* pCmdList, const std::vector<RenderItemCh07*>& renderItems)
//{
//    //auto objectCBuffer = pCurFrameResource_->objectCBuffer->Resource();
//    for (size_t idx = 0; idx < renderItems.size(); ++idx)
//    {
//        auto renderItem = renderItems[idx];
//
//        D3D12_VERTEX_BUFFER_VIEW vertexBufferView = renderItem->geometry->VertexBufferView();
//        D3D12_INDEX_BUFFER_VIEW indexBufferView = renderItem->geometry->IndexBufferView();
//
//        pCmdList->IASetVertexBuffers(0, 1, &vertexBufferView);
//        pCmdList->IASetIndexBuffer(&indexBufferView);
//        pCmdList->IASetPrimitiveTopology(renderItem->primitiveType);
//
//        UINT cbvIndex = curFrameResourceIndex_ * static_cast<UINT> (opaqueRenderItems_.size()) + renderItem->objCBufferIndex;
//        auto cbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(cbvHeap_->GetGPUDescriptorHandleForHeapStart());
//        cbvHandle.Offset(cbvIndex, cbvSrvDescriptorSize_);
//
//        pCmdList->SetGraphicsRootDescriptorTable(0, cbvHandle);
//        pCmdList->DrawIndexedInstanced(renderItem->indexCount, 
//            1, 
//            renderItem->startIndexLocation,
//            renderItem->baseVertexLocation,
//            0);
//    }
//}


#ifndef __CH07_FRAME_RESOURCE__H__
#define __CH07_FRAME_RESOURCE__H__

#include "../Common/d3d_util.h"
#include "../Common/math_helper.h"
#include "../Common/upload_buffer.h"

#include <memory>

//----------------------------------------------------------------
// constant buffer
//----------------------------------------------------------------
struct ObjectConstantCh07
{
    DirectX::XMFLOAT4X4 world = MathHelper::Indentity4x4();
};

struct PassConstantCh07
{
    DirectX::XMFLOAT4X4 view = MathHelper::Indentity4x4();
    DirectX::XMFLOAT4X4 viewInv = MathHelper::Indentity4x4();
    DirectX::XMFLOAT4X4 proj = MathHelper::Indentity4x4();
    DirectX::XMFLOAT4X4 projInv = MathHelper::Indentity4x4();
    DirectX::XMFLOAT4X4 viewProj = MathHelper::Indentity4x4();
    DirectX::XMFLOAT4X4 viewProjInv = MathHelper::Indentity4x4();
    DirectX::XMFLOAT3 eyePosWorld = {0.0f, 0.0f, 0.0f};
    float cbObjectPad1 = 0.0f;
    DirectX::XMFLOAT2 renderTargetSize = {0.0f, 0.0f};
    DirectX::XMFLOAT2 renderTargetSizeInv = {0.0f, 0.0f};
    float zNear = 0.0f;
    float zFar = 0.0f;
    float totalTime = 0.0f;
    float deltaTime = 0.0f;
};

//----------------------------------------------------------------
// constant buffer
//----------------------------------------------------------------
struct VertexCh07
{
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT4 color;
};

//----------------------------------------------------------------
// frame resource
//----------------------------------------------------------------
struct FrameResourceCh07Shape
{
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> cmdlistAlloc;

    std::unique_ptr<UplaodBuffer<PassConstantCh07>> passCBuffer = nullptr;
    std::unique_ptr<UplaodBuffer<ObjectConstantCh07>> objectCBuffer = nullptr;

    UINT64 fence = 0;

    FrameResourceCh07Shape(ID3D12Device* pDevice, UINT passCount, UINT objectCount);
    ~FrameResourceCh07Shape(void);

    FrameResourceCh07Shape(const FrameResourceCh07Shape& other) = delete;
    FrameResourceCh07Shape& operator=(const FrameResourceCh07Shape& other) = delete;
};

struct FrameResourceCh07LandWave
{
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> cmdlistAlloc;

    std::unique_ptr<UplaodBuffer<PassConstantCh07>> passCBuffer = nullptr;
    std::unique_ptr<UplaodBuffer<ObjectConstantCh07>> objectCBuffer = nullptr;
    std::unique_ptr<UplaodBuffer<VertexCh07>> waveVBuffer = nullptr;

    UINT64 fence = 0;

    FrameResourceCh07LandWave(ID3D12Device* pDevice, UINT passCount, UINT objectCount, UINT waveVertCount);
    ~FrameResourceCh07LandWave(void);

    FrameResourceCh07LandWave(const FrameResourceCh07LandWave& other) = delete;
    FrameResourceCh07LandWave& operator=(const FrameResourceCh07LandWave& other) = delete;
};


//----------------------------------------------------------------
// RenderItem
//----------------------------------------------------------------
const int NumFrameResourceCh07 = NumGlobalFrameResource;

struct RenderItemCh07
{
    DirectX::XMFLOAT4X4 world = MathHelper::Indentity4x4();
    int numFrameDirty = NumFrameResourceCh07;
    int objCBufferIndex = -1;    
    MeshGeometry* geometry = nullptr;
    D3D12_PRIMITIVE_TOPOLOGY primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    UINT indexCount = 0;
    UINT startIndexLocation = 0;
    int baseVertexLocation = 0;

    RenderItemCh07(void) = default;
};


#endif /*#define __CH07_FRAME_RESOURCE__H__*/
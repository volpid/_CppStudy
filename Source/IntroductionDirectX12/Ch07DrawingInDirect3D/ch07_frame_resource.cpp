
#include "ch07_frame_resource.h"

//----------------------------------------------------------------
// frame resource
//----------------------------------------------------------------
/* FrameResourceCh07Shape */
FrameResourceCh07Shape::FrameResourceCh07Shape(ID3D12Device* pDevice, UINT passCount, UINT objectCount)
{
    ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(cmdlistAlloc.GetAddressOf())));

    passCBuffer = std::make_unique<UplaodBuffer<PassConstantCh07>> (pDevice, passCount, true);
    objectCBuffer = std::make_unique<UplaodBuffer<ObjectConstantCh07>> (pDevice, objectCount, true);
}

FrameResourceCh07Shape::~FrameResourceCh07Shape(void)
{}

/* FrameResourceCh07LandWave */
FrameResourceCh07LandWave::FrameResourceCh07LandWave(ID3D12Device* pDevice, UINT passCount, UINT objectCount, UINT waveVertCount)
{
    ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(cmdlistAlloc.GetAddressOf())));

    passCBuffer = std::make_unique<UplaodBuffer<PassConstantCh07>> (pDevice, passCount, true);
    objectCBuffer = std::make_unique<UplaodBuffer<ObjectConstantCh07>> (pDevice, objectCount, true);
    waveVBuffer = std::make_unique<UplaodBuffer<VertexCh07>> (pDevice, waveVertCount, false);
}

FrameResourceCh07LandWave::~FrameResourceCh07LandWave(void)
{}

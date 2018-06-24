
#ifndef __D3DX12__H__
#define __D3DX12__H__

#include <d3d12.h>
#include <d3dcompiler.h>

#include <dxgi1_4.h>

#include <DirectXCollision.h>
#include <DirectXColors.h>
#include <DirectXMath.h>

//----------------------------------------------------------------
// d3d data wrapper
//----------------------------------------------------------------
struct CD3DX12_DEFAULT {};
extern const DECLSPEC_SELECTANY CD3DX12_DEFAULT D3D12_DEFAULT;

//----------------------------------------------------------------
struct CD3DX12_CPU_DESCRIPTOR_HANDLE : public D3D12_CPU_DESCRIPTOR_HANDLE
{
    inline static void InitOffsetted(_Out_ D3D12_CPU_DESCRIPTOR_HANDLE& handle, _In_ const D3D12_CPU_DESCRIPTOR_HANDLE& base, INT offsetScaledByIncrementSize);
    inline static void InitOffsetted(_Out_ D3D12_CPU_DESCRIPTOR_HANDLE& handle, _In_ const D3D12_CPU_DESCRIPTOR_HANDLE& base, INT offsetInDescriptors, UINT descriptorIncrementSize);

    CD3DX12_CPU_DESCRIPTOR_HANDLE(void) = default;
    ~CD3DX12_CPU_DESCRIPTOR_HANDLE(void) = default;

    inline explicit CD3DX12_CPU_DESCRIPTOR_HANDLE(const D3D12_CPU_DESCRIPTOR_HANDLE& o);
    inline explicit CD3DX12_CPU_DESCRIPTOR_HANDLE(CD3DX12_DEFAULT);
    inline CD3DX12_CPU_DESCRIPTOR_HANDLE(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other, INT offsetScaledByIncrementSize);
    inline CD3DX12_CPU_DESCRIPTOR_HANDLE(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other, INT offsetInDescriptors, UINT descriptorIncrementSize);

    inline CD3DX12_CPU_DESCRIPTOR_HANDLE& Offset(INT offsetInDescriptors, UINT descriptorIncrementSize);
    inline CD3DX12_CPU_DESCRIPTOR_HANDLE& Offset(UINT offsetScaledByIncrementSize);

    inline bool operator==(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other);
    inline bool operator!=(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other);

    CD3DX12_CPU_DESCRIPTOR_HANDLE& operator=(const CD3DX12_CPU_DESCRIPTOR_HANDLE& other);

    inline void InitOffsetted(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& base, INT offsetScaledByIncrementSize);
    inline void InitOffsetted(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& base, INT offsetInDescriptors, UINT descriptorIncrementSize);
};

//----------------------------------------------------------------
struct CD3DX12_GPU_DESCRIPTOR_HANDLE : public D3D12_GPU_DESCRIPTOR_HANDLE
{
    static inline void InitOffsetted(_Out_ D3D12_GPU_DESCRIPTOR_HANDLE& handle, _In_ const D3D12_GPU_DESCRIPTOR_HANDLE& base, INT offsetScaledByIncrementSize);
    static inline void InitOffsetted(_Out_ D3D12_GPU_DESCRIPTOR_HANDLE& handle, _In_ const D3D12_GPU_DESCRIPTOR_HANDLE& base, INT offsetInDescriptor, UINT descriptorIncrementSize);

    CD3DX12_GPU_DESCRIPTOR_HANDLE(void) = default;
    ~CD3DX12_GPU_DESCRIPTOR_HANDLE(void) = default;

    inline explicit CD3DX12_GPU_DESCRIPTOR_HANDLE(CD3DX12_DEFAULT);
    inline explicit CD3DX12_GPU_DESCRIPTOR_HANDLE(const D3D12_GPU_DESCRIPTOR_HANDLE& o);

    inline CD3DX12_GPU_DESCRIPTOR_HANDLE(_In_ const D3D12_GPU_DESCRIPTOR_HANDLE& other, INT offsetScaledByIncreamentSize);
    inline CD3DX12_GPU_DESCRIPTOR_HANDLE(_In_ const D3D12_GPU_DESCRIPTOR_HANDLE& other, INT offsetInDescriptor, UINT decriptorIncrementSize);

    inline CD3DX12_GPU_DESCRIPTOR_HANDLE& Offset(INT offsetInDescriptor, UINT descriptorIncrementSize);
    inline CD3DX12_GPU_DESCRIPTOR_HANDLE& Offset(INT offsetScaledByIncreamentSize);

    inline bool operator==(_In_ const D3D12_GPU_DESCRIPTOR_HANDLE& other);
    inline bool operator!=(_In_ const D3D12_GPU_DESCRIPTOR_HANDLE& other);

    inline CD3DX12_GPU_DESCRIPTOR_HANDLE& operator=(const D3D12_GPU_DESCRIPTOR_HANDLE& other);
    
    inline void InitOffsetted(_In_ const D3D12_GPU_DESCRIPTOR_HANDLE& base, INT offsetScaledByIncrementSize);
    inline void InitOffsetted(_In_ const D3D12_GPU_DESCRIPTOR_HANDLE& base, INT offsetInDescriptor, UINT descriptorIncrementSize);
};

//----------------------------------------------------------------
struct CD3DX12_HEAP_PROPERTIES : public D3D12_HEAP_PROPERTIES
{
    CD3DX12_HEAP_PROPERTIES(void) = default;
    ~CD3DX12_HEAP_PROPERTIES(void) = default;

    inline explicit CD3DX12_HEAP_PROPERTIES(const D3D12_HEAP_PROPERTIES& o);
    inline CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY cpuPageProperty,
        D3D12_MEMORY_POOL memoryPoolPrepference,
        UINT creationNModeMask = 1,
        UINT nodeMask = 1);
    inline explicit CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE type, 
        UINT creationNModeMask = 1,
        UINT nodeMask = 1);

    inline operator const CD3DX12_HEAP_PROPERTIES&(void) const;

    inline bool IsCPUAccessible(void) const;
};

//----------------------------------------------------------------
struct CD3DX12_RESOURCE_BARRIER : public D3D12_RESOURCE_BARRIER
{
    static inline CD3DX12_RESOURCE_BARRIER Transition(_In_ ID3D12Resource* pResource,
        D3D12_RESOURCE_STATES stateBefore,
        D3D12_RESOURCE_STATES stateAfter,
        UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
        D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE);
    static inline CD3DX12_RESOURCE_BARRIER Aliasing(_In_ ID3D12Resource* pResourceBefore,
        _In_ ID3D12Resource* pResourceAfter);
    static inline CD3DX12_RESOURCE_BARRIER UAV(_In_ ID3D12Resource* pResource);

    CD3DX12_RESOURCE_BARRIER(void) = default;
    ~CD3DX12_RESOURCE_BARRIER(void) = default;
    
    inline explicit CD3DX12_RESOURCE_BARRIER(const D3D12_RESOURCE_BARRIER& o);
};

//----------------------------------------------------------------
struct CD3DX12_RESOURCE_DESC : public D3D12_RESOURCE_DESC
{
    inline static CD3DX12_RESOURCE_DESC Buffer(const D3D12_RESOURCE_ALLOCATION_INFO& resAllocInfo,
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
    inline static CD3DX12_RESOURCE_DESC Buffer(UINT64 width,
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
        UINT64 alignment = 0);
    inline static CD3DX12_RESOURCE_DESC Tex1D(DXGI_FORMAT format,
        UINT64 width,
        UINT16 arraySize = 1,
        UINT16 mipLevels = 0,
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
        D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
        UINT64 alignment = 0);
    inline static CD3DX12_RESOURCE_DESC Tex2D(DXGI_FORMAT format,
        UINT64 width,
        UINT height,
        UINT16 arraySize = 1,
        UINT16 mipLevels = 0,
        UINT sampleCount = 1,
        UINT sampleQuality = 0,
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
        D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
        UINT64 alignment = 0);
    inline static CD3DX12_RESOURCE_DESC Tex3D(DXGI_FORMAT format,
        UINT64 width,
        UINT height,
        UINT16 depth,
        UINT16 mipLevels = 0,
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
        D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
        UINT64 alignment = 0);

    CD3DX12_RESOURCE_DESC(void) = default;
    ~CD3DX12_RESOURCE_DESC(void) = default;

    inline explicit CD3DX12_RESOURCE_DESC(const D3D12_RESOURCE_DESC& o);
    inline CD3DX12_RESOURCE_DESC(D3D12_RESOURCE_DIMENSION dimention,
        UINT64 alignment,
        UINT64 width,
        UINT height,
        UINT16 depthOrArraySize,
        UINT16 mipLevels,
        DXGI_FORMAT format, 
        UINT sampleCount,
        UINT sampleQuality,
        D3D12_TEXTURE_LAYOUT layout,
        D3D12_RESOURCE_FLAGS flags);

    inline UINT16 Depth(void) const;
    inline UINT16 ArraySize(void) const;
    inline UINT8 PlaneCount(_In_ ID3D12Device* pDevice) const;
    inline UINT Subresource(_In_ ID3D12Device* pDevice) const;
    inline UINT CalcSubresource(UINT mipSlice, UINT arraySlice, UINT planeSlice);

    inline operator const D3D12_RESOURCE_DESC& (void) const;
};
//----------------------------------------------------------------
struct CD3DX12_ROOT_DESCRIPTOR_TABLE : public D3D12_ROOT_DESCRIPTOR_TABLE
{
    inline static void Init(_Out_ D3D12_ROOT_DESCRIPTOR_TABLE& rootDescriptorTable,
        UINT numDescriptorRange,
        _In_reads_(numDescriptorRange) const D3D12_DESCRIPTOR_RANGE* pDescriptorRanges);

    CD3DX12_ROOT_DESCRIPTOR_TABLE(void) = default;
    ~CD3DX12_ROOT_DESCRIPTOR_TABLE(void) = default;

    inline explicit CD3DX12_ROOT_DESCRIPTOR_TABLE(const D3D12_ROOT_DESCRIPTOR_TABLE& o);
    inline CD3DX12_ROOT_DESCRIPTOR_TABLE(UINT numDescriptorRange, 
        _In_reads_(numDescriptorRange) const D3D12_DESCRIPTOR_RANGE* pDescriptorRanges);

    inline void Init(UINT numDescriptorRange,
        _In_reads_(numDescriptorRange) const D3D12_DESCRIPTOR_RANGE* pDescriptorRanges);
};

//----------------------------------------------------------------
struct CD3DX12_ROOT_CONSTANTS : public D3D12_ROOT_CONSTANTS
{
    inline static void Init(_Out_ D3D12_ROOT_CONSTANTS& rootConstant,
        UINT num32BitValue, 
        UINT shaderRegister, 
        UINT registerSpace = 0);

    CD3DX12_ROOT_CONSTANTS(void) = default;
    ~CD3DX12_ROOT_CONSTANTS(void) = default;

    inline explicit CD3DX12_ROOT_CONSTANTS(const D3D12_ROOT_CONSTANTS& o);
    inline CD3DX12_ROOT_CONSTANTS(UINT num32BitValue, UINT shaderRegister, UINT registerSpace);

    inline void Init(UINT num32BitValue, UINT shaderRegister, UINT registerSpace = 0);
};

//----------------------------------------------------------------
struct CD3DX12_ROOT_DESCRIPTOR : public D3D12_ROOT_DESCRIPTOR
{
    inline static void Init(_Out_ D3D12_ROOT_DESCRIPTOR& rootConstant, UINT shaderRegister, UINT registerSpace = 0);
    
    CD3DX12_ROOT_DESCRIPTOR(void) = default;
    ~CD3DX12_ROOT_DESCRIPTOR(void) = default;

    inline CD3DX12_ROOT_DESCRIPTOR(const D3D12_ROOT_DESCRIPTOR& o);
    inline CD3DX12_ROOT_DESCRIPTOR(UINT shaderRegister, UINT registerSpace = 0);

    inline void Init(UINT shaderRegister, UINT registerSpace = 0);
};

//----------------------------------------------------------------
struct CD3DX12_ROOT_PARAMETER : public D3D12_ROOT_PARAMETER
{
    inline static void InitAsDescriptorTable(_Out_ D3D12_ROOT_PARAMETER& rootParam,
        UINT numDescriptorRange,
        _In_reads_(numDescriptorRange) const D3D12_DESCRIPTOR_RANGE* pDescriptorRanges,
        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
    inline static void InitAsConstant(_Out_ D3D12_ROOT_PARAMETER& rootParam,
        UINT num32bitValue,
        UINT shaderRegister,
        UINT registerSpace = 0,
        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
    inline static void InitAsConstantBufferView(_Out_ D3D12_ROOT_PARAMETER& rootParam,
        UINT shaderRegister,
        UINT registerSpace = 0,
        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
    inline static void InitAsShaderResourceView(_Out_ D3D12_ROOT_PARAMETER& rootParam,
        UINT shaderRegister,
        UINT registerSpace = 0,
        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
    inline static void InitAsUnorderedaccessView(_Out_ D3D12_ROOT_PARAMETER& rootParam,
        UINT shaderRegister,
        UINT registerSpace = 0,
        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);

    CD3DX12_ROOT_PARAMETER(void) = default;
    ~CD3DX12_ROOT_PARAMETER(void) = default;

    inline explicit CD3DX12_ROOT_PARAMETER(const D3D12_ROOT_PARAMETER& o);

    inline void InitAsDescriptorTable(UINT numDescriptorRange,
        _In_reads_(numDescriptorRange) const D3D12_DESCRIPTOR_RANGE* pDescriptorRanges,
        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
    inline void InitAsConstant(UINT num32bitValue,
        UINT shaderRegister,
        UINT registerSpace = 0,
        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
    inline void InitAsConstantBufferView(UINT shaderRegister,
        UINT registerSpace = 0,
        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
    inline void InitAsShaderResourceView(UINT shaderRegister,
        UINT registerSpace = 0,
        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
    inline void InitAsUnorderedaccessView(UINT shaderRegister,
        UINT registerSpace = 0,
        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
};

//----------------------------------------------------------------
struct CD3DX12_DESCRIPTOR_RANGE : public D3D12_DESCRIPTOR_RANGE
{
    inline static void Init(_Out_ D3D12_DESCRIPTOR_RANGE& range,
        D3D12_DESCRIPTOR_RANGE_TYPE rangeType,
        UINT numDescriptor,
        UINT baseShaderRegister,
        UINT registerSpace = 0,
        UINT offsetInDescriptorFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);

    CD3DX12_DESCRIPTOR_RANGE(void) = default;
    ~CD3DX12_DESCRIPTOR_RANGE(void) = default;
    
    inline CD3DX12_DESCRIPTOR_RANGE(const D3D12_DESCRIPTOR_RANGE& o);
    inline CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE rangeType,
        UINT numDescriptor,
        UINT baseShaderRegister,
        UINT registerSpace = 0,
        UINT offsetInDescriptorFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);

    inline void Init(D3D12_DESCRIPTOR_RANGE_TYPE rangeType,
        UINT numDescriptor,
        UINT baseShaderRegister,
        UINT registerSpace = 0,
        UINT offsetInDescriptorFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
};

//----------------------------------------------------------------
struct CD3DX12_ROOT_SIGNATURE_DESC : public D3D12_ROOT_SIGNATURE_DESC
{
    inline static void Init(_Out_ D3D12_ROOT_SIGNATURE_DESC& desc,
        UINT numParameter,
        _In_reads_opt_(numParameter) const D3D12_ROOT_PARAMETER* pParameters,
        UINT numStaticSampler = 0,
        _In_reads_opt_(numStaticSampler) const D3D12_STATIC_SAMPLER_DESC* pStaticSamplers = nullptr,
        D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE);
        
    CD3DX12_ROOT_SIGNATURE_DESC(void) = default;
    ~CD3DX12_ROOT_SIGNATURE_DESC(void) = default;

    inline explicit CD3DX12_ROOT_SIGNATURE_DESC(const D3D12_ROOT_SIGNATURE_DESC& o);
    inline explicit CD3DX12_ROOT_SIGNATURE_DESC(CD3DX12_DEFAULT def);
    inline CD3DX12_ROOT_SIGNATURE_DESC(UINT numParameter,
        _In_reads_opt_(numParameter) const D3D12_ROOT_PARAMETER* pParameters,
        UINT numStaticSampler = 0,
        _In_reads_opt_(numStaticSampler) const D3D12_STATIC_SAMPLER_DESC* pStaticSamplers = nullptr,
        D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE);

    inline void Init(UINT numParameter,
        _In_reads_opt_(numParameter) const D3D12_ROOT_PARAMETER* pParameters,
        UINT numStaticSampler = 0,
        _In_reads_opt_(numStaticSampler) const D3D12_STATIC_SAMPLER_DESC* pStaticSamplers = nullptr,
        D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE);
};

//----------------------------------------------------------------
struct CD3DX12_BOX : public D3D12_BOX
{
    CD3DX12_BOX(void) = default;
    ~CD3DX12_BOX() = default;

    inline explicit CD3DX12_BOX(const D3D12_BOX& o);
    inline explicit CD3DX12_BOX(LONG lt, LONG rt);
    inline explicit CD3DX12_BOX(LONG lt, LONG tp, LONG rt, LONG bt);
    inline explicit CD3DX12_BOX(LONG lt, LONG tp, LONG fr, LONG rt, LONG bt, LONG ba);
    inline operator const D3D12_BOX& (void) const;
};

bool operator==(const D3D12_BOX& l, const D3D12_BOX& r);
bool operator!=(const D3D12_BOX& l, const D3D12_BOX& r);

//----------------------------------------------------------------
struct CD3DX12_TEXTURE_COPY_LOCATION : public D3D12_TEXTURE_COPY_LOCATION
{
    CD3DX12_TEXTURE_COPY_LOCATION(void) = default;
    ~CD3DX12_TEXTURE_COPY_LOCATION(void) = default;

    inline explicit CD3DX12_TEXTURE_COPY_LOCATION(const D3D12_TEXTURE_COPY_LOCATION& o);
    inline CD3DX12_TEXTURE_COPY_LOCATION(ID3D12Resource* pRes);
    inline CD3DX12_TEXTURE_COPY_LOCATION(ID3D12Resource* pRes, const D3D12_PLACED_SUBRESOURCE_FOOTPRINT& footprint);
    inline CD3DX12_TEXTURE_COPY_LOCATION(ID3D12Resource* pRes, UINT sub);    
};

//----------------------------------------------------------------
struct CD3DX12_RASTERIZER_DESC : public D3D12_RASTERIZER_DESC
{
    CD3DX12_RASTERIZER_DESC(void) = default;
    ~CD3DX12_RASTERIZER_DESC(void) = default;

    inline explicit CD3DX12_RASTERIZER_DESC(const D3D12_RASTERIZER_DESC& o);
    inline explicit CD3DX12_RASTERIZER_DESC(CD3DX12_DEFAULT def);
    inline CD3DX12_RASTERIZER_DESC(D3D12_FILL_MODE fillmode,
        D3D12_CULL_MODE cullmode,
        BOOL frontCounterClockwise,
        INT depthBias,
        FLOAT depthBiasClamp,
        FLOAT slopeScaleDepthBias,
        BOOL depthClipEnable,
        BOOL multisampleEnable,
        BOOL antialiasedLineEnable,
        UINT forcedSampleCount,
        D3D12_CONSERVATIVE_RASTERIZATION_MODE conservativeRaster);

    inline operator const D3D12_RASTERIZER_DESC& (void) const;
};

//----------------------------------------------------------------
struct CD3DX12_BLEND_DESC : public D3D12_BLEND_DESC
{
    CD3DX12_BLEND_DESC(void) = default;
    ~CD3DX12_BLEND_DESC(void) = default;

    inline explicit CD3DX12_BLEND_DESC(const D3D12_BLEND_DESC& o);
    inline explicit CD3DX12_BLEND_DESC(CD3DX12_DEFAULT def);
    inline operator const D3D12_BLEND_DESC& (void) const;
};

//----------------------------------------------------------------
struct CD3DX12_DEPTH_STENCIL_DESC : public D3D12_DEPTH_STENCIL_DESC
{
    CD3DX12_DEPTH_STENCIL_DESC(void) = default;
    ~CD3DX12_DEPTH_STENCIL_DESC(void) = default;

    inline explicit CD3DX12_DEPTH_STENCIL_DESC(const D3D12_DEPTH_STENCIL_DESC& o);
    inline explicit CD3DX12_DEPTH_STENCIL_DESC(CD3DX12_DEFAULT);
    inline CD3DX12_DEPTH_STENCIL_DESC(BOOL depthEnable,
        D3D12_DEPTH_WRITE_MASK depthWriteMask,
        D3D12_COMPARISON_FUNC depthFunc,
        BOOL stencilEnable,
        UINT8 stencilReadMask,
        UINT8 stencilWriteMask,
        D3D12_STENCIL_OP frontStencilFailOp,
        D3D12_STENCIL_OP frontStencilDepthOp,
        D3D12_STENCIL_OP frontStencilPassOp,
        D3D12_COMPARISON_FUNC frontStencilFunc,
        D3D12_STENCIL_OP backStencilFailOp,
        D3D12_STENCIL_OP backStencilDepthOp,
        D3D12_STENCIL_OP backStencilPassOp,
        D3D12_COMPARISON_FUNC backStencilFunc);

    inline operator const D3D12_DEPTH_STENCIL_DESC& (void) const;
};

//----------------------------------------------------------------
// inline section
//----------------------------------------------------------------
inline void MemcopySubresource(
    _In_ const D3D12_MEMCPY_DEST* pDest,
    _In_ const D3D12_SUBRESOURCE_DATA* pSource,
    SIZE_T rowSizeInByte,
    UINT numRows,
    UINT numSlices)
{
    for (UINT idxSlice = 0; idxSlice < numSlices; ++idxSlice)
    {
        BYTE* pDestSlice = reinterpret_cast<BYTE*> (pDest->pData) + pDest->SlicePitch * idxSlice;
        const BYTE* pSourceSlice = reinterpret_cast<const BYTE*> (pSource->pData) + pSource->SlicePitch * idxSlice;
        for (UINT idxRow = 0; idxRow < numRows; ++idxRow)
        {
            memcpy(pDestSlice + pDest->RowPitch * idxRow,
                pSourceSlice + pSource->RowPitch * idxRow,
                rowSizeInByte);
        }
    }
}

inline UINT64 UpdateSubresource(_In_ ID3D12GraphicsCommandList* pCmdList,
    _In_ ID3D12Resource* pDestinationResource,
    _In_ ID3D12Resource* pIntermediate,
    _In_range_(0, D3D12_REQ_SUBRESOURCES) UINT firstSubresource,
    _In_range_(0, D3D12_REQ_SUBRESOURCES - firstSubresource) UINT numSubresouce,
    UINT64 requiredSize,
    _In_range_(numSubresouce) const D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts,
    _In_range_(numSubresouce) const UINT* pNumRows,
    _In_range_(numSubresouce) const UINT64* pRowSizeInBytes,
    _In_range_(numSubresouce) const D3D12_SUBRESOURCE_DATA* pSrcData)
{
    D3D12_RESOURCE_DESC intermediateDesc = pIntermediate->GetDesc();
    D3D12_RESOURCE_DESC destinationDesc = pDestinationResource->GetDesc();

    if ((intermediateDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER)
        || (intermediateDesc.Width < requiredSize + pLayouts[0].Offset)
        || (requiredSize > (SIZE_T) -1)
        || (destinationDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER
            && (firstSubresource != 0 || numSubresouce != 1)))
    {
        return 0;
    }

    BYTE* pData;
    HRESULT hr = pIntermediate->Map(0, nullptr, reinterpret_cast<void**> (&pData));
    if (FAILED(hr))
    {
        return 0;
    }

    for (UINT idx = 0; idx < numSubresouce; ++idx)
    {
        if (pRowSizeInBytes[idx] > (SIZE_T) -1)
        {
            return 0;
        }

        D3D12_MEMCPY_DEST destData = 
        {
            pData + pLayouts[idx].Offset,
            pLayouts[idx].Footprint.RowPitch,
            pLayouts[idx].Footprint.RowPitch * pNumRows[idx]
        };

        MemcopySubresource(&destData,
            &pSrcData[idx], 
            (SIZE_T) pRowSizeInBytes[idx], 
            pNumRows[idx],
            pLayouts[idx].Footprint.Depth);
    }

    pIntermediate->Unmap(0, nullptr);

    if (destinationDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
    {
        CD3DX12_BOX srcBox(UINT(pLayouts[0].Offset), UINT(pLayouts[0].Offset + pLayouts[0].Footprint.Width));
        pCmdList->CopyBufferRegion(pDestinationResource, 0, pIntermediate, pLayouts[0].Offset, pLayouts[0].Footprint.Width);
    }
    else
    {
        for (UINT idx = 0; idx < numSubresouce; ++idx)
        {
            CD3DX12_TEXTURE_COPY_LOCATION dest(pDestinationResource, idx + firstSubresource);
            CD3DX12_TEXTURE_COPY_LOCATION src(pIntermediate, pLayouts[idx]);
            pCmdList->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);
        }
    }

    return requiredSize;
}

inline UINT64 UpdateSubresource(_In_ ID3D12GraphicsCommandList* pCmdList,
    _In_ ID3D12Resource* pDestinationResource,
    _In_ ID3D12Resource* pIntermediate,
    UINT64 itermediateOffset,
    _In_range_(0, D3D12_REQ_SUBRESOURCES) UINT firstSubresource,
    _In_range_(0, D3D12_REQ_SUBRESOURCES - firstSubresource) UINT numSubresource,
    _In_range_(numSubresource) D3D12_SUBRESOURCE_DATA* pSrcData)
{
    UINT64 requiredSize = 0;
    UINT64 memToAlloc = static_cast<UINT64> (sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)) * numSubresource;

    if (memToAlloc > SIZE_MAX)
    {
        return 0;
    }

    void* pMem = HeapAlloc(GetProcessHeap(), 0, static_cast<SIZE_T> (memToAlloc));
    if (pMem == nullptr)
    {
        return 0;
    }

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts = reinterpret_cast<D3D12_PLACED_SUBRESOURCE_FOOTPRINT*> (pMem);
    UINT64* pRowSizeInByte = reinterpret_cast<UINT64*> (pLayouts + numSubresource);
    UINT* pNumRow = reinterpret_cast<UINT*> (pRowSizeInByte + numSubresource);

    D3D12_RESOURCE_DESC desc = pDestinationResource->GetDesc();
    ID3D12Device* pDevice;
    pDestinationResource->GetDevice(__uuidof(*pDevice), reinterpret_cast<void**> (&pDevice));
    pDevice->GetCopyableFootprints(&desc, firstSubresource, numSubresource, itermediateOffset, pLayouts, pNumRow, pRowSizeInByte, &requiredSize);
    pDevice->Release();

    UINT64 result = UpdateSubresource(pCmdList, pDestinationResource, pIntermediate, firstSubresource, numSubresource, requiredSize, pLayouts, pNumRow, pRowSizeInByte, pSrcData);
    HeapFree(GetProcessHeap(), 0, pMem);
    return result;
}

template <UINT _MaxSubresource>
inline UINT64 UpdateSubresource(_In_ ID3D12GraphicsCommandList* pCmdList,
    _In_ ID3D12Resource* pDestinationResource,
    _In_ ID3D12Resource* pIntermediate,
    UINT64 itermediateOffset,
    _In_range_(0, D3D12_REQ_SUBRESOURCES) UINT firstSubresource,
    _In_range_(1, D3D12_REQ_SUBRESOURCES - firstSubresource) UINT numSubresource,
    _In_range_(numSubresource) D3D12_SUBRESOURCE_DATA* pSrcData)
{
    UINT64 requiredSize = 0;
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT layouts[_MaxSubresource];
    UINT numRow[_MaxSubresource];
    UINT64 rowSizeInByte[_MaxSubresource];

    D3D12_RESOURCE_DESC desc = pDestinationResource->GetDesc();
    ID3D12Device* pDevice;
    pDestinationResource->GetDevice(__uuidof(*pDevice), reinterpret_cast<void**> (&pDevice));
    pDevice->GetCopyableFootprints(&desc, firstSubresource, numSubresource, itermediateOffset, layouts, numRow, rowSizeInByte, &requiredSize);
    pDevice->Release();

    return UpdateSubresource(pCmdList, pDestinationResource, pIntermediate, firstSubresource, numSubresource, requiredSize, layouts, numRow, rowSizeInByte, pSrcData);
}

//----------------------------------------------------------------
inline void CD3DX12_CPU_DESCRIPTOR_HANDLE::InitOffsetted(_Out_ D3D12_CPU_DESCRIPTOR_HANDLE& handle, _In_ const D3D12_CPU_DESCRIPTOR_HANDLE& base, INT offsetScaledByIncrementSize)
{
    handle.ptr = base.ptr + offsetScaledByIncrementSize;
}

inline void CD3DX12_CPU_DESCRIPTOR_HANDLE::InitOffsetted(_Out_ D3D12_CPU_DESCRIPTOR_HANDLE& handle, _In_ const D3D12_CPU_DESCRIPTOR_HANDLE& base, INT offsetInDescriptors, UINT descriptorIncrementSize)
{
    handle.ptr = base.ptr + offsetInDescriptors * descriptorIncrementSize;
}    

inline CD3DX12_CPU_DESCRIPTOR_HANDLE::CD3DX12_CPU_DESCRIPTOR_HANDLE(const D3D12_CPU_DESCRIPTOR_HANDLE& o)
    : D3D12_CPU_DESCRIPTOR_HANDLE(o)
{}

inline CD3DX12_CPU_DESCRIPTOR_HANDLE::CD3DX12_CPU_DESCRIPTOR_HANDLE(CD3DX12_DEFAULT def) 
{
    _Unreferenced_parameter_(def);
    ptr = 0;
}

inline CD3DX12_CPU_DESCRIPTOR_HANDLE::CD3DX12_CPU_DESCRIPTOR_HANDLE(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other, INT offsetScaledByIncrementSize)
{
    InitOffsetted(other, offsetScaledByIncrementSize);
}

inline CD3DX12_CPU_DESCRIPTOR_HANDLE::CD3DX12_CPU_DESCRIPTOR_HANDLE(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other, INT offsetInDescriptors, UINT descriptorIncrementSize)
{
    InitOffsetted(other, offsetInDescriptors, descriptorIncrementSize);
}

inline CD3DX12_CPU_DESCRIPTOR_HANDLE& CD3DX12_CPU_DESCRIPTOR_HANDLE::Offset(INT offsetInDescriptors, UINT descriptorIncrementSize)
{
    ptr += offsetInDescriptors * descriptorIncrementSize;
    return *this;
}

inline CD3DX12_CPU_DESCRIPTOR_HANDLE& CD3DX12_CPU_DESCRIPTOR_HANDLE::Offset(UINT offsetScaledByIncrementSize)
{
    ptr += offsetScaledByIncrementSize;
    return *this;
}

inline bool CD3DX12_CPU_DESCRIPTOR_HANDLE::operator==(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other)
{
    return (ptr == other.ptr);
}

inline bool CD3DX12_CPU_DESCRIPTOR_HANDLE::operator!=(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other)
{
    return (ptr != other.ptr);
}

inline CD3DX12_CPU_DESCRIPTOR_HANDLE& CD3DX12_CPU_DESCRIPTOR_HANDLE::operator=(const CD3DX12_CPU_DESCRIPTOR_HANDLE& other)
{
    ptr = other.ptr;
    return *this;
}

inline void CD3DX12_CPU_DESCRIPTOR_HANDLE::InitOffsetted(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& base, INT offsetScaledByIncrementSize)
{
    InitOffsetted(*this, base, offsetScaledByIncrementSize);
}

inline void CD3DX12_CPU_DESCRIPTOR_HANDLE::InitOffsetted(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& base, INT offsetInDescriptors, UINT descriptorIncrementSize)
{
    InitOffsetted(*this, base, offsetInDescriptors, descriptorIncrementSize);
}

//----------------------------------------------------------------
inline CD3DX12_HEAP_PROPERTIES::CD3DX12_HEAP_PROPERTIES(const D3D12_HEAP_PROPERTIES& o)
    : D3D12_HEAP_PROPERTIES(o)
{}

inline CD3DX12_HEAP_PROPERTIES::CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY cpuPageProperty,
    D3D12_MEMORY_POOL memoryPoolPrepference,
    UINT creationNModeMask,
    UINT nodeMask)
{
    Type = D3D12_HEAP_TYPE_CUSTOM;
    CPUPageProperty = cpuPageProperty;
    MemoryPoolPreference = memoryPoolPrepference;
    CreationNodeMask = creationNModeMask;
    VisibleNodeMask = nodeMask;
}

inline CD3DX12_HEAP_PROPERTIES::CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE type, 
    UINT creationNModeMask,
    UINT nodeMask)
{
    Type = type;
    CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    CreationNodeMask = creationNModeMask;
    VisibleNodeMask = nodeMask;
}

inline CD3DX12_HEAP_PROPERTIES::operator const CD3DX12_HEAP_PROPERTIES&(void) const
{
    return *this;
}

bool CD3DX12_HEAP_PROPERTIES::IsCPUAccessible(void) const
{
    return (Type == D3D12_HEAP_TYPE_UPLOAD)
        || (Type == D3D12_HEAP_TYPE_READBACK)
        || ((Type == D3D12_HEAP_TYPE_CUSTOM)
        && (CPUPageProperty == D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE 
        || CPUPageProperty == D3D12_CPU_PAGE_PROPERTY_WRITE_BACK));
}

//----------------------------------------------------------------
inline void CD3DX12_GPU_DESCRIPTOR_HANDLE::InitOffsetted(_Out_ D3D12_GPU_DESCRIPTOR_HANDLE& handle, _In_ const D3D12_GPU_DESCRIPTOR_HANDLE& base, INT offsetScaledByIncrementSize)
{
    handle.ptr = base.ptr + offsetScaledByIncrementSize;
}

inline void CD3DX12_GPU_DESCRIPTOR_HANDLE::InitOffsetted(_Out_ D3D12_GPU_DESCRIPTOR_HANDLE& handle, _In_ const D3D12_GPU_DESCRIPTOR_HANDLE& base, INT offsetInDescriptor, UINT descriptorIncrementSize)
{
    handle.ptr = base.ptr + offsetInDescriptor * descriptorIncrementSize;
}

inline CD3DX12_GPU_DESCRIPTOR_HANDLE::CD3DX12_GPU_DESCRIPTOR_HANDLE(CD3DX12_DEFAULT)    
{
    ptr = 0;
}

inline CD3DX12_GPU_DESCRIPTOR_HANDLE::CD3DX12_GPU_DESCRIPTOR_HANDLE(const D3D12_GPU_DESCRIPTOR_HANDLE& o)
    : D3D12_GPU_DESCRIPTOR_HANDLE(o)
{
}

inline CD3DX12_GPU_DESCRIPTOR_HANDLE::CD3DX12_GPU_DESCRIPTOR_HANDLE(_In_ const D3D12_GPU_DESCRIPTOR_HANDLE& other, INT offsetScaledByIncreamentSize)
{
    InitOffsetted(other, offsetScaledByIncreamentSize);
}

inline CD3DX12_GPU_DESCRIPTOR_HANDLE::CD3DX12_GPU_DESCRIPTOR_HANDLE(_In_ const D3D12_GPU_DESCRIPTOR_HANDLE& other, INT offsetInDescriptor, UINT decriptorIncrementSize)
{
    InitOffsetted(other, offsetInDescriptor, decriptorIncrementSize);
}

inline CD3DX12_GPU_DESCRIPTOR_HANDLE& CD3DX12_GPU_DESCRIPTOR_HANDLE::Offset(INT offsetInDescriptor, UINT descriptorIncrementSize)
{
    ptr += offsetInDescriptor * descriptorIncrementSize;
    return *this;    
}

inline CD3DX12_GPU_DESCRIPTOR_HANDLE& CD3DX12_GPU_DESCRIPTOR_HANDLE::Offset(INT offsetScaledByIncreamentSize)
{
    ptr += offsetScaledByIncreamentSize;
    return *this;
}

inline bool CD3DX12_GPU_DESCRIPTOR_HANDLE::operator==(_In_ const D3D12_GPU_DESCRIPTOR_HANDLE& other)
{
    return (ptr == other.ptr);
}

inline bool CD3DX12_GPU_DESCRIPTOR_HANDLE::operator!=(_In_ const D3D12_GPU_DESCRIPTOR_HANDLE& other)
{
    return (ptr != other.ptr);
}

inline CD3DX12_GPU_DESCRIPTOR_HANDLE& CD3DX12_GPU_DESCRIPTOR_HANDLE::operator=(const D3D12_GPU_DESCRIPTOR_HANDLE& other)
{
    ptr = other.ptr;
    return *this;
}
    
inline void CD3DX12_GPU_DESCRIPTOR_HANDLE::InitOffsetted(_In_ const D3D12_GPU_DESCRIPTOR_HANDLE& base, INT offsetScaledByIncrementSize)
{
    InitOffsetted(*this, base, offsetScaledByIncrementSize);
}

inline void CD3DX12_GPU_DESCRIPTOR_HANDLE::InitOffsetted(_In_ const D3D12_GPU_DESCRIPTOR_HANDLE& base, INT offsetInDescriptor, UINT descriptorIncrementSize)
{
    InitOffsetted(*this, base, offsetInDescriptor, descriptorIncrementSize);
}

//----------------------------------------------------------------
inline CD3DX12_RESOURCE_BARRIER CD3DX12_RESOURCE_BARRIER::Transition(_In_ ID3D12Resource* pResource,
    D3D12_RESOURCE_STATES stateBefore,
    D3D12_RESOURCE_STATES stateAfter,
    UINT subresource,
    D3D12_RESOURCE_BARRIER_FLAGS flags)
{
    CD3DX12_RESOURCE_BARRIER result;
    ZeroMemory(&result, sizeof(result));
    D3D12_RESOURCE_BARRIER& barrier = result;
    result.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    result.Flags = flags;
    barrier.Transition.pResource = pResource;
    barrier.Transition.StateBefore = stateBefore;
    barrier.Transition.StateAfter = stateAfter;
    barrier.Transition.Subresource = subresource;

    return result;
}

inline CD3DX12_RESOURCE_BARRIER CD3DX12_RESOURCE_BARRIER::Aliasing(_In_ ID3D12Resource* pResourceBefore,
    _In_ ID3D12Resource* pResourceAfter)
{
    CD3DX12_RESOURCE_BARRIER result;
    ZeroMemory(&result, sizeof(result));
    D3D12_RESOURCE_BARRIER& barrier = result;
    result.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Aliasing.pResourceBefore = pResourceBefore;
    barrier.Aliasing.pResourceAfter = pResourceAfter;

    return result;
}

inline CD3DX12_RESOURCE_BARRIER CD3DX12_RESOURCE_BARRIER::UAV(_In_ ID3D12Resource* pResource)
{
    CD3DX12_RESOURCE_BARRIER result;
    ZeroMemory(&result, sizeof(result));
    D3D12_RESOURCE_BARRIER& barrier = result;
    result.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.UAV.pResource = pResource;

    return result;
}

CD3DX12_RESOURCE_BARRIER::CD3DX12_RESOURCE_BARRIER(const D3D12_RESOURCE_BARRIER& o)
    : D3D12_RESOURCE_BARRIER(o)
{}

//----------------------------------------------------------------
inline UINT8 D3D12GetFormatPlaneCount(_In_ ID3D12Device* pDevice, DXGI_FORMAT format)
{
    D3D12_FEATURE_DATA_FORMAT_INFO formatInfo = {format};
    if (FAILED(pDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_INFO, &formatInfo, sizeof(formatInfo))))
    {
        return 0;
    }

    return formatInfo.PlaneCount;
}

inline UINT D3D12CalcSubresource(UINT mipSlice, UINT arraySlice, UINT planeSlice, UINT mipLevel, UINT arraySize)
{
    return mipSlice + arraySlice * mipLevel + planeSlice * mipLevel * arraySize;
}

inline CD3DX12_RESOURCE_DESC CD3DX12_RESOURCE_DESC::Buffer(const D3D12_RESOURCE_ALLOCATION_INFO& resAllocInfo,
    D3D12_RESOURCE_FLAGS flags)
{
    return CD3DX12_RESOURCE_DESC(D3D12_RESOURCE_DIMENSION_BUFFER,
        resAllocInfo.Alignment,
        resAllocInfo.SizeInBytes,
        1,
        1,
        1,
        DXGI_FORMAT_UNKNOWN,
        1,
        0,
        D3D12_TEXTURE_LAYOUT_ROW_MAJOR, 
        flags);
}

inline CD3DX12_RESOURCE_DESC CD3DX12_RESOURCE_DESC::Buffer(UINT64 width,
    D3D12_RESOURCE_FLAGS flags,
    UINT64 alignment)
{
    return CD3DX12_RESOURCE_DESC(D3D12_RESOURCE_DIMENSION_BUFFER,
        alignment,
        width,
        1,
        1,
        1,
        DXGI_FORMAT_UNKNOWN,
        1,
        0,
        D3D12_TEXTURE_LAYOUT_ROW_MAJOR, 
        flags);
}

inline CD3DX12_RESOURCE_DESC CD3DX12_RESOURCE_DESC::Tex1D(DXGI_FORMAT format,
    UINT64 width,
    UINT16 arraySize,
    UINT16 mipLevels,
    D3D12_RESOURCE_FLAGS flags,
    D3D12_TEXTURE_LAYOUT layout,
    UINT64 alignment)
{
    return CD3DX12_RESOURCE_DESC(D3D12_RESOURCE_DIMENSION_TEXTURE1D,
        alignment,
        width,
        1,
        arraySize,
        mipLevels,
        format,
        1,
        0,
        layout, 
        flags);
}

inline CD3DX12_RESOURCE_DESC CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT format,
    UINT64 width,
    UINT height,
    UINT16 arraySize,
    UINT16 mipLevels,
    UINT sampleCount,
    UINT sampleQuality,
    D3D12_RESOURCE_FLAGS flags,
    D3D12_TEXTURE_LAYOUT layout,
    UINT64 alignment)
{
    return CD3DX12_RESOURCE_DESC(D3D12_RESOURCE_DIMENSION_TEXTURE2D,
        alignment,
        width,
        height,
        arraySize,
        mipLevels,
        format,
        sampleCount,
        sampleQuality,
        layout, 
        flags);
}

inline CD3DX12_RESOURCE_DESC CD3DX12_RESOURCE_DESC::Tex3D(DXGI_FORMAT format,
    UINT64 width,
    UINT height,
    UINT16 depth,
    UINT16 mipLevels,
    D3D12_RESOURCE_FLAGS flags,
    D3D12_TEXTURE_LAYOUT layout,
    UINT64 alignment)
{
    return CD3DX12_RESOURCE_DESC(D3D12_RESOURCE_DIMENSION_TEXTURE3D,
        alignment,
        width,
        height,        depth,
        mipLevels,
        format,
        1,
        0,
        layout, 
        flags);
}

CD3DX12_RESOURCE_DESC::CD3DX12_RESOURCE_DESC(const D3D12_RESOURCE_DESC& o)
    : D3D12_RESOURCE_DESC(o)
{}

CD3DX12_RESOURCE_DESC::CD3DX12_RESOURCE_DESC(D3D12_RESOURCE_DIMENSION dimention,
    UINT64 alignment,
    UINT64 width,
    UINT height,
    UINT16 depthOrArraySize,
    UINT16 mipLevels,
    DXGI_FORMAT format, 
    UINT sampleCount,
    UINT sampleQuality,
    D3D12_TEXTURE_LAYOUT layout,
    D3D12_RESOURCE_FLAGS flags)    
{
    Dimension = dimention;
    Alignment = alignment;
    Width = width;
    Height = height;
    DepthOrArraySize = depthOrArraySize;
    MipLevels = mipLevels;
    Format = format ;
    SampleDesc.Count = sampleCount;
    SampleDesc.Quality = sampleQuality;
    Layout = layout;
    Flags = flags;
}

inline UINT16 CD3DX12_RESOURCE_DESC::Depth(void) const
{
    return (Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D) ? (DepthOrArraySize) : (1);
}

inline UINT16 CD3DX12_RESOURCE_DESC::ArraySize(void) const
{
    return (Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D) ? (DepthOrArraySize) : (1);
}

inline UINT8 CD3DX12_RESOURCE_DESC::PlaneCount(_In_ ID3D12Device* pDevice) const
{
    return D3D12GetFormatPlaneCount(pDevice, Format);
}

inline UINT CD3DX12_RESOURCE_DESC::Subresource(_In_ ID3D12Device* pDevice) const
{
    return MipLevels * ArraySize() * PlaneCount(pDevice);
}

inline UINT CD3DX12_RESOURCE_DESC::CalcSubresource(UINT mipSlice, UINT arraySlice, UINT planeSlice)
{
    return D3D12CalcSubresource(mipSlice, arraySlice, planeSlice, MipLevels, ArraySize());
}

inline CD3DX12_RESOURCE_DESC::operator const D3D12_RESOURCE_DESC& (void) const
{
    return *this;
}

//----------------------------------------------------------------
inline void CD3DX12_ROOT_DESCRIPTOR_TABLE::Init(_Out_ D3D12_ROOT_DESCRIPTOR_TABLE& rootDescriptorTable,
    UINT numDescriptorRange,
    _In_reads_(numDescriptorRange) const D3D12_DESCRIPTOR_RANGE* pDescriptorRanges)
{
    rootDescriptorTable.NumDescriptorRanges = numDescriptorRange;
    rootDescriptorTable.pDescriptorRanges = pDescriptorRanges;
}

inline CD3DX12_ROOT_DESCRIPTOR_TABLE::CD3DX12_ROOT_DESCRIPTOR_TABLE(const D3D12_ROOT_DESCRIPTOR_TABLE& o)
    : D3D12_ROOT_DESCRIPTOR_TABLE(o)
{}

inline CD3DX12_ROOT_DESCRIPTOR_TABLE::CD3DX12_ROOT_DESCRIPTOR_TABLE(UINT numDescriptorRange, 
    _In_reads_(numDescriptorRange) const D3D12_DESCRIPTOR_RANGE* pDescriptorRanges)
{
    Init(numDescriptorRange, pDescriptorRanges);
}

inline void CD3DX12_ROOT_DESCRIPTOR_TABLE::Init(UINT numDescriptorRange,
    _In_reads_(numDescriptorRange) const D3D12_DESCRIPTOR_RANGE* pDescriptorRange)
{
    Init(*this, numDescriptorRange, pDescriptorRange);
}

//----------------------------------------------------------------
inline void CD3DX12_ROOT_CONSTANTS::Init(
    _Out_ D3D12_ROOT_CONSTANTS& rootConstant,
    UINT num32BitValue, 
    UINT shaderRegister, 
    UINT registerSpace)
{
    rootConstant.Num32BitValues = num32BitValue;
    rootConstant.ShaderRegister = shaderRegister;
    rootConstant.RegisterSpace = registerSpace;
}

inline CD3DX12_ROOT_CONSTANTS::CD3DX12_ROOT_CONSTANTS(const D3D12_ROOT_CONSTANTS& o)
    : D3D12_ROOT_CONSTANTS(o)
{}

inline CD3DX12_ROOT_CONSTANTS::CD3DX12_ROOT_CONSTANTS(UINT num32BitValue, UINT shaderRegister, UINT registerSpace)
{
    Init(num32BitValue, shaderRegister, registerSpace);
}

inline void CD3DX12_ROOT_CONSTANTS::Init(UINT num32BitValue, UINT shaderRegister, UINT registerSpace)
{
    Init(*this, num32BitValue, shaderRegister, registerSpace);
}

//----------------------------------------------------------------
inline void CD3DX12_ROOT_DESCRIPTOR::Init(_Out_ D3D12_ROOT_DESCRIPTOR& rootConstant, UINT shaderRegister, UINT registerSpace)
{
    rootConstant.ShaderRegister = shaderRegister;
    rootConstant.RegisterSpace = registerSpace;
}

inline CD3DX12_ROOT_DESCRIPTOR::CD3DX12_ROOT_DESCRIPTOR(const D3D12_ROOT_DESCRIPTOR& o)
    : D3D12_ROOT_DESCRIPTOR(o)
{}

inline CD3DX12_ROOT_DESCRIPTOR::CD3DX12_ROOT_DESCRIPTOR(UINT shaderRegister, UINT registerSpace)
{
    Init(shaderRegister, registerSpace);
}

inline void CD3DX12_ROOT_DESCRIPTOR::Init(UINT shaderRegister, UINT registerSpace)
{
    Init(*this, shaderRegister, registerSpace);
}

//----------------------------------------------------------------
inline void CD3DX12_ROOT_PARAMETER::InitAsDescriptorTable(_Out_ D3D12_ROOT_PARAMETER& rootParam,
    UINT numDescriptorRange,
    _In_reads_(numDescriptorRange) const D3D12_DESCRIPTOR_RANGE* pDescriptorRange,
    D3D12_SHADER_VISIBILITY visibility)
{
    rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParam.ShaderVisibility = visibility;
        
    CD3DX12_ROOT_DESCRIPTOR_TABLE::Init(rootParam.DescriptorTable, numDescriptorRange, pDescriptorRange);
}

inline void CD3DX12_ROOT_PARAMETER::InitAsConstant(_Out_ D3D12_ROOT_PARAMETER& rootParam,
    UINT num32bitValue,
    UINT shaderRegister,
    UINT registerSpace,
    D3D12_SHADER_VISIBILITY visibility)
{
    rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    rootParam.ShaderVisibility = visibility;
        
    CD3DX12_ROOT_CONSTANTS::Init(rootParam.Constants, num32bitValue, shaderRegister, registerSpace);
}

inline void CD3DX12_ROOT_PARAMETER::InitAsConstantBufferView(_Out_ D3D12_ROOT_PARAMETER& rootParam,
    UINT shaderRegister,
    UINT registerSpace,
    D3D12_SHADER_VISIBILITY visibility)
{
    rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParam.ShaderVisibility = visibility;

    CD3DX12_ROOT_DESCRIPTOR::Init(rootParam.Descriptor, shaderRegister, registerSpace);
}

inline void CD3DX12_ROOT_PARAMETER::InitAsShaderResourceView(_Out_ D3D12_ROOT_PARAMETER& rootParam,
    UINT shaderRegister,
    UINT registerSpace,
    D3D12_SHADER_VISIBILITY visibility)
{
    rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
    rootParam.ShaderVisibility = visibility;

    CD3DX12_ROOT_DESCRIPTOR::Init(rootParam.Descriptor, shaderRegister, registerSpace);
}

inline void CD3DX12_ROOT_PARAMETER::InitAsUnorderedaccessView(_Out_ D3D12_ROOT_PARAMETER& rootParam,
    UINT shaderRegister,
    UINT registerSpace,
    D3D12_SHADER_VISIBILITY visibility)
{
    rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
    rootParam.ShaderVisibility = visibility;

    CD3DX12_ROOT_DESCRIPTOR::Init(rootParam.Descriptor, shaderRegister, registerSpace);
}

inline CD3DX12_ROOT_PARAMETER::CD3DX12_ROOT_PARAMETER(const D3D12_ROOT_PARAMETER& o)
    : D3D12_ROOT_PARAMETER(o)
{}

inline void CD3DX12_ROOT_PARAMETER::InitAsDescriptorTable(UINT numDescriptorRange,
    _In_reads_(numDescriptorRange) const D3D12_DESCRIPTOR_RANGE* pDescriptorRange,
    D3D12_SHADER_VISIBILITY visibility)
{
    InitAsDescriptorTable(*this, numDescriptorRange, pDescriptorRange, visibility);
}

inline void CD3DX12_ROOT_PARAMETER::InitAsConstant(UINT num32bitValue,
    UINT shaderRegister,
    UINT registerSpace,
    D3D12_SHADER_VISIBILITY visibility)
{
    InitAsConstant(*this, num32bitValue, shaderRegister, registerSpace, visibility);
}

inline void CD3DX12_ROOT_PARAMETER::InitAsConstantBufferView(UINT shaderRegister,
    UINT registerSpace,
    D3D12_SHADER_VISIBILITY visibility)
{
    InitAsConstantBufferView(*this, shaderRegister, registerSpace, visibility);
}

inline void CD3DX12_ROOT_PARAMETER::InitAsShaderResourceView(UINT shaderRegister,
    UINT registerSpace,
    D3D12_SHADER_VISIBILITY visibility)
{
    InitAsShaderResourceView(*this, shaderRegister, registerSpace, visibility);
}

inline void CD3DX12_ROOT_PARAMETER::InitAsUnorderedaccessView(UINT shaderRegister,
    UINT registerSpace,
    D3D12_SHADER_VISIBILITY visibility)
{
    InitAsUnorderedaccessView(*this, shaderRegister, registerSpace, visibility);
}

//----------------------------------------------------------------
inline void CD3DX12_DESCRIPTOR_RANGE::Init(_Out_ D3D12_DESCRIPTOR_RANGE& range,
    D3D12_DESCRIPTOR_RANGE_TYPE rangeType,
    UINT numDescriptor,
    UINT baseShaderRegister,
    UINT registerSpace,
    UINT offsetInDescriptorFromTableStart)
{
    range.RangeType = rangeType;
    range.NumDescriptors = numDescriptor;
    range.BaseShaderRegister = baseShaderRegister;
    range.RegisterSpace = registerSpace;
    range.OffsetInDescriptorsFromTableStart = offsetInDescriptorFromTableStart;        
}
    
inline CD3DX12_DESCRIPTOR_RANGE::CD3DX12_DESCRIPTOR_RANGE(const D3D12_DESCRIPTOR_RANGE& o) 
    : D3D12_DESCRIPTOR_RANGE(o)
{}

inline CD3DX12_DESCRIPTOR_RANGE::CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE rangeType,
    UINT numDescriptor,
    UINT baseShaderRegister,
    UINT registerSpace,
    UINT offsetInDescriptorFromTableStart)
{
    Init(rangeType, numDescriptor, baseShaderRegister, registerSpace, offsetInDescriptorFromTableStart);
}

inline void CD3DX12_DESCRIPTOR_RANGE::Init(D3D12_DESCRIPTOR_RANGE_TYPE rangeType,
    UINT numDescriptor,
    UINT baseShaderRegister,
    UINT registerSpace,
    UINT offsetInDescriptorFromTableStart)
{
    Init(*this, rangeType, numDescriptor, baseShaderRegister, registerSpace, offsetInDescriptorFromTableStart);
}

//----------------------------------------------------------------
inline void CD3DX12_ROOT_SIGNATURE_DESC::Init(_Out_ D3D12_ROOT_SIGNATURE_DESC& desc,
    UINT numParameter,
    _In_reads_opt_(numParameter) const D3D12_ROOT_PARAMETER* pParameters,
    UINT numStaticSampler,
    _In_reads_opt_(numStaticSampler) const D3D12_STATIC_SAMPLER_DESC* pStaticSamplers,
    D3D12_ROOT_SIGNATURE_FLAGS flags)
{
    desc.NumParameters = numParameter;
    desc.pParameters = pParameters;
    desc.NumStaticSamplers = numStaticSampler;
    desc.pStaticSamplers = pStaticSamplers;
    desc.Flags = flags;
}

inline CD3DX12_ROOT_SIGNATURE_DESC::CD3DX12_ROOT_SIGNATURE_DESC(const D3D12_ROOT_SIGNATURE_DESC& o)
    : D3D12_ROOT_SIGNATURE_DESC(o)
{}

inline CD3DX12_ROOT_SIGNATURE_DESC::CD3DX12_ROOT_SIGNATURE_DESC(CD3DX12_DEFAULT def)
{
    _Unreferenced_parameter_(def);

    Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_NONE);
}

inline CD3DX12_ROOT_SIGNATURE_DESC::CD3DX12_ROOT_SIGNATURE_DESC(UINT numParameter,
    _In_reads_opt_(numParameter) const D3D12_ROOT_PARAMETER* pParameters,
    UINT numStaticSampler,
    _In_reads_opt_(numStaticSampler) const D3D12_STATIC_SAMPLER_DESC* pStaticSampler,
    D3D12_ROOT_SIGNATURE_FLAGS flags)
{
    Init(numParameter, pParameters, numStaticSampler, pStaticSampler, flags);
}

inline void CD3DX12_ROOT_SIGNATURE_DESC::Init(UINT numParameter,
    _In_reads_opt_(numParameter) const D3D12_ROOT_PARAMETER* pParameter,
    UINT numStaticSampler,
    _In_reads_opt_(numStaticSampler) const D3D12_STATIC_SAMPLER_DESC* pStaticSampler,
    D3D12_ROOT_SIGNATURE_FLAGS flags)
{
    Init(*this, numParameter, pParameter, numStaticSampler, pStaticSampler, flags);
}


//----------------------------------------------------------------
inline CD3DX12_BOX::CD3DX12_BOX(const D3D12_BOX& o)
    : D3D12_BOX(o)
{}

inline CD3DX12_BOX::CD3DX12_BOX(LONG lt, LONG rt)
{
    left = lt;
    top = 0;
    front = 0;
    right = rt;
    bottom = 1;
    back = 1;
}

inline CD3DX12_BOX::CD3DX12_BOX(LONG lt, LONG tp, LONG rt, LONG bt)
{
    left = lt;
    top = tp;
    front = 0;
    right = rt;
    bottom = bt;
    back = 1;
}

inline CD3DX12_BOX::CD3DX12_BOX(LONG lt, LONG tp, LONG fr, LONG rt, LONG bt, LONG ba)
{
    left = lt;
    top = tp;
    front = fr;
    right = rt;
    bottom = bt;
    back = ba;
}

inline CD3DX12_BOX::operator const D3D12_BOX& (void) const 
{
    return *this;
}

inline bool operator==(const D3D12_BOX& l, const D3D12_BOX& r)
{
    return (l.left == r.left) 
        && (l.top == r.top) 
        && (l.front == r.front) 
        && (l.right == r.right) 
        && (l.bottom == r.bottom) 
        && (l.back == r.back);
}

inline bool operator!=(const D3D12_BOX& l, const D3D12_BOX& r)
{
    return !(l == r);
}


//----------------------------------------------------------------
inline CD3DX12_TEXTURE_COPY_LOCATION::CD3DX12_TEXTURE_COPY_LOCATION(const D3D12_TEXTURE_COPY_LOCATION& o)
    : D3D12_TEXTURE_COPY_LOCATION(o)
{}

inline CD3DX12_TEXTURE_COPY_LOCATION::CD3DX12_TEXTURE_COPY_LOCATION(ID3D12Resource* pRes)
{
    pResource = pRes;
}

inline CD3DX12_TEXTURE_COPY_LOCATION::CD3DX12_TEXTURE_COPY_LOCATION(ID3D12Resource* pRes, const D3D12_PLACED_SUBRESOURCE_FOOTPRINT& footprint)
{
    pResource = pRes;
    Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    PlacedFootprint = footprint;
}

inline CD3DX12_TEXTURE_COPY_LOCATION::CD3DX12_TEXTURE_COPY_LOCATION(ID3D12Resource* pRes, UINT sub)
{
    pResource = pRes;
    Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    SubresourceIndex = sub;
}

//----------------------------------------------------------------
inline CD3DX12_RASTERIZER_DESC::CD3DX12_RASTERIZER_DESC(const D3D12_RASTERIZER_DESC& o)
    : D3D12_RASTERIZER_DESC(o)
{}

inline CD3DX12_RASTERIZER_DESC::CD3DX12_RASTERIZER_DESC(CD3DX12_DEFAULT)
{
    FillMode = D3D12_FILL_MODE_SOLID;
    CullMode = D3D12_CULL_MODE_BACK;
    FrontCounterClockwise = FALSE;
    DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    DepthClipEnable = TRUE;
    MultisampleEnable = FALSE;
    AntialiasedLineEnable = FALSE;
    ForcedSampleCount = 0;
    ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
}

inline CD3DX12_RASTERIZER_DESC::CD3DX12_RASTERIZER_DESC(D3D12_FILL_MODE fillmode,
    D3D12_CULL_MODE cullmode,
    BOOL frontCounterClockwise,
    INT depthBias,
    FLOAT depthBiasClamp,
    FLOAT slopeScaleDepthBias,
    BOOL depthClipEnable,
    BOOL multisampleEnable,
    BOOL antialiasedLineEnable,
    UINT forcedSampleCount,
    D3D12_CONSERVATIVE_RASTERIZATION_MODE conservativeRaster)
{
    FillMode = fillmode;
    CullMode = cullmode;
    FrontCounterClockwise = frontCounterClockwise;
    DepthBias = depthBias;
    DepthBiasClamp = depthBiasClamp;
    SlopeScaledDepthBias = slopeScaleDepthBias;
    DepthClipEnable = depthClipEnable;
    MultisampleEnable = multisampleEnable;
    AntialiasedLineEnable = antialiasedLineEnable;
    ForcedSampleCount = forcedSampleCount;
    ConservativeRaster = conservativeRaster;
}

inline CD3DX12_RASTERIZER_DESC::operator const D3D12_RASTERIZER_DESC& (void) const
{
    return *this;
}

//----------------------------------------------------------------
inline CD3DX12_BLEND_DESC::CD3DX12_BLEND_DESC(const D3D12_BLEND_DESC& o)
    : D3D12_BLEND_DESC(o)
{}

inline CD3DX12_BLEND_DESC::CD3DX12_BLEND_DESC(CD3DX12_DEFAULT def)
{
    _Unreferenced_parameter_(def);

    AlphaToCoverageEnable = FALSE;
    IndependentBlendEnable = FALSE;

    const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
    {
        FALSE, 
        FALSE,
        D3D12_BLEND_ONE,
        D3D12_BLEND_ZERO,
        D3D12_BLEND_OP_ADD,
        D3D12_BLEND_ONE,
        D3D12_BLEND_ZERO,
        D3D12_BLEND_OP_ADD,
        D3D12_LOGIC_OP_NOOP,
        D3D12_COLOR_WRITE_ENABLE_ALL,
    };

    for (UINT idx = 0; idx < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++idx)
    {
        RenderTarget[idx] = defaultRenderTargetBlendDesc;
    }
}

inline CD3DX12_BLEND_DESC::operator const D3D12_BLEND_DESC& (void) const
{
    return *this;
}

//----------------------------------------------------------------
inline CD3DX12_DEPTH_STENCIL_DESC::CD3DX12_DEPTH_STENCIL_DESC(const D3D12_DEPTH_STENCIL_DESC& o)
    : D3D12_DEPTH_STENCIL_DESC(o)
{}

inline CD3DX12_DEPTH_STENCIL_DESC::CD3DX12_DEPTH_STENCIL_DESC(CD3DX12_DEFAULT)
{
    DepthEnable = TRUE;
    DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    StencilEnable = FALSE;
    StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;

    const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp = 
    {
        D3D12_STENCIL_OP_KEEP,
        D3D12_STENCIL_OP_KEEP,
        D3D12_STENCIL_OP_KEEP,
        D3D12_COMPARISON_FUNC_ALWAYS,
    };

    FrontFace = defaultStencilOp;
    BackFace = defaultStencilOp;
}

inline CD3DX12_DEPTH_STENCIL_DESC::CD3DX12_DEPTH_STENCIL_DESC(BOOL depthEnable,
    D3D12_DEPTH_WRITE_MASK depthWriteMask,
    D3D12_COMPARISON_FUNC depthFunc,
    BOOL stencilEnable,
    UINT8 stencilReadMask,
    UINT8 stencilWriteMask,
    D3D12_STENCIL_OP frontStencilFailOp,
    D3D12_STENCIL_OP frontStencilDepthOp,
    D3D12_STENCIL_OP frontStencilPassOp,
    D3D12_COMPARISON_FUNC frontStencilFunc,
    D3D12_STENCIL_OP backStencilFailOp,
    D3D12_STENCIL_OP backStencilDepthOp,
    D3D12_STENCIL_OP backStencilPassOp,
    D3D12_COMPARISON_FUNC backStencilFunc)
{
    DepthEnable = depthEnable;
    DepthWriteMask = depthWriteMask;
    DepthFunc = depthFunc;
    StencilEnable = stencilEnable;
    StencilReadMask = stencilReadMask;
    StencilWriteMask = stencilWriteMask;
    FrontFace.StencilFailOp = frontStencilFailOp;
    FrontFace.StencilDepthFailOp = frontStencilDepthOp;
    FrontFace.StencilPassOp = frontStencilPassOp;
    FrontFace.StencilFunc = frontStencilFunc;
    BackFace.StencilFailOp = backStencilFailOp;
    BackFace.StencilDepthFailOp = backStencilDepthOp;
    BackFace.StencilPassOp = backStencilPassOp;
    BackFace.StencilFunc = backStencilFunc;
}

inline CD3DX12_DEPTH_STENCIL_DESC::operator const D3D12_DEPTH_STENCIL_DESC& (void) const
{
    return *this;
}

#endif /*__D3DX12__H__*/

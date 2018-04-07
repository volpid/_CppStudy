
#ifndef __D3DX12__H__
#define __D3DX12__H__

#include <d3d12.h>

//----------------------------------------------------------------
struct CD3DX12_DEFAULT
{};

//----------------------------------------------------------------
struct CD3DX12_CPU_DESCRIPTOR_HANDLE : public D3D12_CPU_DESCRIPTOR_HANDLE
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE(void) {}
    explicit CD3DX12_CPU_DESCRIPTOR_HANDLE(const D3D12_CPU_DESCRIPTOR_HANDLE& o)
        : D3D12_CPU_DESCRIPTOR_HANDLE(o)
    {}

    CD3DX12_CPU_DESCRIPTOR_HANDLE(CD3DX12_DEFAULT) 
    {
        ptr = 0;
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other, INT offsetScaledByIncrementSize)
    {
        InitOffsetted(other, offsetScaledByIncrementSize);
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other, INT offsetInDescriptors, UINT descriptorIncrementSize)
    {
        InitOffsetted(other, offsetInDescriptors, descriptorIncrementSize);
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE& Offset(INT offsetInDescriptors, UINT descriptorIncrementSize)
    {
        ptr += offsetInDescriptors * descriptorIncrementSize;
        return *this;
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE& Offset(UINT offsetScaledByIncrementSize)
    {
        ptr += offsetScaledByIncrementSize;
        return *this;
    }

    bool operator==(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other)
    {
        return (ptr == other.ptr);
    }

    bool operator!=(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other)
    {
        return (ptr != other.ptr);
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE& operator=(const CD3DX12_CPU_DESCRIPTOR_HANDLE& other)
    {
        ptr = other.ptr;
        return *this;
    }

    inline void InitOffsetted(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& base, INT offsetScaledByIncrementSize)
    {
        InitOffsetted(*this, base, offsetScaledByIncrementSize);
    }

    inline void InitOffsetted(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& base, INT offsetInDescriptors, UINT descriptorIncrementSize)
    {
        InitOffsetted(*this, base, offsetInDescriptors, descriptorIncrementSize);
    }

    static inline void InitOffsetted(_Out_ D3D12_CPU_DESCRIPTOR_HANDLE& handle, _In_ const D3D12_CPU_DESCRIPTOR_HANDLE& base, INT offsetScaledByIncrementSize)
    {
        handle.ptr = base.ptr + offsetScaledByIncrementSize;
    }

    static inline void InitOffsetted(_Out_ D3D12_CPU_DESCRIPTOR_HANDLE& handle, _In_ const D3D12_CPU_DESCRIPTOR_HANDLE& base, INT offsetInDescriptors, UINT descriptorIncrementSize)
    {
        handle.ptr = base.ptr + offsetInDescriptors * descriptorIncrementSize;
    }    
};

//----------------------------------------------------------------
struct CD3DX12_HEAP_PROPERTIES : public D3D12_HEAP_PROPERTIES
{
    CD3DX12_HEAP_PROPERTIES(void)
    {}

    explicit CD3DX12_HEAP_PROPERTIES(const D3D12_HEAP_PROPERTIES& o)
    {}

    CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY cpuPageProperty,
        D3D12_MEMORY_POOL memoryPoolPrepference,
        UINT creationNModeMask = 1,
        UINT nodeMask = 1)
    {
        Type = D3D12_HEAP_TYPE_CUSTOM;
        CPUPageProperty = cpuPageProperty;
        MemoryPoolPreference = memoryPoolPrepference;
        CreationNodeMask = creationNModeMask;
        VisibleNodeMask = nodeMask;
    }

    explicit CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE type, 
        UINT creationNModeMask = 1,
        UINT nodeMask = 1)
    {
        Type = type;
        CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        CreationNodeMask = creationNModeMask;
        VisibleNodeMask = nodeMask;
    }

    operator const CD3DX12_HEAP_PROPERTIES&(void) const
    {
        return *this;
    }

    bool IsCPUAccessible(void) const
    {
        return (Type == D3D12_HEAP_TYPE_UPLOAD)
            || (Type == D3D12_HEAP_TYPE_READBACK)
            || ((Type == D3D12_HEAP_TYPE_CUSTOM)
            && (CPUPageProperty == D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE 
            || CPUPageProperty == D3D12_CPU_PAGE_PROPERTY_WRITE_BACK));
    }
};

//----------------------------------------------------------------
struct CD3DX12_RESOURCE_BARRIER : public D3D12_RESOURCE_BARRIER
{
    CD3DX12_RESOURCE_BARRIER(void)
    {}

    explicit CD3DX12_RESOURCE_BARRIER(const D3D12_RESOURCE_BARRIER& o)
        : D3D12_RESOURCE_BARRIER(o)
    {}

    static inline CD3DX12_RESOURCE_BARRIER Transition(_In_ ID3D12Resource* pResource,
        D3D12_RESOURCE_STATES stateBefore,
        D3D12_RESOURCE_STATES stateAfter,
        UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
        D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE)
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

    static inline CD3DX12_RESOURCE_BARRIER Aliasing(_In_ ID3D12Resource* pResourceBefore,
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

    static inline CD3DX12_RESOURCE_BARRIER UAV(_In_ ID3D12Resource* pResource)
    {
        CD3DX12_RESOURCE_BARRIER result;
        ZeroMemory(&result, sizeof(result));
        D3D12_RESOURCE_BARRIER& barrier = result;
        result.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.UAV.pResource = pResource;

        return result;
    }
};

#endif /*__D3DX12__H__*/

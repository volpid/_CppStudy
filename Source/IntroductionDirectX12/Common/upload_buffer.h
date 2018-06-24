
#ifndef __UPLOAD_BUFFER__H__
#define __UPLOAD_BUFFER__H__

#include "d3d_util.h"

//----------------------------------------------------------------
// UplaodBuffer
//----------------------------------------------------------------
template <typename _Type>
class UplaodBuffer
{
public:
    UplaodBuffer(ID3D12Device* pDevice, UINT elementCount, bool isConstantbuffer);
    ~UplaodBuffer(void);

    UplaodBuffer(const UplaodBuffer& other) = delete;
    UplaodBuffer& operator=(const UplaodBuffer& other) = delete;

    ID3D12Resource* Resource(void) const;
    void CopyData(int elementIndex, const _Type& data);

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer_ = nullptr;
    BYTE* mappedData_ = nullptr;

    UINT elementByteSize_ = 0;
    bool isConstantBuffer_ = false;
};

//----------------------------------------------------------------
template <typename _Type>
inline UplaodBuffer<_Type>::UplaodBuffer(ID3D12Device* pDevice, UINT elementCount, bool isConstantbuffer)
    : isConstantBuffer_(isConstantbuffer)
{
    elementByteSize_ = sizeof(_Type);
    if (isConstantbuffer == true)
    {
        elementByteSize_ = D3DUtil::CalcConstantBufferByteSize(sizeof(_Type));
    }

    ThrowIfFailed(pDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(elementByteSize_ * elementCount),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&uploadBuffer_)));

    ThrowIfFailed(uploadBuffer_->Map(0, nullptr, reinterpret_cast<void**> (&mappedData_)));
}

template <typename _Type>
inline UplaodBuffer<_Type>::~UplaodBuffer(void)
{
    if (uploadBuffer_ != nullptr)
    {
        uploadBuffer_->Unmap(0, nullptr);
    }
    mappedData_ = nullptr;
}

template <typename _Type>
inline ID3D12Resource* UplaodBuffer<_Type>::Resource(void) const
{
    return uploadBuffer_.Get();
}

template <typename _Type>
inline void UplaodBuffer<_Type>::CopyData(int elementIndex, const _Type& data)
{
    memcpy(&mappedData_[elementIndex * elementByteSize_], &data, sizeof(_Type));
}

#endif /*__UPLOAD_BUFFER__H__*/
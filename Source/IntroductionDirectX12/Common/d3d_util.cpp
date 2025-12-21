
#include "d3d_util.h"

#include <comdef.h>

//----------------------------------------------------------------
// D3DUtil function
//----------------------------------------------------------------
UINT D3DUtil::CalcConstantBufferByteSize(UINT byteSize)
{
    return (byteSize + 0xff) & (~0xff);
}

Microsoft::WRL::ComPtr<ID3D12Resource> D3DUtil::CreataDefaultBuffer(ID3D12Device* pDevice,
    ID3D12GraphicsCommandList* pCmdList,
    const void* pInitData,
    UINT64 byteSize,
    Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer)
{
    Microsoft::WRL::ComPtr<ID3D12Resource> defaultBuffer;    
    CD3DX12_HEAP_PROPERTIES heapPropertyDefault = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC resourceDescDefault = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
    ThrowIfFailed(pDevice->CreateCommittedResource(&heapPropertyDefault,
        D3D12_HEAP_FLAG_NONE,
        &resourceDescDefault,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(defaultBuffer.GetAddressOf())));

    CD3DX12_HEAP_PROPERTIES heapPropertyUpload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC resourceDescUpload = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
    ThrowIfFailed(pDevice->CreateCommittedResource(&heapPropertyUpload,
        D3D12_HEAP_FLAG_NONE,
        &resourceDescUpload,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(uploadBuffer.GetAddressOf())));

    D3D12_SUBRESOURCE_DATA subResourceData = {};
    subResourceData.pData = pInitData;
    subResourceData.RowPitch = static_cast<LONG_PTR>(byteSize);
    subResourceData.SlicePitch = subResourceData.RowPitch;

    CD3DX12_RESOURCE_BARRIER barrierCommontToCopyDest = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(), 
        D3D12_RESOURCE_STATE_COMMON, 
        D3D12_RESOURCE_STATE_COPY_DEST);
    pCmdList->ResourceBarrier(1, &barrierCommontToCopyDest);

    UpdateSubresource<1>(pCmdList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);

    CD3DX12_RESOURCE_BARRIER barrierCopyDescToGenericRead = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST, 
        D3D12_RESOURCE_STATE_GENERIC_READ);
    pCmdList->ResourceBarrier(1, &barrierCopyDescToGenericRead);

    return defaultBuffer;
}

Microsoft::WRL::ComPtr<ID3DBlob> D3DUtil::CompileShader(const std::string& filename, 
    const D3D_SHADER_MACRO* pDefines,
    const std::string& entryPoint, 
    const std::string& target)
{
    UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif /**/

    //nanpid fix codecvt
    std::wstring wfilename;
    wfilename.assign(filename.begin(), filename.end());        
    
    Microsoft::WRL::ComPtr<ID3DBlob> bytecodeBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3DCompileFromFile(wfilename.c_str(), 
        pDefines,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint.c_str(),
        target.c_str(),
        compileFlags,
        0,
        &bytecodeBlob,
        &errorBlob);

    if (errorBlob != nullptr)
    {   
        OutputDebugString(static_cast<char*> (errorBlob->GetBufferPointer()));
    }
    ThrowIfFailed(hr);

    return bytecodeBlob;
}

//----------------------------------------------------------------
// DxException
//----------------------------------------------------------------
DxException::DxException(HRESULT hr, const std::string& functionName, const std::string& filename, int lineNumber)
    : errorCode_(hr),
    functionName_(functionName),
    filename_(filename),
    lineNumber_(lineNumber)
{}

std::string DxException::ToString(void) const
{
    _com_error err(errorCode_);
    std::string msg = err.ErrorMessage();

    return functionName_ 
        + " failed in " 
        + filename_ 
        + "; line " 
        + std::to_string(lineNumber_)
        + "; error: "
        + msg;
}

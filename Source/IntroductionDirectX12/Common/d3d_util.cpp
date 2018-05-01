
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
    ID3D12GraphicsCommandList* cmdList,
    const void* initData,
    UINT64 byteSize,
    Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer)
{
    Microsoft::WRL::ComPtr<ID3D12Resource> defaultBuffer;
    ThrowIfFailed(pDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(byteSize),
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(defaultBuffer.GetAddressOf())));

    ThrowIfFailed(pDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(byteSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(uploadBuffer.GetAddressOf())));

    D3D12_SUBRESOURCE_DATA subResourceData = {};
    subResourceData.pData = initData;
    subResourceData.RowPitch = byteSize;
    subResourceData.SlicePitch = subResourceData.RowPitch;

    cmdList->ResourceBarrier(1, 
        &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
            D3D12_RESOURCE_STATE_COMMON, 
            D3D12_RESOURCE_STATE_COPY_DEST));

    UpdateSubresource<1>(cmdList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);

    cmdList->ResourceBarrier(1, 
        &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST, 
            D3D12_RESOURCE_STATE_GENERIC_READ));

    return defaultBuffer;
}

Microsoft::WRL::ComPtr<ID3DBlob> D3DUtil::CompileShader(const std::string& filename, 
    const D3D_SHADER_MACRO* defines,
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
        defines,
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


#ifndef __D3D_UTIL__H__
#define __D3D_UTIL__H__

#include "d3dx_12.h"

#include <string>
#include <unordered_map>
#include <wrl/client.h>

//----------------------------------------------------------------
// D3DUtil function
//----------------------------------------------------------------
namespace D3DUtil
{
    UINT CalcConstantBufferByteSize(UINT byteSize);

    Microsoft::WRL::ComPtr<ID3D12Resource> CreataDefaultBuffer(ID3D12Device* pDevice,
        ID3D12GraphicsCommandList* cmdList,
        const void* initData,
        UINT64 byteSize,
        Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer);

    Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(const std::string& filename, 
        const D3D_SHADER_MACRO* defines,
        const std::string& entryPoint, 
        const std::string& target);
}

//----------------------------------------------------------------
// mesh geometry
//----------------------------------------------------------------
struct SubmeshGeometry
{
    UINT indexCount = 0;
    UINT startIndexLocation = 0;
    INT baseVertexLocation = 0;

    DirectX::BoundingBox boundbox;
};

struct MeshGeometry
{
    std::string name;

    Microsoft::WRL::ComPtr<ID3DBlob> vertexBufferCPU = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> indexBufferCPU = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferGPU = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferGPU = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferUploader = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferUploader = nullptr;

    UINT vertexByteStride = 0;
    UINT vertexBufferByteSize = 0;
    DXGI_FORMAT indexFormat = DXGI_FORMAT_R16_UINT;
    UINT indexBufferByteSize = 0;

    std::unordered_map<std::string, SubmeshGeometry> drawArgs;

    D3D12_VERTEX_BUFFER_VIEW VertexBufferView(void) const
    {
        D3D12_VERTEX_BUFFER_VIEW vbv;
        vbv.BufferLocation = vertexBufferGPU->GetGPUVirtualAddress();
        vbv.StrideInBytes = vertexByteStride;
        vbv.SizeInBytes = vertexBufferByteSize;
        return vbv;
    }

    D3D12_INDEX_BUFFER_VIEW IndexBufferView(void) const
    {
        D3D12_INDEX_BUFFER_VIEW ibv;
        ibv.BufferLocation = indexBufferGPU->GetGPUVirtualAddress();
        ibv.Format = indexFormat;
        ibv.SizeInBytes = indexBufferByteSize;
        return ibv;
    }

    void DisposeUploader(void)
    {
        vertexBufferUploader = nullptr;
        indexBufferUploader = nullptr;
    }
};

//----------------------------------------------------------------
// DxException
//----------------------------------------------------------------
class DxException
{
public:
    DxException(void) = default;
    DxException(HRESULT hr, const std::string& functionName, const std::string& filename, int lineNumber);

    std::string ToString(void) const;

    HRESULT errorCode_ = S_OK;
    std::string functionName_;
    std::string filename_;
    int lineNumber_ = -1;
};

//----------------------------------------------------------------
// macro
//----------------------------------------------------------------

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)    \
    {   \
        HRESULT hr__ = (x); \
        std::string fn = __FILE__;  \
        if (FAILED(hr__))   \
        {   \
            throw DxException(hr__, #x, fn, __LINE__);  \
        }   \
    }
#endif /**/

#ifndef ReleaseCom
#define ReleaseCom(x)   \
    {   \
        if (x) \
        {   \
            x->Release();   \
            x = 0;  \
        }   \
    }
#endif /**/

#endif /*__D3D_UTIL__H__*/
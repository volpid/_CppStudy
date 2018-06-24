
#ifndef __GEOMETRY_GENERATOR__H__
#define __GEOMETRY_GENERATOR__H__

#include "d3dx_12.h"

#include <cstdint>
#include <vector>

//----------------------------------------------------------------
// GeometryGenerator
//----------------------------------------------------------------
class GeometryGenerator
{
public:
    using uint16 = std::uint16_t;
    using uint32 = std::uint32_t;

    struct Vertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT3 normal;
        DirectX::XMFLOAT3 tangent;
        DirectX::XMFLOAT2 texcoord;

        Vertex(void) = default;
        Vertex(const DirectX::XMFLOAT3& p, const DirectX::XMFLOAT3& n, const DirectX::XMFLOAT3& t, const DirectX::XMFLOAT2& uv)
            : position(p), normal(n), tangent(t), texcoord(uv)
        {}
        Vertex(float px, float py, float pz,
            float nx, float ny, float nz,
            float tx, float ty, float tz,
            float u, float v)
            : position(px, py, pz), normal(nx, ny, nz), tangent(tx, ty, tz), texcoord(u, v)
        {}
    };

    struct MeshData
    {
        std::vector<Vertex> vertices;        
        std::vector<uint32> indices32;

        std::vector<uint16> indices16;
        
        std::vector<uint16>& GetIndex16(void)
        {
            indices16.resize(indices32.size());
            for (size_t idx = 0; idx < indices32.size(); ++idx)
            {
                indices16[idx] = static_cast<uint16> (indices32[idx]);
            }
            return indices16;
        }
    };


public:
    MeshData CreateBox(float width, float height, float depth, uint32 numSubdivision);
    MeshData CreateSphere(float radius, uint32 sliceCount, uint32 stackCount);
    MeshData CreateGeoSphere(float radius, uint32 numSubdivision);
    MeshData CreateCylinder(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount);
    MeshData CreateGrid(float width, float depth, uint32 m, uint32 n);
    MeshData CreateQuad(float x, float y, float w, float h, float depth);

private:
    void Subdivide(MeshData& meshData);
    Vertex MidPoint(const Vertex& v0, const Vertex& v1);
    void BuildCylinderTopCap(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount, MeshData& meshData);
    void BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount, MeshData& meshData);
};

#endif /*__GEOMETRY_GENERATOR__H__*/

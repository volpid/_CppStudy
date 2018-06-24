
#include "geometry_generator.h"

#include "math_helper.h"

#include <algorithm>

//----------------------------------------------------------------
// GeometryGenerator
//----------------------------------------------------------------
GeometryGenerator::MeshData GeometryGenerator::CreateBox(float width, float height, float depth, uint32 numSubdivision)
{
    MeshData meshData;

    Vertex v[24];
    float w2 = 0.5f * width;
    float h2 = 0.5f * height;
    float d2 = 0.5f * depth;

    //front 
    v[0] = Vertex(-w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    v[1] = Vertex(-w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    v[2] = Vertex(+w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    v[3] = Vertex(+w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
    // back
    v[4] = Vertex(-w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
    v[5] = Vertex(+w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    v[6] = Vertex(+w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    v[7] = Vertex(-w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    // top
    v[8] = Vertex(-w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    v[9] = Vertex(-w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    v[10] = Vertex(+w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    v[11] = Vertex(+w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
    // bottom
    v[12] = Vertex(-w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
    v[13] = Vertex(+w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    v[14] = Vertex(+w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    v[15] = Vertex(-w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    // left
    v[16] = Vertex(-w2, -h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
    v[17] = Vertex(-w2, +h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
    v[18] = Vertex(-w2, +h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
    v[19] = Vertex(-w2, -h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);
    // right
    v[20] = Vertex(+w2, -h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
    v[21] = Vertex(+w2, +h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
    v[22] = Vertex(+w2, +h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
    v[23] = Vertex(+w2, -h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

    meshData.vertices.assign(&v[0], &v[24]);

    uint32 i[36];
    // front
    i[0] = 0; i[1] = 1; i[2] = 2;
    i[3] = 0; i[4] = 2; i[5] = 3;
    // back
    i[6] = 4; i[7] = 5; i[8] = 6;
    i[9] = 4; i[10] = 6; i[11] = 7;
    // top
    i[12] = 8; i[13] = 9; i[14] = 10;
    i[15] = 8; i[16] = 10; i[17] = 11;
    // bottom
    i[18] = 12; i[19] = 13; i[20] = 14;
    i[21] = 12; i[22] = 14; i[23] = 15;
    // left
    i[24] = 16; i[25] = 17; i[26] = 18;
    i[27] = 16; i[28] = 18; i[29] = 19;
    // right
    i[30] = 20; i[31] = 21; i[32] = 22;
    i[33] = 20; i[34] = 22; i[35] = 23;

    meshData.indices32.assign(&i[0], &i[36]);

    numSubdivision = std::min<uint32> (numSubdivision, 6u);

    for (uint32 idx = 0; idx < numSubdivision; ++idx)
    {
        Subdivide(meshData);
    }

    return meshData;
}

GeometryGenerator::MeshData GeometryGenerator::CreateSphere(float radius, uint32 sliceCount, uint32 stackCount)
{
    MeshData meshData;

    // vertex
    Vertex topVertex(0.0f, +radius, 0.0f, 0.0f, +1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    Vertex bottomVertex(0.0f, -radius, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    
    meshData.vertices.push_back(topVertex);

    float phiStep = MathHelper::Pi / stackCount;
    float thetaStep = 2.0f * MathHelper::Pi / stackCount;
    for (uint32 stackIdx = 1; stackIdx <= stackCount - 1; ++stackIdx)
    {
        float phi = stackIdx * phiStep;
        for (uint32 sliceIdx = 0; sliceIdx <= sliceCount; ++sliceIdx)
        {
            float theta = sliceIdx * thetaStep;
            
            Vertex v;
            v.position.x = radius * sinf(phi) * cosf(theta);
            v.position.y = radius * cosf(phi);
            v.position.z = radius * sinf(phi) * sinf(theta);
            v.tangent.x = -radius * sinf(phi) * sinf(theta);
            v.tangent.y = 0.0f;
            v.tangent.z = radius * sinf(phi) * cosf(theta);

            DirectX::XMVECTOR t = DirectX::XMLoadFloat3(&v.tangent);
            DirectX::XMStoreFloat3(&v.tangent, DirectX::XMVector3Normalize(t));
            DirectX::XMVECTOR p = DirectX::XMLoadFloat3(&v.position);
            DirectX::XMStoreFloat3(&v.normal, DirectX::XMVector3Normalize(p));

            v.texcoord.x = theta / (MathHelper::Pi * 2);
            v.texcoord.y = phi / MathHelper::Pi;

            meshData.vertices.push_back(v);
        }
    }

    meshData.vertices.push_back(bottomVertex);

    // index
    for (uint32 idx = 1; idx <= sliceCount; ++idx)
    {
        meshData.indices32.push_back(0);
        meshData.indices32.push_back(idx + 1);
        meshData.indices32.push_back(idx);
    }

    uint32 baseIndex = 1;
    uint32 ringVertexCount = sliceCount + 1;
    for (uint32 stackIdx = 0; stackIdx < stackCount - 2; ++stackIdx)
    {
        for (uint32 sliceIdx = 0; sliceIdx < sliceCount; ++sliceIdx)
        {
            meshData.indices32.push_back(baseIndex + stackIdx * ringVertexCount + sliceIdx);
            meshData.indices32.push_back(baseIndex + stackIdx * ringVertexCount + sliceIdx + 1);
            meshData.indices32.push_back(baseIndex + (stackIdx + 1) * ringVertexCount + sliceIdx);

            meshData.indices32.push_back(baseIndex + (stackIdx + 1) * ringVertexCount + sliceIdx);
            meshData.indices32.push_back(baseIndex + stackIdx * ringVertexCount + sliceIdx + 1);
            meshData.indices32.push_back(baseIndex + (stackIdx + 1) * ringVertexCount + sliceIdx + 1);
        }
    }

    uint32 southpoleIndex = (uint32) meshData.vertices.size() - 1;
    baseIndex = southpoleIndex - ringVertexCount;

    for (uint32 idx = 0; idx < sliceCount; ++idx)
    {
        meshData.indices32.push_back(southpoleIndex);
        meshData.indices32.push_back(baseIndex + idx);
        meshData.indices32.push_back(baseIndex + idx + 1);
    }

    return meshData;
}

GeometryGenerator::MeshData GeometryGenerator::CreateGeoSphere(float radius, uint32 numSubdivision)
{
    MeshData meshData;
    numSubdivision = std::min<uint32>(numSubdivision, 6u);

    const float X = 0.525731f;
    const float Z = 0.850651f;

    DirectX::XMFLOAT3 pos[12] = 
    {
        DirectX::XMFLOAT3(-X, 0.0f, Z), 
        DirectX::XMFLOAT3(X, 0.0f, Z), 
        DirectX::XMFLOAT3(-X, 0.0f, -Z), 
        DirectX::XMFLOAT3(X, 0.0f, -Z),
        DirectX::XMFLOAT3(0.0f, Z, X),
        DirectX::XMFLOAT3(0.0f, Z, -X),
        DirectX::XMFLOAT3(0.0f, -Z, X),
        DirectX::XMFLOAT3(0.0f, -Z, -X),
        DirectX::XMFLOAT3(Z, X, 0.0f),
        DirectX::XMFLOAT3(-Z, X, 0.0f),
        DirectX::XMFLOAT3(Z, -X, 0.0f),
        DirectX::XMFLOAT3(-Z, -X, 0.0f),
    };

    uint32 k[60] = 
    {
        1, 4, 0,
        4, 9, 0,
        4, 5, 9,
        8, 5, 4,
        1, 8, 4,
        
        1, 10, 8, 
        10, 3, 8,
        8, 3, 5, 
        3, 2, 5,
        3, 7, 2,

        3, 10, 7, 
        10, 6, 7, 
        6, 11, 7,
        6, 0, 11,
        6, 1, 0, 

        10, 1, 6,
        11, 0, 9,
        2, 11, 9, 
        5, 2, 9,
        11, 2, 7
    };

    meshData.vertices.resize(12);
    for (uint32 idx = 0; idx < 12; ++idx)
    {
        meshData.vertices[idx].position = pos[idx];
    }
    meshData.indices32.assign(&k[0], &k[60]);

    for (uint32 idx = 0; idx < numSubdivision; ++idx)
    {
        Subdivide(meshData);
    }

    for (uint32 idx = 0; idx < meshData.vertices.size(); ++idx)
    {
        DirectX::XMVECTOR n = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&meshData.vertices[idx].position));
        DirectX::XMVECTOR p = DirectX::operator*(radius, n);

        DirectX::XMStoreFloat3(&meshData.vertices[idx].position, p);
        DirectX::XMStoreFloat3(&meshData.vertices[idx].normal, n);

        float theta = atan2f(meshData.vertices[idx].position.z, meshData.vertices[idx].position.x);
        if (theta < 0)
        {
            theta += (MathHelper::Pi * 2);
        }

        float phi =  acosf(meshData.vertices[idx].position.y / radius);
        meshData.vertices[idx].texcoord.x = theta / (MathHelper::Pi * 2);
        meshData.vertices[idx].texcoord.y = phi / (MathHelper::Pi);

        meshData.vertices[idx].tangent.x = -radius * sinf(phi) * sinf(theta);
        meshData.vertices[idx].tangent.y = 0.0f;
        meshData.vertices[idx].tangent.z = -radius * sinf(phi) * cosf(theta);

        DirectX::XMVECTOR t = DirectX::XMLoadFloat3(&meshData.vertices[idx].tangent);
        DirectX::XMStoreFloat3(&meshData.vertices[idx].tangent, DirectX::XMVector3Normalize(t));
    }

    return meshData;
}

GeometryGenerator::MeshData GeometryGenerator::CreateCylinder(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount)
{
    MeshData meshData;

    float stackHeight = height / stackCount;
    float radiusStep = (topRadius - bottomRadius) / stackCount;
    uint32 ringCount = stackCount + 1;

    for (uint32 ringIdx = 0; ringIdx < ringCount; ++ringIdx)
    {
        float y = -0.5f * height + ringIdx * stackHeight;
        float r = bottomRadius + ringIdx * radiusStep;
        float dTheta = 2.0f * MathHelper::Pi / sliceCount;

        for (uint32 sliceIdx = 0; sliceIdx <= sliceCount; ++sliceIdx)
        {
            Vertex vertex;

            float c = cosf(sliceIdx * dTheta);
            float s = sinf(sliceIdx * dTheta);
            vertex.position = DirectX::XMFLOAT3(r * c, y, r * s);
            vertex.texcoord.x = (float) sliceIdx / sliceCount;
            vertex.texcoord.y = 1.0f - ((float) ringCount / stackCount);
            vertex.tangent = DirectX::XMFLOAT3(-s, 0.0f, c);
            float dr = bottomRadius - topRadius;
            DirectX::XMFLOAT3 bitangent(dr * c, -height, dr * s);

            DirectX::XMVECTOR t = DirectX::XMLoadFloat3(&vertex.tangent);
            DirectX::XMVECTOR b = DirectX::XMLoadFloat3(&bitangent);
            DirectX::XMVECTOR n = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(t, b));
            DirectX::XMStoreFloat3(&vertex.normal, n);

            meshData.vertices.push_back(vertex);
        }
    }

    uint32 ringVertexCount = sliceCount + 1;
    for (uint32 stackIdx = 0; stackIdx < stackCount; ++stackIdx)
    {
        for (uint32 sliceIdx = 0; sliceIdx < sliceCount; ++sliceIdx)
        {
            meshData.indices32.push_back(stackIdx * ringVertexCount + sliceIdx);
            meshData.indices32.push_back((stackIdx + 1) * ringVertexCount + sliceIdx);
            meshData.indices32.push_back((stackIdx + 1) * ringVertexCount + sliceIdx + 1);

            meshData.indices32.push_back(stackIdx * ringVertexCount + sliceIdx);
            meshData.indices32.push_back((stackIdx + 1) * ringVertexCount + sliceIdx + 1);
            meshData.indices32.push_back(stackIdx * ringVertexCount + sliceIdx + 1);
        }
    }

    BuildCylinderTopCap(bottomRadius, topRadius, height, sliceCount, stackCount, meshData);
    BuildCylinderBottomCap(bottomRadius, topRadius, height, sliceCount, stackCount, meshData);

    return meshData;
}

GeometryGenerator::MeshData GeometryGenerator::CreateGrid(float width, float depth, uint32 m, uint32 n)
{
    MeshData meshData;

    uint32 vertexCount = m * n;
    uint32 faceCount = (m - 1) * (n - 1) * 2;

    float halfWidth = 0.5f * width;
    float halfDepth = 0.5f * depth;
    float dx = width / (n - 1);
    float dz = depth / (m - 1);
    float du = 1.0f / (n - 1);
    float dv = 1.0f / (m - 1);

    meshData.vertices.resize(vertexCount);
    for (uint32 rowIdx = 0; rowIdx < m; ++rowIdx)
    {
        float z = halfDepth - (rowIdx * dz);
        for (uint32 colIdx = 0; colIdx < n; ++colIdx)
        {
            float x = -halfWidth + (colIdx * dx);
            
            meshData.vertices[n * rowIdx + colIdx].position = DirectX::XMFLOAT3(x, 0.0f, z);
            meshData.vertices[n * rowIdx + colIdx].normal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
            meshData.vertices[n * rowIdx + colIdx].tangent = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);

            meshData.vertices[n * rowIdx + colIdx].texcoord.x = du * colIdx;
            meshData.vertices[n * rowIdx + colIdx].texcoord.x = dv * rowIdx;
        }
    }

    meshData.indices32.resize(faceCount * 3);
    uint32 index = 0;
    for (uint32 rowIdx = 0; rowIdx < m - 1; ++rowIdx)
    {
        for (uint32 colIdx = 0; colIdx < n - 1; ++colIdx)
        {
            meshData.indices32[index] = rowIdx * n + colIdx;
            meshData.indices32[index + 1] = rowIdx * n + colIdx + 1;
            meshData.indices32[index + 2] = (rowIdx + 1) * n + colIdx;

            meshData.indices32[index + 3] = (rowIdx + 1) * n + colIdx;
            meshData.indices32[index + 4] = rowIdx * n + colIdx + 1;
            meshData.indices32[index + 5] = (rowIdx + 1) * n + colIdx + 1;

            index += 6;
        }
    }

    return meshData;
}
GeometryGenerator::MeshData GeometryGenerator::CreateQuad(float x, float y, float w, float h, float depth)
{
    MeshData meshData;

    meshData.vertices.resize(4);
    meshData.indices32.reserve(6);

    meshData.vertices[0] = Vertex(x, y - h, depth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    meshData.vertices[1] = Vertex(x, y, depth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    meshData.vertices[2] = Vertex(x + w, y, depth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    meshData.vertices[3] = Vertex(x + w, y - h, depth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

    meshData.indices32[0] = 0;
    meshData.indices32[1] = 1;
    meshData.indices32[2] = 2;

    meshData.indices32[3] = 0;
    meshData.indices32[4] = 2;
    meshData.indices32[5] = 3;

    return meshData;
}

void GeometryGenerator::Subdivide(MeshData& meshData)
{
    MeshData inputCopy = meshData;
    meshData.vertices.resize(0);
    meshData.indices32.resize(0);

    uint32 numTris = (uint32) inputCopy.indices32.size() / 3;
    for (uint32 idx = 0; idx < numTris; ++idx)
    {
        Vertex v0 = inputCopy.vertices[inputCopy.indices32[idx * 3 + 0]];
        Vertex v1 = inputCopy.vertices[inputCopy.indices32[idx * 3 + 1]];
        Vertex v2 = inputCopy.vertices[inputCopy.indices32[idx * 3 + 2]];

        Vertex m0 = MidPoint(v0, v1);
        Vertex m1 = MidPoint(v1, v2);
        Vertex m2 = MidPoint(v0, v2);

        meshData.vertices.push_back(v0);
        meshData.vertices.push_back(v1);
        meshData.vertices.push_back(v2);
        meshData.vertices.push_back(m0);
        meshData.vertices.push_back(m1);
        meshData.vertices.push_back(m2);

        meshData.indices32.push_back(idx * 6 + 0);
        meshData.indices32.push_back(idx * 6 + 3);
        meshData.indices32.push_back(idx * 6 + 5);

        meshData.indices32.push_back(idx * 6 + 3);
        meshData.indices32.push_back(idx * 6 + 4);
        meshData.indices32.push_back(idx * 6 + 5);

        meshData.indices32.push_back(idx * 6 + 5);
        meshData.indices32.push_back(idx * 6 + 4);
        meshData.indices32.push_back(idx * 6 + 2);

        meshData.indices32.push_back(idx * 6 + 3);
        meshData.indices32.push_back(idx * 6 + 1);
        meshData.indices32.push_back(idx * 6 + 4);
    }
}

GeometryGenerator::Vertex GeometryGenerator::MidPoint(const Vertex& v0, const Vertex& v1)
{
    DirectX::XMVECTOR p0 = DirectX::XMLoadFloat3(&v0.position);
    DirectX::XMVECTOR p1 = DirectX::XMLoadFloat3(&v1.position);

    DirectX::XMVECTOR n0 = DirectX::XMLoadFloat3(&v0.normal);
    DirectX::XMVECTOR n1 = DirectX::XMLoadFloat3(&v1.normal);

    DirectX::XMVECTOR tan0 = DirectX::XMLoadFloat3(&v0.tangent);
    DirectX::XMVECTOR tan1 = DirectX::XMLoadFloat3(&v1.tangent);

    DirectX::XMVECTOR tex0 = DirectX::XMLoadFloat2(&v0.texcoord);
    DirectX::XMVECTOR tex1 = DirectX::XMLoadFloat2(&v1.texcoord);
    
    DirectX::XMVECTOR position = DirectX::operator*(0.5f, (DirectX::operator+(p0, p1)));
    DirectX::XMVECTOR normal = DirectX::XMVector3Normalize(DirectX::operator*(0.5f, DirectX::operator+(n0, n1)));
    DirectX::XMVECTOR tangent = DirectX::XMVector3Normalize(DirectX::operator*(0.5f, DirectX::operator+(tan0, tan1)));
    DirectX::XMVECTOR tex = DirectX::operator*(0.5f, DirectX::operator+(tex0, tex1));

    Vertex v;
    DirectX::XMStoreFloat3(&v.position, position);
    DirectX::XMStoreFloat3(&v.normal, normal);
    DirectX::XMStoreFloat3(&v.tangent, tangent);
    DirectX::XMStoreFloat2(&v.texcoord, tex);

    return v;
}

void GeometryGenerator::BuildCylinderTopCap(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount, MeshData& meshData)
{
    _Unreferenced_parameter_(bottomRadius);
    _Unreferenced_parameter_(stackCount);

    uint32 baseIndex = (uint32) meshData.vertices.size();

    float y = 0.5f * height;
    float dTheta = 2.0f * MathHelper::Pi / sliceCount;

    for (uint32 sliceIdx = 0; sliceIdx <= sliceCount; ++sliceIdx)
    {
        float x = topRadius * cosf(sliceIdx * dTheta);
        float z = topRadius * sinf(sliceIdx * dTheta);

        float u = x / height + 0.5f;
        float v = z / height + 0.5f;

        meshData.vertices.push_back(Vertex(x, y, z, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
    }
    meshData.vertices.push_back(Vertex(0.0f, y, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

    uint32 centerIndex = (uint32) meshData.vertices.size() - 1;
    for (uint32 sliceIdx = 0; sliceIdx < sliceCount; ++sliceIdx)
    {
        meshData.indices32.push_back(centerIndex);
        meshData.indices32.push_back(baseIndex + sliceIdx + 1);
        meshData.indices32.push_back(baseIndex + sliceIdx);
    }
}

void GeometryGenerator::BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount, MeshData& meshData)
{
    _Unreferenced_parameter_(topRadius);
    _Unreferenced_parameter_(stackCount);

    uint32 baseIndex = (uint32) meshData.vertices.size();

    float y = -0.5f * height;
    float dTheta = 2.0f * MathHelper::Pi / sliceCount;

    for (uint32 sliceIdx = 0; sliceIdx <= sliceCount; ++sliceIdx)
    {
        float x = bottomRadius * cosf(sliceIdx * dTheta);
        float z = bottomRadius * sinf(sliceIdx * dTheta);

        float u = x / height + 0.5f;
        float v = z / height + 0.5f;

        meshData.vertices.push_back(Vertex(x, y, z, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
    }
    meshData.vertices.push_back(Vertex(0.0f, y, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

    uint32 centerIndex = (uint32) meshData.vertices.size() - 1;
    for (uint32 sliceIdx = 0; sliceIdx < sliceCount; ++sliceIdx)
    {
        meshData.indices32.push_back(centerIndex);
        meshData.indices32.push_back(baseIndex + sliceIdx);
        meshData.indices32.push_back(baseIndex + sliceIdx + 1);
    }
}
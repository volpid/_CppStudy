
#include "ch10_helper_recipes.h"

#include <iostream>
#include <memory>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

//---------------------------------------------------------------------------------------
// local
//---------------------------------------------------------------------------------------
namespace
{
    void CalculateTangentAndBitangent(const float* normalData,
        const Vector3& faceTangent,
        const Vector3& faceBitangent,
        float* tangentData,
        float* bitangentData)
    {
        const Vector3 normal = {normalData[0], normalData[1], normalData[2]};
        const Vector3 tangent = Normalize(faceTangent - normal * Dot(normal, faceTangent));

        float handedness = (Dot(Cross(normal, tangent), faceBitangent) < 0.0f) ? -1.0f : 1.0f;
        
        Vector3 bitangent = handedness * Cross(normal, tangent);

        tangentData[0] = tangent[0];
        tangentData[1] = tangent[1];
        tangentData[2] = tangent[2];
        bitangentData[0] = bitangent[0];
        bitangentData[1] = bitangent[1];
        bitangentData[2] = bitangent[2];
    }

    void GenerateTangentSpaceVerter(Cookbook::Mesh& mesh)
    {
        const size_t normalOffset =3;
        const size_t texcoordOffset = 6;
        const size_t tagentOffset = 8;
        const size_t bitangentOffset = 11;
        const size_t stride = bitangentOffset + 3;

        for (auto& part : mesh.parts)
        {
            part;
            for (size_t idx = 0; idx <mesh.data.size(); idx += stride * 3)
            {
                size_t i1 = idx;
                size_t i2 = i1 + stride;
                size_t i3 = i2 + stride;

                const Vector3 v1 = {mesh.data[i1], mesh.data[i1 + 1], mesh.data[i1 + 2]};
                const Vector3 v2 = {mesh.data[i2], mesh.data[i2 + 1], mesh.data[i2 + 2]};
                const Vector3 v3 = {mesh.data[i3], mesh.data[i3 + 1], mesh.data[i3 + 2]};

                const std::array<float, 2> w1 = {mesh.data[i1 + texcoordOffset], mesh.data[i1 + texcoordOffset + 1]};
                const std::array<float, 2> w2 = {mesh.data[i2 + texcoordOffset], mesh.data[i2 + texcoordOffset + 1]};
                const std::array<float, 2> w3 = {mesh.data[i3 + texcoordOffset], mesh.data[i3 + texcoordOffset + 1]};

                float x1 = v2[0] - v1[0];
                float x2 = v3[0] - v1[0];
                float y1 = v2[1] - v1[1];
                float y2 = v3[1] - v1[1];
                float z1 = v2[2] - v1[2];
                float z2 = v3[2] - v1[2];

                float s1 = w2[0] - w1[0];
                float s2 = w3[0] - w1[0];
                float t1 = w2[1] - w1[1];
                float t2 = w3[1] - w1[1];

                float r = 1.0f / (s1 * t2 - s2 * t1);
                Vector3 faceTangent = {(t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r};
                Vector3 faceBitangent = {(s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r};

                CalculateTangentAndBitangent(&mesh.data[i1 + normalOffset], 
                    faceTangent, 
                    faceBitangent,
                    &mesh.data[i1 + tagentOffset],
                    &mesh.data[i1 + bitangentOffset]);
                CalculateTangentAndBitangent(&mesh.data[i2 + normalOffset], 
                    faceTangent, 
                    faceBitangent,
                    &mesh.data[i2 + tagentOffset],
                    &mesh.data[i2 + bitangentOffset]);
                CalculateTangentAndBitangent(&mesh.data[i3 + normalOffset], 
                    faceTangent, 
                    faceBitangent,
                    &mesh.data[i3 + tagentOffset],
                    &mesh.data[i3 + bitangentOffset]);
            }
        }
    }
}

//---------------------------------------------------------------------------------------
// helper
//---------------------------------------------------------------------------------------
bool Cookbook::LoadTextureDataFromFile(const char* filename, 
    int numRequestedComponents,
    std::vector<unsigned char>& imageData,
    int* imageWidth,
    int* imageHeight,
    int* imageNumComponenets,
    int* imageDataSize)
{
    int width = 0;
    int height = 0;
    int numComponent = 0;
    std::unique_ptr<unsigned char, void (*)(void*)> stbiData(stbi_load(filename, &width, &height, &numComponent, numRequestedComponents), stbi_image_free);

    if ((stbiData == nullptr)
        || (width <= 0)
        || (height <= 0)
        || (numComponent <= 0))
    {
        std::cout << "Could not read image!" << std::endl;
        return false;
    }

    int dataSize = width * height  * ((numRequestedComponents > 0) ? numRequestedComponents : numComponent);
    
    if (imageDataSize != nullptr)
    {
        *imageDataSize = dataSize;
    }
    if (imageWidth != nullptr)
    {
        *imageWidth = width;
    }
    if (imageHeight != nullptr)
    {
        *imageHeight = height;
    }
    if (imageHeight != nullptr)
    {
        *imageNumComponenets = numComponent;
    }

    imageData.resize(dataSize);
    std::memcpy(imageData.data(), stbiData.get(), dataSize);
    return true;
}

bool Cookbook::Load3DModelFromObjFile(const char* filename,
    bool loadNormal,
    bool loadTexCoord,
    bool generateTangentSpaceVector,
    bool unify,
    Mesh& mesh,
    uint32_t* vertexStride)
{
    tinyobj::attrib_t attribs;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string error;

    bool result = tinyobj::LoadObj(&attribs, &shapes, &materials, &error, filename);
    if (result == false)
    {
        std::cout << "Could not open the '" << filename << "' file.";
        if (error.size() > 0)
        {
            std::cout << " " << error;
        }
        std::cout << std::endl;

        return false;
    }

    if (loadNormal == false || loadTexCoord == false)
    {
        generateTangentSpaceVector = false;
    }

    float minX = attribs.vertices[0];
    float maxX = attribs.vertices[0];
    float minY = attribs.vertices[1];
    float maxY = attribs.vertices[1];
    float minZ = attribs.vertices[2];
    float maxZ = attribs.vertices[2];

    mesh = {};
    uint32_t offset = 0;
    for (auto& shape : shapes)
    {
        uint32_t partOffset = offset;

        for (auto& index : shape.mesh.indices)
        {
            mesh.data.emplace_back(attribs.vertices[3 * index.vertex_index + 0]);
            mesh.data.emplace_back(attribs.vertices[3 * index.vertex_index + 1]);
            mesh.data.emplace_back(attribs.vertices[3 * index.vertex_index + 2]);
            ++offset;

            if (loadNormal == true)
            {
                if (attribs.normals.size() == 0)
                {
                    std::cout << "Could not load normal vectors data in the '" << filename << "' file";
                    return false;
                }
                else                
                {
                    mesh.data.emplace_back(attribs.normals[3 * index.normal_index + 0]);
                    mesh.data.emplace_back(attribs.normals[3 * index.normal_index + 1]);
                    mesh.data.emplace_back(attribs.normals[3 * index.normal_index + 2]);
                }
            }

            if (loadTexCoord == true)
            {
                if (attribs.texcoords.size() == 0)
                {
                    std::cout << "Could not load texture coordinate data in the '" << filename << "' file";
                    return false;
                }
                else                
                {
                    mesh.data.emplace_back(attribs.texcoords[2 * index.texcoord_index + 0]);
                    mesh.data.emplace_back(attribs.texcoords[2 * index.texcoord_index + 1]);
                }
            }

            if (generateTangentSpaceVector == true)
            {
                for (int idx = 0; idx < 6; ++idx)
                {
                    mesh.data.emplace_back(0.0f);
                }
            }

            if (unify == true)
            {
                if (attribs.vertices[3 * index.vertex_index + 0] < minX)
                {
                    minX = attribs.vertices[3 * index.vertex_index + 0];
                }
                if (attribs.vertices[3 * index.vertex_index + 0] > maxX)
                {
                    maxX = attribs.vertices[3 * index.vertex_index + 0];
                }
                if (attribs.vertices[3 * index.vertex_index + 1] < minY)
                {
                    minY = attribs.vertices[3 * index.vertex_index + 1];
                }
                if (attribs.vertices[3 * index.vertex_index + 1] > maxY)
                {
                    maxY = attribs.vertices[3 * index.vertex_index + 1];
                }
                if (attribs.vertices[3 * index.vertex_index + 2] < minZ)
                {
                    minZ = attribs.vertices[3 * index.vertex_index + 2];
                }
                if (attribs.vertices[3 * index.vertex_index + 2] > maxZ)
                {
                    maxZ = attribs.vertices[3 * index.vertex_index + 2];
                }
            }
        }
        
        uint32_t partVertexCount = offset - partOffset;
        if (partVertexCount > 0)
        {
            mesh.parts.push_back({partOffset, partVertexCount});
        }
    }

    uint32_t stride = 3 
        + ((loadNormal == true) ? 3 : 0) 
        + ((loadTexCoord == true) ? 2 : 0)
        + ((generateTangentSpaceVector == true) ? 6 : 0);

    if (vertexStride != nullptr)
    {
        *vertexStride = stride * sizeof(float);
    }

    if (generateTangentSpaceVector == true)
    {
        GenerateTangentSpaceVerter(mesh);
    }

    if (unify == true)
    {
        float offsetX = (minX + maxX) * 0.5f;
        float offsetY = (minY + maxY) * 0.5f;
        float offsetZ = (minZ + maxZ) * 0.5f;
        float scaleX = (std::abs(minX - offsetX) > std::abs(maxX - offsetX)) 
            ? abs(minX - offsetX)
            : abs(maxX - offsetX);
        float scaleY = (std::abs(minY - offsetY) > std::abs(maxY - offsetY)) 
            ? abs(minY - offsetY)
            : abs(maxY - offsetY);
        float scaleZ = (std::abs(minZ - offsetZ) > std::abs(maxZ - offsetZ)) 
            ? abs(minZ - offsetZ)
            : abs(maxZ - offsetZ);
        float scale = (scaleX > scaleY) ? scaleX : scaleY;
        scale = (scaleZ > scale) ? (1.0f / scaleZ) : (1.0f / scale);

        for (size_t i = 0; i < mesh.data.size() - 2; i += stride)
        {
            mesh.data[i + 0] = scale * (mesh.data[i + 0] - offsetX);
            mesh.data[i + 1] = scale * (mesh.data[i + 1] - offsetY);
            mesh.data[i + 2] = scale * (mesh.data[i + 2] - offsetZ);
        }
    }

    return true;
}

Matrix4x4 Cookbook::PrepareRotationMatrix(float angle, const Vector3& axis, float normalizedAxis)
{
    float x;
    float y;
    float z;

    if (normalizedAxis)
    {
        Vector3 normalized = Normalize(axis);
        x = normalized[0];
        y = normalized[1];
        z = normalized[2];
    }
    else
    {
        x = axis[0];
        y = axis[1];
        z = axis[2];
    }

    const float c = cos(Deg2Rad(angle));
    const float oneMinusCos = 1 - c;
    const float s = sin(Deg2Rad(angle));

    Matrix4x4 rotationMatrix = 
    {
        x * x * oneMinusCos + c,
        y * x * oneMinusCos - z * s,
        z * x * oneMinusCos + y * s,
        0.0f,

        x * y * oneMinusCos + z * s,
        y * y * oneMinusCos + c,
        z * y * oneMinusCos - x * s,
        0.0f,

        x * z * oneMinusCos - y * s,
        y * z * oneMinusCos + x * s,
        z * z * oneMinusCos + c,
        0.0f,

        0.0f,
        0.0f,
        0.0f,
        1.0f
    };

    return rotationMatrix;
}

Matrix4x4 Cookbook::PrepareTranslationMatrix(float x, float y, float z)
{
    Matrix4x4 translationMatrix =
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        x, y, z, 1.0f,
    };

    return translationMatrix;
}

Matrix4x4 Cookbook::PreparePerspectiveProjectionMatrix(float aspectRatio, float fov, float near, float far)
{
    float f = 1.0f / tan(Deg2Rad(fov * 0.5f));
    Matrix4x4 perspectiveProjectionMatrix = 
    {
        f / aspectRatio, 0.0f, 0.0f, 0.0f,
        0.0f, -f, 0.0f, 0.0f,
        0.0f, 0.0f, far / (near - far), -1.0f,
        0.0f, 0.0f, (near * far) / (near - far), 0.0f
    };

    return perspectiveProjectionMatrix;
}

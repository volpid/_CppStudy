
#ifndef __CH10_HELPER_RECIPES__H__
#define __CH10_HELPER_RECIPES__H__

#include "../Common/vulkan_tool.h"

#include <vector>

namespace Cookbook
{
    struct Mesh
    {
        struct Part
        {
            uint32_t vertexOffset;
            uint32_t vertexCount;
        };

        std::vector<float> data;
        std::vector<Part> parts;
    };

    bool LoadTextureDataFromFile(const char* filename, 
        int numRequestedComponents,
        std::vector<unsigned char>& imageData,
        int* imageWidth = nullptr,
        int* imageHeight = nullptr,
        int* imageNumComponenets = nullptr,
        int* imageDataSize = nullptr);

    bool Load3DModelFromObjFile(const char* filename,
        bool loadNormal,
        bool loadTexCoord,
        bool generateTangentSpaceVector,
        bool unify,
        Mesh& mesh,
        uint32_t* vertexStride = nullptr);

    Matrix4x4 PrepareRotationMatrix(float angle, const Vector3& axis, float normalizedAxis = false);
    Matrix4x4 PrepareTranslationMatrix(float x, float y, float z);
    Matrix4x4 PreparePerspectiveProjectionMatrix(float aspectRatio, float fov, float near, float far);
}

#endif /*__CH10_HELPER_RECIPES__H__*/
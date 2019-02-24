
#ifndef __CH10_HELPER_RECIPES__H__
#define __CH10_HELPER_RECIPES__H__

#include <vector>

namespace Cookbook
{
    bool LoadTextureDataFromFile(const char* filename, 
        int numRequestedComponents,
        std::vector<unsigned char>& imageData,
        int* imageWidth = nullptr,
        int* imageHeight = nullptr,
        int* imageNumComponenets = nullptr,
        int* imageDataSize = nullptr);
}

#endif /*__CH10_HELPER_RECIPES__H__*/
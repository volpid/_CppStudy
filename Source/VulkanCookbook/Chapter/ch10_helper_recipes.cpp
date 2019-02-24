
#include "ch10_helper_recipes.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <memory>

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

    imageData.resize(dataSize);
    std::memcpy(imageData.data(), stbiData.get(), dataSize);
    return true;
}
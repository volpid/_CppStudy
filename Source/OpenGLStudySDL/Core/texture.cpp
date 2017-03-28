
#include "texture.h"

#include <iostream>
#include <cassert>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

//---------------------------------------------------------------------------
// define - Texture
//---------------------------------------------------------------------------
Texture::Texture(const std::string& filename)
{
    int width;
    int height;
    int numComponent;
    unsigned char* pImageData = stbi_load(filename.c_str(), &width, &height, &numComponent, 4);

    if (pImageData == nullptr)
    {
        std::cerr << "Error : texture load failed!" << std::endl;
    }

    glGenTextures(1, &texture_);
    glBindTexture(GL_TEXTURE_2D, texture_);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImageData);

    stbi_image_free(pImageData);
}

Texture::~Texture(void)
{
    glDeleteTextures(1, &texture_);
}

void Texture::Bind(unsigned int unit)
{
    assert(unit >= 0 && unit <= 31);

    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, texture_);
}
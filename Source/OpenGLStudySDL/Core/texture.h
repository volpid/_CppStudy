
#ifndef __TEXTURE__H__
#define __TEXTURE__H__

#include <glew/glew.h>
#include <string>

class Texture
{
public:
    Texture(const std::string& filename);
    virtual ~Texture(void);

    void Bind(unsigned int unit);
private:
    GLuint texture_;
};

#endif /*__TEXTURE__H__*/
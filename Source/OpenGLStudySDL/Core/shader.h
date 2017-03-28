
#ifndef __SHADER__H__
#define __SHADER__H__

#include "camera.h"
#include "transform.h"

#include <glew/glew.h>
#include <string>

class Shader
{
public:
    enum Uniform : unsigned int
    {
        TransformMatrix,
        TotalNumber,
    };
    static const unsigned int NumShader = 2;

public:
    Shader(const std::string& filename);
    virtual ~Shader(void);

    void Bind(void);
    void Update(const Transform& transform, const Camera& camera);

private:
    GLuint program_;
    GLuint shaders_[NumShader];
    GLuint uniforms_[Uniform::TotalNumber];
};

#endif /*_SHADER__H__*/
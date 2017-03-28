
#ifndef __MESH__H__
#define __MESH__H__

#include "obj_loader.h"

#include <glm/glm.hpp>
#include <glew/glew.h>
#include <string>

class Vertex
{
public:
    Vertex(const glm::vec3& pos, const glm::vec2& texCoord, const glm::vec3& normal = glm::vec3());

    inline const glm::vec3& GetPosition(void) const;
    inline const glm::vec2& GetTexCoord(void) const;
    inline const glm::vec3& GetNormal(void) const;

private:
    glm::vec3 pos_;
    glm::vec2 texCoord_;
    glm::vec3 normal_;
};

class Mesh
{
public:
    Mesh(const std::string& filename);
    Mesh(Vertex* vertices, unsigned int numVertices, unsigned int* indices, unsigned int numIndices);    
    virtual ~Mesh(void);

    void Draw(void);

private:
    void InitMesh(const IndexedModel& model);

private:
    enum Buffers : unsigned int 
    {
        Position_VB = 0,
        Texture_VB,
        Normal_VB,
        Index_VB,
        NumBuffers,
    };
    
    GLuint vertexArrayObject_;
    GLuint vertexArrayBuffers_[Buffers::NumBuffers];
    unsigned int indexCount_ = 0;
};

//---------------------------------------------------------------------------
// inline Section
//---------------------------------------------------------------------------
inline Vertex::Vertex(const glm::vec3& pos, const glm::vec2& texCoord, const glm::vec3& normal)   
    : pos_(pos), texCoord_(texCoord), normal_(normal)
{}

inline const glm::vec3& Vertex::GetPosition(void) const
{
    return pos_;
}

inline const glm::vec2& Vertex::GetTexCoord(void) const
{
    return texCoord_;
}

inline const glm::vec3& Vertex::GetNormal(void) const
{
    return normal_;
}


#endif /*__MESH__H__*/
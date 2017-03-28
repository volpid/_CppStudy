
#ifndef __TRANSFORM__H__
#define __TRANSFORM__H__

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

class Transform
{
public:
    inline Transform(const glm::vec3& pos = glm::vec3(), const glm::vec3& rot = glm::vec3(), const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f));

    inline glm::mat4 GetModel(void) const;
    inline glm::vec3& GetPos(void);
    inline glm::vec3& GetRot(void);
    inline glm::vec3& GetScale(void);
    
    inline void SetPos(const glm::vec3& pos);
    inline void SetRot(const glm::vec3& rot);
    inline void SetScale(const glm::vec3& scale);
private:

    glm::vec3 pos_;
    glm::vec3 rot_;
    glm::vec3 scale_;
};

//---------------------------------------------------------------------------
// inline section - Transform
//---------------------------------------------------------------------------
inline Transform::Transform(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale)
    : pos_(pos), rot_(rot), scale_(scale)
{}

inline glm::mat4 Transform::GetModel(void) const
{
    glm::mat4 posMatrix = glm::translate(pos_);
    glm::mat4 scaleMatrix = glm::scale(scale_);
    glm::mat4 rotXMatrix = glm::rotate(rot_.x, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 rotYMatrix = glm::rotate(rot_.y, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rotZMatrix = glm::rotate(rot_.z, glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 rotMatrix = rotZMatrix * rotYMatrix * rotXMatrix;

    return posMatrix * rotMatrix * scaleMatrix;
}

inline glm::vec3& Transform::GetPos(void)
{
    return pos_;
}

inline glm::vec3& Transform::GetRot(void)
{
    return rot_;
}

inline glm::vec3& Transform::GetScale(void)
{
    return scale_;
}
    
inline void Transform::SetPos(const glm::vec3& pos)
{
    pos_ = pos;
}

inline void Transform::SetRot(const glm::vec3& rot)
{
    rot_ = rot;
}

inline void Transform::SetScale(const glm::vec3& scale)
{
    scale_ = scale;
}

#endif /*__TRANSFORM__H__*/

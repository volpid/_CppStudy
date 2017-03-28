
#ifndef __CAMERA__H__
#define __CAMERA__H__

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

class Camera
{
public:
    inline Camera(const glm::vec3& pos, float fov, float aspect, float zNear, float zFar);
    inline ~Camera(void);

    inline glm::mat4 GetViewPorjection(void) const;

private:
    glm::mat4 perspective_;
    glm::vec3 position_;
    glm::vec3 forward_;
    glm::vec3 up_;
    glm::vec3 right_;
};

//---------------------------------------------------------------------------
// inline section - Transform
//---------------------------------------------------------------------------
inline Camera::Camera(const glm::vec3& pos, float fov, float aspect, float zNear, float zFar)
{
    position_ = pos;
    forward_ = glm::vec3(0.0f, 0.0f, 1.0f);
    up_ = glm::vec3(0.0f, 1.0f, 0.0f);
    right_ = glm::vec3(1.0f, 0.0f, 0.0f);

    perspective_ = glm::perspective(fov, aspect, zNear, zFar);
}

inline Camera::~Camera(void)
{}

inline glm::mat4 Camera::GetViewPorjection(void) const
{
    return perspective_ * glm::lookAt(position_, position_ + forward_, up_);
}

#endif /*__CAMERA__H__*/

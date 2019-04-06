
#ifndef __SAMPLE_CAMERA__H__
#define __SAMPLE_CAMERA__H__

#include "vulkan_tool.h"

//-----------------------------------------------------------------------
// Camera
//-----------------------------------------------------------------------
class Camera
{
protected:
    Camera(void);
    Camera(const Vector3& position, const Vector3& rightVector, const Vector3& upVector, const Vector3& forwardVector);
    Camera(const Camera& other);
    virtual ~Camera() = 0;

    Camera& operator=(const Camera& other);

public:
    virtual Matrix4x4 GetMatrix(void) const final;
    virtual Vector3 GetPosition(void) const final;
    virtual Vector3 GetRightVector(void) const final;
    virtual Vector3 GetUpVector(void) const final;
    virtual Vector3 GetForwardVector(void) const final;

protected:
    mutable Matrix4x4 _ViewMatrix;
    Vector3 _position;
    Vector3 _rightVector;
    Vector3 _upVector;
    Vector3 _forwardVector;

    mutable bool _dirty;
};

#endif /*__SAMPLE_CAMERA__H__*/
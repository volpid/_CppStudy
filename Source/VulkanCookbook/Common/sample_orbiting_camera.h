
#ifndef __SAMPLE_ORBITING_CAMERA__H__
#define __SAMPLE_ORBITING_CAMERA__H__

#include "sample_camera.h"

//-----------------------------------------------------------------------
// OrbitingCamera
//-----------------------------------------------------------------------
class OrbitingCamera : public Camera
{
public:
    OrbitingCamera(void);
    OrbitingCamera(const Vector3& target, float distance, float horizontalAngle = 0.0f, float verticalAngle = 0.0f);
    OrbitingCamera(const OrbitingCamera& other);
    virtual ~OrbitingCamera(void);

    OrbitingCamera& operator=(const OrbitingCamera& other);

    virtual Vector3 GetTarget(void) const final;
    virtual float GetDistance(void) const final;
    virtual float GetHorizontalAngle(void) const final;
    virtual float GetVerticalAngle(void) const final;

    void ChangeDistance(float distanceDelta);
    void RotateHorizontally(float angleDelta);
    void RotateVertically(float angleDelta);

private:
    Vector3 _target;
    float _distance;
    float _horizontalAngle;
    float _verticalAngle;
};

#endif /*__SAMPLE_ORBITING_CAMERA__H__*/
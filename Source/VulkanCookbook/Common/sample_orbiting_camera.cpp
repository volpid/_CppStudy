
#include "sample_orbiting_camera.h"

#include "../Chapter/ch10_helper_recipes.h"

//-----------------------------------------------------------------------
// OrbitingCamera
//-----------------------------------------------------------------------
OrbitingCamera::OrbitingCamera(void)
    : OrbitingCamera({0.0f, 0.0f, 0.0f}, 1.0f, 0.0f, 0.0f)
{}

OrbitingCamera::OrbitingCamera(const Vector3& target, float distance, float horizontalAngle, float verticalAngle)
    : Camera(target - distance * Vector3{0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}),
    _target(target),
    _distance(distance),
    _horizontalAngle(0.0f),
    _verticalAngle(0.0f)
{
    RotateHorizontally(horizontalAngle);
    RotateVertically(verticalAngle);
}

OrbitingCamera::OrbitingCamera(const OrbitingCamera& other)
{
    *this = other;
}

OrbitingCamera::~OrbitingCamera(void)
{}

OrbitingCamera& OrbitingCamera::operator=(const OrbitingCamera& other)
{
    if (this != &other)
    {
        Camera::operator=(other);
        _target = other._target;
        _distance = other._distance;
        _horizontalAngle = other._horizontalAngle;
        _verticalAngle = other._verticalAngle;
    }

    return *this;
}

Vector3 OrbitingCamera::GetTarget(void) const 
{
    return _target;
}

float OrbitingCamera::GetDistance(void) const 
{
    return _distance;
}

float OrbitingCamera::GetHorizontalAngle(void) const 
{
    return _horizontalAngle;
}
float OrbitingCamera::GetVerticalAngle(void) const 
{
    return _verticalAngle;
}

void OrbitingCamera::ChangeDistance(float distanceDelta)
{
    _distance -= distanceDelta;
    if (_distance < 0.0f)
    {
        _distance = 0.0f;
    }

    _position =_target - _distance * _forwardVector;
    _dirty = true; 
}

void OrbitingCamera::RotateHorizontally(float angleDelta)
{
    const Matrix4x4 rotation = Cookbook::PrepareRotationMatrix(angleDelta, {0.0f, -1.0f, 0.0f});
    
    _horizontalAngle += angleDelta;
    _forwardVector = _forwardVector * rotation;
    _upVector = _upVector * rotation;
    _rightVector = _rightVector * rotation;
    _position = _target - _distance * _forwardVector;
    _dirty = true;
}

void OrbitingCamera::RotateVertically(float angleDelta)
{
    const float oldAngle = _verticalAngle;
    float tempDelta = angleDelta;

    _verticalAngle += angleDelta;
    if (_verticalAngle > 90.0f)
    {
        _verticalAngle = 90.0f;
        tempDelta = 90.0f - oldAngle;
    }
    else if (_verticalAngle < -90.0f)
    {
        _verticalAngle = -90.0f;
        tempDelta = -90.0f - oldAngle;
    }

    const Matrix4x4 rotation = Cookbook::PrepareRotationMatrix(tempDelta, _rightVector);
    _forwardVector = _forwardVector * rotation;
    _upVector = _upVector * rotation;
    _position = _target - _distance * _forwardVector;
    _dirty = true;
}

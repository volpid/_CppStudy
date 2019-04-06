
#include "sample_camera.h"

//-----------------------------------------------------------------------
// Camera
//-----------------------------------------------------------------------
Camera::Camera(void)
    : Camera({0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f})
{}

Camera::Camera(const Vector3& position, const Vector3& rightVector, const Vector3& upVector, const Vector3& forwardVector)
    : _ViewMatrix({}), _position(position), _rightVector(rightVector), _upVector(upVector), _forwardVector(forwardVector), _dirty(true)
{}

Camera::Camera(const Camera& other)
{
    *this = other;
}

Camera::~Camera()
{}

Camera& Camera::operator=(const Camera& other)
{
    if (this != &other)
    {
        _ViewMatrix = other._ViewMatrix;
        _position = other._position;
        _rightVector = other._rightVector;
        _upVector = other._upVector;
        _forwardVector = other._forwardVector;
        _dirty = other._dirty;
    }

    return *this;
}

Matrix4x4 Camera::GetMatrix(void) const
{
    if (_dirty == true)
    {
        _ViewMatrix = 
        {
            _rightVector[0], _upVector[0], -_forwardVector[0], 0.0f,
            _rightVector[1], _upVector[1], -_forwardVector[1], 0.0f,
            _rightVector[2], _upVector[2], -_forwardVector[2], 0.0f,
            Dot(_position, _rightVector), Dot(_position, _upVector), Dot(_position, _forwardVector), 1.0f,
        };
        _dirty = false;
    }
    
    return _ViewMatrix;
}

Vector3 Camera::GetPosition(void) const
{
    return _position;
}

Vector3 Camera::GetRightVector(void) const
{
    return _rightVector;
}

Vector3 Camera::GetUpVector(void) const
{
    return _upVector;
}

Vector3 Camera::GetForwardVector(void) const
{
    return _forwardVector;
}

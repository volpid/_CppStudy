
#ifndef __VULKAN_TOOL__H__
#define __VULKAN_TOOL__H__

#include <array>
#include <string>
#include <vector>

using Vector3 = std::array<float, 3>;
using Matrix4x4 = std::array<float, 16>;
   
bool GetBinaryFileContent(const std::string& filename, std::vector<unsigned char>& content);

float Deg2Rad(float value);
Vector3 Normalize(const Vector3& vec);
float Dot(const Vector3 left, const Vector3 right);
Vector3 Cross(const Vector3 left, const Vector3 right);

Vector3 operator+(const Vector3& lhs, const Vector3& rhs);
Vector3 operator-(const Vector3& lhs, const Vector3& rhs);

Vector3 operator+(const float& lhs, const Vector3& rhs);
Vector3 operator-(const float& lhs, const Vector3& rhs);
Vector3 operator+(const Vector3& lhs, const float& rhs);
Vector3 operator-(const Vector3& lhs, const float& rhs);

Vector3 operator*(const float& lhs, const Vector3& rhs);
Vector3 operator*(const Vector3& lhs, const float& rhs);
Vector3 operator*(const Vector3& lhs, const Matrix4x4& rhs);

Vector3 operator-(const Vector3& lhs);

bool operator==(const Vector3& lhs, const Vector3& rhs);

Matrix4x4 operator*(const Matrix4x4& lhs, const Matrix4x4& rhs);

#endif /*__VULKAN_TOOL__H__*/


#include "vulkan_tool.h"

#include <fstream>
#include <iostream>

bool GetBinaryFileContent(const std::string& filename, std::vector<unsigned char>& content)
{
    content.clear();

    std::ifstream file(filename, std::ios::binary);
    if (file.fail() == true)
    {
        std::cout << "Could not open " << filename << " file." << std::endl;
        return false;
    }

    std::streampos begin;
    std::streampos end;
    begin = file.tellg();
    file.seekg(0, std::ios::end);
    end = file.tellg();

    if ((end - begin) == 0)
    {
        std::cout << "The " << filename << " file is empty" << std::endl;
        return false;
    }
    content.resize(static_cast<size_t> (end- begin));
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char *> (content.data()), end - begin);
    file.close();

    return true;
}

float Deg2Rad(float value)
{
    return value * 0.01745329251994329576923690768489f;
}

Vector3 Normalize(const Vector3& vec)
{
    float length = std::sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
    return {vec[0] / length, vec[1] / length, vec[2] / length};    
}

float Dot(const Vector3 left, const Vector3 right)
{
    return (left[0] * right[0] + left[1] * right[1] + left[2] * right[2]);
}

Vector3 Cross(const Vector3 left, const Vector3 right)
{
    return {left[1] * right[2] - left[2] * right[1],
        left[2] * right[0] - left[0] * right[2],
        left[0] * right[1] - left[1] * right[0]};
}

Vector3 operator+(const Vector3& lhs, const Vector3& rhs)
{
    return {lhs[0] + rhs[0], lhs[1] + rhs[1], lhs[2] + rhs[2]};
}

Vector3 operator-(const Vector3& lhs, const Vector3& rhs)
{
    return {lhs[0] - rhs[0], lhs[1] - rhs[1], lhs[2] - rhs[2]};
}

Vector3 operator+(const float& lhs, const Vector3& rhs)
{
    return {lhs + rhs[0], lhs + rhs[1], lhs + rhs[2]};
}

Vector3 operator-(const float& lhs, const Vector3& rhs)
{
    return {lhs - rhs[0], lhs - rhs[1], lhs - rhs[2]};
}

Vector3 operator+(const Vector3& lhs, const float& rhs)
{
    return {lhs[0] + rhs, lhs[1] + rhs, lhs[2] + rhs};
}

Vector3 operator-(const Vector3& lhs, const float& rhs)
{
    return {lhs[0] - rhs, lhs[1] - rhs, lhs[2] - rhs};
}

Vector3 operator*(const float& lhs, const Vector3& rhs)
{
    return {lhs * rhs[0], lhs * rhs[1], lhs * rhs[2]};
}

Vector3 operator*(const Vector3& lhs, const float& rhs)
{
    return {lhs[0] * rhs, lhs[1] * rhs, lhs[2] * rhs};
}

Vector3 operator*(const Vector3& lhs, const Matrix4x4& rhs)
{
    return {lhs[0] * rhs[0] + lhs[1] * rhs[1] + lhs[2] * rhs[2],
        lhs[0] * rhs[4] + lhs[1] * rhs[5] + lhs[2] * rhs[6],
        lhs[0] * rhs[8] + lhs[1] * rhs[9] + lhs[2] * rhs[10]};
}

Vector3 operator-(const Vector3& lhs)
{
    return {-lhs[0], -lhs[1], -lhs[2]};
}

bool operator==(const Vector3& lhs, const Vector3& rhs)
{
    const float minValue = 0.00001f;

    if ((std::abs(lhs[0] - rhs[0]) > minValue)
        || (std::abs(lhs[1] - rhs[1]) > minValue)
        || (std::abs(lhs[2] - rhs[2]) > minValue))
    {
        return false;
    }
    
    return true;
}

Matrix4x4 operator*(const Matrix4x4& lhs, const Matrix4x4& rhs)
{   
    return {lhs[0] * rhs[0] + lhs[4] * rhs[1] + lhs[8] * rhs[2] + lhs[12] * rhs[3],
        lhs[1] * rhs[0] + lhs[5] * rhs[1] + lhs[9] * rhs[2] + lhs[13] * rhs[3],
        lhs[2] * rhs[0] + lhs[6] * rhs[1] + lhs[10] * rhs[2] + lhs[14] * rhs[3],
        lhs[3] * rhs[0] + lhs[7] * rhs[1] + lhs[11] * rhs[2] + lhs[15] * rhs[3],

        lhs[0] * rhs[4] + lhs[4] * rhs[5] + lhs[8] * rhs[6] + lhs[12] * rhs[7],
        lhs[1] * rhs[4] + lhs[5] * rhs[5] + lhs[9] * rhs[6] + lhs[13] * rhs[7],
        lhs[2] * rhs[4] + lhs[6] * rhs[5] + lhs[10] * rhs[6] + lhs[14] * rhs[7],
        lhs[3] * rhs[4] + lhs[7] * rhs[5] + lhs[11] * rhs[6] + lhs[15] * rhs[7],

        lhs[0] * rhs[8] + lhs[4] * rhs[9] + lhs[8] * rhs[10] + lhs[12] * rhs[11],
        lhs[1] * rhs[8] + lhs[5] * rhs[9] + lhs[9] * rhs[10] + lhs[13] * rhs[11],
        lhs[2] * rhs[8] + lhs[6] * rhs[9] + lhs[10] * rhs[10] + lhs[14] * rhs[11],
        lhs[3] * rhs[8] + lhs[7] * rhs[9] + lhs[11] * rhs[10] + lhs[15] * rhs[11],

        lhs[0] * rhs[12] + lhs[4] * rhs[13] + lhs[8] * rhs[14] + lhs[12] * rhs[15],
        lhs[1] * rhs[12] + lhs[5] * rhs[13] + lhs[9] * rhs[14] + lhs[13] * rhs[15],
        lhs[2] * rhs[12] + lhs[6] * rhs[13] + lhs[10] * rhs[14] + lhs[14] * rhs[15],
        lhs[3] * rhs[12] + lhs[7] * rhs[13] + lhs[11] * rhs[14] + lhs[15] * rhs[15]};
}
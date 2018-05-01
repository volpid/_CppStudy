
#include "math_helper.h"

const float MathHelper::Infinity = FLT_MAX;
const float MathHelper::Pi = DirectX::XM_PI;

DirectX::XMFLOAT4X4 MathHelper::Indentity4x4(void)
{
    static DirectX::XMFLOAT4X4 indentity4x4 = 
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    return indentity4x4;
}
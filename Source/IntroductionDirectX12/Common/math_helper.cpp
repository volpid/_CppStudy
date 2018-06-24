
#include "math_helper.h"

#include <random>

//----------------------------------------------------------------
// MathHelper
//----------------------------------------------------------------
namespace
{
    std::random_device randomDevice;
    std::mt19937 engine(randomDevice());
    std::uniform_real_distribution<> distributionReal(0.0f, 1.0f);
}

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

float MathHelper::RandF(void)
{
    return distributionReal(engine);
}

float MathHelper::RandF(float a, float b)
{
    std::uniform_real_distribution<> distributionRealWithCoef(a, b);
    return distributionRealWithCoef(engine);
}

int MathHelper::Rand(int a, int b)
{
    std::uniform_int_distribution<> distributionIntWithCoef(a, b);
    return distributionIntWithCoef(engine);
}
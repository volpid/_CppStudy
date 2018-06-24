
#ifndef __MATH_HELPER__H__
#define __MATH_HELPER__H__

#include "d3dx_12.h"

//----------------------------------------------------------------
// MathHelper
//----------------------------------------------------------------
namespace MathHelper
{
    extern const float Infinity;
    extern const float Pi;

    DirectX::XMFLOAT4X4 Indentity4x4(void);

    template <typename _T>
    inline _T Clamp(const _T& x, const _T& low, const _T& high);

    float RandF(void);
    float RandF(float a, float b);
    int Rand(int a, int b);
}

//----------------------------------------------------------------
// inline section
//----------------------------------------------------------------
template <typename _T>
inline _T MathHelper::Clamp(const _T& x, const _T& low, const _T& high)
{
    return (x < low) ? low : ((x > high) ? (high) : (x));
}

#endif /*__MATH_HELPER__H__*/
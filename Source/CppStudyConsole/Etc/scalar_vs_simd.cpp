
#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>

class SimpleTimer
{
public:
    SimpleTimer(void)
    {
        startTime_ = std::chrono::steady_clock::now();
    }

    ~SimpleTimer(void)
    {
        auto endTime = std::chrono::steady_clock::now();
        float druationTime = std::chrono::duration_cast<std::chrono::duration<float>> (endTime - startTime_).count();

        std::cout << "elpased : " << druationTime << std::endl;
    }

private:
    std::chrono::time_point<std::chrono::steady_clock> startTime_;
};

class SimpleRNG
{
public:
    void Seed(unsigned int seed)
    {
        engine_.seed(seed);
    }

    unsigned int RandUint(void)
    {
        return uniformDistInt_(engine_);
    }

    float RandFloat(void)
    {
        return uniformDistReal_(engine_);
    }

    float RandFloat(float min, float max)
    {
        return min + (max - min) * RandFloat();
    }

private:
    std::mt19937 engine_;
    std::uniform_int_distribution<> uniformDistInt_;
    std::uniform_real_distribution<> uniformDistReal_;
};

//--------------------------------------------------------------------
/*
    @Note : normal worldToCamera

    struct AABB 
    {
        float3 min;
        float3 max;
    }

    void TransformAABB(int n, const AABB* inAABB, const float4x4* mat, AABB* outAABB)
    {
        for (int i = 0; i < n; ++i)
        {
            float3 newMin(FLT_MAX), newMax(-FLT_MAX);
            for (int j = 0; j < 8; ++j)
            {
                float3 corner;
                corner.x = ((j & 1) != 0) ? inAABB[i].max.x : inAABB[i].min.x;
                corner.y = ((j & 2) != 0) ? inAABB[i].max.y : inAABB[i].min.y;
                corner.z = ((j & 4) != 0) ? inAABB[i].max.z : inAABB[i].min.z;
                corner = corner * mat;
                newMin = min(newMin, corner);
                newMax = max(newMax, corner);
            }
            outAABB[i].min = newMin;
            outAABB[i].max = newMax;
        }
    }
*/

struct AABB
{
    float min[3];
    float max[3];
};

void TransformAABBToScalar(int n, const AABB* inAABB, const float mat[][4][4], AABB* outAABB)
{
    for (int i = 0; i < n; ++i)
    {
        float newMin[] = {FLT_MAX, FLT_MAX, FLT_MAX};
        float newMax[] = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
        for (int j = 0; j < 8; ++j)
        {
            float corner[] =
            {
                ((j & 1) != 0) ? (inAABB[i].max[0]) : (inAABB[i].min[0]),
                ((j & 2) != 0) ? (inAABB[i].max[1]) : (inAABB[i].min[1]),
                ((j & 4) != 0) ? (inAABB[i].max[2]) : (inAABB[i].min[2]),
            };

            float connerTransform[] = 
            {
                corner[0] * mat[i][0][0] + corner[1] * mat[i][1][0] + corner[2] * mat[i][2][0] + mat[i][3][0],
                corner[0] * mat[i][0][1] + corner[1] * mat[i][1][1] + corner[2] * mat[i][2][1] + mat[i][3][1],
                corner[0] * mat[i][0][2] + corner[1] * mat[i][1][2] + corner[2] * mat[i][2][2] + mat[i][3][2],
            };

            newMin[0] = std::min(newMin[0], connerTransform[0]);
            newMin[1] = std::min(newMin[1], connerTransform[1]);
            newMin[2] = std::min(newMin[2], connerTransform[2]);
            newMax[0] = std::max(newMax[0], connerTransform[0]);
            newMax[1] = std::max(newMax[1], connerTransform[1]);
            newMax[2] = std::max(newMax[2], connerTransform[2]);
        }

        outAABB[i].min[0] = newMin[0];
        outAABB[i].min[1] = newMin[1];
        outAABB[i].min[2] = newMin[2];
        outAABB[i].max[0] = newMax[0];
        outAABB[i].max[1] = newMax[1];
        outAABB[i].max[2] = newMax[2];
    }
}

struct AABB_AOS
{
    __m128 min;
    __m128 max;
};

void TransformAABBToSSEAOS(int n, const AABB_AOS* inAABB, const __m128 mat[][4], AABB_AOS* outAABB)
{
    for (int i = 0; i < n; ++i)
    {
        __m128 newMin = {FLT_MAX, FLT_MAX, FLT_MAX};
        __m128 newMax = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

        for (int j = 0; j < 8; ++j)
        {
            __m128 corner = 
            {
                ((j & 1) != 0) ? (inAABB[i].max.m128_f32[0]) : (inAABB[i].min.m128_f32[0]),
                ((j & 2) != 0) ? (inAABB[i].max.m128_f32[1]) : (inAABB[i].min.m128_f32[1]),
                ((j & 4) != 0) ? (inAABB[i].max.m128_f32[2]) : (inAABB[i].min.m128_f32[2]),
            };

            __m128 cornerX = _mm_shuffle_ps(corner, corner, _MM_SHUFFLE(0, 0, 0, 0));
            __m128 cornerY = _mm_shuffle_ps(corner, corner, _MM_SHUFFLE(1, 1, 1, 1));
            __m128 cornerZ = _mm_shuffle_ps(corner, corner, _MM_SHUFFLE(2, 2, 2, 2));
            __m128 cornerTransformed = _mm_setzero_ps();

            cornerTransformed = _mm_add_ps(cornerTransformed, _mm_mul_ps(cornerX, mat[i][0]));
            cornerTransformed = _mm_add_ps(cornerTransformed, _mm_mul_ps(cornerY, mat[i][1]));
            cornerTransformed = _mm_add_ps(cornerTransformed, _mm_mul_ps(cornerZ, mat[i][2]));
            cornerTransformed = _mm_add_ps(cornerTransformed, mat[i][3]);
            newMin = _mm_min_ps(newMin, cornerTransformed);
            newMax = _mm_max_ps(newMax, cornerTransformed);
        }
        outAABB[i].min = newMin;
        outAABB[i].max = newMax;
    }

    /*
        @Note implementation with shuffle

        int i = 0;
        __m128 xyXY = _mm_shuffle_ps(inAABB[i].min, inAABB[i].max, _MM_SHUFFLE(1, 0, 1, 0));
        __m128 zwZW = _mm_shuffle_ps(inAABB[i].min, inAABB[i].max, _MM_SHUFFLE(3, 2, 3, 2));
        __m128 corner[] =
        {
            _mm_shuffle_ps(xyXY, zwZW, _MM_SHUFFLE(1, 0, 1, 0)),
            _mm_shuffle_ps(xyXY, zwZW, _MM_SHUFFLE(1, 0, 1, 2)),
            _mm_shuffle_ps(xyXY, zwZW, _MM_SHUFFLE(1, 0, 3, 0)),
            _mm_shuffle_ps(xyXY, zwZW, _MM_SHUFFLE(1, 0, 3, 2)),
            _mm_shuffle_ps(xyXY, zwZW, _MM_SHUFFLE(1, 2, 1, 0)),
            _mm_shuffle_ps(xyXY, zwZW, _MM_SHUFFLE(1, 2, 1, 2)),
            _mm_shuffle_ps(xyXY, zwZW, _MM_SHUFFLE(1, 2, 3, 0)),
            _mm_shuffle_ps(xyXY, zwZW, _MM_SHUFFLE(1, 2, 3, 2)),
        };
    */
}

struct AABB_SOA
{
    __m128* xmin;
    __m128* ymin;
    __m128* zmin;
    __m128* xmax;
    __m128* ymax;
    __m128* zmax;
};

struct Matrix_SOA
{   
    const __m128* comp[4][4];
};

void TransformAABBToSSESOA(int n, const AABB_SOA inAABB, Matrix_SOA mat, AABB_SOA outAABB)
{
    int nSIMD = (n + 3) / 4;
    for (int i = 0; i < nSIMD; ++i)
    {
        __m128 xMinsNew = {FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX};
        __m128 yMinsNew = {FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX};
        __m128 zMinsNew = {FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX};
        __m128 xMaxsNew = {-FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX};
        __m128 yMaxsNew = {-FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX};
        __m128 zMaxsNew = {-FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX};

        for (int j = 0; j < 8; ++j)
        {
            __m128 xsCorner = ((j & 1) != 0) ? (inAABB.xmax[i]) : (inAABB.xmin[i]);
            __m128 ysCorner = ((j & 2) != 0) ? (inAABB.ymax[i]) : (inAABB.ymin[i]);
            __m128 zsCorner = ((j & 4) != 0) ? (inAABB.zmax[i]) : (inAABB.zmin[i]);
            
            __m128 xsCornerTransformed = _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(xsCorner, mat.comp[0][0][i]),
                    _mm_mul_ps(ysCorner, mat.comp[1][0][i])
                ),
                _mm_add_ps(
                    _mm_mul_ps(zsCorner, mat.comp[2][0][i]),
                    mat.comp[3][0][i]));
            __m128 ysCornerTransformed = _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(xsCorner, mat.comp[0][1][i]),
                    _mm_mul_ps(ysCorner, mat.comp[1][1][i])
                ),
                _mm_add_ps(
                    _mm_mul_ps(zsCorner, mat.comp[2][1][i]),
                    mat.comp[3][1][i]));
            __m128 zsCornerTransformed = _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(xsCorner, mat.comp[0][2][i]),
                    _mm_mul_ps(ysCorner, mat.comp[1][2][i])
                ),
                _mm_add_ps(
                    _mm_mul_ps(zsCorner, mat.comp[2][2][i]),
                    mat.comp[3][2][i]));

            xMinsNew = _mm_min_ps(xMinsNew, xsCornerTransformed);
            yMinsNew = _mm_min_ps(yMinsNew, ysCornerTransformed);
            zMinsNew = _mm_min_ps(zMinsNew, zsCornerTransformed);
            xMaxsNew = _mm_max_ps(xMaxsNew, xsCornerTransformed);
            yMaxsNew = _mm_max_ps(yMaxsNew, ysCornerTransformed);
            zMaxsNew = _mm_max_ps(zMaxsNew, zsCornerTransformed);
        }

        outAABB.xmin[i] = xMinsNew;
        outAABB.ymin[i] = yMinsNew;
        outAABB.zmin[i] = zMinsNew;
        outAABB.xmax[i] = xMaxsNew;
        outAABB.ymax[i] = yMaxsNew;
        outAABB.zmax[i] = zMaxsNew;
    }
}

//--------------------------------------------------------------------
/*
    @Note : normal worldToCamera
    void WorldToCamera(float yaw, float pitch, float3& cameraPos, float4x4 outWorldToCamera)
    {
        float sinYaw = sin(yaw);
        float cosYaw = cos(yaw);
        float sinPitch = sin(pitch);
        float cosPitch = cos(pitch);

        float vecView = {cosYaw * cosPitch, sinYaw * cosPitch, sinPitch};
        float vecUp = {-cosYaw * sinPitch, -sinYaw * sinPitch, cosPitch};
        float vecLeft = cross(vecUp, vecView);

        float3x3 matWorldToCamera = makefloat3x3Cols(vecView, VecLeft, vecUp);

        return float4x4(matWorldToCamera, -cameraPos * matWorldToCamera);
    }

*/

void CalcWorldToCameraScalar(float yaw, float pitch, const float cameraPos[3], float outWorldToCamera[4][4])
{
    float sinYaw = sin(yaw);
    float cosYaw = cos(yaw);
    float sinPitch = sin(pitch);
    float cosPitch = cos(pitch);

    float vecView[3] = {cosYaw * cosPitch, sinYaw * cosPitch, sinPitch};
    float vecUp[3] = {-cosYaw * sinPitch, -sinYaw * sinPitch, cosPitch};
    float vecLeft[3] = 
    {
        vecUp[1] * vecView[2] - vecUp[2] * vecView[1],
        vecUp[2] * vecView[0] - vecUp[0] * vecView[2],
        vecUp[0] * vecView[1] - vecUp[1] * vecView[0],
    };

    //memset(outWorldToCamera, 0, sizeof(outWorldToCamera));
    memset(outWorldToCamera, 0, sizeof(float) * 4 * 4);
    outWorldToCamera[0][0] = vecView[0];
    outWorldToCamera[1][0] = vecView[1];
    outWorldToCamera[2][0] = vecView[2];
    outWorldToCamera[0][1] = vecLeft[0];
    outWorldToCamera[1][1] = vecLeft[1];
    outWorldToCamera[2][1] = vecLeft[2];
    outWorldToCamera[0][2] = vecUp[0];
    outWorldToCamera[1][2] = vecUp[1];
    outWorldToCamera[2][2] = vecUp[2];

    float invTranslation [] = 
    {
        -(cameraPos[0] * outWorldToCamera[0][0] + cameraPos[1] * outWorldToCamera[1][0] + cameraPos[2] * outWorldToCamera[2][0]),
        -(cameraPos[0] * outWorldToCamera[0][1] + cameraPos[1] * outWorldToCamera[1][1] + cameraPos[2] * outWorldToCamera[2][1]),
        -(cameraPos[0] * outWorldToCamera[0][2] + cameraPos[1] * outWorldToCamera[1][2] + cameraPos[2] * outWorldToCamera[2][2]),
    };

    outWorldToCamera[3][0] = invTranslation[0];
    outWorldToCamera[3][1] = invTranslation[1];
    outWorldToCamera[3][2] = invTranslation[2];
    outWorldToCamera[3][3] = 1.0;
}

void CalcWorldToCameraSSE_AOS(float yaw, float pitch, const __m128 cameraPos, __m128 outWorldToCamera[4])
{
    float sinYaw = sin(yaw);
    float cosYaw = cos(yaw);
    float sinPitch = sin(pitch);
    float cosPitch = cos(pitch);
    
    __m128 vecView = {cosYaw * cosPitch, sinYaw * cosPitch, sinPitch};
    __m128 vecUp = {-cosYaw * sinPitch, -sinYaw * sinPitch, cosPitch};
    __m128 vecViewShuffle120 = _mm_shuffle_ps(vecView, vecView, _MM_SHUFFLE(0, 0, 2, 1));
    __m128 vecViewShuffle201 = _mm_shuffle_ps(vecView, vecView, _MM_SHUFFLE(0, 1, 0, 2));
    __m128 vecUpShuffle120 = _mm_shuffle_ps(vecUp, vecUp, _MM_SHUFFLE(0, 0, 2, 1)); 
    __m128 vecUpShuffle201 = _mm_shuffle_ps(vecUp, vecUp, _MM_SHUFFLE(0, 1, 0, 2));
    __m128 vecLeft = _mm_sub_ps(_mm_mul_ps(vecUpShuffle120, vecViewShuffle201),
        _mm_mul_ps(vecUpShuffle201, vecViewShuffle120));
    
    outWorldToCamera[0] = vecView;
    outWorldToCamera[1] = vecLeft;
    outWorldToCamera[2] = vecUp;
    outWorldToCamera[3] = _mm_setzero_ps();
    _MM_TRANSPOSE4_PS(outWorldToCamera[0], outWorldToCamera[1], outWorldToCamera[2], outWorldToCamera[3]);

    __m128 ameraPosX = _mm_shuffle_ps(cameraPos, cameraPos, _MM_SHUFFLE(0, 0, 0, 0));
    __m128 ameraPosY = _mm_shuffle_ps(cameraPos, cameraPos, _MM_SHUFFLE(1, 1, 1, 1));
    __m128 ameraPosZ = _mm_shuffle_ps(cameraPos, cameraPos, _MM_SHUFFLE(2, 2, 2, 2));
    __m128 invTranslation = _mm_setzero_ps();
    invTranslation = _mm_sub_ps(invTranslation, _mm_mul_ps(ameraPosX, outWorldToCamera[0]));
    invTranslation = _mm_sub_ps(invTranslation, _mm_mul_ps(ameraPosY, outWorldToCamera[1]));
    invTranslation = _mm_sub_ps(invTranslation, _mm_mul_ps(ameraPosZ, outWorldToCamera[2]));

    outWorldToCamera[3] = invTranslation;
    outWorldToCamera[3].m128_f32[3] = 1.0f;
}

//--------------------------------------------------------------------
int _main(int argc, char** argv)
{    
    float absEpsilon = 1e-5f;
    float relEpsilon = 1e-5f;

    {
        float yaw = 0.74f;
        float pitch = 0.47f;
        float cameraPos[] = {1.0f, 2.0f, 3.0f};
        __m128 cameraPosAos = {1.0f, 2.0f, 3.0f};
        float worldToCamera[4][4];
        __m128 worldToCameraAos[4];

#if defined(_DEBUG) || defined(DEBUG) 
        int trial = 1'000'000;
#else 
        int trial = 10'000'000;
#endif 
        
        std::cout << "calc world to camera " << trial / 1000 << " times"<< std::endl;
        {
            SimpleTimer timer;

            for (int i = 0; i < trial; ++i)
            {
                CalcWorldToCameraScalar(yaw, pitch, cameraPos, worldToCamera);
            }

        }

        std::cout << "calc world to camera with sse_aos " << trial / 1000 << " times"<< std::endl;
        {
            SimpleTimer timer;
            for (int i = 0; i < trial; ++i)
            {
                CalcWorldToCameraSSE_AOS(yaw, pitch, cameraPosAos, worldToCameraAos);
            }
        }

        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                float diff = fabs(worldToCamera[i][j] - worldToCameraAos[i].m128_f32[j]);
                float maxTolerrance = std::max(absEpsilon, relEpsilon * fabs(worldToCamera[i][j]));
                if (diff > maxTolerrance)
                {
                    std::cout << "warning : significant mismatch in component " << i << " " << j << std::endl;
                }
            }
        }
    }

    {
        const int n = 1000;
        AABB inAABB[n];        
        AABB outAABB[n];
        float mat[n][4][4];

        AABB_AOS inAABB_aos[n];
        AABB_AOS outAABB_aos[n];
        __m128 mat_aos[n][4];

        __m128 inXMin[n / 4];
        __m128 inYMin[n / 4];
        __m128 inZMin[n / 4];
        __m128 inXMax[n / 4];
        __m128 inYMax[n / 4];
        __m128 inZMax[n / 4];

        __m128 outXMin[n / 4];
        __m128 outYMin[n / 4];
        __m128 outZMin[n / 4];
        __m128 outXMax[n / 4];
        __m128 outYMax[n / 4];
        __m128 outZMax[n / 4];

        AABB_SOA inAABB_soa = {inXMin, inYMin, inZMin, inXMax, inYMax, inZMax};
        AABB_SOA outAABB_soa = {outXMin, outYMin, outZMin, outXMax, outYMax, outZMax};

        __m128 matComp_soa[4][4][n / 4];
        Matrix_SOA mat_soa;
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                mat_soa.comp[i][j] = matComp_soa[i][j];
            }
        }

        SimpleRNG rng;
        rng.Seed(47);
        for (int i = 0; i < n; ++i)
        {
            float aabbMin[3] = 
            {
                rng.RandFloat(-10, 10),
                rng.RandFloat(-10, 10),
                rng.RandFloat(-10, 10),
            };
            float aabbMax[3] = 
            {
                aabbMin[0] + rng.RandFloat(-10, 10),
                aabbMin[1] + rng.RandFloat(-10, 10),
                aabbMin[2] + rng.RandFloat(-10, 10),
            };

            inAABB[i].min[0] = aabbMin[0];
            inAABB[i].min[1] = aabbMin[1];
            inAABB[i].min[2] = aabbMin[2];
            inAABB[i].max[0] = aabbMax[0];
            inAABB[i].max[1] = aabbMax[1];
            inAABB[i].max[2] = aabbMax[2];

            inAABB_aos[i].min.m128_f32[0] = aabbMin[0]; 
            inAABB_aos[i].min.m128_f32[1] = aabbMin[1];
            inAABB_aos[i].min.m128_f32[2] = aabbMin[2];
            inAABB_aos[i].max.m128_f32[0] = aabbMax[0];
            inAABB_aos[i].max.m128_f32[1] = aabbMax[1];
            inAABB_aos[i].max.m128_f32[2] = aabbMax[2];

            inXMin[i / 4].m128_f32[i % 4] = aabbMin[0];
            inYMin[i / 4].m128_f32[i % 4] = aabbMin[1];
            inZMin[i / 4].m128_f32[i % 4] = aabbMin[2];
            inXMax[i / 4].m128_f32[i % 4] = aabbMax[0];
            inYMax[i / 4].m128_f32[i % 4] = aabbMax[1];
            inZMax[i / 4].m128_f32[i % 4] = aabbMax[2];

            float translate[] =
            {
                rng.RandFloat(-10, 10),
                rng.RandFloat(-10, 10),
                rng.RandFloat(-10, 10),
            };

            float yaw = rng.RandFloat(0, 2.0f * 3.14f);
            float pitch = rng.RandFloat();
            CalcWorldToCameraScalar(yaw, pitch, translate, mat[i]);
            for (int j = 0; j < 4; ++j)
            {
                for (int k = 0; k < 4; ++k)
                {
                    matComp_soa[j][k][i / 4].m128_f32[i % 4] = mat_aos[i][j].m128_f32[k] = mat[i][j][k];
                }
            }
        }
#if defined(_DEBUG)
        int trials = 1'000;
#else 
        int trials = 10'000;
#endif /**/

        std::cout << "TransformAABBToScalar " << n / 1000 << " AABB " << trials / 1000 << " " << std::endl;
        {
            SimpleTimer timer;
            for (int i = 0; i < trials; ++i)
            {
                TransformAABBToScalar(n, inAABB, mat, outAABB);
            }
        }

        std::cout << "TransformAABBToSSEAOS " << n / 1000 << " AABB " << trials / 1000 << " " << std::endl;
        {
            SimpleTimer timer;
            for (int i = 0; i < trials; ++i)
            {
                TransformAABBToSSEAOS(n, inAABB_aos, mat_aos, outAABB_aos);
            }
        }

        std::cout << "TransformAABBToSSESOA " << n / 1000 << " AABB " << trials / 1000 << " " << std::endl;
        {
            SimpleTimer timer;
            for (int i = 0; i < trials; ++i)
            {
                TransformAABBToSSESOA(n, inAABB_soa, mat_soa, outAABB_soa);
            }
        }

        int i = 0;
        for (int i = 0; i < n; ++i)
        {
            float diffAOS_min0 = fabs(outAABB[i].min[0] - outAABB_aos[i].min.m128_f32[0]);
            float diffAOS_min1 = fabs(outAABB[i].min[1] - outAABB_aos[i].min.m128_f32[1]);
            float diffAOS_min2 = fabs(outAABB[i].min[2] - outAABB_aos[i].min.m128_f32[2]);
            float diffAOS_max0 = fabs(outAABB[i].max[0] - outAABB_aos[i].max.m128_f32[0]);
            float diffAOS_max1 = fabs(outAABB[i].max[1] - outAABB_aos[i].max.m128_f32[1]);
            float diffAOS_max2 = fabs(outAABB[i].max[2] - outAABB_aos[i].max.m128_f32[2]);
                
            float maxTolerranceAOS_min0 = std::max(absEpsilon, relEpsilon * fabs(outAABB[i].min[0]));
            float maxTolerranceAOS_min1 = std::max(absEpsilon, relEpsilon * fabs(outAABB[i].min[1]));
            float maxTolerranceAOS_min2 = std::max(absEpsilon, relEpsilon * fabs(outAABB[i].min[2]));
            float maxTolerranceAOS_max0 = std::max(absEpsilon, relEpsilon * fabs(outAABB[i].max[0]));
            float maxTolerranceAOS_max1 = std::max(absEpsilon, relEpsilon * fabs(outAABB[i].max[1]));
            float maxTolerranceAOS_max2 = std::max(absEpsilon, relEpsilon * fabs(outAABB[i].max[2]));

            if ((diffAOS_min0 > maxTolerranceAOS_min0)
                || (diffAOS_min1 > maxTolerranceAOS_min1)
                || (diffAOS_min2 > maxTolerranceAOS_min2)
                || (diffAOS_max0 > maxTolerranceAOS_max0)
                || (diffAOS_max1 > maxTolerranceAOS_max1)
                || (diffAOS_max2 > maxTolerranceAOS_max2))
            {
                std::cout << "warnning : significant mismatch in AOS aabb " << i << std::endl;
            }

            float diffSOA_minX = fabs(outAABB[i].min[0] - outXMin[i / 4].m128_f32[i % 4]);
            float diffSOA_minY = fabs(outAABB[i].min[1] - outYMin[i / 4].m128_f32[i % 4]);
            float diffSOA_minZ = fabs(outAABB[i].min[2] - outZMin[i / 4].m128_f32[i % 4]);
            float diffSOA_maxX = fabs(outAABB[i].max[0] - outXMax[i / 4].m128_f32[i % 4]);
            float diffSOA_maxY = fabs(outAABB[i].max[1] - outYMax[i / 4].m128_f32[i % 4]);
            float diffSOA_maxZ = fabs(outAABB[i].max[2] - outZMax[i / 4].m128_f32[i % 4]);

            float maxTolerranceAOS_minX = std::max(absEpsilon, relEpsilon * fabs(outAABB[i].min[0]));
            float maxTolerranceAOS_minY = std::max(absEpsilon, relEpsilon * fabs(outAABB[i].min[1]));
            float maxTolerranceAOS_minZ = std::max(absEpsilon, relEpsilon * fabs(outAABB[i].min[2]));
            float maxTolerranceAOS_maxX = std::max(absEpsilon, relEpsilon * fabs(outAABB[i].max[0]));
            float maxTolerranceAOS_maxY = std::max(absEpsilon, relEpsilon * fabs(outAABB[i].max[1]));
            float maxTolerranceAOS_maxZ = std::max(absEpsilon, relEpsilon * fabs(outAABB[i].max[2]));

            if ((diffSOA_minX > maxTolerranceAOS_minX)
                || (diffSOA_minY > maxTolerranceAOS_minY)
                || (diffSOA_minZ > maxTolerranceAOS_minZ)
                || (diffSOA_maxX > maxTolerranceAOS_maxX)
                || (diffSOA_maxY > maxTolerranceAOS_maxY)
                || (diffSOA_maxZ > maxTolerranceAOS_maxZ))
            {
                std::cout << "warnning : significant mismatch in SOA aabb " << i << std::endl;
            }
        }
    }

    return 0;
}
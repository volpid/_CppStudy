
#include "ch07_waves.h"

#include <algorithm>
#include <ppl.h>

//----------------------------------------------------------------
// Waves
//----------------------------------------------------------------
WavesCh07::WavesCh07(int row, int col, float dx, float dt, float speed, float damping)
{
    numRow_ = row;
    numCol_ = col;

    vertexCount_ = row * col;
    triangleCount = (row - 1) * (col - 1) * 2;

    timeStep_ = dt;
    spatialStep_ = dx;

    float d = damping * dt + 2.0f;
    float e = (speed * speed) * (dt * dt) / (dx * dx);
    k1_ = (damping * dt - 2.0f) / d;
    k2_ = (4.0f - 8.0f * e) / d;
    k3_ = (2.0f * e) / d;

    prevSolution_.resize(vertexCount_);
    curSolution_.resize(vertexCount_);
    normals_.resize(vertexCount_);
    tangentX_.resize(vertexCount_);

    float halfWidth = (col - 1) * dx * 0.5f;
    float halfDepth = (row - 1) * dx * 0.5f;
    for (int rowIdx = 0; rowIdx < row; ++rowIdx)
    {
        float z = halfDepth - rowIdx * dx;
        for (int colIdx = 0; colIdx < col; ++colIdx)
        {
            float x = -halfWidth + colIdx * dx;

            prevSolution_[rowIdx * col + colIdx] = DirectX::XMFLOAT3(x, 0.0f, z);
            curSolution_[rowIdx * col + colIdx] = DirectX::XMFLOAT3(x, 0.0f, z);
            normals_[rowIdx * col + colIdx] = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
            tangentX_[rowIdx * col + colIdx] = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
        }
    }
}
    
void WavesCh07::Update(float dt)
{
    static float ts = 0.0f;
    ts += dt;

    if (ts >= timeStep_)
    {
        auto calcSolution = [this](int rowIdx)
        {
            for (int colIdx = 1; colIdx < numCol_ - 1; ++colIdx)
            {
                prevSolution_[rowIdx * numCol_ + colIdx].y = k1_ * prevSolution_[rowIdx * numCol_ + colIdx].y
                    + k2_ * curSolution_[rowIdx * numCol_ + colIdx].y
                    + k3_ * (curSolution_[(rowIdx + 1) * numCol_ + colIdx].y
                        + curSolution_[(rowIdx - 1) * numCol_ + colIdx].y
                        + curSolution_[rowIdx * numCol_ + colIdx + 1].y
                        + curSolution_[rowIdx * numCol_ + colIdx - 1].y);
            }
        };
        
        concurrency::parallel_for(1, numRow_ - 1, calcSolution);
        std::swap(prevSolution_, curSolution_);

        ts = 0.0f;

        auto calcNormalTangent = [this](int rowIdx)
        {
            for (int colIdx = 1; colIdx < numCol_ - 1; ++colIdx)
            {
                float left = curSolution_[rowIdx * numCol_ + colIdx - 1].y;
                float right = curSolution_[rowIdx * numCol_ + colIdx + 1].y;
                float top = curSolution_[(rowIdx - 1) * numCol_ + colIdx].y;
                float bottom = curSolution_[(rowIdx + 1) * numCol_ + colIdx].y;

                normals_[rowIdx * numCol_ + colIdx].x = - right + left;
                normals_[rowIdx * numCol_ + colIdx].y = 2.0f * spatialStep_;
                normals_[rowIdx * numCol_ + colIdx].z = bottom - top;

                DirectX::XMVECTOR n = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&normals_[rowIdx * numCol_ + colIdx]));
                DirectX::XMStoreFloat3(&normals_[rowIdx * numCol_ + colIdx], n);

                tangentX_[rowIdx * numCol_ + colIdx] = DirectX::XMFLOAT3(2.0f * spatialStep_, right - left, 0.0f);
                DirectX::XMVECTOR t = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&tangentX_[rowIdx * numCol_ + colIdx]));
                DirectX::XMStoreFloat3(&tangentX_[rowIdx * numCol_ + colIdx], t);
            }
        };

        concurrency::parallel_for(1, numRow_ - 1, calcNormalTangent);
    }
}

void WavesCh07::Disturb(int row, int col, float magnitude)
{
    assert((row > 1) && (row < numRow_ -  2));
    assert((col > 1) && (col < numCol_ -  2));

    float halfMag = 0.5f * magnitude;    
    curSolution_[row * numCol_ + col].y += magnitude;
    curSolution_[row * numCol_ + col + 1].y += halfMag;
    curSolution_[row * numCol_ + col - 1].y += halfMag;
    curSolution_[(row + 1) * numCol_ + col].y += halfMag;
    curSolution_[(row - 1) * numCol_ + col].y += halfMag;        
}

#ifndef __CH07_WAVES__H__
#define __CH07_WAVES__H__

#include <DirectXMath.h>
#include <vector>

//----------------------------------------------------------------
// Waves
//----------------------------------------------------------------
class WavesCh07
{
public:
    WavesCh07(int row, int col, float dx, float dt, float speed, float damping);
    
    ~WavesCh07(void) = default;
    
    WavesCh07(const WavesCh07& rhs) = delete;
    WavesCh07& operator=(const WavesCh07& rhs) = delete;

    void Update(float dt);
    void Disturb(int row, int col, float magnitude);

    inline int RowCount(void) const;
    inline int ColumnCount(void) const;
    inline int VertexCount(void) const;
    inline int TriangleCount(void) const;
    inline float Width(void) const;
    inline float Height(void) const;

    inline const DirectX::XMFLOAT3& Position(int idx) const;
    inline const DirectX::XMFLOAT3& Normal(int idx) const;
    inline const DirectX::XMFLOAT3& TangentX(int idx) const;

private:
    int numRow_ = 0;
    int numCol_ = 0;

    int vertexCount_ = 0;
    int triangleCount = 0;

    float k1_ = 0.0f;
    float k2_ = 0.0f;
    float k3_ = 0.0f;

    float timeStep_ = 0.0f;
    float spatialStep_ = 0.0f;

    std::vector<DirectX::XMFLOAT3> prevSolution_;
    std::vector<DirectX::XMFLOAT3> curSolution_;
    std::vector<DirectX::XMFLOAT3> normals_;
    std::vector<DirectX::XMFLOAT3> tangentX_;
};

//----------------------------------------------------------------
// inline section
//----------------------------------------------------------------
inline int WavesCh07::RowCount(void) const
{
    return numRow_;
}

inline int WavesCh07::ColumnCount(void) const
{
    return numCol_;
}

inline int WavesCh07::VertexCount(void) const
{
    return vertexCount_;
}

inline int WavesCh07::TriangleCount(void) const
{
    return triangleCount;
}

inline float WavesCh07::Width(void) const
{
    return numRow_ * spatialStep_;
}

inline float WavesCh07::Height(void) const
{
    return numCol_ * spatialStep_;
}

inline const DirectX::XMFLOAT3& WavesCh07::Position(int idx) const
{
    return curSolution_[idx];
}

inline const DirectX::XMFLOAT3& WavesCh07::Normal(int idx) const
{
    return normals_[idx];
}

inline const DirectX::XMFLOAT3& WavesCh07::TangentX(int idx) const
{
    return tangentX_[idx];
}

#endif /*__CH07_WAVES__H__*/
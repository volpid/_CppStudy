
#ifndef __CH08_FRAME_RESOURCE__H__
#define __CH08_FRAME_RESOURCE__H__

#include "../Common/d3d_util.h"
#include "../Common/math_helper.h"

//----------------------------------------------------------------
// render item
//----------------------------------------------------------------
const int NumFrameResourceCh08 = NumGlobalFrameResource;

struct RenderItemCh08
{
    DirectX::XMFLOAT4X4 world = MathHelper::Indentity4x4();
    DirectX::XMFLOAT4X4 texTransform = MathHelper::Indentity4x4();

    int numFrameDirty = NumFrameResourceCh08;
    int objCBIndex = -1;

    Material* material = nullptr;
    MeshGeometry* geometry = nullptr;

    D3D12_PRIMITIVE_TOPOLOGY primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    UINT indexCount = 0;
    UINT startIndexLocation = 0;
    int baseVertexLocation = 0;

    RenderItemCh08(void) = default;
};

#endif /*__CH08_FRAME_RESOURCE__H__*/

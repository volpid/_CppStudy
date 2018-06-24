
#ifndef __CH09_COMMAND_RECORDING_AND_DRAWING__H__
#define __CH09_COMMAND_RECORDING_AND_DRAWING__H__

#include "../Common/vulkan_common.h"

namespace Cookbook
{
    void SetViewportStateDynamically(VkCommandBuffer commandBuffer, uint32_t firstViewport, const std::vector<VkViewport>& viewports);
    void SetScissorStateDynamically(VkCommandBuffer commandBuffer, uint32_t firstScissor, const std::vector<VkRect2D>& scissors);
    void BindVertexBuffer(VkCommandBuffer commandBuffer, 
        uint32_t firstBinding, 
        const std::vector<VertexBufferParameter>& bufferParameters);
    void DrawGeometry(VkCommandBuffer commandBuffer, 
        uint32_t vertexCount,
        uint32_t instanceCount,
        uint32_t firstVertex,
        uint32_t firstInstance);
}

#endif /*__CH09_COMMAND_RECORDING_AND_DRAWING__H__*/
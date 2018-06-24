
#include "ch09_command_recording_and_drawing.h"

void Cookbook::SetViewportStateDynamically(VkCommandBuffer commandBuffer, uint32_t firstViewport, const std::vector<VkViewport>& viewports)
{
    vkCmdSetViewport(commandBuffer, firstViewport, static_cast<int32_t> (viewports.size()), viewports.data());
}

void Cookbook::SetScissorStateDynamically(VkCommandBuffer commandBuffer, uint32_t firstScissor, const std::vector<VkRect2D>& scissors)
{
    vkCmdSetScissor(commandBuffer, firstScissor, static_cast<int32_t> (scissors.size()), scissors.data());
}

void Cookbook::BindVertexBuffer(VkCommandBuffer commandBuffer, 
    uint32_t firstBinding, 
    const std::vector<VertexBufferParameter>& bufferParameters)
{
    if (bufferParameters.size() > 0)
    {
        std::vector<VkBuffer> buffers;
        std::vector<VkDeviceSize> offsets;
        for (auto& bufferParameter : bufferParameters)
        {
            buffers.push_back(bufferParameter.buffer);
            offsets.push_back(bufferParameter.memoryOffset);
        }

        vkCmdBindVertexBuffers(commandBuffer, firstBinding, static_cast<uint32_t> (bufferParameters.size()), buffers.data(), offsets.data());
    }
}

void Cookbook::DrawGeometry(VkCommandBuffer commandBuffer, 
    uint32_t vertexCount,
    uint32_t instanceCount,
    uint32_t firstVertex,
    uint32_t firstInstance)
{
    vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}
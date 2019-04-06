
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
    bool PrepareSingleFrameOfAnimation(VkDevice logicalDevice, 
        VkQueue graphicsQueue,
        VkQueue presentQueue,
        VkSwapchainKHR swapChain,
        VkExtent2D swapChainSize,
        const std::vector<VkImageView>& swapChainImageViews,
        VkImageView depthAttachment,
        const std::vector<WaitSemaphoreInfo>& waitInfos,
        VkSemaphore imageAcquiredSemaphore,
        VkSemaphore readyToDrawingSemaphore,
        VkFence finishDrawingFence,
        std::function<bool (VkCommandBuffer, uint32_t, VkFramebuffer)> recordCommandBuffer,
        VkCommandBuffer commandBuffer,
        VkRenderPass renderPass,
        VkDestroyer(VkFramebuffer)& framebuffer);
    bool IncreasePerformanceThroughIncreasingTheNumberOfSeparatelyRenderedFrames(VkDevice logicalDevice,
        VkQueue graphicsQueue,
        VkQueue presentQueue,
        VkSwapchainKHR swapChain,
        VkExtent2D swapChainSize,
        const std::vector<VkImageView>& swapChainImageViews,
        VkRenderPass renderPass,
        const std::vector<WaitSemaphoreInfo>& waitInfos,
        std::function<bool (VkCommandBuffer, uint32_t, VkFramebuffer)> recordCommandBuffer,
        std::vector<FrameResources>& frameResources);
    void ProvideDataToShaderThroughPushConstants(VkCommandBuffer commandBuffer,
        VkPipelineLayout pipelineLayout,
        VkShaderStageFlags pipelineStages,
        uint32_t offset,
        uint32_t size,
        void* data);
    void DispatchComputeWork(VkCommandBuffer commandBuffer, uint32_t xSize,  uint32_t ySize,  uint32_t zSize);
}

#endif /*__CH09_COMMAND_RECORDING_AND_DRAWING__H__*/
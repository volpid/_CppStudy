
#include "ch09_command_recording_and_drawing.h"

#include "ch02_image_presentation.h"
#include "ch03_command_buffer_and_synchronization.h"
#include "ch06_renderpass_and_framebuffer.h"

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

bool Cookbook::PrepareSingleFrameOfAnimation(VkDevice logicalDevice, 
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
    VkDestroyer(VkFramebuffer)& framebuffer)
{
    uint32_t imageIndex;
    if (AcquireSwapchainImage(logicalDevice, swapChain, imageAcquiredSemaphore, VK_NULL_HANDLE, imageIndex) == false)
    {
        return false;
    }

    std::vector<VkImageView> attachments = {swapChainImageViews[imageIndex]};
    if (depthAttachment != VK_NULL_HANDLE)
    {
        attachments.push_back(depthAttachment);
    }

    if (CreateFramebuffer(logicalDevice, renderPass, attachments, swapChainSize.width, swapChainSize.height, 1, *framebuffer) == false)
    {
        return false;
    }

    if (recordCommandBuffer(commandBuffer, imageIndex, *framebuffer) == false)
    {
        return false;
    }

    std::vector<WaitSemaphoreInfo> waitSemaphoreInfos = waitInfos;
    waitSemaphoreInfos.push_back({imageAcquiredSemaphore, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT});

    if (SubmitCommandBufferToQueue(graphicsQueue,
        waitSemaphoreInfos, 
        {commandBuffer},
        {readyToDrawingSemaphore},
        finishDrawingFence) == false)
    {
        return false;
    }

    PresentInfo presentInfo = 
    {
        swapChain,
        imageIndex
    };

    if (PresentImage(presentQueue, {readyToDrawingSemaphore}, {presentInfo}) == false)
    {
        return false;
    }

    return true;
}

bool Cookbook::IncreasePerformanceThroughIncreasingTheNumberOfSeparatelyRenderedFrames(VkDevice logicalDevice,
    VkQueue graphicsQueue,
    VkQueue presentQueue,
    VkSwapchainKHR swapChain,
    VkExtent2D swapChainSize,
    const std::vector<VkImageView>& swapChainImageViews,
    VkRenderPass renderPass,
    const std::vector<WaitSemaphoreInfo>& waitInfos,
    std::function<bool (VkCommandBuffer, uint32_t, VkFramebuffer)> recordCommandBuffer,
    std::vector<FrameResources>& frameResources)
{
    static uint32_t frameIndex = 0;
    FrameResources& currentFrame = frameResources[frameIndex];

    if (WaitForFence(logicalDevice, {*currentFrame.drawingFinishFence}, false, 2'000'000'000) == false)
    {
        return false;
    }
    if (ResetFence(logicalDevice, {*currentFrame.drawingFinishFence}) == false)
    {
        return false;
    }

    InitVkDestroyer(logicalDevice, currentFrame.frameBuffer);
    if (PrepareSingleFrameOfAnimation(logicalDevice,
        graphicsQueue,
        presentQueue,
        swapChain,
        swapChainSize,
        swapChainImageViews,
        *currentFrame.depthAttachment,
        waitInfos,
        *currentFrame.imageAcquiredSemaphore,
        *currentFrame.readyToPresentSemaphore,
        *currentFrame.drawingFinishFence,
        recordCommandBuffer,
        currentFrame.commandBuffer,
        renderPass,
        currentFrame.frameBuffer) == false)
    {
        return false;
    }

    frameIndex = (frameIndex + 1) % frameResources.size();
    return true;
}

void Cookbook::ProvideDataToShaderThroughPushConstants(VkCommandBuffer commandBuffer,
    VkPipelineLayout pipelineLayout,
    VkShaderStageFlags pipelineStages,
    uint32_t offset,
    uint32_t size,
    void* data)
{
    vkCmdPushConstants(commandBuffer, pipelineLayout, pipelineStages, offset, size, data);
}

void Cookbook::DispatchComputeWork(VkCommandBuffer commandBuffer, uint32_t xSize,  uint32_t ySize,  uint32_t zSize)
{
    vkCmdDispatch(commandBuffer, xSize, ySize, zSize);
}
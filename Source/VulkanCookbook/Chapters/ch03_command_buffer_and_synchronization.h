
#ifndef __CH3_COMMAND_BUFFER_AND_SYNCHRONIZATION__H__
#define __CH3_COMMAND_BUFFER_AND_SYNCHRONIZATION__H__

#include "../Common/vulkan_common.h"

#include <vector>

namespace Cookbook
{
    bool CreateCommandPool(VkDevice logicalDevice, 
        VkCommandPoolCreateFlags commandPoolFlags, 
        uint32_t queueFamily, 
        VkCommandPool& commandPool);
    bool AllocateCommandBuffer(VkDevice logicalDevice,
        VkCommandPool commandPool,
        VkCommandBufferLevel level,
        uint32_t count,
        std::vector<VkCommandBuffer>& commandBuffers);
    bool CreateSemaphore(VkDevice logicalDevice, VkSemaphore& semaphore);
    bool CreateFence(VkDevice logicalDevice, bool signaled, VkFence& fence);
    bool WaitForAllSubmittedCommandToBeFinished(VkDevice logicalDevice);
    bool EndCommandBufferRecordingOperation(VkCommandBuffer commandBuffer);
    bool SubmitCommandBufferToQueue(VkQueue queue,
        std::vector<WaitSemaphoreInfo> waitSemephoreInfos,
        std::vector<VkCommandBuffer> commandBuffers,
        std::vector<VkSemaphore> signalSemaphores,
        VkFence fence);
}

#endif /*__CH3_COMMAND_BUFFER_AND_SYNCHRONIZATION__H__*/

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
    bool WaitForFence(VkDevice logicalDevice, const std::vector<VkFence>& fences, VkBool32 waitForAll, uint64_t timeout);
    bool ResetFence(VkDevice logicalDevice, const std::vector<VkFence>& fences);     
    bool BeginCommandBufferRecordingOperation(VkCommandBuffer commandbuffer,
        VkCommandBufferUsageFlags usage,
        VkCommandBufferInheritanceInfo* secondaryCommandBufferInfo);
    bool ResetCommandBuffer(VkCommandBuffer commandBuffer, bool releaseResource);
    bool ResetCommandPool(VkDevice logicalDevice, VkCommandPool commandpool, bool releaseResource);
    bool SynchronizeTwoCommandBuffer(VkQueue firstQueue,
        std::vector<WaitSemaphoreInfo> firstWaitSemaphoreInfos,
        std::vector<VkCommandBuffer> firstCommandBuffers,
        std::vector<WaitSemaphoreInfo> synchronizingSemaphoreInfos,
        VkQueue secondQueue,
        std::vector<VkCommandBuffer> secondCommandBuffers,
        std::vector<VkSemaphore> secondSignalSemaphores,
        VkFence secondFence);
    bool CheckIfProcessingOfSubmittedCommandBufferHasFinished(VkDevice logicalDevice,
        VkQueue queue,
        std::vector<WaitSemaphoreInfo> waitSemaphoreInfos,
        std::vector<VkCommandBuffer> commandBuffers,
        std::vector<VkSemaphore> signalSemaphores,
        VkFence fence,
        uint64_t timeout,
        VkResult& waitStatus);
    bool WaitUntilAllCommandSubmittedToQueueAreFinished(VkQueue queue);
    void DestroyFence(VkDevice logicalDevice, VkFence& fence);
    void DestroySemaphore(VkDevice logicalDevice, VkSemaphore& semaphore);
    void FreeCommandBuffer(VkDevice logicalDevice, VkCommandPool commandPool, std::vector<VkCommandBuffer>& commandbuffers);
    void DestroyCommandPool(VkDevice logicalDevice, VkCommandPool& commandPool);
}

#endif /*__CH3_COMMAND_BUFFER_AND_SYNCHRONIZATION__H__*/
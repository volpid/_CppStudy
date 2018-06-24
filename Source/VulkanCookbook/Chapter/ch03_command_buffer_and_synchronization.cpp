
#include "ch03_command_buffer_and_synchronization.h"

#include <iostream>

bool Cookbook::CreateCommandPool(VkDevice logicalDevice, 
    VkCommandPoolCreateFlags commandPoolFlags, 
    uint32_t queueFamily, 
    VkCommandPool& commandPool)
{
    VkCommandPoolCreateInfo commandPoolCraeteInfo =
    {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        nullptr,
        commandPoolFlags,
        queueFamily
    };
    
    VkResult result = vkCreateCommandPool(logicalDevice, &commandPoolCraeteInfo, nullptr, &commandPool);
    if (result != VK_SUCCESS)
    {
        std::cout << "Could not create command pool." << std::endl;
        return false;
    }

    return true;
}

bool Cookbook::AllocateCommandBuffer(VkDevice logicalDevice,
    VkCommandPool commandPool,
    VkCommandBufferLevel level,
    uint32_t count,
    std::vector<VkCommandBuffer>& commandBuffers)
{
    VkCommandBufferAllocateInfo commandbufferAllocateInfo = 
    {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        nullptr,
        commandPool,
        level,
        count
    };

    commandBuffers.resize(count);
    VkResult result = vkAllocateCommandBuffers(logicalDevice, &commandbufferAllocateInfo, commandBuffers.data());
    if (result != VK_SUCCESS)
    {
        std::cout << "Could not alllocate command buffers." << std::endl;
        return false;
    }

    return true;
}

bool Cookbook::CreateSemaphore(VkDevice logicalDevice, VkSemaphore& semaphore)
{
    VkSemaphoreCreateInfo semaphoreCreteInfo = 
    {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        nullptr,
        0
    };

    VkResult result = vkCreateSemaphore(logicalDevice, &semaphoreCreteInfo, nullptr, &semaphore);
    if (result != VK_SUCCESS)
    {
        std::cout << "Could not create a semaphore." << std::endl;
        return false;
    }
    
    return true;
}

bool Cookbook::CreateFence(VkDevice logicalDevice, bool signaled, VkFence& fence)
{
    VkFenceCreateInfo fenceCreateInfo = 
    {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        nullptr,
        (signaled == true) ? (VK_FENCE_CREATE_SIGNALED_BIT) : (0u)
    };

    VkResult result = vkCreateFence(logicalDevice, &fenceCreateInfo, nullptr, &fence);
    if (result != VK_SUCCESS)
    {
        std::cout << "Could not create a create." << std::endl;
        return false;
    }
    
    return true;
}

bool Cookbook::WaitForAllSubmittedCommandToBeFinished(VkDevice logicalDevice)
{
    VkResult result = vkDeviceWaitIdle(logicalDevice);
    if (result != VK_SUCCESS)
    {
        std::cout << "Waiting on a device failed." << std::endl;
        return false;
    }

    return true;
}

bool Cookbook::EndCommandBufferRecordingOperation(VkCommandBuffer commandBuffer)
{
    VkResult result = vkEndCommandBuffer(commandBuffer);
    if (result != VK_SUCCESS)
    {
        std::cout << "Error occurred during command buffer recoding." << std::endl;
        return false;
    }

    return true;
}

bool Cookbook::SubmitCommandBufferToQueue(VkQueue queue,
    std::vector<WaitSemaphoreInfo> waitSemephoreInfos,
    std::vector<VkCommandBuffer> commandBuffers,
    std::vector<VkSemaphore> signalSemaphores,
    VkFence fence)
{
    std::vector<VkSemaphore> waitSemaphoreHandles;
    std::vector<VkPipelineStageFlags> waitSemaphoreStages;

    for (auto& waitSemaphore : waitSemephoreInfos)
    {
        waitSemaphoreHandles.push_back(waitSemaphore.semaphore);
        waitSemaphoreStages.push_back(waitSemaphore.waitingStage);
    }

    VkSubmitInfo submitInfo =
    {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        nullptr,
        static_cast<uint32_t> (waitSemephoreInfos.size()),
        waitSemaphoreHandles.data(),
        waitSemaphoreStages.data(),
        static_cast<uint32_t> (commandBuffers.size()),
        commandBuffers.data(),
        static_cast<uint32_t> (signalSemaphores.size()),
        signalSemaphores.data(),
    };

    VkResult result = vkQueueSubmit(queue, 1, &submitInfo, fence);
    if (result != VK_SUCCESS)
    {
        std::cout << "Error occurred during command buffer submission." << std::endl;
        return false;
    }

    return true;
}

bool Cookbook::WaitForFence(VkDevice logicalDevice, const std::vector<VkFence>& fences, VkBool32 waitForAll, uint64_t timeout)
{
    if (fences.size() > 0)
    {
        VkResult result = vkWaitForFences(logicalDevice,
            static_cast<uint32_t> (fences.size()),
            fences.data(),
            waitForAll,
            timeout);
        if (result != VK_SUCCESS)
        {
            std::cout << "Waiting on fence failed." << std::endl;
            return false;
        }

        return true;
    }

    return false;
}

bool Cookbook::ResetFence(VkDevice logicalDevice, const std::vector<VkFence>& fences)
{
    if (fences.size() > 0)
    {   
        VkResult result = vkResetFences(logicalDevice, static_cast<uint32_t> (fences.size()), fences.data());
        if (result =VK_SUCCESS)
        {
            std::cout << "Error occured when tired reset fences." << std::endl;
            return false;
        }
        return true;
    }

    return false;
}

bool Cookbook::BeginCommandBufferRecordingOperation(VkCommandBuffer commandbuffer,
    VkCommandBufferUsageFlags usage,
    VkCommandBufferInheritanceInfo* secondaryCommandBufferInfo)
{
    VkCommandBufferBeginInfo commandBufferBeginInfo = 
    {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        nullptr,
        usage,
        secondaryCommandBufferInfo
    };

    VkResult result = vkBeginCommandBuffer(commandbuffer, &commandBufferBeginInfo);
    if (result != VK_SUCCESS)
    {
        std::cout << "Could not begin command buffer recording operation." << std::endl;
        return false;
    }

    return true;
}

bool Cookbook::ResetCommandBuffer(VkCommandBuffer commandBuffer, bool releaseResource)
{
    VkResult result = vkResetCommandBuffer(commandBuffer, (releaseResource ? VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT : 0));
    if (result != VK_SUCCESS)
    {
        std::cout << "Error occurred during command buffer reset." << std::endl;
        return false;
    }

    return true;
}

bool Cookbook::ResetCommandPool(VkDevice logicalDevice, VkCommandPool commandpool, bool releaseResource)
{   
    VkResult result = vkResetCommandPool(logicalDevice, commandpool, (releaseResource ? VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT: 0));
    if (result != VK_SUCCESS)
    {
        std::cout << "Error occurred during command pool reset." << std::endl;
        return false;
    }

    return true;
}

bool Cookbook::SynchronizeTwoCommandBuffer(VkQueue firstQueue,
    std::vector<WaitSemaphoreInfo> firstWaitSemaphoreInfos,
    std::vector<VkCommandBuffer> firstCommandBuffers,
    std::vector<WaitSemaphoreInfo> synchronizingSemaphoreInfos,
    VkQueue secondQueue,
    std::vector<VkCommandBuffer> secondCommandBuffers,
    std::vector<VkSemaphore> secondSignalSemaphores,
    VkFence secondFence)
{
    std::vector<VkSemaphore> firstSignalSemaphores;
    for (auto& semaphoreInfo : synchronizingSemaphoreInfos)
    {
        firstSignalSemaphores.push_back(semaphoreInfo.semaphore);
    }

    if (SubmitCommandBufferToQueue(firstQueue, firstWaitSemaphoreInfos, firstCommandBuffers, firstSignalSemaphores, VK_NULL_HANDLE) == false)
    {
        return false;
    }

    if (SubmitCommandBufferToQueue(secondQueue, synchronizingSemaphoreInfos, secondCommandBuffers, secondSignalSemaphores, secondFence) == false)
    {
        return false;
    }

    return true;
}

bool Cookbook::CheckIfProcessingOfSubmittedCommandBufferHasFinished(VkDevice logicalDevice,
    VkQueue queue,
    std::vector<WaitSemaphoreInfo> waitSemaphoreInfos,
    std::vector<VkCommandBuffer> commandBuffers,
    std::vector<VkSemaphore> signalSemaphores,
    VkFence fence,
    uint64_t timeout,
    VkResult& waitStatus)
{
    if (SubmitCommandBufferToQueue(queue, waitSemaphoreInfos, commandBuffers, signalSemaphores, fence) == false)
    {
        return false;
    }
    
    return WaitForFence(logicalDevice, {fence}, VK_FALSE, timeout);
}

bool Cookbook::WaitUntilAllCommandSubmittedToQueueAreFinished(VkQueue queue)
{
    VkResult result = vkQueueWaitIdle(queue);
    if (result != VK_SUCCESS)
    {
        std::cout << "Waiting for all operations submitted to queue failed." << std::endl;
        return false;
    }

    return true;
}

void Cookbook::DestroyFence(VkDevice logicalDevice, VkFence& fence)
{
    if (fence != VK_NULL_HANDLE)
    {
        vkDestroyFence(logicalDevice, fence, nullptr);
        fence = VK_NULL_HANDLE;
    }
}

void Cookbook::DestroySemaphore(VkDevice logicalDevice, VkSemaphore& semaphore)
{
    if (semaphore != VK_NULL_HANDLE)
    {
        vkDestroySemaphore(logicalDevice, semaphore, nullptr);
        semaphore = VK_NULL_HANDLE;
    }
}

void Cookbook::FreeCommandBuffer(VkDevice logicalDevice, VkCommandPool commandPool, std::vector<VkCommandBuffer>& commandbuffers)
{
    if (commandbuffers.size() > 0)
    {
        vkFreeCommandBuffers(logicalDevice, 
            commandPool, 
            static_cast<uint32_t> (commandbuffers.size()), 
            commandbuffers.data());
        commandbuffers.clear();
    }
}

void Cookbook::DestroyCommandPool(VkDevice logicalDevice, VkCommandPool& commandPool)
{
    if (commandPool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(logicalDevice, commandPool, nullptr);
        commandPool = VK_NULL_HANDLE;
    }
}
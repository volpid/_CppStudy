
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
    
    return false;
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
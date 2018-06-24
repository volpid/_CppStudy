
#include "ch04_resource_and_memory.h"

#include "ch03_command_buffer_and_synchronization.h"

#include <iostream>

bool Cookbook::CreateImageView(VkDevice logicalDevice,
    VkImage image,
    VkImageViewType viewType,
    VkFormat format,
    VkImageAspectFlags aspect,
    VkImageView& imageView)
{
    VkImageViewCreateInfo imageViewCreateInfo = 
    {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        nullptr,
        0,
        image,
        viewType,
        format,
        {
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
        },
        {
            aspect,
            0,
            VK_REMAINING_MIP_LEVELS,
            0,
            VK_REMAINING_ARRAY_LAYERS
        },
    };

    VkResult result = vkCreateImageView(logicalDevice, &imageViewCreateInfo, nullptr, &imageView);
    if (result != VK_SUCCESS)
    {
        std::cout << "Could not create an image view." << std::endl;
        return false;
    }
    return true;
}

bool Cookbook::Create2DImageAndView(VkPhysicalDevice physicalDevice,
    VkDevice logicalDevice,
    VkFormat format,
    VkExtent2D size,
    uint32_t numMipmaps,
    uint32_t numLayers,
    VkSampleCountFlagBits samples,
    VkImageUsageFlags usage,
    VkImageAspectFlags aspect,
    VkImage& image,
    VkDeviceMemory& memoryObject,
    VkImageView& imageView)
{
    if (CreateImage(logicalDevice, VK_IMAGE_TYPE_2D, format, {size.width, size.height, 1}, numMipmaps, numLayers, samples, usage, false, image) == false)
    {
        return false;
    }

    if (AllocateAndBindMemoryObjectToImage(physicalDevice, logicalDevice, image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memoryObject) == false)
    {
        return false;
    }

    if (CreateImageView(logicalDevice, image, VK_IMAGE_VIEW_TYPE_2D, format, aspect, imageView) == false)
    {
        return false;
    }

    return true;
}

bool Cookbook::CreateImage(VkDevice logicalDevice,
    VkImageType type,
    VkFormat format,
    VkExtent3D size,
    uint32_t numMipmaps,
    uint32_t numLayers,
    VkSampleCountFlagBits samples,
    VkImageUsageFlags usageScenarios,
    bool cubemap,
    VkImage& image)
{
    VkImageCreateInfo imageCreateInfo = 
    {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        nullptr,
        cubemap ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0u,
        type, 
        format,
        size,
        numMipmaps,
        cubemap ? (numLayers * 6) : (numLayers),
        samples,
        VK_IMAGE_TILING_OPTIMAL,
        usageScenarios,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr,
        VK_IMAGE_LAYOUT_UNDEFINED
    };

    VkResult result = vkCreateImage(logicalDevice, &imageCreateInfo, nullptr, &image);
    if (result != VK_SUCCESS)
    {
        std::cout << "Could not create an image." << std::endl;
        return false;
    }

    return true;
}

bool Cookbook::AllocateAndBindMemoryObjectToImage(VkPhysicalDevice physicalDevice,
    VkDevice logicalDevice,
    VkImage image,
    VkMemoryPropertyFlagBits memoryProperty,
    VkDeviceMemory& memoryObject)
{
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(logicalDevice, image, &memoryRequirements);

    memoryObject = VK_NULL_HANDLE;
    for (uint32_t type = 0; type < physicalDeviceMemoryProperties.memoryTypeCount; ++type)
    {
        if ((memoryRequirements.memoryTypeBits & (1 << type))
            && ((physicalDeviceMemoryProperties.memoryTypes[type].propertyFlags & memoryProperty) == memoryProperty))
        {
            VkMemoryAllocateInfo imageMemoryAllocateInfo = 
            {
                VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                nullptr,
                memoryRequirements.size,
                type
            };

            VkResult result = vkAllocateMemory(logicalDevice, &imageMemoryAllocateInfo, nullptr, &memoryObject);
            if (result == VK_SUCCESS)
            {
                break;
            }
        }
    }

    if (memoryObject == VK_NULL_HANDLE)
    {
        std::cout << "Could not allocate memory for an image." << std::endl;
        return false;
    }

    VkResult result = vkBindImageMemory(logicalDevice, image, memoryObject, 0);
    if (result != VK_SUCCESS)
    {
        std::cout << "Could not bind memory object to an image." << std::endl;
        return false;
    }

    return true;
}

void Cookbook::SetImageMemoryBarrier(VkCommandBuffer commandBuffer, 
    VkPipelineStageFlags generatingStages,
    VkPipelineStageFlags consumingStages,
    std::vector<ImageTransition> imageTransitions)
{
    std::vector<VkImageMemoryBarrier> imageMemoryBarriers;

    for (auto& imageTransition : imageTransitions)
    {
        imageMemoryBarriers.push_back(
        {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            nullptr,
            imageTransition.currentAccess,
            imageTransition.newAccess,
            imageTransition.currentLayout,
            imageTransition.newLayout,
            imageTransition.currentQueueFamily,
            imageTransition.newQueueFamily,
            imageTransition.image,
            {
                imageTransition.aspect,
                0,
                VK_REMAINING_MIP_LEVELS,
                0,
                VK_REMAINING_ARRAY_LAYERS
            }
        });
    }

    if (imageMemoryBarriers.size() > 0)
    {
        vkCmdPipelineBarrier(commandBuffer, 
            generatingStages, 
            consumingStages, 
            0,
            0, 
            nullptr,
            0,
            nullptr,
            static_cast<uint32_t> (imageMemoryBarriers.size()),
            imageMemoryBarriers.data());
    }
}

bool Cookbook::CreateBuffer(VkDevice logicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer& buffer)
{
    VkBufferCreateInfo bufferCreateInfo = 
    {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        nullptr,
        0,
        size,
        usage,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr
    };

    VkResult result = vkCreateBuffer(logicalDevice, &bufferCreateInfo, nullptr, &buffer);
    if (result != VK_SUCCESS)
    {
        std::cout << "Could not create buffer." << std::endl;
        return false;
    }

    return true;
}

bool Cookbook::AllocateAndBindMemoryObjectToBuffer(VkPhysicalDevice physicalDevice,
    VkDevice logicalDevice,
    VkBuffer buffer,
    VkMemoryPropertyFlagBits memoryProperty,
    VkDeviceMemory& memoryObject)
{
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);
    
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(logicalDevice, buffer, &memoryRequirements);

    memoryObject = VK_NULL_HANDLE;
    for (uint32_t type = 0; type < physicalDeviceMemoryProperties.memoryTypeCount; ++type)
    {
        if ((memoryRequirements.memoryTypeBits & (1 << type))
            && ((physicalDeviceMemoryProperties.memoryTypes[type].propertyFlags & memoryProperty) == memoryProperty))
        {
            VkMemoryAllocateInfo bufferMemoryAllocateInfo = 
            {
                VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                nullptr,
                memoryRequirements.size,
                type
            };

            VkResult result = vkAllocateMemory(logicalDevice, &bufferMemoryAllocateInfo, nullptr, &memoryObject);
            if (result == VK_SUCCESS)
            {
                break;
            }
        }
    }

    if (memoryObject == VK_NULL_HANDLE)
    {
        std::cout << "Could not allocate memory for an buffer." << std::endl;
        return false;
    }

    VkResult result = vkBindBufferMemory(logicalDevice, buffer, memoryObject, 0);
    if (result != VK_SUCCESS)
    {
        std::cout << "Could not bind memory object to an image." << std::endl;
        return false;
    }
    return true;
}

bool Cookbook::UseStagingBufferToUpdateBufferWithDeviceLocalMemoryBound(VkPhysicalDevice physicalDevice,
    VkDevice logicalDevice,
    VkDeviceSize dataSize,
    void* data,
    VkBuffer destinationBuffer,
    VkDeviceSize destinationOffset,
    VkAccessFlags destinationBufferCurrentAccess,
    VkAccessFlags destinationBufferNewAccess,
    VkPipelineStageFlags destinationBufferGeneratingStage,
    VkPipelineStageFlags destinationBufferComputingStage,
    VkQueue queue,
    VkCommandBuffer commandBuffer,
    std::vector<VkSemaphore> signalSemaphores)
{
    VkDestroyer(VkBuffer) stagingBuffer;
    InitVkDestroyer(logicalDevice, stagingBuffer);
    if (CreateBuffer(logicalDevice, dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, *stagingBuffer) == false)
    {
        return false;
    }

    VkDestroyer(VkDeviceMemory) memoryObject;
    InitVkDestroyer(logicalDevice, memoryObject);
    if (AllocateAndBindMemoryObjectToBuffer(physicalDevice, 
        logicalDevice, 
        *stagingBuffer, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 
        *memoryObject) == false)
    {
        return false;
    }

    if (MapUpdateAndUnmapHostVisibleMemory(logicalDevice, *memoryObject, 0, dataSize, data, true, nullptr) == false)
    {
        return false;
    }

    if (BeginCommandBufferRecordingOperation(commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr) == false)
    {
        return false;
    }

    SetBufferMemoryBarrier(commandBuffer, 
        destinationBufferGeneratingStage, 
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        {{destinationBuffer, destinationBufferCurrentAccess, VK_ACCESS_TRANSFER_WRITE_BIT, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED}});

    CopyDataBetweenBuffer(commandBuffer, *stagingBuffer, destinationBuffer, {{0, destinationOffset, dataSize}});

    SetBufferMemoryBarrier(commandBuffer, 
        VK_PIPELINE_STAGE_TRANSFER_BIT, 
        destinationBufferComputingStage,
        {{destinationBuffer, VK_ACCESS_TRANSFER_WRITE_BIT, destinationBufferNewAccess, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED}});

    if (EndCommandBufferRecordingOperation(commandBuffer) == false)
    {
        return false;
    }

    VkDestroyer(VkFence) fence;
    InitVkDestroyer(logicalDevice, fence);
    if (CreateFence(logicalDevice, false, *fence) == false)
    {
        return false;
    }

    if (SubmitCommandBufferToQueue(queue, {}, {commandBuffer}, signalSemaphores, *fence) == false)
    {
        return false;
    }

    if (WaitForFence(logicalDevice, {*fence}, VK_FALSE, 500'000'000) == false)
    {
        return false;
    }

    return true;
}

bool Cookbook::UseStagingBufferToUpdateImageWithDeviceLocalMemoryBound(VkPhysicalDevice physicalDevice,
    VkDevice logicalDevice,
    VkDeviceSize dataSize,
    void* data,
    VkImage destinationImage,
    VkImageSubresourceLayers destinationImageSubresource,
    VkOffset3D destinationImageOffset,
    VkExtent3D destinationImageSize,
    VkImageLayout destinationImageCurrentLayout,
    VkImageLayout destinationImageNewLayout,
    VkAccessFlags destinationImageCurrentAccess,
    VkAccessFlags destinationImageNewAccess,
    VkImageAspectFlags destinationImageAspect,
    VkPipelineStageFlags destinationImageGeneratingStage,
    VkPipelineStageFlags destinationImageConsumingStage,
    VkQueue queue,
    VkCommandBuffer commandBuffer,
    std::vector<VkSemaphore> signalSemaphores)
{
    VkDestroyer(VkBuffer) stagingBuffer;
    InitVkDestroyer(logicalDevice, stagingBuffer);
    if (CreateBuffer(logicalDevice, dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, *stagingBuffer) == false)
    {
        return false;
    }

    VkDestroyer(VkDeviceMemory) memoryObject;
    InitVkDestroyer(logicalDevice, memoryObject);
    if (AllocateAndBindMemoryObjectToBuffer(physicalDevice, 
        logicalDevice, 
        *stagingBuffer, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 
        *memoryObject) == false)
    {
        return false;
    }

    if (MapUpdateAndUnmapHostVisibleMemory(logicalDevice, *memoryObject, 0, dataSize, data, true, nullptr) == false)
    {
        return false;
    }

    if (BeginCommandBufferRecordingOperation(commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr) == false)
    {
        return false;
    }

    SetImageMemoryBarrier(commandBuffer, 
        destinationImageGeneratingStage, 
        VK_PIPELINE_STAGE_TRANSFER_BIT, 
        {
            {
                destinationImage,
                destinationImageCurrentAccess,
                VK_ACCESS_TRANSFER_WRITE_BIT,
                destinationImageCurrentLayout,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_QUEUE_FAMILY_IGNORED,
                VK_QUEUE_FAMILY_IGNORED,
                destinationImageAspect
            }
        });

    CopyDataFromBufferToImage(commandBuffer,
        *stagingBuffer,
        destinationImage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        {
            {
                0, 
                0, 
                0,
                destinationImageSubresource,
                destinationImageOffset,
                destinationImageSize
            }
        });

    SetImageMemoryBarrier(commandBuffer, 
        destinationImageGeneratingStage, 
        VK_PIPELINE_STAGE_TRANSFER_BIT, 
        {
            {
                destinationImage,
                destinationImageCurrentAccess,
                VK_ACCESS_TRANSFER_WRITE_BIT,
                destinationImageCurrentLayout,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_QUEUE_FAMILY_IGNORED,
                VK_QUEUE_FAMILY_IGNORED,
                destinationImageAspect
            }
        });

    if (EndCommandBufferRecordingOperation(commandBuffer) == false)
    {
        return false;
    }

    VkDestroyer(VkFence) fence;
    InitVkDestroyer(logicalDevice, fence);
    if (CreateFence(logicalDevice, false, *fence) == false)
    {
        return false;
    }

    if (SubmitCommandBufferToQueue(queue, {}, {commandBuffer}, signalSemaphores, *fence) == false)
    {
        return false;
    }

    if (WaitForFence(logicalDevice, {*fence}, VK_FALSE, 500'000'000) == false)
    {
        return false;
    }

    return true;
}

bool Cookbook::MapUpdateAndUnmapHostVisibleMemory(VkDevice logicalDevice,
    VkDeviceMemory memoryObject,
    VkDeviceSize offset,
    VkDeviceSize dataSize,
    void* data,
    bool unmap,
    void** pointer)
{
    void* localPointer = nullptr;
    VkResult result = vkMapMemory(logicalDevice, memoryObject, offset, dataSize, 0, &localPointer);
    if (result != VK_SUCCESS)
    {
        std::cout << "Could not map memory object." << std::endl;
        return false;
    }

    std::memcpy(localPointer, data, static_cast<size_t> (dataSize));

    std::vector<VkMappedMemoryRange> memoryRanges = 
    {
        {
            VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
            nullptr,
            memoryObject,
            offset,
            VK_WHOLE_SIZE
        }
    };

    result = vkFlushMappedMemoryRanges(logicalDevice, static_cast<uint32_t> (memoryRanges.size()), memoryRanges.data());
    if (result != VK_SUCCESS)
    {
        std::cout << "Could not flush mapped memory." << std::endl;
        return false;
    }

    if (unmap == true)
    {
        vkUnmapMemory(logicalDevice, memoryObject);
    }
    else if (pointer != nullptr)
    {
        *pointer = localPointer;
    }

    return true;
}

void Cookbook::SetBufferMemoryBarrier(VkCommandBuffer commandBuffer, 
    VkPipelineStageFlags generatingStages,
    VkPipelineStageFlags computingStages,
    std::vector<BufferTransition> bufferTransitions)
{
    std::vector<VkBufferMemoryBarrier> bufferMemoryBarriers;
    for (auto& bufferTransition : bufferTransitions)
    {
        bufferMemoryBarriers.push_back(
        {
            VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            nullptr,
            bufferTransition.currentAccess,
            bufferTransition.newAccess,
            bufferTransition.currentQueueFamily,
            bufferTransition.newQueueFamily,
            bufferTransition.buffer,
            0,
            VK_WHOLE_SIZE
        });
    }

    if (bufferMemoryBarriers.size() > 0)
    {
        vkCmdPipelineBarrier(commandBuffer, 
            generatingStages, 
            computingStages, 
            0, 
            0, 
            nullptr, 
            static_cast<uint32_t> (bufferMemoryBarriers.size()),
            bufferMemoryBarriers.data(),
            0,
            nullptr);
    }
}

void Cookbook::CopyDataBetweenBuffer(VkCommandBuffer commandBuffer,
    VkBuffer sourcegBuffer,
    VkBuffer destinationBuffer,
    std::vector<VkBufferCopy> regions)
{
    if (regions.size() > 0)
    {
        vkCmdCopyBuffer(commandBuffer, 
            sourcegBuffer, 
            destinationBuffer, static_cast<uint32_t> (regions.size()),
            regions.data());
    }
}

void Cookbook::CopyDataFromBufferToImage(VkCommandBuffer commandBuffer,
    VkBuffer sourceBuffer,
    VkImage destinationImage,
    VkImageLayout imageLayout,
    std::vector<VkBufferImageCopy> regions)
{
    if (regions.size() > 0)
    {
        vkCmdCopyBufferToImage(commandBuffer,
            sourceBuffer,
            destinationImage,
            imageLayout,
            static_cast<uint32_t> (regions.size()),
            regions.data());
    }
}

void Cookbook::CopyDataFromImageToBuffer(VkCommandBuffer commandBuffer,
    VkImage  sourceImage,
    VkImageLayout imageLayout,
    VkBuffer destinationBuffer,        
    std::vector<VkBufferImageCopy> regions)
{
    if (regions.size() > 0)
    {
        vkCmdCopyImageToBuffer(commandBuffer,            
            sourceImage,
            imageLayout,
            destinationBuffer,
            static_cast<uint32_t> (regions.size()),
            regions.data());
    }
}

bool Cookbook::CreateBufferView(VkDevice logicalDevice,
    VkBuffer buffer,
    VkFormat format,
    VkDeviceSize memoryOffset,
    VkDeviceSize memoryRange,
    VkBufferView& bufferView)
{
    VkBufferViewCreateInfo bufferViewCreateInfo = 
    {
        VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
        nullptr,
        0,
        buffer,
        format,
        memoryOffset,
        memoryRange
    };

    VkResult result = vkCreateBufferView(logicalDevice, &bufferViewCreateInfo, nullptr, &bufferView);
    if (result != VK_SUCCESS)
    {
        std::cout << "Could not create an buffer view." << std::endl;
        return false;
    }
    return true;
}

void Cookbook::DestroyImageView(VkDevice logicalDevice, VkImageView& imageView)
{
    if (imageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(logicalDevice, imageView, nullptr);
        imageView = VK_NULL_HANDLE;
    }
}

void Cookbook::DestroyImage(VkDevice logicalDevice, VkImage& image)
{
    if (image != VK_NULL_HANDLE)
    {
        vkDestroyImage(logicalDevice, image, nullptr);
        image = VK_NULL_HANDLE;
    }
}

void Cookbook::DestroyBufferView(VkDevice logicalDevice, VkBufferView& bufferView)
{
    if (bufferView != VK_NULL_HANDLE)
    {
        vkDestroyBufferView(logicalDevice, bufferView, nullptr);
        bufferView = VK_NULL_HANDLE;
    }
}

void Cookbook::FreeMemoryObject(VkDevice logicalDevice, VkDeviceMemory& memoryObject)
{
    if (memoryObject != VK_NULL_HANDLE)
    {
        vkFreeMemory(logicalDevice, memoryObject, nullptr);
        memoryObject = VK_NULL_HANDLE;
    }
}

void Cookbook::DestroyBuffer(VkDevice logicalDevice, VkBuffer& buffer)
{
    if (buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(logicalDevice, buffer, nullptr);
        buffer = VK_NULL_HANDLE;
    }
}
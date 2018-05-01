
#include "ch04_resource_and_memory.h"

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
    if (CreateImage(logicalDevice, VK_IMAGE_TYPE_2D, format, {size.width, size.height}, numMipmaps, numLayers, samples, usage, false, image) == false)
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
    VkMemoryPropertyFlagBits memoryProperties,
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
            && ((physicalDeviceMemoryProperties.memoryTypes[type].propertyFlags & memoryProperties) == memoryProperties))
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
    if (result == VK_SUCCESS)
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
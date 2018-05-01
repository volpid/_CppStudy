
#ifndef __CH4_RESOURCE_AND_MEMORY__H__
#define __CH4_RESOURCE_AND_MEMORY__H__

#include "../Common/vulkan_common.h"

namespace Cookbook
{
    bool CreateImageView(VkDevice logicalDevice,
        VkImage image,
        VkImageViewType viewType,
        VkFormat format,
        VkImageAspectFlags aspect,
        VkImageView& imageView);
    bool Create2DImageAndView(VkPhysicalDevice physicalDevice,
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
        VkImageView& imageView);
    bool CreateImage(VkDevice logicalDevice,
        VkImageType type,
        VkFormat format,
        VkExtent3D size,
        uint32_t numMipmaps,
        uint32_t numLayers,
        VkSampleCountFlagBits samples,
        VkImageUsageFlags usageScenarios,
        bool cubemap,
        VkImage& image);
    bool AllocateAndBindMemoryObjectToImage(VkPhysicalDevice physicalDevice,
        VkDevice logicalDevice,
        VkImage image,
        VkMemoryPropertyFlagBits memoryProperties,
        VkDeviceMemory& memoryObject);
    void SetImageMemoryBarrier(VkCommandBuffer commandBuffer, 
        VkPipelineStageFlags generatingStages,
        VkPipelineStageFlags consumingStages,
        std::vector<ImageTransition> imageTransitions);

}

#endif /*__CH4_RESOURCE_AND_MEMORY__H__*/
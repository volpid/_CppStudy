
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
        VkMemoryPropertyFlagBits memoryProperty,
        VkDeviceMemory& memoryObject);
    void SetImageMemoryBarrier(VkCommandBuffer commandBuffer, 
        VkPipelineStageFlags generatingStages,
        VkPipelineStageFlags consumingStages,
        std::vector<ImageTransition> imageTransitions);
    bool CreateBuffer(VkDevice logicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer& buffer);
    bool AllocateAndBindMemoryObjectToBuffer(VkPhysicalDevice physicalDevice,
        VkDevice logicalDevice,
        VkBuffer buffer,
        VkMemoryPropertyFlagBits memoryProperty,
        VkDeviceMemory& memoryObject);
    bool UseStagingBufferToUpdateBufferWithDeviceLocalMemoryBound(VkPhysicalDevice physicalDevice,
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
        std::vector<VkSemaphore> signalSemaphores);
    bool UseStagingBufferToUpdateImageWithDeviceLocalMemoryBound(VkPhysicalDevice physicalDevice,
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
        std::vector<VkSemaphore> signalSemaphores);
    bool MapUpdateAndUnmapHostVisibleMemory(VkDevice logicalDevice,
        VkDeviceMemory memoryObject,
        VkDeviceSize offset,
        VkDeviceSize dataSize,
        void* data,
        bool unmap,
        void** pointer);
    void SetBufferMemoryBarrier(VkCommandBuffer commandBuffer, 
        VkPipelineStageFlags generatingStages,
        VkPipelineStageFlags computingStages,
        std::vector<BufferTransition> bufferTransitions);
    void CopyDataBetweenBuffer(VkCommandBuffer commandBuffer,
        VkBuffer sourcegBuffer,
        VkBuffer destinationBuffer,
        std::vector<VkBufferCopy> regions);
    void CopyDataFromBufferToImage(VkCommandBuffer commandBuffer,
        VkBuffer sourceBuffer,
        VkImage destinationImage,
        VkImageLayout imageLayout,
        std::vector<VkBufferImageCopy> regions);
    void CopyDataFromImageToBuffer(VkCommandBuffer commandBuffer,
        VkImage  sourceImage,
        VkImageLayout imageLayout,
        VkBuffer destinationBuffer,        
        std::vector<VkBufferImageCopy> regions);
    bool CreateBufferView(VkDevice logicalDevice,
        VkBuffer buffer,
        VkFormat format,
        VkDeviceSize memoryOffset,
        VkDeviceSize memoryRange,
        VkBufferView& bufferView);
    void DestroyImageView(VkDevice logicalDevice, VkImageView& imageView);
    void DestroyImage(VkDevice logicalDevice, VkImage& image);
    void DestroyBufferView(VkDevice logicalDevice, VkBufferView& bufferView);
    void FreeMemoryObject(VkDevice logicalDevice, VkDeviceMemory& memoryObject);
    void DestroyBuffer(VkDevice logicalDevice, VkBuffer& buffer);
}

#endif /*__CH4_RESOURCE_AND_MEMORY__H__*/
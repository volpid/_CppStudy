
#ifndef __CH05_DESCRIPTOR_SETS__H__
#define __CH05_DESCRIPTOR_SETS__H__

#include "../Common/vulkan_common.h"

namespace Cookbook
{
    struct ImageDesciptorInfo
    {
        VkDescriptorSet targetDescriptorSet;
        uint32_t targetDescriptorBinding;
        uint32_t targetArrayElement;
        VkDescriptorType targetDescriptorType;
        std::vector<VkDescriptorImageInfo> imageInfos;
    };

    struct BufferDescriptorInfo
    {
        VkDescriptorSet targetDescriptorSet;
        uint32_t targetDescriptorBinding;
        uint32_t targetArrayElement;
        VkDescriptorType targetDescriptorType;
        std::vector<VkDescriptorBufferInfo> bufferInfos;
    };

    struct TexelBufferDescriptorInfo
    {
        VkDescriptorSet targetDescriptorSet;
        uint32_t targetDescriptorBinding;
        uint32_t targetArrayElement;
        VkDescriptorType targetDescriptorType;
        std::vector<VkBufferView> TexelBufferViews;
    };

    struct CopyDescriptorInfo
    {
        VkDescriptorSet targetDescriptorSet;
        uint32_t targetDescriptorBinding;
        uint32_t targetArrayElement;
        VkDescriptorSet sourceDescriptorSet;
        uint32_t sourceDescriptorBinding;
        uint32_t sourceArrayElement;
        uint32_t DescriptorCount;
    };
    
    bool CreateSampler(VkDevice logicalDevice,
        VkFilter magFilter,
        VkFilter minFilter,
        VkSamplerMipmapMode mipmapMode,
        VkSamplerAddressMode uAddressMode,
        VkSamplerAddressMode vAddressMode,
        VkSamplerAddressMode wAddressMode,
        float lodBias,
        bool anisotropyEnable,
        float maxAnisotropy,
        bool compareEnable,
        VkCompareOp compareOperator,
        float minLod,
        float maxLod,
        VkBorderColor borderColor,
        bool unnormalizedCoords,
        VkSampler& sampler);

    bool CreateSampledImage(VkPhysicalDevice physicalDevice,
        VkDevice logicalDevice,
        VkImageType type,
        VkFormat format,
        VkExtent3D size,
        uint32_t numMinmaps,
        uint32_t numLayers,
        VkImageUsageFlags usage,
        bool cubemap,
        VkImageViewType viewType,
        VkImageAspectFlags aspect,
        bool linearFiltering,
        VkImage& sampledImage,
        VkDeviceMemory& memoryObject,
        VkImageView& sampledImageView); 

    bool CreateCombinedImageSampler(VkPhysicalDevice physicalDevice,
        VkDevice logicalDevice,
        VkImageType type,
        VkFormat format,
        VkExtent3D size,
        uint32_t numMinmaps,
        uint32_t numLayers,
        VkImageUsageFlags usage,
        bool cubemap,
        VkImageViewType viewType,
        VkImageAspectFlags aspect,
        VkFilter magFilter,
        VkFilter minFilter,
        VkSamplerMipmapMode mipmapMode,
        VkSamplerAddressMode uAddressMode,
        VkSamplerAddressMode vAddressMode,
        VkSamplerAddressMode wAddressMode,
        float lodBias,
        bool anisotropyEnable,
        float maxAnisotropy,
        bool compareEnable,
        VkCompareOp compareOperator,
        float minLod,
        float maxLod,
        VkBorderColor borderColor,
        bool unnormalizedCoords,
        VkSampler& sampler,
        VkImage& sampledImage,
        VkDeviceMemory& memoryObject,
        VkImageView& sampledImageView);

    bool CreateDescriptorSetLayout(VkDevice logicalDevice,
        const std::vector<VkDescriptorSetLayoutBinding>& bindings,
        VkDescriptorSetLayout& descriptorSetLayout);

    bool CreateDescriptorPool(VkDevice logicalDevice,
        bool freeIndividualSets,
        uint32_t maxSetCount,
        const std::vector<VkDescriptorPoolSize>& descriptorTypes,
        VkDescriptorPool& descriptorPool);

    bool AllocateDescriptorSets(VkDevice logicalDevice,
        VkDescriptorPool descriptorPool,
        const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
        std::vector<VkDescriptorSet>& descriptorSets);

    void UpdateDescriptorSets(VkDevice logicalDevice,
        const std::vector<ImageDesciptorInfo>& imageDesciptorInfos,
        const std::vector<BufferDescriptorInfo>& bufferDesciptorInfos,
        const std::vector<TexelBufferDescriptorInfo>& texelDesciptorInfos,
        const std::vector<CopyDescriptorInfo>& copyDesciptorInfos);

    void BindDescriptorSets(VkCommandBuffer commandBuffer,
        VkPipelineBindPoint pipelineType,
        VkPipelineLayout pipelineLayout,
        uint32_t indexForFirstSet,
        const std::vector<VkDescriptorSet>& descriptorSets,
        const std::vector<uint32_t>& dynamicOffsets);

    bool CreateUniformBuffer(VkPhysicalDevice physicalDevice,
        VkDevice logicalDevice,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkBuffer& uniformBuffer,
        VkDeviceMemory& memoryObject);

    bool CreateStorageTexelBuffer(VkPhysicalDevice physicalDevice,
        VkDevice logicalDevice,
        VkFormat format,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        bool atomicOperation,
        VkBuffer& storageTexelBuffer,
        VkDeviceMemory& memoryObject,
        VkBufferView& storageTexelBufferView);

    bool CreateInputAttachment(VkPhysicalDevice physicalDevice,
        VkDevice logicalDevice,
        VkImageType type,
        VkFormat format,
        VkExtent3D size,
        VkBufferUsageFlags usage,
        VkImageViewType viewType,
        VkImageAspectFlags aspect,
        VkImage& inputAttachment,
        VkDeviceMemory& memoryObject,
        VkImageView& inputAttachmentImageView);
}

#endif /*__CH05_DESCRIPTOR_SETS__H__*/





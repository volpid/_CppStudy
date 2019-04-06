
#include "ch05_descriptor_sets.h"

#include "ch04_resource_and_memory.h"

#include <iostream>

bool Cookbook::CreateSampler(VkDevice logicalDevice,
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
    VkBorderColor boarderColor,
    bool unnormalizedCoords,
    VkSampler& sampler)
{
    VkSamplerCreateInfo samplerCreateInfo =
    {
        VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        nullptr,
        0, 
        magFilter,
        minFilter,
        mipmapMode,
        uAddressMode,
        vAddressMode,
        wAddressMode,
        lodBias,
        anisotropyEnable,
        maxAnisotropy,
        compareEnable,
        compareOperator,
        minLod,
        maxLod,
        boarderColor,
        unnormalizedCoords
    };

    VkResult result = vkCreateSampler(logicalDevice, &samplerCreateInfo, nullptr, &sampler);
    if (result != VK_SUCCESS)
    {
        std::cout << "Could not create sampler." << std::endl;
        return false;
    }

    return true;
}

bool Cookbook::CreateSampledImage(VkPhysicalDevice physicalDevice,
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
    VkImageView& sampledImageView)
{
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);
    if ((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) == false)
    {
        std::cout << "Provided format is not supported for a sampled image." << std::endl;
        return false;
    }

    if ((linearFiltering == true)
        && (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) == false)
    {
        std::cout << "Provided format is not supported for a linear image." << std::endl;
        return false;
    }

    if (CreateImage(logicalDevice, 
        type, 
        format, 
        size, 
        numMinmaps, 
        numLayers, 
        VK_SAMPLE_COUNT_1_BIT,
        usage | VK_IMAGE_USAGE_SAMPLED_BIT,
        cubemap,
        sampledImage) == false)
    {
        return false;
    }

    if (AllocateAndBindMemoryObjectToImage(physicalDevice, logicalDevice, sampledImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memoryObject) == false)
    {
        return false;
    }

    if (CreateImageView(logicalDevice, sampledImage, viewType, format, aspect, sampledImageView) == false)
    {
        return false;
    }

    return true;
}

bool Cookbook::CreateCombinedImageSampler(VkPhysicalDevice physicalDevice,
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
    VkImageView& sampledImageView)
{
    if (CreateSampler(logicalDevice, 
        magFilter,
        minFilter, 
        mipmapMode,
        uAddressMode,
        vAddressMode,
        wAddressMode,
        lodBias,
        anisotropyEnable,
        maxAnisotropy,
        compareEnable,
        compareOperator,
        minLod,
        maxLod,
        borderColor,
        unnormalizedCoords,
        sampler) == false)
    {
        return false;
    }

    bool linearFiltering = (magFilter == VK_FILTER_LINEAR) 
        || (minFilter == VK_FILTER_LINEAR) 
        || (mipmapMode == VK_SAMPLER_MIPMAP_MODE_LINEAR);

    if (CreateSampledImage(physicalDevice, 
        logicalDevice, 
        type, 
        format,
        size,
        numMinmaps,
        numLayers,
        usage,
        cubemap,
        viewType, 
        aspect,
        linearFiltering,
        sampledImage,
        memoryObject,
        sampledImageView) == false)
    {
        return false;
    }

    return true;
}

bool Cookbook::CreateDescriptorSetLayout(VkDevice logicalDevice,
    const std::vector<VkDescriptorSetLayoutBinding>& bindings,
    VkDescriptorSetLayout& descriptorSetLayout)
{
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo =
    {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        nullptr,
        0,
        static_cast<uint32_t>(bindings.size()),
        bindings.data()
    };

    VkResult result = vkCreateDescriptorSetLayout(logicalDevice, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout);
    if (result != VK_SUCCESS)
    {
        return false;
    }

    return true;
}

bool Cookbook::CreateDescriptorPool(VkDevice logicalDevice,
    bool freeIndividualSets,
    uint32_t maxSetCount,
    const std::vector<VkDescriptorPoolSize>& descriptorTypes,
    VkDescriptorPool& descriptorPool)
{
    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = 
    {
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        nullptr,
        (freeIndividualSets == true) ? VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT : 0u,
        maxSetCount,
        static_cast<uint32_t>(descriptorTypes.size()),
        descriptorTypes.data()
    };

    VkResult result = vkCreateDescriptorPool(logicalDevice, &descriptorPoolCreateInfo, nullptr, &descriptorPool);
    if (result != VK_SUCCESS)
    {
        std::cout << "Could not create a descriptor pool." << std::endl;
        return false;
    }

    return true;
}

bool Cookbook::AllocateDescriptorSets(VkDevice logicalDevice,
    VkDescriptorPool descriptorPool,
    const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
    std::vector<VkDescriptorSet>& descriptorSets)
{
    if (descriptorSetLayouts.size() > 0)
    {
        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo =
        {
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            nullptr,
            descriptorPool,
            static_cast<uint32_t>(descriptorSetLayouts.size()),
            descriptorSetLayouts.data()
        };

        descriptorSets.resize(descriptorSetLayouts.size());
        VkResult result = vkAllocateDescriptorSets(logicalDevice, &descriptorSetAllocateInfo, descriptorSets.data());
        if (result != VK_SUCCESS)
        {
            return false;
        }
        return true;
    }
    
    return false;
}


void Cookbook::UpdateDescriptorSets(VkDevice logicalDevice,
    const std::vector<ImageDesciptorInfo>& imageDesciptorInfos,
    const std::vector<BufferDescriptorInfo>& bufferDesciptorInfos,
    const std::vector<TexelBufferDescriptorInfo>& texelDesciptorInfos,
    const std::vector<CopyDescriptorInfo>& copyDesciptorInfos)
{
    std::vector<VkWriteDescriptorSet> writeDescriptors;
    std::vector<VkCopyDescriptorSet> copyDescriptors;
   
    // image descriptors
    for (auto& imageDescriptor : imageDesciptorInfos)
    {
        writeDescriptors.push_back({VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            nullptr,
            imageDescriptor.targetDescriptorSet,
            imageDescriptor.targetDescriptorBinding,
            imageDescriptor.targetArrayElement,
            static_cast<uint32_t>(imageDescriptor.imageInfos.size()),
            imageDescriptor.targetDescriptorType,
            imageDescriptor.imageInfos.data(),
            nullptr,
            nullptr});
    }

    // buffer descriptors
    for (auto& bufferDescriptor : bufferDesciptorInfos)
    {
        writeDescriptors.push_back({VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            nullptr,
            bufferDescriptor.targetDescriptorSet,
            bufferDescriptor.targetDescriptorBinding,
            bufferDescriptor.targetArrayElement,
            static_cast<uint32_t>(bufferDescriptor.bufferInfos.size()),
            bufferDescriptor.targetDescriptorType,
            nullptr,
            bufferDescriptor.bufferInfos.data(),
            nullptr});
    }

    // texel descriptors
    for (auto& texelDescriptor : texelDesciptorInfos)
    {
        writeDescriptors.push_back({VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            nullptr,
            texelDescriptor.targetDescriptorSet,
            texelDescriptor.targetDescriptorBinding,
            texelDescriptor.targetArrayElement,
            static_cast<uint32_t>(texelDescriptor.TexelBufferViews.size()),
            texelDescriptor.targetDescriptorType,
            nullptr,            
            nullptr,
            texelDescriptor.TexelBufferViews.data()});
    }

     // copy descriptors
     for (auto& copyDescriptor : copyDesciptorInfos)
     {
        copyDescriptors.push_back({VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET,
            nullptr,
            copyDescriptor.sourceDescriptorSet,
            copyDescriptor.sourceDescriptorBinding,
            copyDescriptor.sourceArrayElement,
            copyDescriptor.targetDescriptorSet,
            copyDescriptor.targetDescriptorBinding,
            copyDescriptor.targetArrayElement,
            copyDescriptor.DescriptorCount});
     }

     vkUpdateDescriptorSets(logicalDevice, 
        static_cast<uint32_t>(writeDescriptors.size()),
        writeDescriptors.data(),        
        static_cast<uint32_t>(copyDescriptors.size()),
        copyDescriptors.data());
}

void Cookbook::BindDescriptorSets(VkCommandBuffer commandBuffer,
    VkPipelineBindPoint pipelineType,
    VkPipelineLayout pipelineLayout,
    uint32_t indexForFirstSet,
    const std::vector<VkDescriptorSet>& descriptorSets,
    const std::vector<uint32_t>& dynamicOffsets)
{
    vkCmdBindDescriptorSets(commandBuffer,
        pipelineType,
        pipelineLayout,
        indexForFirstSet,
        static_cast<uint32_t>(descriptorSets.size()),
        descriptorSets.data(),
        static_cast<uint32_t>(dynamicOffsets.size()),
        dynamicOffsets.data());
}

bool Cookbook::CreateUniformBuffer(VkPhysicalDevice physicalDevice,
    VkDevice logicalDevice,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkBuffer& uniformBuffer,
    VkDeviceMemory& memoryObject)
{
    if (CreateBuffer(logicalDevice, size, usage, uniformBuffer) == false)
    {
        return false;
    }

    if (AllocateAndBindMemoryObjectToBuffer(physicalDevice, 
        logicalDevice, 
        uniformBuffer, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        memoryObject) == false)
    {
        return false;
    }

    return true;
}


bool Cookbook::CreateStorageTexelBuffer(VkPhysicalDevice physicalDevice,
    VkDevice logicalDevice,
    VkFormat format,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    bool atomicOperation,
    VkBuffer& storageTexelBuffer,
    VkDeviceMemory& memoryObject,
    VkBufferView& storageTexelBufferView)
{
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);
    if (!(formatProperties.bufferFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT))
    {
        std::cout << "Provided format is not supported for a uniform texel." << std::endl;
        return false;
    }

    if ((atomicOperation == true) && !(formatProperties.bufferFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT))
    {
        std::cout << "Provided format is not supported for atomic op for texel buffer." << std::endl;
        return false;
    }
    
    if (CreateBuffer(logicalDevice, size, usage | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, storageTexelBuffer) == false)
    {
        return false;
    }

    if (AllocateAndBindMemoryObjectToBuffer(physicalDevice, logicalDevice, storageTexelBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memoryObject) == false)
    {
        return false;
    }

    if (CreateBufferView(logicalDevice, storageTexelBuffer, format, 0, VK_WHOLE_SIZE, storageTexelBufferView) == false)
    {
        return false;
    }
    
    return true;
}

bool Cookbook::CreateInputAttachment(VkPhysicalDevice physicalDevice,
    VkDevice logicalDevice,
    VkImageType type,
    VkFormat format,
    VkExtent3D size,
    VkBufferUsageFlags usage,
    VkImageViewType viewType,
    VkImageAspectFlags aspect,
    VkImage& inputAttachment,
    VkDeviceMemory& memoryObject,
    VkImageView& inputAttachmentImageView)
{
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);
    if ((aspect & VK_IMAGE_ASPECT_COLOR_BIT) && !(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))
    {
        std::cout << "Provided format is not supported for a input attachment." << std::endl;
        return false;
    }

    if ((aspect & VK_IMAGE_ASPECT_DEPTH_BIT) 
        && !(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
    {
        std::cout << "Provided format is not supported for a input attachment." << std::endl;
        return false;
    }

    if (CreateImage(logicalDevice, 
        type, 
        format, 
        size, 
        1, 
        1, 
        VK_SAMPLE_COUNT_1_BIT,
        usage | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
        false,
        inputAttachment) == false)
    {
        return false;
    }

    if (AllocateAndBindMemoryObjectToImage(physicalDevice, logicalDevice, inputAttachment, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memoryObject) == false)
    {
        return false;
    }

    if (CreateImageView(logicalDevice, inputAttachment, viewType, format, aspect, inputAttachmentImageView) == false)
    {
        return false;
    }

    return true;

}
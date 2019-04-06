
#include "../../Chapter/ch02_image_presentation.h"
#include "../../Chapter/ch03_command_buffer_and_synchronization.h"
#include "../../Chapter/ch04_resource_and_memory.h"
#include "../../Chapter/ch05_descriptor_sets.h"
#include "../../Chapter/ch06_renderpass_and_framebuffer.h"
#include "../../Chapter/ch08_graphics_and_compute_pipeline.h"
#include "../../Chapter/ch09_command_recording_and_drawing.h"

#include "../../Common/sample_framework.h"
#include "../../Common/vulkan_tool.h"

#include <array>

//----------------------------------------------------------------
// SampleChapter10 : using compute shader
//----------------------------------------------------------------
class SampleChapter10 : public VulkanSample
{
public:
    virtual bool Initialize(WindowParameters parameters) override;
    virtual bool Draw(void) override;
    virtual bool Resize(void) override;    

private:
    VkDestroyer(VkCommandPool) _commandPool;
    VkCommandBuffer _commandBuffer;

    VkDestroyer(VkFence) _drawingFence;
    VkDestroyer(VkSemaphore) _imageAcquiredSemaphore;
    VkDestroyer(VkSemaphore) _readyToPresentSemaphore;

    VkDestroyer(VkImage) _image;
    VkDestroyer(VkDeviceMemory) _imageMemory;
    VkDestroyer(VkImageView) _imageView;

    VkDestroyer(VkDescriptorSetLayout) _descriptorSetLayout;
    VkDestroyer(VkDescriptorPool) _descriptorPool;
    std::vector<VkDescriptorSet> _descriptorSets;

    VkDestroyer(VkPipelineLayout) _pipelineLayout;
    VkDestroyer(VkPipeline) _computePipeline;
};

//VULKAN_SAMPLE_FRAMEWORK("10_using_compute_shaders", 50, 25, 800, 600, SampleChapter10);

//----------------------------------------------------------------
bool SampleChapter10::Initialize(WindowParameters parameters)
{   
    if (InitializeVulkan(parameters, 
        nullptr, 
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, 
        false) == false)
    {
        return false;
    }

    InitVkDestroyer(_logicalDevice, _commandPool);
    if (Cookbook::CreateCommandPool(*_logicalDevice, 
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        _computeQueue.familyIndex,
        *_commandPool) == false)
    {
        return false;
    }

    std::vector<VkCommandBuffer> commandBuffers;
    if(Cookbook::AllocateCommandBuffer(*_logicalDevice, *_commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, commandBuffers) == false)
    {
        return false;
    }
    _commandBuffer = commandBuffers[0];

    InitVkDestroyer(_logicalDevice, _drawingFence);
    if (Cookbook::CreateFence(*_logicalDevice, true, *_drawingFence) == false)
    {
        return false;
    }

    InitVkDestroyer(_logicalDevice, _imageAcquiredSemaphore);
    if (Cookbook::CreateSemaphore(*_logicalDevice, *_imageAcquiredSemaphore) == false)
    {
        return false;
    }

    InitVkDestroyer(_logicalDevice, _readyToPresentSemaphore);
    if (Cookbook::CreateSemaphore(*_logicalDevice, *_readyToPresentSemaphore) == false)
    {
        return false;
    }

    InitVkDestroyer(_logicalDevice, _image);
    InitVkDestroyer(_logicalDevice, _imageMemory);
    InitVkDestroyer(_logicalDevice, _imageView);

    if (Cookbook::Create2DImageAndView(_physicalDevice,
        *_logicalDevice,
        _swapchain.format,
        _swapchain.size,
        1,
        1,
        VK_SAMPLE_COUNT_1_BIT,
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT,
        *_image,
        *_imageMemory,
        *_imageView) == false)
    {
        return false;
    }

    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = 
    {
        0,
        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        1,
        VK_SHADER_STAGE_COMPUTE_BIT,
        nullptr
    };

    
    InitVkDestroyer(_logicalDevice, _descriptorSetLayout);
    if (Cookbook::CreateDescriptorSetLayout(*_logicalDevice, {descriptorSetLayoutBinding}, *_descriptorSetLayout) == false)
    {
        return false;
    }
    
    VkDescriptorPoolSize descriptorPoolSize = 
    {
        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        1
    };

    InitVkDestroyer(_logicalDevice, _descriptorPool);
    if (Cookbook::CreateDescriptorPool(*_logicalDevice, false, 1, {descriptorPoolSize}, *_descriptorPool) == false)
    {
        return false;
    }

    if (Cookbook::AllocateDescriptorSets(*_logicalDevice, *_descriptorPool, {*_descriptorSetLayout}, _descriptorSets) == false)
    {
        return false;
    }

    Cookbook::ImageDesciptorInfo imageDescriptorUpdate = 
    {
        _descriptorSets[0],
        0,
        0,
        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        {
            {
                VK_NULL_HANDLE,
                *_imageView,
                VK_IMAGE_LAYOUT_GENERAL
            }
        }
    };

    Cookbook::UpdateDescriptorSets(*_logicalDevice, {imageDescriptorUpdate}, {}, {}, {});

    std::vector<unsigned char> computeShaderSpirv;
    if (GetBinaryFileContent("..\\..\\Source\\VulkanCookbook\\Shader\\Basic\\10_shader.comp.spv", computeShaderSpirv) == false)
    {
        return false;
    }

    VkDestroyer(VkShaderModule) computeShaderModule;
    InitVkDestroyer(_logicalDevice, computeShaderModule);
    if (Cookbook::CreateShader(*_logicalDevice, computeShaderSpirv, *computeShaderModule) == false)
    {
        return false;
    }

    std::vector<ShaderStageParameter> shaderStageParams =
    {
        {
            VK_SHADER_STAGE_COMPUTE_BIT,
            *computeShaderModule,
            "main",
            nullptr
        }
    };

    std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos;
    Cookbook::SpecifyPipelineShaderStage(shaderStageParams, shaderStageCreateInfos);

    InitVkDestroyer(_logicalDevice, _pipelineLayout);
    if (Cookbook::CreatePipelineLayout(*_logicalDevice, {*_descriptorSetLayout}, {}, *_pipelineLayout) == false)
    {
        return false;
    }

    InitVkDestroyer(_logicalDevice, _computePipeline);    
    if (Cookbook::CreateComputePipeline(*_logicalDevice,
        0,
        shaderStageCreateInfos[0],
        *_pipelineLayout,
        VK_NULL_HANDLE,
        VK_NULL_HANDLE,
        *_computePipeline) == false)
    {
        return false;
    }

    return true;
}

bool SampleChapter10::Draw(void)
{
    if (Cookbook::WaitForFence(*_logicalDevice, {*_drawingFence}, false, 5'000'000'000) == false)
    {
        return false;
    }

    if (Cookbook::ResetFence(*_logicalDevice, {*_drawingFence}) == false)
    {
        return false;
    }

    uint32_t imageIndex;
    if (Cookbook::AcquireSwapchainImage(*_logicalDevice,
        *_swapchain.handle,
        *_imageAcquiredSemaphore,
        VK_NULL_HANDLE,
        imageIndex) == false)
    {
        return false;
    }

    if (Cookbook::BeginCommandBufferRecordingOperation(_commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr) == false)
    {
        return false;
    }

    uint32_t presentQueueFamilyIndex = _presentQueue.familyIndex;
    uint32_t computeQueueFamilyIndex = _computeQueue.familyIndex;
    if (_presentQueue.familyIndex == _computeQueue.familyIndex)
    {
        presentQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        computeQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    }

    ImageTransition imageTransitionForComputeShader =
    {
        *_image,
        0,
        VK_ACCESS_SHADER_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_GENERAL,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        VK_IMAGE_ASPECT_COLOR_BIT
    };
    Cookbook::SetImageMemoryBarrier(_commandBuffer, 
        VK_PIPELINE_STAGE_TRANSFER_BIT, 
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        {imageTransitionForComputeShader});

    Cookbook::BindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, *_pipelineLayout, 0, _descriptorSets, {});
    Cookbook::BindPipelineObject(_commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, *_computePipeline);
    
    Cookbook::DispatchComputeWork(_commandBuffer, _swapchain.size.width / 32 + 1, _swapchain.size.height / 32 + 1, 1);

    std::vector<ImageTransition> imageTransitionForTransfer =
    {
        {
            *_image,
            VK_ACCESS_SHADER_WRITE_BIT,
            VK_ACCESS_TRANSFER_READ_BIT,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            VK_IMAGE_ASPECT_COLOR_BIT
        },
        {
            _swapchain.images[imageIndex],
            0,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            presentQueueFamilyIndex,
            computeQueueFamilyIndex,
            VK_IMAGE_ASPECT_COLOR_BIT
        }
    };

    Cookbook::SetImageMemoryBarrier(_commandBuffer, 
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        imageTransitionForTransfer);

    VkImageCopy imageCopy =
    {
        {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
        {0, 0, 0},
        {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
        {0, 0, 0},
        {_swapchain.size.width, _swapchain.size.height, 1}
    };

    vkCmdCopyImage(_commandBuffer, 
        *_image, 
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        _swapchain.images[imageIndex], 
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &imageCopy);

    ImageTransition imageTransitionBeforePresent =
    {
        _swapchain.images[imageIndex],
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_ACCESS_MEMORY_READ_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        computeQueueFamilyIndex,
        presentQueueFamilyIndex,
        VK_IMAGE_ASPECT_COLOR_BIT
    };

    Cookbook::SetImageMemoryBarrier(_commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        {imageTransitionBeforePresent});

    if (Cookbook::EndCommandBufferRecordingOperation(_commandBuffer) == false)
    {
        return false;
    }

    WaitSemaphoreInfo waitSemaphoreInfo = 
    {
        *_imageAcquiredSemaphore,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
    };
    if (Cookbook::SubmitCommandBufferToQueue(_computeQueue.handle, 
        {waitSemaphoreInfo},
        {_commandBuffer},
        {*_readyToPresentSemaphore},
        *_drawingFence) == false)
    {
        return false;
    }

    PresentInfo presentInfo = 
    {
        *_swapchain.handle,
        imageIndex
    };
    if (Cookbook::PresentImage(_presentQueue.handle, {*_readyToPresentSemaphore}, {presentInfo}) == false)
    {
        return false;
    }

    return true;
}

bool SampleChapter10::Resize(void)
{
    if (CreateSwapchain(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, false) == false)
    {
        return false;
    }

    Cookbook::DestroyImageView(*_logicalDevice, *_imageView);
    Cookbook::FreeMemoryObject(*_logicalDevice, *_imageMemory);
    Cookbook::DestroyImage(*_logicalDevice, *_image);

    if (Cookbook::Create2DImageAndView(_physicalDevice,
        *_logicalDevice,
        _swapchain.format,
        _swapchain.size,
        1,
        1,
        VK_SAMPLE_COUNT_1_BIT,
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT,
        *_image,
        *_imageMemory,
        *_imageView) == false)
    {
        return false;
    }

    Cookbook::ImageDesciptorInfo imageDescriptorUpdate = 
    {
        _descriptorSets[0],
        0,
        0,
        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        {
            {
                VK_NULL_HANDLE,
                *_imageView,
                VK_IMAGE_LAYOUT_GENERAL
            }
        }
    };

    Cookbook::UpdateDescriptorSets(*_logicalDevice, {imageDescriptorUpdate}, {}, {}, {});
    return true;
}
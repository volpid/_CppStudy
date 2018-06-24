
#include "../Chapter/ch01_instance_and_device.h"
#include "../Chapter/ch02_image_presentation.h"
#include "../Chapter/ch03_command_buffer_and_synchronization.h"
#include "../Chapter/ch04_resource_and_memory.h"
#include "../Chapter/ch05_creating_framebuffer.h"
#include "../Chapter/ch06_renderpass_and_framebuffer.h"

#include "../Common/sample_framework.h"

//----------------------------------------------------------------
// SampleChapter03 : render pass
//----------------------------------------------------------------
class SampleChapter03 : public VulkanSample
{
public:
    virtual bool Initialize(WindowParameters parameters) override;
    virtual bool Draw(void) override;
    virtual bool Resize(void) override;    

private:
    VkCommandBuffer commandBuffer_;
    VkDestroyer(VkRenderPass) renderpass_;
    VkDestroyer(VkFramebuffer) framebuffer_;
    VkDestroyer(VkFence) drawingFence_;
    VkDestroyer(VkSemaphore) imageAcquiredSemaphore_;
    VkDestroyer(VkSemaphore) readyToPresentSemaphore_;
};

//VULKAN_SAMPLE_FRAMEWORK("03_UseRenderPass", 50, 25, 800, 600, SampleChapter03);

//----------------------------------------------------------------
bool SampleChapter03::Initialize(WindowParameters parameters)
{   
    if (InitializeVulkan(parameters) == false)
    {
        return false;
    }
    
    std::vector<VkCommandBuffer> commandBuffers;
    if (Cookbook::AllocateCommandBuffer(*logicalDevice_, *commandPool_, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, commandBuffers) == false)
    {
        return false;
    }
    commandBuffer_ = commandBuffers[0];

    InitVkDestroyer(logicalDevice_, drawingFence_);
    if (Cookbook::CreateFence(*logicalDevice_, true, *drawingFence_) == false)
    {
        return false;
    }

    InitVkDestroyer(logicalDevice_, imageAcquiredSemaphore_);
    if (Cookbook::CreateSemaphore(*logicalDevice_, *imageAcquiredSemaphore_) == false)
    {
        return false;
    }

    InitVkDestroyer(logicalDevice_, readyToPresentSemaphore_);
    if (Cookbook::CreateSemaphore(*logicalDevice_, *readyToPresentSemaphore_) == false)
    {
        return false;
    }
    
    std::vector<VkAttachmentDescription> attachmentDescriptions = 
    {
        {
            0,
            swapchain_.format,
            VK_SAMPLE_COUNT_1_BIT,
            VK_ATTACHMENT_LOAD_OP_CLEAR,
            VK_ATTACHMENT_STORE_OP_STORE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        }
    };
    
    std::vector<SubpassParameter> subpassParameter = 
    {
        {
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            {},
            {   
                {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
            },
            {},
            nullptr,
            {}
        }
    };

    std::vector<VkSubpassDependency> subpassDependencies = 
    {
        {
            VK_SUBPASS_EXTERNAL,
            0, 
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_MEMORY_READ_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_DEPENDENCY_BY_REGION_BIT
        },
        {
            VK_SUBPASS_EXTERNAL,
            0, 
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_ACCESS_MEMORY_READ_BIT,
            VK_DEPENDENCY_BY_REGION_BIT
        },
    };

    InitVkDestroyer(logicalDevice_, renderpass_);
    if (Cookbook::CreateRenderpass(*logicalDevice_, attachmentDescriptions, subpassParameter, subpassDependencies, *renderpass_) == false)
    {
        return false;
    }

    return true;
}

bool SampleChapter03::Draw(void)
{
    if (Cookbook::WaitForFence(*logicalDevice_, {*drawingFence_}, false, 5'000'000'000) == false)
    {
        return false;
    }

    if (Cookbook::ResetFence(*logicalDevice_, {*drawingFence_}) == false)
    {
        return false;
    }

    uint32_t imageIndex;
    if (Cookbook::AccquireSwapchainImage(*logicalDevice_, *swapchain_.handle, *imageAcquiredSemaphore_, VK_NULL_HANDLE, imageIndex) == false)
    {
        return false;
    }

    InitVkDestroyer(logicalDevice_, framebuffer_);
    if (Cookbook::CreateFramebuffer(*logicalDevice_, 
        *renderpass_, 
        {*swapchain_.imageViews[imageIndex]}, 
        swapchain_.size.width,
        swapchain_.size.height,
        1,
        *framebuffer_) == false)
    {
        return false;
    }

    if (Cookbook::BeginCommandBufferRecordingOperation(commandBuffer_, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr) == false)
    {
        return false;
    }

    if (presentQueue_.familyIndex != graphicsQueue_.familyIndex)
    {
        ImageTransition image_transition_before_drawing =
        {
            swapchain_.images[imageIndex],
            VK_ACCESS_MEMORY_READ_BIT,
            VK_ACCESS_MEMORY_READ_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            presentQueue_.familyIndex,
            graphicsQueue_.familyIndex,
            VK_IMAGE_ASPECT_COLOR_BIT
        };

        Cookbook::SetImageMemoryBarrier(commandBuffer_, 
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            {image_transition_before_drawing});
    }

    Cookbook::BeginRenderPass(commandBuffer_, 
        *renderpass_, 
        *framebuffer_, 
        {{0, 0}, swapchain_.size},
        {{0.2f, 0.5f, 0.8f, 1.0f}},
        VK_SUBPASS_CONTENTS_INLINE);

    Cookbook::EndRenderPass(commandBuffer_);

    if (presentQueue_.familyIndex != graphicsQueue_.familyIndex)
    {
        ImageTransition image_transition_before_present =
        {
            swapchain_.images[imageIndex],
            VK_ACCESS_MEMORY_READ_BIT,
            VK_ACCESS_MEMORY_READ_BIT,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            graphicsQueue_.familyIndex,
            presentQueue_.familyIndex,
            VK_IMAGE_ASPECT_COLOR_BIT
        };

        Cookbook::SetImageMemoryBarrier(commandBuffer_, 
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            {image_transition_before_present});
    }

    if (Cookbook::EndCommandBufferRecordingOperation(commandBuffer_) == false)
    {
        return false;
    }

    WaitSemaphoreInfo waitSemaphoreInfo = 
    {
        *imageAcquiredSemaphore_,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
    };
    if (Cookbook::SubmitCommandBufferToQueue(graphicsQueue_.handle, 
        {waitSemaphoreInfo},
        {commandBuffer_},
        {*readyToPresentSemaphore_},
        *drawingFence_) == false)
    {
        return false;
    }

    PresentInfo presentInfo =
    {
        *swapchain_.handle,
        imageIndex
    };

    if (Cookbook::PresentImage(presentQueue_.handle, {*readyToPresentSemaphore_}, {presentInfo}) == false)
    {
        return false;
    }

    return true;
}

bool SampleChapter03::Resize(void)
{
    return CreateSwapchain();
}

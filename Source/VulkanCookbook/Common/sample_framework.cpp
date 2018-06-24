
#include "sample_framework.h"

#include "../Chapter/ch01_instance_and_device.h"
#include "../Chapter/ch02_image_presentation.h"
#include "../Chapter/ch03_command_buffer_and_synchronization.h"
#include "../Chapter/ch04_resource_and_memory.h"

//nanpi
#include <iostream>

//----------------------------------------------------------------
// TimerStateParameters
//---------------------------------------------------------------
TimerStateParameters::TimerStateParameters(void)
{
    Update();
}

float TimerStateParameters::GetTime(void) const
{
    auto durtaion = time_.time_since_epoch();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds> (durtaion).count();

    return static_cast<float> (milliseconds * 0.001f);
}

float TimerStateParameters::GetDeltaTime(void) const
{
    return deltaTime_.count();
}

void TimerStateParameters::Update(void)
{
    auto previousTime = time_;
    time_ = std::chrono::high_resolution_clock::now();
    deltaTime_ = time_ - previousTime;
}

//----------------------------------------------------------------
// VulkanSampleBase
//----------------------------------------------------------------
VulkanSampleBase::VulkanSampleBase(void)
    : vulkanLibrary_(nullptr), ready_(false)
{
}

VulkanSampleBase::~VulkanSampleBase(void)
{   
    Cookbook::ReleaseVulkanLibrary(vulkanLibrary_);
}

void VulkanSampleBase::MouseClick(size_t buttonIndex, bool state)
{
    if (buttonIndex < 2)
    {
        mouseState_.buttons[buttonIndex].isPressed = state;
        mouseState_.buttons[buttonIndex].wasClicked = state;
        mouseState_.buttons[buttonIndex].wasClicked = !state;

        OnMouseEvent();
    }
}

void VulkanSampleBase::MouseMove(int x, int y)
{
    mouseState_.position.delta.x = x - mouseState_.position.x;
    mouseState_.position.delta.y = y - mouseState_.position.y;
    mouseState_.position.x = x;
    mouseState_.position.y = y;
    
    OnMouseEvent();
}

void VulkanSampleBase::MouseWheel(float distance)
{
    mouseState_.wheel.wasMoved = true;
    mouseState_.wheel.distance = distance;

    OnMouseEvent();
}

void VulkanSampleBase::MouseReset(void)
{
    mouseState_.position.delta.x = 0;
    mouseState_.position.delta.y = 0;
    mouseState_.buttons[0].wasClicked = false;
    mouseState_.buttons[0].wasReleased = false;
    mouseState_.buttons[1].wasClicked = false;
    mouseState_.buttons[1].wasReleased = false;
    mouseState_.wheel.wasMoved = false;
    mouseState_.wheel.distance = 0.0f;
}

void VulkanSampleBase::UpdateTime(void)
{
    timerState_.Update();
}

bool VulkanSampleBase::IsReady(void)
{
    return ready_;
}

void VulkanSampleBase::OnMouseEvent(void)
{
    /* override */
}

//----------------------------------------------------------------

VulkanSample::VulkanSample(void)
{
}

VulkanSample::~VulkanSample(void)
{
}

bool VulkanSample::InitializeVulkan(WindowParameters params, 
    VkPhysicalDeviceFeatures* desiredDeviceFeatures,
    VkImageUsageFlags swapchainImageUsage,
    bool useDepth,
    VkImageUsageFlags depthAttachmentUsage)
{
    if (Cookbook::ConnectWithVulkanLoaderLibrary(vulkanLibrary_) == false)
    {
        return false;
    }
    
    if (Cookbook::LoadFunctionExportedFromVulkanLoaderLibrary(vulkanLibrary_) == false)
    {
        return false;
    }

    if (Cookbook::LoadGlobalLevelFunctions() == false)
    {
        return false;
    }

    std::vector<const char*> instanceExtensions;
    InitVkDestroyer(instance_);
    if (Cookbook::CreateVulkanInstanceWithWsiExtensionEnabled(instanceExtensions, "vulkan cookbook", *instance_) == false)
    {
        return false;
    }

    if (Cookbook::LoadInstanceLevelFunction(*instance_,  instanceExtensions) == false)
    {
        return false;
    }
        
    InitVkDestroyer(instance_, presentationSurface_);
    if (Cookbook::CreatePresentaionSurface(*instance_, params, *presentationSurface_) == false)
    {
        return false;
    }

    std::vector<VkPhysicalDevice> physicalDevices;
    Cookbook::EnumerateAvailablePhysicalDevice(*instance_, physicalDevices);

    for (auto& physicalDevice : physicalDevices)
    {
        if (Cookbook::SelectIndexOfQueueFamilyWithDesiredCapability(physicalDevice, VK_QUEUE_GRAPHICS_BIT, graphicsQueue_.familyIndex) == false)
        {
            continue;
        }
        if (Cookbook::SelectIndexOfQueueFamilyWithDesiredCapability(physicalDevice, VK_QUEUE_COMPUTE_BIT, computeQueue_.familyIndex) == false)
        {
            continue;
        }

        if (Cookbook::SelectQueueFamilyThatSupportPresentToGivenSurface(physicalDevice, *presentationSurface_, presentQueue_.familyIndex) == false)
        {
            continue;
        }

        std::vector<QueueInfo> requestedQueues = 
        {
            {graphicsQueue_.familyIndex, {1.0f}},
        };

        if (graphicsQueue_.familyIndex != computeQueue_.familyIndex)
        {
            requestedQueues.push_back({computeQueue_.familyIndex, {1.0f}});
        }
        if ((graphicsQueue_.familyIndex != presentQueue_.familyIndex)
            && (computeQueue_.familyIndex != presentQueue_.familyIndex))
        {
            requestedQueues.push_back({presentQueue_.familyIndex, {1.0f}});
        }

        std::vector<const char*> deviceExtensions;
        InitVkDestroyer(logicalDevice_);

        if (Cookbook::CreateLogicalDeviceWithWsiExtensionEnabled(physicalDevice, requestedQueues, deviceExtensions, desiredDeviceFeatures, *logicalDevice_) == true)
        {
            physicalDevice_ = physicalDevice;
            Cookbook::LoadDeviceLevelFunction(*logicalDevice_, deviceExtensions);
            Cookbook::GetDeviceQueue(*logicalDevice_, graphicsQueue_.familyIndex, 0, graphicsQueue_.handle);
            Cookbook::GetDeviceQueue(*logicalDevice_, graphicsQueue_.familyIndex, 0, computeQueue_.handle);
            Cookbook::GetDeviceQueue(*logicalDevice_, presentQueue_.familyIndex, 0, presentQueue_.handle);
            break;
        }
        else
        {
            continue;
        }
    }

    if (logicalDevice_ == false)
    {
        return false;
    }
        
    InitVkDestroyer(logicalDevice_, commandPool_);
    if (Cookbook::CreateCommandPool(*logicalDevice_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, graphicsQueue_.familyIndex, *commandPool_) == false)
    {
        return false;
    }

    for (uint32_t idx = 0; idx < frameCount; ++idx)
    {
        std::vector<VkCommandBuffer> commandBuffer;
        VkDestroyer(VkSemaphore) imageAcquiredSemaphore;
        InitVkDestroyer(logicalDevice_, imageAcquiredSemaphore);
        VkDestroyer(VkSemaphore) readyToPresentSemaphore;
        InitVkDestroyer(logicalDevice_, readyToPresentSemaphore);
        VkDestroyer(VkFence) drawingFinishedFence;
        InitVkDestroyer(logicalDevice_, drawingFinishedFence);
        VkDestroyer(VkImageView) depthAttachment;
        InitVkDestroyer(logicalDevice_, depthAttachment);

        if (Cookbook::AllocateCommandBuffer(*logicalDevice_, *commandPool_, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, commandBuffer) == false)
        {
            return false;
        }
        if (Cookbook::CreateSemaphore(*logicalDevice_, *imageAcquiredSemaphore) == false)
        {
            return false;
        }
        if (Cookbook::CreateSemaphore(*logicalDevice_, *readyToPresentSemaphore) == false)
        {
            return false;
        }
        if (Cookbook::CreateFence(*logicalDevice_, true, *drawingFinishedFence) == false)
        {
            return false;
        }

        frameResources_.emplace_back(commandBuffer[0],
            std::move(imageAcquiredSemaphore),
            std::move(readyToPresentSemaphore),
            std::move(drawingFinishedFence),
            std::move(depthAttachment),
            VkDestroyer(VkFramebuffer)());
    }

    if (CreateSwapchain(swapchainImageUsage, useDepth, depthAttachmentUsage) == false)
    {
        return false;
    }

    return true;
}

bool VulkanSample::CreateSwapchain(VkImageUsageFlags swapchainImageUsage,
    bool useDepth, 
    VkImageCreateFlags depthAttachmentUsage)
{
    Cookbook::WaitForAllSubmittedCommandToBeFinished(*logicalDevice_);
    
    ready_ = false;

    swapchain_.imageViewsRaw.clear();
    swapchain_.imageViews.clear();
    swapchain_.images.clear();

    if (swapchain_.handle == false)
    {
        InitVkDestroyer(logicalDevice_, swapchain_.handle);
    }

    VkDestroyer(VkSwapchainKHR) oldSwapChain = std::move(swapchain_.handle);
    InitVkDestroyer(logicalDevice_, swapchain_.handle);
    if (Cookbook::CreateSwapchainWithR8G8B8A8FormatAndMailboxPresentMode(physicalDevice_, *presentationSurface_, *logicalDevice_, swapchainImageUsage, swapchain_.size, swapchain_.format, *oldSwapChain, *swapchain_.handle, swapchain_.images) == false)
    {
        return false;
    }

    if (swapchain_.handle == false)
    {
        return true;
    }

    for (size_t idx = 0; idx < swapchain_.images.size(); ++idx)
    {
        swapchain_.imageViews.emplace_back(VkDestroyer(VkImageView)());
        InitVkDestroyer(logicalDevice_, swapchain_.imageViews.back());
        if (Cookbook::CreateImageView(*logicalDevice_, swapchain_.images[idx], VK_IMAGE_VIEW_TYPE_2D, swapchain_.format, VK_IMAGE_ASPECT_COLOR_BIT, *swapchain_.imageViews.back()) == false)
        {
            return false;
        }
        swapchain_.imageViewsRaw.push_back(*swapchain_.imageViews.back());
    }

    depthImages.clear();
    depthImagesMemory.clear();

    if (useDepth == true)
    {
        for (uint32_t idx = 0; idx < frameCount; ++idx)
        {
            depthImages.emplace_back(VkDestroyer(VkImage)());
            InitVkDestroyer(logicalDevice_, depthImages.back());
            depthImagesMemory.emplace_back(VkDestroyer(VkDeviceMemory)());
            InitVkDestroyer(logicalDevice_, depthImagesMemory.back());
            InitVkDestroyer(logicalDevice_, frameResources_[idx].depthAttachment);

            if (Cookbook::Create2DImageAndView(physicalDevice_,
                *logicalDevice_,
                depthFormat, 
                swapchain_.size, 
                1, 
                1, 
                VK_SAMPLE_COUNT_1_BIT,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_IMAGE_ASPECT_DEPTH_BIT,
                *depthImages.back(),
                *depthImagesMemory.back(),
                *frameResources_[idx].depthAttachment) == false)
            {
                return false;
            }
        }
    }

    ready_ = true;
    return true;
}

void VulkanSample::Deinitialize(void)
{
    if (logicalDevice_)
    {
        Cookbook::WaitForAllSubmittedCommandToBeFinished(*logicalDevice_);
    }
}

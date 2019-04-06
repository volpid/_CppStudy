
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
    auto durtaion = _time.time_since_epoch();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds> (durtaion).count();

    return static_cast<float> (milliseconds * 0.001f);
}

float TimerStateParameters::GetDeltaTime(void) const
{
    return _deltaTime.count();
}

void TimerStateParameters::Update(void)
{
    auto previousTime = _time;
    _time = std::chrono::high_resolution_clock::now();
    _deltaTime = _time - previousTime;
}

//----------------------------------------------------------------
// VulkanSampleBase
//----------------------------------------------------------------
VulkanSampleBase::VulkanSampleBase(void)
    : _vulkanLibrary(nullptr), _ready(false)
{
}

VulkanSampleBase::~VulkanSampleBase(void)
{   
    Cookbook::ReleaseVulkanLibrary(_vulkanLibrary);
}

void VulkanSampleBase::MouseClick(size_t buttonIndex, bool state)
{
    if (buttonIndex < 2)
    {
        _mouseState.buttons[buttonIndex].isPressed = state;
        _mouseState.buttons[buttonIndex].wasClicked = state;
        _mouseState.buttons[buttonIndex].wasClicked = !state;

        OnMouseEvent();
    }
}

void VulkanSampleBase::MouseMove(int x, int y)
{
    _mouseState.position.delta.x = x - _mouseState.position.x;
    _mouseState.position.delta.y = y - _mouseState.position.y;
    _mouseState.position.x = x;
    _mouseState.position.y = y;
    
    OnMouseEvent();
}

void VulkanSampleBase::MouseWheel(float distance)
{
    _mouseState.wheel.wasMoved = true;
    _mouseState.wheel.distance = distance;

    OnMouseEvent();
}

void VulkanSampleBase::MouseReset(void)
{
    _mouseState.position.delta.x = 0;
    _mouseState.position.delta.y = 0;
    _mouseState.buttons[0].wasClicked = false;
    _mouseState.buttons[0].wasReleased = false;
    _mouseState.buttons[1].wasClicked = false;
    _mouseState.buttons[1].wasReleased = false;
    _mouseState.wheel.wasMoved = false;
    _mouseState.wheel.distance = 0.0f;
}

void VulkanSampleBase::UpdateTime(void)
{
    _timerState.Update();
}

bool VulkanSampleBase::IsReady(void)
{
    return _ready;
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
    if (Cookbook::ConnectWithVulkanLoaderLibrary(_vulkanLibrary) == false)
    {
        return false;
    }
    
    if (Cookbook::LoadFunctionExportedFromVulkanLoaderLibrary(_vulkanLibrary) == false)
    {
        return false;
    }

    if (Cookbook::LoadGlobalLevelFunctions() == false)
    {
        return false;
    }

    std::vector<const char*> instanceExtensions;
    InitVkDestroyer(_instance);
    if (Cookbook::CreateVulkanInstanceWithWsiExtensionEnabled(instanceExtensions, "vulkan cookbook", *_instance) == false)
    {
        return false;
    }

    if (Cookbook::LoadInstanceLevelFunction(*_instance,  instanceExtensions) == false)
    {
        return false;
    }
        
    InitVkDestroyer(_instance, _presentationSurface);
    if (Cookbook::CreatePresentaionSurface(*_instance, params, *_presentationSurface) == false)
    {
        return false;
    }

    std::vector<VkPhysicalDevice> physicalDevices;
    Cookbook::EnumerateAvailablePhysicalDevice(*_instance, physicalDevices);

    for (auto& physicalDevice : physicalDevices)
    {
        if (Cookbook::SelectIndexOfQueueFamilyWithDesiredCapability(physicalDevice, VK_QUEUE_GRAPHICS_BIT, _graphicsQueue.familyIndex) == false)
        {
            continue;
        }
        if (Cookbook::SelectIndexOfQueueFamilyWithDesiredCapability(physicalDevice, VK_QUEUE_COMPUTE_BIT, _computeQueue.familyIndex) == false)
        {
            continue;
        }

        if (Cookbook::SelectQueueFamilyThatSupportPresentToGivenSurface(physicalDevice, *_presentationSurface, _presentQueue.familyIndex) == false)
        {
            continue;
        }

        std::vector<QueueInfo> requestedQueues = 
        {
            {_graphicsQueue.familyIndex, {1.0f}},
        };

        if (_graphicsQueue.familyIndex != _computeQueue.familyIndex)
        {
            requestedQueues.push_back({_computeQueue.familyIndex, {1.0f}});
        }
        if ((_graphicsQueue.familyIndex != _presentQueue.familyIndex)
            && (_computeQueue.familyIndex != _presentQueue.familyIndex))
        {
            requestedQueues.push_back({_presentQueue.familyIndex, {1.0f}});
        }

        std::vector<const char*> deviceExtensions;
        InitVkDestroyer(_logicalDevice);

        if (Cookbook::CreateLogicalDeviceWithWsiExtensionEnabled(physicalDevice, requestedQueues, deviceExtensions, desiredDeviceFeatures, *_logicalDevice) == true)
        {
            _physicalDevice = physicalDevice;
            Cookbook::LoadDeviceLevelFunction(*_logicalDevice, deviceExtensions);
            Cookbook::GetDeviceQueue(*_logicalDevice, _graphicsQueue.familyIndex, 0, _graphicsQueue.handle);
            Cookbook::GetDeviceQueue(*_logicalDevice, _graphicsQueue.familyIndex, 0, _computeQueue.handle);
            Cookbook::GetDeviceQueue(*_logicalDevice, _presentQueue.familyIndex, 0, _presentQueue.handle);
            break;
        }
        else
        {
            continue;
        }
    }

    if (_logicalDevice == false)
    {
        return false;
    }
        
    InitVkDestroyer(_logicalDevice, _commandPool);
    if (Cookbook::CreateCommandPool(*_logicalDevice, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, _graphicsQueue.familyIndex, *_commandPool) == false)
    {
        return false;
    }

    for (uint32_t idx = 0; idx < FrameCount; ++idx)
    {
        std::vector<VkCommandBuffer> commandBuffer;
        VkDestroyer(VkSemaphore) imageAcquiredSemaphore;
        InitVkDestroyer(_logicalDevice, imageAcquiredSemaphore);
        VkDestroyer(VkSemaphore) readyToPresentSemaphore;
        InitVkDestroyer(_logicalDevice, readyToPresentSemaphore);
        VkDestroyer(VkFence) drawingFinishedFence;
        InitVkDestroyer(_logicalDevice, drawingFinishedFence);
        VkDestroyer(VkImageView) depthAttachment;
        InitVkDestroyer(_logicalDevice, depthAttachment);

        if (Cookbook::AllocateCommandBuffer(*_logicalDevice, *_commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, commandBuffer) == false)
        {
            return false;
        }
        if (Cookbook::CreateSemaphore(*_logicalDevice, *imageAcquiredSemaphore) == false)
        {
            return false;
        }
        if (Cookbook::CreateSemaphore(*_logicalDevice, *readyToPresentSemaphore) == false)
        {
            return false;
        }
        if (Cookbook::CreateFence(*_logicalDevice, true, *drawingFinishedFence) == false)
        {
            return false;
        }

        _frameResources.emplace_back(commandBuffer[0],
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
    depthAttachmentUsage;

    Cookbook::WaitForAllSubmittedCommandToBeFinished(*_logicalDevice);
    
    _ready = false;

    _swapchain.imageViewsRaw.clear();
    _swapchain.imageViews.clear();
    _swapchain.images.clear();

    if (_swapchain.handle == false)
    {
        InitVkDestroyer(_logicalDevice, _swapchain.handle);
    }

    VkDestroyer(VkSwapchainKHR) oldSwapChain = std::move(_swapchain.handle);
    InitVkDestroyer(_logicalDevice, _swapchain.handle);
    if (Cookbook::CreateSwapchainWithR8G8B8A8FormatAndMailboxPresentMode(_physicalDevice, *_presentationSurface, *_logicalDevice, swapchainImageUsage, _swapchain.size, _swapchain.format, *oldSwapChain, *_swapchain.handle, _swapchain.images) == false)
    {
        return false;
    }

    if (_swapchain.handle == false)
    {
        return true;
    }

    for (size_t idx = 0; idx < _swapchain.images.size(); ++idx)
    {
        _swapchain.imageViews.emplace_back(VkDestroyer(VkImageView)());
        InitVkDestroyer(_logicalDevice, _swapchain.imageViews.back());
        if (Cookbook::CreateImageView(*_logicalDevice, _swapchain.images[idx], VK_IMAGE_VIEW_TYPE_2D, _swapchain.format, VK_IMAGE_ASPECT_COLOR_BIT, *_swapchain.imageViews.back()) == false)
        {
            return false;
        }
        _swapchain.imageViewsRaw.push_back(*_swapchain.imageViews.back());
    }

    _depthImages.clear();
    _depthImagesMemory.clear();

    if (useDepth == true)
    {
        for (uint32_t idx = 0; idx < FrameCount; ++idx)
        {
            _depthImages.emplace_back(VkDestroyer(VkImage)());
            InitVkDestroyer(_logicalDevice, _depthImages.back());
            _depthImagesMemory.emplace_back(VkDestroyer(VkDeviceMemory)());
            InitVkDestroyer(_logicalDevice, _depthImagesMemory.back());
            InitVkDestroyer(_logicalDevice, _frameResources[idx].depthAttachment);

            if (Cookbook::Create2DImageAndView(_physicalDevice,
                *_logicalDevice,
                DepthFormat, 
                _swapchain.size, 
                1, 
                1, 
                VK_SAMPLE_COUNT_1_BIT,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_IMAGE_ASPECT_DEPTH_BIT,
                *_depthImages.back(),
                *_depthImagesMemory.back(),
                *_frameResources[idx].depthAttachment) == false)
            {
                return false;
            }
        }
    }

    _ready = true;
    return true;
}

void VulkanSample::Deinitialize(void)
{
    if (_logicalDevice)
    {
        Cookbook::WaitForAllSubmittedCommandToBeFinished(*_logicalDevice);
    }
}

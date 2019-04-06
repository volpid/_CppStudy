
#include "../../Chapter/ch01_instance_and_device.h"
#include "../../Chapter/ch02_image_presentation.h"
#include "../../Chapter/ch03_command_buffer_and_synchronization.h"
#include "../../Chapter/ch04_resource_and_memory.h"

#include "../../Common/sample_framework.h"

//----------------------------------------------------------------
// SampleChapter02 : create swap chain
//----------------------------------------------------------------
class SampleChapter02 : public VulkanSampleBase
{
public:
    virtual bool Initialize(WindowParameters parameters) override;
    virtual bool Draw(void) override;
    virtual bool Resize(void) override;
    virtual void Deinitialize(void) override;

private:
    bool CreateSwapchain(void);

private:
    VkDestroyer(VkInstance) _instance;
    VkPhysicalDevice _physicalDevice;
    VkDestroyer(VkSurfaceKHR) _presentationSurface;
    VkDestroyer(VkDevice) _logicalDevice;
    uint32_t graphicsQueueFamilyIndex_;
    uint32_t presentQueueFamilyIndex_;
    VkQueue _graphicsQueue;
    VkQueue _presentQueue;
    VkDestroyer(VkSwapchainKHR) _swapchain;
    std::vector<VkImage> swapchainImage_;
    VkDestroyer(VkSemaphore) imageAcquiredSemaphore_;
    VkDestroyer(VkSemaphore) readyToPresentSemaphore_;
    VkDestroyer(VkCommandPool) _commandPool;
    VkCommandBuffer commandBuffer_;
};

//VULKAN_SAMPLE_FRAMEWORK("02_CreateSwapchain", 50, 25, 800, 600, SampleChapter02);

//----------------------------------------------------------------
bool SampleChapter02::Initialize(WindowParameters parameters)
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
    if (Cookbook::CreateVulkanInstanceWithWsiExtensionEnabled(instanceExtensions, "vulkanCookbook_ch02", *_instance) == false)
    {
        return false;
    }

    if (Cookbook::LoadInstanceLevelFunction(*_instance, instanceExtensions) == false)
    {
        return false;
    }

    InitVkDestroyer(_instance, _presentationSurface);
    if (Cookbook::CreatePresentaionSurface(*_instance, parameters, *_presentationSurface) == false)
    {
        return false;
    }

    std::vector<VkPhysicalDevice> physicalDevices;
    Cookbook::EnumerateAvailablePhysicalDevice(*_instance, physicalDevices);
    for (auto& physicalDevice : physicalDevices)
    {
        if (Cookbook::SelectIndexOfQueueFamilyWithDesiredCapability(physicalDevice, VK_QUEUE_GRAPHICS_BIT, graphicsQueueFamilyIndex_) == false)
        {
            continue;
        }        
        if (Cookbook::SelectQueueFamilyThatSupportPresentToGivenSurface(physicalDevice, *_presentationSurface, presentQueueFamilyIndex_) == false)
        {
            continue;
        }

        std::vector<QueueInfo> requestedQueues = 
        {
            {graphicsQueueFamilyIndex_, {1.0f}},
        };

        if (graphicsQueueFamilyIndex_ != presentQueueFamilyIndex_)
        {
            requestedQueues.push_back({presentQueueFamilyIndex_, {1.0f}});
        }

        VkDestroyer(VkDevice) logicalDevice;
        std::vector<const char*> deviceExtensions;
        if (Cookbook::CreateLogicalDeviceWithWsiExtensionEnabled(physicalDevice, requestedQueues, deviceExtensions, nullptr, *logicalDevice) == true)
        {
            if (Cookbook::LoadDeviceLevelFunction(*logicalDevice, deviceExtensions) == false)
            {
                continue;
            }

            _physicalDevice = physicalDevice;            
            _logicalDevice = std::move(logicalDevice);
            Cookbook::GetDeviceQueue(*_logicalDevice, graphicsQueueFamilyIndex_, 0, _graphicsQueue);
            Cookbook::GetDeviceQueue(*_logicalDevice, presentQueueFamilyIndex_, 0, _presentQueue);
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

    if (CreateSwapchain() == false)
    {
        return false;
    }

    InitVkDestroyer(_logicalDevice, imageAcquiredSemaphore_);
    if (Cookbook::CreateSemaphore(*_logicalDevice, *imageAcquiredSemaphore_) == false)
    {
        return false;
    }
    InitVkDestroyer(_logicalDevice, readyToPresentSemaphore_);
    if (Cookbook::CreateSemaphore(*_logicalDevice, *readyToPresentSemaphore_) == false)
    {
        return false;
    }

    InitVkDestroyer(_logicalDevice, _commandPool);
    if (Cookbook::CreateCommandPool(*_logicalDevice, 
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        graphicsQueueFamilyIndex_,
        *_commandPool) == false)
    {
        return false;
    }

    std::vector<VkCommandBuffer> commandBuffers;
    if (Cookbook::AllocateCommandBuffer(*_logicalDevice, *_commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, commandBuffers) == false)
    {
        return false;
    }

    commandBuffer_ = commandBuffers[0];

    return true;
}

bool SampleChapter02::Draw(void)
{
    Cookbook::WaitForAllSubmittedCommandToBeFinished(*_logicalDevice);

    uint32_t imageIndex;
    if (Cookbook::AcquireSwapchainImage(*_logicalDevice, *_swapchain, *imageAcquiredSemaphore_, VK_NULL_HANDLE, imageIndex) == false)
    {
        return false;
    }

    if (Cookbook::BeginCommandBufferRecordingOperation(commandBuffer_, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr) == false)
    {
        return false;
    }

    ImageTransition imageTransitionBeforeDrawing =
    {
        swapchainImage_[imageIndex],
        0,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        VK_IMAGE_ASPECT_COLOR_BIT
    };

    Cookbook::SetImageMemoryBarrier(commandBuffer_, 
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        {imageTransitionBeforeDrawing});

    ImageTransition imageTransitionBeforePresent =
    {
        swapchainImage_[imageIndex],
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        VK_IMAGE_ASPECT_COLOR_BIT
    };

    Cookbook::SetImageMemoryBarrier(commandBuffer_, 
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        {imageTransitionBeforePresent});

    if (Cookbook::EndCommandBufferRecordingOperation(commandBuffer_) == false)
    {
        return false;
    }

    WaitSemaphoreInfo waitSemaphoreInfo = 
    {
        *imageAcquiredSemaphore_,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
    };
    if (Cookbook::SubmitCommandBufferToQueue(_presentQueue, 
        {waitSemaphoreInfo},
        {commandBuffer_},
        {*readyToPresentSemaphore_},
        VK_NULL_HANDLE) == false)
    {
        return false;
    }

    PresentInfo presentInfo =
    {
        *_swapchain,
        imageIndex
    };

    if (Cookbook::PresentImage(_presentQueue, {*readyToPresentSemaphore_}, {presentInfo}) == false)
    {
        return false;
    }

    return true;
}

bool SampleChapter02::Resize(void)
{
    return CreateSwapchain();
}

void SampleChapter02::Deinitialize(void)
{
    if (_logicalDevice == true)
    {
        Cookbook::WaitForAllSubmittedCommandToBeFinished(*_logicalDevice);
    }
}

bool SampleChapter02::CreateSwapchain(void)
{
    Cookbook::WaitForAllSubmittedCommandToBeFinished(*_logicalDevice);

    _ready = false;
    VkFormat swapchainImageFormat;
    VkExtent2D swapchainImageSize;
    VkDestroyer(VkSwapchainKHR) oldSwapchain = std::move(_swapchain);
    InitVkDestroyer(_logicalDevice, _swapchain);

    if (Cookbook::CreateSwapchainWithR8G8B8A8FormatAndMailboxPresentMode(_physicalDevice, 
        *_presentationSurface, 
        *_logicalDevice, 
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        swapchainImageSize,
        swapchainImageFormat,
        *oldSwapchain,
        *_swapchain,
        swapchainImage_) == false)
    {
        return false;
    }

    if (*_swapchain)
    {
        _ready = true;
    }

    return true;
}
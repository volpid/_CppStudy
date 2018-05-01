//
//#include "ch01_instance_and_device.h"
//#include "ch02_image_presentation.h"
//#include "ch03_command_buffer_and_synchronization.h"
//#include "ch04_resource_and_memory.h"
//
//#include "../Common/sample_framework.h"
//
////----------------------------------------------------------------
//// SampleChapter03
////----------------------------------------------------------------
//class SampleChapter03 : public VulkanSampleBase
//{
//public:
//    virtual bool Initialize(WindowParameters parameters) override;
//    virtual bool Draw(void) override;
//    virtual bool Resize(void) override;
//    virtual void Deinitialize(void) override;
//
//private:
//    bool CreateSwapchain(void);
//
//private:
//    VkDestroyer(VkInstance) instance_;
//    VkPhysicalDevice physicalDevice_;
//    VkDestroyer(VkSurfaceKHR) presentationSurface_;
//    VkDestroyer(VkDevice) logicalDevice_;
//    uint32_t graphicsQueueFamilyIndex_;
//    uint32_t presentQueueFamilyIndex_;
//    VkQueue graphicsQueue_;
//    VkQueue presentQueue_;
//    VkDestroyer(VkSwapchainKHR) swapchain_;
//    std::vector<VkImage> swapchainImage_;
//    VkDestroyer(VkSemaphore) imageAcquiredSemaphore_;
//    VkDestroyer(VkSemaphore) readyToPresentSemaphore_;
//    VkDestroyer(VkCommandPool) commandPool_;
//    VkCommandBuffer commandBuffer_;
//};
//
//VULKAN_SAMPLE_FRAMEWORK("02_ImagePresentation", 50, 25, 1280, 800, SampleChapter03);
//
////----------------------------------------------------------------
//bool SampleChapter03::Initialize(WindowParameters parameters)
//{   
//    if (Cookbook::ConnectWithVulkanLoaderLibrary(vulkanLibrary_) == false)
//    {
//        return false;
//    }
//
//    if (Cookbook::LoadFunctionExportedFromVulkanLoaderLibrary(vulkanLibrary_) == false)
//    {
//        return false;
//    }
//
//    if (Cookbook::LoadGlobalFunctions() == false)
//    {
//        return false;
//    }
//
//    std::vector<const char*> instanceExtensions;
//    if (Cookbook::CreateVulkanInstanceWithWsiExtensionEnabled(instanceExtensions, "vulkanCookbook_ch02", *instance_) == false)
//    {
//        return false;
//    }
//
//    if (Cookbook::LoadInstanceLevelFunction(*instance_, instanceExtensions) == false)
//    {
//        return false;
//    }
//
//    InitVkDestroyer(instance_, presentationSurface_);
//    if (Cookbook::CreatePresentaionSurface(*instance_, parameters, *presentationSurface_) == false)
//    {
//        return false;
//    }
//
//    std::vector<VkPhysicalDevice> physicalDevices;
//    Cookbook::EnumerateAvailablePhysicalDevice(*instance_, physicalDevices);
//    for (auto& physicalDevice : physicalDevices)
//    {
//        if (Cookbook::SelectIndexOfQueueFamilyWithDesiredCapability(physicalDevice, VK_QUEUE_GRAPHICS_BIT, graphicsQueueFamilyIndex_) == false)
//        {
//            continue;
//        }        
//        if (Cookbook::SelectQueueFamilyThatSupportPresentToGivenSurface(physicalDevice, *presentationSurface_, presentQueueFamilyIndex_) == false)
//        {
//            continue;
//        }
//
//        std::vector<Cookbook::QueueInfo> requestedQueues = 
//        {
//            {graphicsQueueFamilyIndex_, {1.0f}},
//        };
//
//        if (graphicsQueueFamilyIndex_ != presentQueueFamilyIndex_)
//        {
//            requestedQueues.push_back({presentQueueFamilyIndex_, {1.0f}});
//        }
//
//        VkDestroyer(VkDevice) logicalDevice;
//        std::vector<const char*> deviceExtensions;
//        if (Cookbook::CreateLogicalDeviceWithWsiExtensionEnabled(physicalDevice, requestedQueues, deviceExtensions, nullptr, *logicalDevice) == true)
//        {
//            if (Cookbook::LoadDeviceLevelFunction(*logicalDevice, deviceExtensions) == false)
//            {
//                continue;
//            }
//
//            physicalDevice_ = physicalDevice;            
//            logicalDevice_ = std::move(logicalDevice);
//            Cookbook::GetDeviceQueue(*logicalDevice_, graphicsQueueFamilyIndex_, 0, graphicsQueue_);
//            Cookbook::GetDeviceQueue(*logicalDevice_, presentQueueFamilyIndex_, 0, presentQueue_);
//            break;
//        }
//        else
//        {
//            continue;
//        }
//    }
//
//    if (logicalDevice_ == false)
//    {
//        return false;
//    }
//
//    if (CreateSwapchain() == false)
//    {
//        return false;
//    }
//
//    InitVkDestroyer(logicalDevice_, imageAcquiredSemaphore_);
//    if (Cookbook::CreateSemaphore(*logicalDevice_, *imageAcquiredSemaphore_) == false)
//    {
//        return false;
//    }
//    InitVkDestroyer(logicalDevice_, readyToPresentSemaphore_);
//    if (Cookbook::CreateSemaphore(*logicalDevice_, *readyToPresentSemaphore_) == false)
//    {
//        return false;
//    }
//
//    InitVkDestroyer(logicalDevice_, commandPool_);
//    if (Cookbook::CreateCommandPool(*logicalDevice_, 
//        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
//        graphicsQueueFamilyIndex_,
//        *commandPool_) == false)
//    {
//        return false;
//    }
//
//    std::vector<VkCommandBuffer> commandBuffers;
//    if (Cookbook::AllocateCommandBuffer(*logicalDevice_, *commandPool_, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, commandBuffers) == false)
//    {
//        return false;
//    }
//
//    commandBuffer_ = commandBuffers[0];
//
//    return true;
//}
//
//bool SampleChapter03::Draw(void)
//{
//    Cookbook::WaitForAllSubmittedCommandToBeFinished(*logicalDevice_);
//
//    uint32_t imageIndex;
//    if (Cookbook::AccquireSwapchainImage(*logicalDevice_, *swapchain_, *imageAcquiredSemaphore_, VK_NULL_HANDLE, imageIndex) == false)
//    {
//        return false;
//    }
//
//    if (Cookbook::BeginCommandBufferRecordingOperation(commandBuffer_, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr) == false)
//    {
//        return false;
//    }
//
//    ImageTransition imageTransitionBeforeDrawing =
//    {
//        swapchainImage_[imageIndex],
//        0,
//        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
//        VK_IMAGE_LAYOUT_UNDEFINED,
//        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
//        VK_QUEUE_FAMILY_IGNORED,
//        VK_QUEUE_FAMILY_IGNORED,
//        VK_IMAGE_ASPECT_COLOR_BIT
//    };
//
//    Cookbook::SetImageMemoryBarrier(commandBuffer_, 
//        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
//        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
//        {imageTransitionBeforeDrawing});
//
//    ImageTransition imageTransitionBeforePresent =
//    {
//        swapchainImage_[imageIndex],
//        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
//        VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
//        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
//        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
//        VK_QUEUE_FAMILY_IGNORED,
//        VK_QUEUE_FAMILY_IGNORED,
//        VK_IMAGE_ASPECT_COLOR_BIT
//    };
//
//    Cookbook::SetImageMemoryBarrier(commandBuffer_, 
//        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
//        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
//        {imageTransitionBeforePresent});
//
//    if (Cookbook::EndCommandBufferRecordingOperation(commandBuffer_) == false)
//    {
//        return false;
//    }
//
//    WaitSemaphoreInfo waitSemaphoreInfo = 
//    {
//        *imageAcquiredSemaphore_,
//        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
//    };
//    if (Cookbook::SubmitCommandBufferToQueue(presentQueue_, 
//        {waitSemaphoreInfo},
//        {commandBuffer_},
//        {*readyToPresentSemaphore_},
//        VK_NULL_HANDLE) == false)
//    {
//        return false;
//    }
//
//    PresentInfo presentInfo =
//    {
//        *swapchain_,
//        imageIndex
//    };
//
//    if (Cookbook::PresentImage(presentQueue_, {*readyToPresentSemaphore_}, {presentInfo}) == false)
//    {
//        return false;
//    }
//
//    return true;
//}
//
//bool SampleChapter03::Resize(void)
//{
//    return CreateSwapchain();
//}
//
//void SampleChapter03::Deinitialize(void)
//{
//    if (logicalDevice_ == true)
//    {
//        Cookbook::WaitForAllSubmittedCommandToBeFinished(*logicalDevice_);
//    }
//}
//
//bool SampleChapter03::CreateSwapchain(void)
//{
//    Cookbook::WaitForAllSubmittedCommandToBeFinished(*logicalDevice_);
//
//    ready_ = false;
//    VkFormat swapchainImageFormat;
//    VkExtent2D swapchainImageSize;
//    VkDestroyer(VkSwapchainKHR) oldSwapchain = std::move(swapchain_);
//    InitVkDestroyer(logicalDevice_, swapchain_);
//
//    if (Cookbook::CreateSwapchainWithR8G8B8A8FormatAndMailboxPresentMode(physicalDevice_, 
//        *presentationSurface_, 
//        *logicalDevice_, 
//        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
//        swapchainImageSize,
//        swapchainImageFormat,
//        *oldSwapchain,
//        *swapchain_,
//        swapchainImage_) == false)
//    {
//        return false;
//    }
//
//    if (*swapchain_)
//    {
//        ready_ = true;
//    }
//
//    return true;
//}
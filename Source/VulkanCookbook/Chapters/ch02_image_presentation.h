
#ifndef __CH2_IMAGE_PRESENTATION__H__
#define __CH2_IMAGE_PRESENTATION__H__

#include "../Common/vulkan_common.h"

#include <vector>

namespace Cookbook
{
    bool SelectQueueFamilyThatSupportPresentToGivenSurface(VkPhysicalDevice physicalDevice,
        VkSurfaceKHR presentationSurface,
        uint32_t& queueFamilyIndex);
    bool CreateLogicalDeviceWithWsiExtensionEnabled(VkPhysicalDevice physicalDevice,
        std::vector<QueueInfo> queueInfos,
        std::vector<const char*>& desiredExtensions,
        VkPhysicalDeviceFeatures* desiredFeatures,
        VkDevice& logicalDevice);
    bool CreateSwapchain(VkDevice logicalDevice,
        VkSurfaceKHR presentationSurface,
        uint32_t imageCount,
        VkSurfaceFormatKHR surfaceFormat,
        VkExtent2D imageSize,
        VkImageUsageFlags imageUsage,
        VkSurfaceTransformFlagBitsKHR surfaceTransform,
        VkPresentModeKHR presentMode,
        VkSwapchainKHR& oldSwapchain,
        VkSwapchainKHR& swapchain);
    bool CreateSwapchainWithR8G8B8A8FormatAndMailboxPresentMode(VkPhysicalDevice physicalDevice,
        VkSurfaceKHR presentationSurface,
        VkDevice logicalDevice,
        VkImageUsageFlags swapchainImageUsage,
        VkExtent2D& imageSize,
        VkFormat& imageFormat,
        VkSwapchainKHR& oldSwapchain,
        VkSwapchainKHR& swapchain,
        std::vector<VkImage>& swapchainImages);
    bool SelectDesiredPresentationMode(VkPhysicalDevice physicalDevice,
        VkSurfaceKHR presentationSurface,
        VkPresentModeKHR desiredPresentMode,
        VkPresentModeKHR& presentMode);
    bool GetCapabilityOfPresentationSurface(VkPhysicalDevice physicalDevice,
        VkSurfaceKHR presentationSurface,
        VkSurfaceCapabilitiesKHR& surfaceCapabilities);
    bool SelectNumberOfSwapchainImage(const VkSurfaceCapabilitiesKHR& surfaceCapabilities, uint32_t& numberOfImages);
    bool ChooseSizeOfSwapchainImage(const VkSurfaceCapabilitiesKHR& surfaceCapabilities, VkExtent2D& sizeOfImages);
    bool SelectDesiredUsageScenarioOfSwapchainImage(const VkSurfaceCapabilitiesKHR& surfaceCapabilities,
        VkImageUsageFlags desiredUsage,
        VkImageUsageFlags& imageUsage);
    bool SelectTransformationOfSwapchainImage(const VkSurfaceCapabilitiesKHR& surfaceCapabilities,
        VkSurfaceTransformFlagBitsKHR desiredTransform,
        VkSurfaceTransformFlagBitsKHR& surfaceTransform);
    bool SelectFormatOfSwapchainImage(VkPhysicalDevice physicalDevice,
        VkSurfaceKHR presentationSurface,
        VkSurfaceFormatKHR desiredSurfaceFormat,
        VkFormat& iamgeFormat,
        VkColorSpaceKHR& imageColorSpace);
    bool GetHandleOfSwapchainImage(VkDevice logicalDevice, VkSwapchainKHR swapchain, std::vector<VkImage>& swapchainImages);
    bool AccquireSwapchainImage(VkDevice logicalDevice,
        VkSwapchainKHR swapchain,
        VkSemaphore semaphore,
        VkFence fence,
        uint32_t& imageIndex);
    bool BeginCommandBufferRecordingOperation(VkCommandBuffer commandbuffer,
        VkCommandBufferUsageFlags usage,
        VkCommandBufferInheritanceInfo* secondaryCommandBufferInfo);
    bool PresentImage(VkQueue queue, std::vector<VkSemaphore> renderingSemaphore, std::vector<PresentInfo> imagesToPresent);
}

#endif /*__CH2_IMAGE_PRESENTATION__H__*/
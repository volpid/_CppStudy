
#include "ch01_instance_and_device.h"
#include "ch02_image_presentation.h"

#include <iostream>

bool Cookbook::CreateVulkanInstanceWithWsiExtensionEnabled(std::vector<const char*>& desiredExtension,
    const char* const applicationName, 
    VkInstance& instance)
{
    desiredExtension.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
    desiredExtension.emplace_back(    
#if defined(VK_USE_PLATFORM_WIN32_KHR)
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
        VK_KHR_XCB_SURFACE_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_XCB_KHR)
        VK_KHR_XLIB_SURFACE_EXTENSION_NAME
#endif /**/
    );
    
    return CreateVulkanInstance(desiredExtension, applicationName, instance);
}

bool Cookbook::CreatePresentaionSurface(VkInstance instance, WindowParameters windowParams, VkSurfaceKHR& presentationSurface)
{
    VkResult result;

#if defined(VK_USE_PLATFORM_WIN32_KHR)
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = 
    {
        VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        nullptr, 
        0, 
        windowParams.hIntance,
        windowParams.hWnd
    };
    result = vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &presentationSurface);

#elif defined(VK_USE_PLATFORM_XLIB_KHR)
    VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = 
    {
        VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
        nullptr, 
        0, 
        windowParams.pDisplay,
        windowParams.window
    };

    result = vkCreateXlibSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &presentationSurface);

#elif defined(VK_USE_PLATFORM_XCB_KHR)
    VkXcbSurfaceCreateInfoKHR surfaceCreateInfo = 
    {
        VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
        nullptr, 
        0, 
        windowParams.connection,
        windowParams.window
    };

    result = vkCreateXcbSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &presentationSurface);

#endif /**/

    if (result != VK_SUCCESS || presentationSurface == VK_NULL_HANDLE)
    {
        std::cout << "Could not craete presentation surface." << std::endl;
        return false;
    }

    return true;
}

bool Cookbook::SelectQueueFamilyThatSupportPresentToGivenSurface(VkPhysicalDevice physicalDevice,
    VkSurfaceKHR presentationSurface,
    uint32_t& queueFamilyIndex)
{
    std::vector<VkQueueFamilyProperties> queueFamilies;
    if (CheckAvailableQueueFamilyAndProperty(physicalDevice, queueFamilies) == false)
    {
        return false;
    }

    for (uint32_t index = 0; index < static_cast<uint32_t> (queueFamilies.size()); ++index)
    {
        VkBool32 presentationSupported = VK_FALSE;
        VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, index, presentationSurface, &presentationSupported);
        if (result == VK_SUCCESS && presentationSupported == VK_TRUE)
        {
            queueFamilyIndex = index;
            return true;
        }
    }
    
    return false;
}

bool Cookbook::CreateLogicalDeviceWithWsiExtensionEnabled(VkPhysicalDevice physicalDevice,
    std::vector<QueueInfo> queueInfos,
    std::vector<const char*>& desiredExtensions,
    VkPhysicalDeviceFeatures* desiredFeatures,
    VkDevice& logicalDevice)
{
    desiredExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        
    return CreateLogicalDevice(physicalDevice, queueInfos, desiredExtensions, desiredFeatures, logicalDevice);
}

bool Cookbook::CreateSwapchain(VkDevice logicalDevice,
    VkSurfaceKHR presentationSurface,
    uint32_t imageCount,
    VkSurfaceFormatKHR surfaceFormat,
    VkExtent2D imageSize,
    VkImageUsageFlags imageUsage,
    VkSurfaceTransformFlagBitsKHR surfaceTransform,
    VkPresentModeKHR presentMode,
    VkSwapchainKHR& oldSwapchain,
    VkSwapchainKHR& swapchain)
{
    VkSwapchainCreateInfoKHR swapchainCreateInfo = 
    {
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        nullptr,
        0,
        presentationSurface,
        imageCount,
        surfaceFormat.format,
        surfaceFormat.colorSpace,
        imageSize,
        1,
        imageUsage,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr,
        surfaceTransform,
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        presentMode,
        VK_TRUE,
        oldSwapchain
    };
    
    VkResult result = vkCreateSwapchainKHR(logicalDevice, &swapchainCreateInfo, nullptr, &swapchain);
    if (result != VK_SUCCESS || swapchain == VK_NULL_HANDLE)
    {
        std::cout << "Could not create a swapchain." << std::endl;
        return false;
    }

    if (oldSwapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(logicalDevice, oldSwapchain, nullptr);
        oldSwapchain = VK_NULL_HANDLE;
    }

    return true;
}

bool Cookbook::CreateSwapchainWithR8G8B8A8FormatAndMailboxPresentMode(VkPhysicalDevice physicalDevice,
    VkSurfaceKHR presentationSurface,
    VkDevice logicalDevice,
    VkImageUsageFlags swapchainImageUsage,
    VkExtent2D& imageSize,
    VkFormat& imageFormat,
    VkSwapchainKHR& oldSwapchain,
    VkSwapchainKHR& swapchain,
    std::vector<VkImage>& swapchainImages)
{
    VkPresentModeKHR desiredPresentMode;
    if (SelectDesiredPresentationMode(physicalDevice, presentationSurface, VK_PRESENT_MODE_MAILBOX_KHR, desiredPresentMode) == false)
    {
        return false;
    }

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    if (GetCapabilityOfPresentationSurface(physicalDevice, presentationSurface, surfaceCapabilities) == false)
    {
        return false;
    }

    uint32_t numberOfImages;
    if (SelectNumberOfSwapchainImage(surfaceCapabilities, numberOfImages) == false)
    {
        return false;
    }

    if (ChooseSizeOfSwapchainImage(surfaceCapabilities, imageSize) == false)
    {
        return false;
    }

    if (imageSize.width == 0 || imageSize.height == 0)
    {
        return true;
    }

    VkImageUsageFlags imageUsage;
    if (SelectDesiredUsageScenarioOfSwapchainImage(surfaceCapabilities, swapchainImageUsage, imageUsage) == false)
    {
        return false;
    }
    
    VkSurfaceTransformFlagBitsKHR surfaceTransform;
    SelectTransformationOfSwapchainImage(surfaceCapabilities, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR, surfaceTransform);

    VkColorSpaceKHR imageColorSpace;
    if (SelectFormatOfSwapchainImage(physicalDevice, presentationSurface, {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}, imageFormat, imageColorSpace) == false)
    {
        return false;
    }

    if (CreateSwapchain(logicalDevice, presentationSurface, numberOfImages, {imageFormat, imageColorSpace}, imageSize, imageUsage, surfaceTransform, desiredPresentMode, oldSwapchain, swapchain) == false)
    {
        return false;
    }

    if (GetHandleOfSwapchainImage(logicalDevice, swapchain, swapchainImages) == false)
    {
        return false;
    }

    return true;
}

bool Cookbook::SelectDesiredPresentationMode(VkPhysicalDevice physicalDevice,
    VkSurfaceKHR presentationSurface,
    VkPresentModeKHR desiredPresentMode,
    VkPresentModeKHR& presentMode)
{
    uint32_t presentModeCount = 0;
    VkResult result = VK_SUCCESS;

    result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, presentationSurface, &presentModeCount, nullptr);
    if (result != VK_SUCCESS || presentModeCount == 0)
    {
        std::cout << "Could not get the number of supported present mode." << std::endl;
        return false;
    }
    
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, presentationSurface, &presentModeCount, presentModes.data());
    if (result != VK_SUCCESS || presentModeCount == 0)
    {
        std::cout << "Could not enumerate present mode." << std::endl;
        return false;
    }

    for (auto& currentPresentMode : presentModes)
    {
        if (currentPresentMode == desiredPresentMode)
        {
            presentMode = desiredPresentMode;
            return true;
        }
    }

    std::cout << "Desired present mode is not supported. selecting default FIFO mode" << std::endl;
    for (auto& currentPresentMode : presentModes)
    {
        if (currentPresentMode == VK_PRESENT_MODE_FIFO_KHR)
        {
            presentMode = VK_PRESENT_MODE_FIFO_KHR;
            return true;
        }
    }

    std::cout << "VK_PRESENT_MODE_FIFO_KHR is not supported!" << std::endl;
    return false;
}

bool Cookbook::GetCapabilityOfPresentationSurface(VkPhysicalDevice physicalDevice,
    VkSurfaceKHR presentationSurface,
    VkSurfaceCapabilitiesKHR& surfaceCapabilities)
{
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, presentationSurface, &surfaceCapabilities);
    if (result != VK_SUCCESS)
    {
        std::cout << "Could not get the capabilities of a presentation surface." << std::endl;
        return false;
    }

    return true;
}

bool Cookbook::SelectNumberOfSwapchainImage(const VkSurfaceCapabilitiesKHR& surfaceCapabilities, uint32_t& numberOfImages)
{
    numberOfImages = surfaceCapabilities.minImageCount + 1;
    if ((surfaceCapabilities.maxImageCount > 0) && (numberOfImages > surfaceCapabilities.maxImageCount))
    {
        numberOfImages = surfaceCapabilities.maxImageCount;
    }
    
    return true;
}

bool Cookbook::ChooseSizeOfSwapchainImage(const VkSurfaceCapabilitiesKHR& surfaceCapabilities, VkExtent2D& sizeOfImages)
{
    if (surfaceCapabilities.currentExtent.width == 0xFFFFFFFF)
    {
        sizeOfImages = {640, 480};

        if (sizeOfImages.width < surfaceCapabilities.minImageExtent.width)
        {
            sizeOfImages.width = surfaceCapabilities.minImageExtent.width;
        }
        if (sizeOfImages.width > surfaceCapabilities.maxImageExtent.width)
        {
            sizeOfImages.width = surfaceCapabilities.maxImageExtent.width;
        }

        if (sizeOfImages.height < surfaceCapabilities.minImageExtent.height)
        {
            sizeOfImages.height = surfaceCapabilities.minImageExtent.height;
        }
        if (sizeOfImages.height > surfaceCapabilities.maxImageExtent.height)
        {
            sizeOfImages.height = surfaceCapabilities.maxImageExtent.height;
        }
    }
    else
    {
        sizeOfImages = surfaceCapabilities.currentExtent;
    }

    return  true;
}

bool Cookbook::SelectDesiredUsageScenarioOfSwapchainImage(const VkSurfaceCapabilitiesKHR& surfaceCapabilities,
    VkImageUsageFlags desiredUsage,
    VkImageUsageFlags& imageUsage)
{
    imageUsage = desiredUsage & surfaceCapabilities.supportedUsageFlags;
    return desiredUsage == imageUsage;
}

bool Cookbook::SelectTransformationOfSwapchainImage(const VkSurfaceCapabilitiesKHR& surfaceCapabilities,
    VkSurfaceTransformFlagBitsKHR desiredTransform,
    VkSurfaceTransformFlagBitsKHR& surfaceTransform)
{
    if (surfaceCapabilities.supportedTransforms & desiredTransform)
    {
        surfaceTransform = desiredTransform;
    }
    else
    {
        surfaceTransform = surfaceCapabilities.currentTransform;
    }

    return true;
}

bool Cookbook::SelectFormatOfSwapchainImage(VkPhysicalDevice physicalDevice,
    VkSurfaceKHR presentationSurface,
    VkSurfaceFormatKHR desiredSurfaceFormat,
    VkFormat& iamgeFormat,
    VkColorSpaceKHR& imageColorSpace)
{
    uint32_t formatCount = 0;
    VkResult result = VK_SUCCESS;

    result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, presentationSurface, &formatCount, nullptr);
    if (result != VK_SUCCESS || formatCount == 0)
    {   
        std::cout << "Could not get number of supported surface format." << std::endl;
        return false;
    }

    std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, presentationSurface, &formatCount, surfaceFormats.data());
    if (result != VK_SUCCESS || formatCount == 0)
    {
        std::cout << "Could not enumerate supported surface format." << std::endl;
        return false;
    }

    if (surfaceFormats.size() == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
    {
        iamgeFormat = desiredSurfaceFormat.format;
        imageColorSpace = desiredSurfaceFormat.colorSpace;
        return true;
    }

    for (auto& surfaceFormat : surfaceFormats)
    {
        if ((desiredSurfaceFormat.format == surfaceFormat.format)
            && (desiredSurfaceFormat.colorSpace == surfaceFormat.colorSpace))
        {
            iamgeFormat = desiredSurfaceFormat.format;
            imageColorSpace = desiredSurfaceFormat.colorSpace;
            return true;
        }
    }

    for (auto& surfaceFormat : surfaceFormats)
    {
        if (desiredSurfaceFormat.format == surfaceFormat.format)
        {
            iamgeFormat = desiredSurfaceFormat.format;
            imageColorSpace = surfaceFormat.colorSpace;
            std::cout << "Desired combiantion of format and colorspace is not supported. Select other colorspace" << std::endl;
            return true;
        }
    }

    iamgeFormat = surfaceFormats[0].format;
    imageColorSpace = surfaceFormats[0].colorSpace;
    std::cout << "Desired format is not suppored. Select available format" << std::endl;
    return true;
}

bool Cookbook::GetHandleOfSwapchainImage(VkDevice logicalDevice, VkSwapchainKHR swapchain, std::vector<VkImage>& swapchainImages)
{
    uint32_t imageCount = 0;
    VkResult result = VK_SUCCESS;

    result = vkGetSwapchainImagesKHR(logicalDevice, swapchain, &imageCount, nullptr);
    if (result != VK_SUCCESS || imageCount == 0)
    {
        std::cout << "Could not get the number of swapchain image" << std::endl;
        return false;
    }

    swapchainImages.resize(imageCount);
    result = vkGetSwapchainImagesKHR(logicalDevice, swapchain, &imageCount, swapchainImages.data());
    if (result != VK_SUCCESS || imageCount == 0)
    {
        std::cout << "Could not enumerate swapchain image" << std::endl;
        return false;
    }

    return true;
}

bool Cookbook::AccquireSwapchainImage(VkDevice logicalDevice,
    VkSwapchainKHR swapchain,
    VkSemaphore semaphore,
    VkFence fence,
    uint32_t& imageIndex)
{
    VkResult result;
    result = vkAcquireNextImageKHR(logicalDevice, swapchain, 200000000, semaphore, fence, &imageIndex);

    return (result == VK_SUCCESS) || (result == VK_SUBOPTIMAL_KHR);
}

bool Cookbook::PresentImage(VkQueue queue, std::vector<VkSemaphore> renderingSemaphore, std::vector<PresentInfo> imagesToPresent)
{
    VkResult result;
    std::vector<VkSwapchainKHR> swapchains;
    std::vector<uint32_t> imageIndices;

    for (auto& imageToPresent : imagesToPresent)
    {
        swapchains.push_back(imageToPresent.swapchain);
        imageIndices.push_back(imageToPresent.imageIndex);
    }

    VkPresentInfoKHR presentInfo = 
    {
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        nullptr,
        static_cast<uint32_t> (renderingSemaphore.size()),
        renderingSemaphore.data(),
        static_cast<uint32_t> (swapchains.size()),
        swapchains.data(),
        imageIndices.data(),
        nullptr
    };

    result = vkQueuePresentKHR(queue, &presentInfo);
    return (result == VK_SUCCESS);
}

void Cookbook::DestroySwapchain(VkDevice logicalDevice, VkSwapchainKHR& swapchain)
{
    if (swapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(logicalDevice, swapchain, nullptr);
        swapchain = VK_NULL_HANDLE;
    }
}

void Cookbook::DestroyPresentationSurface(VkInstance instance, VkSurfaceKHR& presentationSurface)
{
    if (presentationSurface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(instance, presentationSurface, nullptr);
        presentationSurface = VK_NULL_HANDLE;
    }
}
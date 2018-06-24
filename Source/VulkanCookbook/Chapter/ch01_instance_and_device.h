
#ifndef __CH1_INSTANCE_AND_DEVICE__H__
#define __CH1_INSTANCE_AND_DEVICE__H__

#include "../Common/vulkan_common.h"

#include <vector>

namespace Cookbook
{
    bool ConnectWithVulkanLoaderLibrary(LIBRARY_TYPE& vulaknLibrary);  
    bool LoadFunctionExportedFromVulkanLoaderLibrary(const LIBRARY_TYPE& vulkanLibrary)  ;
    bool LoadGlobalLevelFunctions(void);    
    bool CreateVulkanInstance(const std::vector<const char*>& desiredExtension,
        const char* const applicationName, 
        VkInstance& instance);
    bool CheckAvailableInstanceExtension(std::vector<VkExtensionProperties>& availableExtensions);
    bool IsExtensionSupported(const std::vector<VkExtensionProperties>& availableExtensions, const char* const extensionName);
    bool LoadInstanceLevelFunction(VkInstance instance, const std::vector<const char*>& enabledExtensions);    
    bool EnumerateAvailablePhysicalDevice(VkInstance instance, std::vector<VkPhysicalDevice>& availableDevices);
    bool SelectIndexOfQueueFamilyWithDesiredCapability(VkPhysicalDevice physicalDevice,
        VkQueueFlags desiredCapabilities,
        uint32_t& queueFamilyIndex);
    bool CheckAvailableQueueFamilyAndProperty(VkPhysicalDevice& physicalDevice,
        std::vector<VkQueueFamilyProperties>& queueFamilies);
    bool CreateLogicalDevice(VkPhysicalDevice& physicalDevice,
        std::vector<QueueInfo> queueInfos,
        const std::vector<const char*>& desiredExtensions, 
        VkPhysicalDeviceFeatures* desiredFeatures,
        VkDevice& logicalDevice);
    bool CheckAvailableDeviceExtension(VkPhysicalDevice physicalDevice,
        std::vector<VkExtensionProperties>& availableExtensions);
    bool LoadDeviceLevelFunction(VkDevice logicalDevice, const std::vector<const char*>& enabledExtensions);
    void GetDeviceQueue(VkDevice logicalDevice, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue& queue);
    bool CreateLogicalDeviceWithGeometryShaderAndGraphicAndComputeQueue(VkInstance instance,
        VkDevice& logicalDevice,
        VkQueue& graphicsQueue,
        VkQueue& computeQueue);
    void GetFeatureAndPropertyOfPhysicalDevice(VkPhysicalDevice physicalDevice,
        VkPhysicalDeviceFeatures& deviceFeatures,
        VkPhysicalDeviceProperties& deviceProperties);
    void DestroyLogicalDevice(VkDevice& logicalDevice);
    void DestroyVulkanInstance(VkInstance& instance);
    void ReleaseVulkanLibrary(LIBRARY_TYPE& vulkanLibrary);    
}

#endif /**/

#include "ch01_instance_and_device.h"

#include <iostream>

bool Cookbook::ConnectWithVulkanLoaderLibrary(LIBRARY_TYPE& vulaknLibrary)
{
#if defined(_WIN32)
    vulaknLibrary = LoadLibrary("vulkan-1.dll");
#elif defined(__linux)
    vulaknLibrary = dlopen("libvulkan.so.1", RTLD_NOW)
#endif    
    
    if (vulaknLibrary == nullptr)
    {
        std::cout << "Couldnot connect with a vulkan runtime library." << std::endl;
        return false;
    }

    return true;
}

bool Cookbook::LoadFunctionExportedFromVulkanLoaderLibrary(const LIBRARY_TYPE& vulkanLibrary)  
{
#if defined(_WIN32)
    #define LoadFunction GetProcAddress
#elif defined(__linux)
    #define LoadFunction dlsym
#endif /**/

    #define EXPORT_VULKAN_FUNTION(name)  \
        name = (PFN_##name) LoadFunction(vulkanLibrary, #name); \
        if (name == nullptr)    \
        {   \
            std::cout << "Could not load exported vulkan function name :"   \
                #name << std::endl; \
            return false;   \
        }

    #include "../Common/list_vulkan_functions.inl"

    return true;
}

bool Cookbook::LoadGlobalLevelFunctions(void)
{
    #define GLOBAL_LEVEL_VULKAN_FUNCTION(name)  \
        name = (PFN_##name) vkGetInstanceProcAddr(nullptr, #name);  \
        if (name == nullptr)    \
        {   \
            std::cout << "Could not load global level function name :" << name << std::endl;    \
            return false;   \
        }

    #include "../Common/list_vulkan_functions.inl"

    return true;
}

bool Cookbook::CreateVulkanInstance(const std::vector<const char*>& desiredExtension,
    const char* const applicationName, 
    VkInstance& instance)
{
    std::vector<VkExtensionProperties> availableExtensions;
    if (CheckAvailableInstanceExtension(availableExtensions) == false)
    {
        return false;
    }
    
    for (auto& extension : desiredExtension)
    {
        if (IsExtensionSupported(availableExtensions, extension) == false)
        {
            std::cout << "Extension name : " << extension << " is not supported by instance object!" << std::endl;
            return false;
        }
    }

    VkApplicationInfo applicationInfo = 
    {
        VK_STRUCTURE_TYPE_APPLICATION_INFO,
        nullptr,
        applicationName,
        VK_MAKE_VERSION(1, 0, 0),
        "vulkan cookbook",
        VK_MAKE_VERSION(1, 0, 0),
        VK_MAKE_VERSION(1, 0, 0)
    };

    std::vector<const char*> desiredlayers;
#if defined(DEBUG) || defined(_DEBUG)
    desiredlayers.emplace_back("VK_LAYER_LUNARG_standard_validation");
#endif /**/

    VkInstanceCreateInfo intanceCreateInfo = 
    {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        nullptr, 
        0, 
        &applicationInfo,
        desiredlayers.size(),
        desiredlayers.data(),
        static_cast<size_t> (desiredExtension.size()),
        desiredExtension.data()
    };


    VkResult result = vkCreateInstance(&intanceCreateInfo, nullptr, &instance);
    if (result != VK_SUCCESS || instance == VK_NULL_HANDLE)
    {
        std::cout << "Could not careate vulkan instace." << std::endl;
        return false;
    }

    return true; 
}

bool Cookbook::CheckAvailableInstanceExtension(std::vector<VkExtensionProperties>& availableExtensions)
{
    uint32_t extensionCount = 0;
    VkResult result = VK_SUCCESS;

    result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    if (result != VK_SUCCESS || extensionCount == 0)
    {
        std::cout << "Could not get number of instance extensions." << std::endl;
        return false;
    }

    availableExtensions.resize(extensionCount);
    result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());
    if (result != VK_SUCCESS || extensionCount == 0)
    {
        std::cout << "Could not enumerate instance extensions." << std::endl;
        return false;
    }

    return true;
}

bool Cookbook::IsExtensionSupported(const std::vector<VkExtensionProperties>& availableExtensions, const char* const extensionName)
{
    for (auto& availableExtension : availableExtensions)
    {
        if (strstr(availableExtension.extensionName, extensionName))
        {
            return true;
        }
    }

    return false;
}

bool Cookbook::LoadInstanceLevelFunction(VkInstance instance, const std::vector<const char*>& enabledExtensions)
{
    #define INSTANCE_LEVEL_VULKAN_FUNCTION(name)    \
        name = (PFN_##name) vkGetInstanceProcAddr(instance, #name); \
        if (name == nullptr)    \
        {   \
            std::cout << "Could not load instance-level vulkan function name : " << name  << std::endl; \
            return false;   \
        }

    #define INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(name, extension)  \
        for (auto& enabledExtension : enabledExtensions)    \
        {   \
            if (std::string(enabledExtension) == std::string(extension))    \
            {   \
                name = (PFN_##name) vkGetInstanceProcAddr(instance, #name); \
                if (name == nullptr)    \
                {   \
                    std::cout << "Could not load instance-level vulkan function name : " << name  << std::endl; \
                    return false;   \
                }   \
            }   \
        }

    #include "../Common/list_vulkan_functions.inl"

    return true;
}

bool Cookbook::EnumerateAvailablePhysicalDevice(VkInstance instance, std::vector<VkPhysicalDevice>& availableDevices)
{
    uint32_t deviceCount = 0;
    VkResult result = VK_SUCCESS;

    result = vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (result != VK_SUCCESS || deviceCount == 0)
    {
        std::cout << "Could not get the number of physical device." << std::endl;
        return false;
    }

    availableDevices.resize(deviceCount);
    result = vkEnumeratePhysicalDevices(instance, &deviceCount, availableDevices.data());
    if (result != VK_SUCCESS || deviceCount == 0)
    {
        std::cout << "Could not get physical device." << std::endl;
        return false;
    }

    return true;
}

bool Cookbook::SelectIndexOfQueueFamilyWithDesiredCapability(VkPhysicalDevice physicalDevice,
    VkQueueFlags desiredCapabilities,
    uint32_t& queueFamilyIndex)
{
    std::vector<VkQueueFamilyProperties> queueFamilies;
    if (CheckAvailableQueueFamilyAndProperty(physicalDevice, queueFamilies) == false)
    {
        return false;
    }

    for (uint32_t index = 0; index < static_cast<uint32_t> (queueFamilies.size()); ++index)
    {
        bool checkQueueCount = queueFamilies[index].queueCount > 0;
        bool checkQueueFlags = (queueFamilies[index].queueFlags & desiredCapabilities) == desiredCapabilities;
        if (checkQueueCount == true && checkQueueFlags == true)
        {
            queueFamilyIndex = index;
            return true;
        }
    }

    return false;
}

bool Cookbook::CheckAvailableQueueFamilyAndProperty(VkPhysicalDevice& physicalDevice,
    std::vector<VkQueueFamilyProperties>& queueFamilies)
{
    uint32_t queueFamilyCount = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    if (queueFamilyCount == 0)
    {
        std::cout << "Could not get the number of queue family" << std::endl;
        return false;
    }

    queueFamilies.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());
    if (queueFamilyCount == 0)
    {
        std::cout << "Could not get the properties of queue family" << std::endl;
        return false;
    }

    return true;
}

bool Cookbook::CreateLogicalDevice(VkPhysicalDevice& physicalDevice,
    std::vector<QueueInfo> queueInfos,
    const std::vector<const char*>& desiredExtensions, 
    VkPhysicalDeviceFeatures* desiredFeatures,
    VkDevice& logicalDevice)
{
    std::vector<VkExtensionProperties> availableExtensions;
    if (CheckAvailableDeviceExtension(physicalDevice, availableExtensions) == false)
    {
        return false;
    }

    for (auto& extension : desiredExtensions)
    {
        if (IsExtensionSupported(availableExtensions, extension) == false)
        {
            std::cout << "Extension name : " << extension << " is not supported by instance object!" << std::endl;
            return false;
        }
    }

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    for (auto& info : queueInfos)
    {
        VkDeviceQueueCreateInfo queueCreateInfo =
        {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            nullptr,
            0,
            info.familyIndex,
            static_cast<uint32_t> (info.priorities.size()),
            info.priorities.data()
        };
        queueCreateInfos.push_back(queueCreateInfo);
    }

    std::vector<const char*> desiredlayers;
#if defined(DEBUG) || defined(_DEBUG)
    desiredlayers.emplace_back("VK_LAYER_LUNARG_standard_validation");
#endif /**/

    VkDeviceCreateInfo devcieCreateInfo =
    {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        nullptr,
        0,
        static_cast<uint32_t> (queueCreateInfos.size()),
        queueCreateInfos.data(),
        desiredlayers.size(),
        desiredlayers.data(),
        static_cast<uint32_t> (desiredExtensions.size()),
        desiredExtensions.data(),
        desiredFeatures
    };

    VkResult result = vkCreateDevice(physicalDevice, &devcieCreateInfo, nullptr, &logicalDevice);
    if (result != VK_SUCCESS || logicalDevice == VK_NULL_HANDLE)
    {
        std::cout << "Could not create logical device." << std::endl;
        return false;
    }

    return true;
}

bool Cookbook::CheckAvailableDeviceExtension(VkPhysicalDevice physicalDevice,
    std::vector<VkExtensionProperties>& availableExtensions)
{
    uint32_t extensionCount = 0;
    VkResult result = VK_SUCCESS;

    result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
    if (result != VK_SUCCESS || extensionCount == 0)
    {
        std::cout << "Could not get the number of device extension." << std::endl;
        return false;
    }

    availableExtensions.resize(extensionCount);
    result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());
    if (result != VK_SUCCESS || extensionCount == 0)
    {
        std::cout << "Could not get device extension." << std::endl;
        return false;
    }

    return true;
}

bool Cookbook::LoadDeviceLevelFunction(VkDevice logicalDevice, const std::vector<const char*>& enabledExtensions)
{
    #define DEVICE_LEVEL_VULKAN_FUNCTION(name)  \
        name = (PFN_##name) vkGetDeviceProcAddr(logicalDevice, #name);  \
        if (name == nullptr)    \
        {   \
            std::cout << "Could not load device-level vulkan function : " << #name << std::endl;    \
            return false;   \
        }

    #define DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(name, extension)    \
        for (auto& enabledExtension : enabledExtensions)    \
        {   \
            if (std::string(enabledExtension) == std::string(extension))    \
            {   \
                name = (PFN_##name) vkGetDeviceProcAddr(logicalDevice, #name);  \
                if (name == nullptr)    \
                {   \
                    std::cout << "Could not load device-level vulkan function : " << #name << std::endl;    \
                    return false;   \
                }   \
            }   \
        }

    #include "../Common/list_vulkan_functions.inl"

    return true;
}

void Cookbook::GetDeviceQueue(VkDevice logicalDevice, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue& queue)
{
    vkGetDeviceQueue(logicalDevice, queueFamilyIndex, queueIndex, &queue);
}

bool Cookbook::CreateLogicalDeviceWithGeometryShaderAndGraphicAndComputeQueue(VkInstance instance,
    VkDevice& logicalDevice,
    VkQueue& graphicsQueue,
    VkQueue& computeQueue)
{
    std::vector<VkPhysicalDevice> physicalDevices;
    EnumerateAvailablePhysicalDevice(instance, physicalDevices);

    for (auto& physicalDevice : physicalDevices)
    {
        VkPhysicalDeviceFeatures deviceFeatures;
        VkPhysicalDeviceProperties deviceProperties;
        GetFeatureAndPropertyOfPhysicalDevice(physicalDevice, deviceFeatures, deviceProperties);

        if (deviceFeatures.geometryShader == true)
        {
            deviceFeatures = {};
            deviceFeatures.geometryShader = VK_TRUE;
        }
        else
        {
            continue;
        }

        uint32_t graphicsQueueFamilyIndex;
        if (SelectIndexOfQueueFamilyWithDesiredCapability(physicalDevice, VK_QUEUE_GRAPHICS_BIT, graphicsQueueFamilyIndex) == false)
        {
            continue;
        }

        uint32_t computeQueueFamilyIndex;
        if (SelectIndexOfQueueFamilyWithDesiredCapability(physicalDevice, VK_QUEUE_COMPUTE_BIT, computeQueueFamilyIndex) == false)
        {
            continue;
        }

        std::vector<QueueInfo> requestedQueues = {{graphicsQueueFamilyIndex, {1.0f}}};
        if (graphicsQueueFamilyIndex != computeQueueFamilyIndex)
        {
            requestedQueues.push_back({computeQueueFamilyIndex, {1.0f}});
        }

        if (CreateLogicalDevice(physicalDevice, requestedQueues, {}, &deviceFeatures, logicalDevice) == false)
        {
            continue;
        }
        else
        {
            if (LoadDeviceLevelFunction(logicalDevice, {}) == false)
            {
                return false;
            }

            GetDeviceQueue(logicalDevice, graphicsQueueFamilyIndex, 0, graphicsQueue);
            GetDeviceQueue(logicalDevice, computeQueueFamilyIndex, 0, computeQueue);
            return true;
        }
    }

    return false;
}

void Cookbook::GetFeatureAndPropertyOfPhysicalDevice(VkPhysicalDevice physicalDevice,
    VkPhysicalDeviceFeatures& deviceFeatures,
    VkPhysicalDeviceProperties& deviceProperties)
{
    vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
}

void Cookbook::DestroyLogicalDevice(VkDevice& logicalDevice)
{
    if (logicalDevice != VK_NULL_HANDLE)
    {
        vkDestroyDevice(logicalDevice, nullptr);
        logicalDevice = VK_NULL_HANDLE;
    }
}

void Cookbook::DestroyVulkanInstance(VkInstance& instance)
{
    if (instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(instance, nullptr);
        instance = VK_NULL_HANDLE;
    }
}

void Cookbook::ReleaseVulkanLibrary(LIBRARY_TYPE& vulkanLibrary)
{
    if (vulkanLibrary != nullptr)
    {
#if defined(_WIN32)
        FreeLibrary(vulkanLibrary);
#elif defined(__linux)
        dlclose(vulkanLibrary);
#endif /**/
        vulkanLibrary = nullptr;
    }
}
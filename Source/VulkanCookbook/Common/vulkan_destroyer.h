
#ifndef __VULKAN_DESTROYER__H__
#define __VULKAN_DESTROYER__H__

#include "vulkan_functions.h"

#include <functional>

//----------------------------------------------------------------
// wrapper
//----------------------------------------------------------------
struct VkInstanceWrapper
{
    VkInstance handle;
};

struct VkDeviceWrapper
{
    VkDevice handle;
};

struct VkSurfaceKHRWrapper
{
    VkSurfaceKHR handle;
};

template <typename _VkType>
void DestroyVulkanObject(_VkType object);

template <>
inline void DestroyVulkanObject<VkInstanceWrapper>(VkInstanceWrapper object)
{
    vkDestroyInstance(object.handle, nullptr);
}

template <>
inline void DestroyVulkanObject<VkDeviceWrapper>(VkDeviceWrapper object)
{
    vkDestroyDevice(object.handle, nullptr);
}

template <typename _VkParent, class _VkChild>
void DestroyVulkanObject(_VkParent parent, _VkChild object);

template <>
inline void DestroyVulkanObject<VkInstance, VkSurfaceKHRWrapper>(VkInstance instance, VkSurfaceKHRWrapper surface)
{
    vkDestroySurfaceKHR(instance, surface.handle, nullptr);
}

#define VK_DESTOYER_SPECIALIZATION(vkChild, vkDeleter)  \
    struct vkChild##Wrapper \
    {   \
        vkChild handle; \
    };  \
        \
    template <> \
    inline void DestroyVulkanObject<VkDevice, vkChild##Wrapper>(VkDevice device, vkChild##Wrapper object)   \
    {   \
        vkDeleter(device, object.handle, nullptr);  \
    }

VK_DESTOYER_SPECIALIZATION(VkSemaphore, vkDestroySemaphore);
//VK_DESTOYER_SPECIALIZATION(VkCommandBuffer, vkFreeCommandBuffers);
VK_DESTOYER_SPECIALIZATION(VkFence, vkDestroyFence);
VK_DESTOYER_SPECIALIZATION(VkDeviceMemory, vkFreeMemory);
VK_DESTOYER_SPECIALIZATION(VkBuffer, vkDestroyBuffer);
VK_DESTOYER_SPECIALIZATION(VkImage, vkDestroyImage);
VK_DESTOYER_SPECIALIZATION(VkEvent, vkDestroyEvent);
VK_DESTOYER_SPECIALIZATION(VkQueryPool, vkDestroyQueryPool);
VK_DESTOYER_SPECIALIZATION(VkBufferView, vkDestroyBufferView);
VK_DESTOYER_SPECIALIZATION(VkImageView, vkDestroyImageView);
VK_DESTOYER_SPECIALIZATION(VkShaderModule, vkDestroyShaderModule);
VK_DESTOYER_SPECIALIZATION(VkPipelineCache, vkDestroyPipelineCache);
VK_DESTOYER_SPECIALIZATION(VkPipelineLayout, vkDestroyPipelineLayout);
VK_DESTOYER_SPECIALIZATION(VkPipeline, vkDestroyPipeline);
VK_DESTOYER_SPECIALIZATION(VkDescriptorSetLayout, vkDestroyDescriptorSetLayout);
VK_DESTOYER_SPECIALIZATION(VkSampler, vkDestroySampler);
VK_DESTOYER_SPECIALIZATION(VkDescriptorPool, vkDestroyDescriptorPool);
//VK_DESTOYER_SPECIALIZATION(VkDescriptorSet, vkFreeDescriptorSets);
VK_DESTOYER_SPECIALIZATION(VkFramebuffer, vkDestroyFramebuffer);
VK_DESTOYER_SPECIALIZATION(VkCommandPool, vkDestroyCommandPool);
VK_DESTOYER_SPECIALIZATION(VkSwapchainKHR, vkDestroySwapchainKHR);

//----------------------------------------------------------------
// VkDestroyer
//----------------------------------------------------------------
template <typename _VkTypeWrapper>
class VkDestroyer
{
public:
    VkDestroyer(void)
        : destroyerFunc_(nullptr)
    {
        object_.handle = VK_NULL_HANDLE;
    }

    VkDestroyer(std::function<void (_VkTypeWrapper)> destroyerFunc)
        : destroyerFunc_(destroyerFunc)
    {
        object_.handle = VK_NULL_HANDLE;
    }

    VkDestroyer(_VkTypeWrapper object, std::function<void (_VkTypeWrapper)> destroyerFunc)
        : destroyerFunc_(destroyerFunc)
    {
        object_.handle = object_.handle;
    }

    VkDestroyer(VkDestroyer<_VkTypeWrapper>&& other)
        : destroyerFunc_(other.destroyerFunc_)
    {
        object_.handle = other.object_.handle;
        other.object_.handle = VK_NULL_HANDLE;
        other.destroyerFunc_ = nullptr;
    }

    ~VkDestroyer(void)
    {
        if (destroyerFunc_ && object_.handle)
        {
            destroyerFunc_(object_);
        }
    }

    VkDestroyer& operator=(VkDestroyer<_VkTypeWrapper>&& other)
    {
        if (this != &other)
        {
            _VkTypeWrapper object = object_;
            std::function<void (_VkTypeWrapper)> destroyerFunc = destroyerFunc_;

            object_.handle = other.object_.handle;
            destroyerFunc_ = other.destroyerFunc_;

            other.object_.handle = object.handle;
            other.destroyerFunc_ = destroyerFunc;
        }

        return *this;
    }

    decltype(_VkTypeWrapper::handle)& operator*(void)
    {
        return object_.handle;
    }

    const decltype(_VkTypeWrapper::handle)& operator*(void) const
    {
        return object_.handle;
    }

    bool operator!(void) const
    {
        return object_.handle == VK_NULL_HANDLE;
    }

    operator bool(void) const
    {
        return object_.handle != VK_NULL_HANDLE;
    }
    
    VkDestroyer(const VkDestroyer<_VkTypeWrapper>& other) = delete;
    VkDestroyer& operator=(const VkDestroyer<_VkTypeWrapper>& other) = delete;

private:
    _VkTypeWrapper object_;
    std::function<void (_VkTypeWrapper)> destroyerFunc_;
};

#define VkDestroyer(vkType) VkDestroyer<vkType##Wrapper>

/* helper function */
inline void InitVkDestroyer(VkDestroyer<VkInstanceWrapper>& destroyer)
{
    destroyer = VkDestroyer<VkInstanceWrapper>(std::bind(DestroyVulkanObject<VkInstanceWrapper>, std::placeholders::_1));
}

inline void InitVkDestroyer(VkDestroyer<VkDeviceWrapper>& destroyer)
{
    destroyer = VkDestroyer<VkDeviceWrapper>(std::bind(DestroyVulkanObject<VkDeviceWrapper>, std::placeholders::_1));
}

template <typename _VkParent, typename _VkType>
inline void InitVkDestroyer(const _VkParent& parent, VkDestroyer<_VkType>& destroyer)
{
    destroyer = VkDestroyer<_VkType>(std::bind(DestroyVulkanObject<_VkParent, _VkType>, parent, std::placeholders::_1));
}

template <typename _VkParent, typename _VkType>
inline void InitVkDestroyer(const VkDestroyer<_VkParent>& parent, VkDestroyer<_VkType>& destroyer)
{
    destroyer = VkDestroyer<_VkType>(std::bind(DestroyVulkanObject<decltype(_VkParent::handle), _VkType>, *parent, std::placeholders::_1));
}


#endif /*__VULKAN_DESTROYER__H__*/
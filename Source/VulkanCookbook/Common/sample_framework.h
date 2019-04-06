
#ifndef __VULKAN_SAMPLE_FRAMEWORK__H__
#define __VULKAN_SAMPLE_FRAMEWORK__H__

#include "vulkan_os.h"

#include <chrono>
#include <vector>

//----------------------------------------------------------------
// Data
//----------------------------------------------------------------
struct ButtonsState
{
    bool isPressed = false;
    bool wasClicked = false;
    bool wasReleased = false;
};

struct Detla
{
    int x = 0;
    int y = 0;
};

struct Position
{
    int x = 0;
    int y = 0;
    Detla delta;
};

struct WheelState
{
    bool wasMoved = false;
    float distance = 0.0f;
};

struct MouseStateParameters
{
    ButtonsState buttons[2];
    Position position;
    WheelState wheel;
};

/* TimerStateParameters */
class TimerStateParameters
{
public:
    TimerStateParameters(void);
    ~TimerStateParameters(void) = default;

    float GetTime(void) const;
    float GetDeltaTime(void) const;

    void Update(void);

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> _time;
    std::chrono::duration<float> _deltaTime;
};

//----------------------------------------------------------------
// VulkanSampleBase
//----------------------------------------------------------------
class VulkanSampleBase
{
public:
    VulkanSampleBase(void);
    virtual ~VulkanSampleBase(void);

    virtual bool Initialize(WindowParameters parameters) = 0;
    virtual bool Draw(void) = 0;
    virtual bool Resize(void) = 0;
    virtual void Deinitialize(void) = 0;
    virtual void MouseClick(size_t buttonIndex, bool state) final;
    virtual void MouseMove(int x, int y) final;
    virtual void MouseWheel(float distance) final;
    virtual void MouseReset(void) final;
    virtual void UpdateTime(void) final;
    virtual bool IsReady(void) final;

protected:
    virtual void OnMouseEvent(void);

protected:
    LIBRARY_TYPE _vulkanLibrary;
    bool _ready;
    MouseStateParameters _mouseState;
    TimerStateParameters _timerState;
};

class VulkanSample : public VulkanSampleBase
{
public:
    static const uint32_t FrameCount = 3;
    static const VkFormat DepthFormat = VK_FORMAT_D16_UNORM;

public:
    VulkanSample(void);
    virtual ~VulkanSample(void);

    virtual bool InitializeVulkan(WindowParameters params, 
        VkPhysicalDeviceFeatures* desiredDeviceFeatures = nullptr,
        VkImageUsageFlags swapcahinImageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        bool useDepth = true,
        VkImageUsageFlags depthAttachmentUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) final;

    virtual bool CreateSwapchain(VkImageUsageFlags swapchainImageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        bool useDepth = true, 
        VkImageCreateFlags depthAttachmentUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) final;

    virtual void Deinitialize(void) final;

protected:
    VkDestroyer(VkInstance) _instance;
    VkPhysicalDevice _physicalDevice;
    VkDestroyer(VkDevice) _logicalDevice;
    VkDestroyer(VkSurfaceKHR) _presentationSurface;

    QueueParameters _graphicsQueue;
    QueueParameters _computeQueue;
    QueueParameters _presentQueue;
    SwapchainParamters _swapchain;

    VkDestroyer(VkCommandPool) _commandPool;
    std::vector<VkDestroyer(VkImage)> _depthImages;
    std::vector<VkDestroyer(VkDeviceMemory)> _depthImagesMemory;
    std::vector<FrameResources> _frameResources;
};

//----------------------------------------------------------------
#define VULKAN_SAMPLE_FRAMEWORK(title, x, y, width, height, sample_type)    \
    int main(int argc, char** argv) \
    {   \
        argc;   \
        argv;   \
        sample_type sample; \
        WindowFramework window("vulkan "#title, x, y, width, height, sample);   \
            \
        window.Render();    \
            \
        return 0;   \
    }

#endif /*__VULKAN_SAMPLE_FRAMEWORK__H__*/
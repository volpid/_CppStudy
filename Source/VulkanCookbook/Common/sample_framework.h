
#ifndef __VULKAN_SAMPLE_FRAMEWORK__H__
#define __VULKAN_SAMPLE_FRAMEWORK__H__

#include "os.h"

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
    std::chrono::time_point<std::chrono::high_resolution_clock> time_;
    std::chrono::duration<float> deltaTime_;
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
    LIBRARY_TYPE vulkanLibrary_;
    bool ready_;
    MouseStateParameters mouseState_;
    TimerStateParameters timerState_;
};

class VulkanSample : public VulkanSampleBase
{
public:
    static const uint32_t frameCount = 3;
    static const VkFormat depthFormat = VK_FORMAT_D16_UNORM;

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
    VkDestroyer(VkInstance) instance_;
    VkPhysicalDevice physicalDevice_;
    VkDestroyer(VkDevice) logicalDevice_;
    VkDestroyer(VkSurfaceKHR) presentationSurface_;
    QueueParameters graphicsQueue_;
    QueueParameters computeQueue_;
    QueueParameters presentQueue_;
    SwapchainParamters swapchain_;
    VkDestroyer(VkCommandPool) commandPool_;
    std::vector<VkDestroyer(VkImage)> depthImages;
    std::vector<VkDestroyer(VkDeviceMemory)> depthImagesMemory;
    std::vector<FrameResources> frameResources_;
};

//----------------------------------------------------------------
#define VULKAN_SAMPLE_FRAMEWORK(title, x, y, width, height, sample_type)    \
    int main(int argc, char** argv) \
    {   \
        sample_type sample; \
        WindowFramework window("vulkan "#title, x, y, width, height, sample);   \
            \
        window.Render();    \
            \
        return 0;   \
    }

#endif /*__VULKAN_SAMPLE_FRAMEWORK__H__*/
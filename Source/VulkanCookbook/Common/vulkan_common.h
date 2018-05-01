
#ifndef __VULKAN_COMMON__H__
#define __VULKAN_COMMON__H__

#include "vulkan_destroyer.h"
#include "vulkan_functions.h"

#include <vector>

#if defined(_WIN32)
    #define LIBRARY_TYPE HMODULE
#elif defined(__linux)
    #define LIBRARY_TYPE void*
#endif /**/

//----------------------------------------------------------------
// vulkan common data structure
//----------------------------------------------------------------
struct WindowParameters
{
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    HINSTANCE hIntance;
    HWND hWnd;    
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
    Display* pDisplay;
    Window window;
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    xcb_connection_t* connection;
    xcb_window_t window;
#endif /**/
};

//----------------------------------------------------------------
struct FrameResources
{
    VkCommandBuffer commandBuffer;
    VkDestroyer(VkSemaphore) imageAcquiredSemaphore;
    VkDestroyer(VkSemaphore) readyToPresentSemaphore;
    VkDestroyer(VkFence) drawingFinishFence;
    VkDestroyer(VkImageView) depthAttachment;
    VkDestroyer(VkFramebuffer) frameBuffer;

    FrameResources(VkCommandBuffer& cmdBuffer,
        VkDestroyer(VkSemaphore)& imgAcquiredSemaphore,
        VkDestroyer(VkSemaphore)& readyPresentSemaphore,
        VkDestroyer(VkFence)& finishFence,
        VkDestroyer(VkImageView)& depthAttach,
        VkDestroyer(VkFramebuffer)& framebuffer)
        : commandBuffer(cmdBuffer),
        imageAcquiredSemaphore(std::move(imgAcquiredSemaphore)),
        readyToPresentSemaphore(std::move(readyPresentSemaphore)),
        drawingFinishFence(std::move(finishFence)),
        depthAttachment(std::move(depthAttach)),
        frameBuffer(std::move(framebuffer))
    {
    };  

    FrameResources(FrameResources&& other)
    {
        *this = std::move(other);
    }

    FrameResources& operator=(FrameResources&& other)
    {
        if (this != &other)
        {
            VkCommandBuffer cmdBuffer = commandBuffer;
                        
            commandBuffer = other.commandBuffer;            
            other.commandBuffer = cmdBuffer;
            imageAcquiredSemaphore = std::move(other.imageAcquiredSemaphore);
            readyToPresentSemaphore = std::move(other.readyToPresentSemaphore);
            drawingFinishFence = std::move(other.drawingFinishFence);
            depthAttachment = std::move(other.depthAttachment);
            frameBuffer = std::move(other.frameBuffer);
        }

        return *this;
    }

    FrameResources(const FrameResources& other) = delete;
    FrameResources& operator=(const FrameResources& other) = delete;
};

//----------------------------------------------------------------
struct QueueParameters
{
    VkQueue handle;
    uint32_t familyIndex;
};

//----------------------------------------------------------------
struct SwapchainParamters
{
    VkDestroyer(VkSwapchainKHR) handle;
    VkFormat format;
    VkExtent2D size;
    std::vector<VkImage> images;
    std::vector<VkDestroyer(VkImageView)> imageViews;
    std::vector<VkImageView> imageViewsRaw;
};

//----------------------------------------------------------------
struct ImageTransition
{
    VkImage image;
    VkAccessFlags currentAccess;
    VkAccessFlags newAccess;
    VkImageLayout currentLayout;
    VkImageLayout newLayout;
    uint32_t currentQueueFamily;
    uint32_t newQueueFamily;
    VkImageAspectFlags aspect;
};

//----------------------------------------------------------------
struct WaitSemaphoreInfo
{
    VkSemaphore semaphore;
    VkPipelineStageFlags waitingStage;
};

//----------------------------------------------------------------
struct PresentInfo
{
    VkSwapchainKHR swapchain;
    uint32_t imageIndex;
};

#endif /*__VULKAN_COMMON__H__*/
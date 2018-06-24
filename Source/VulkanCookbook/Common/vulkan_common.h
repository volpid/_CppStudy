
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
struct QueueInfo
{
    uint32_t familyIndex;
    std::vector<float> priorities;
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

//----------------------------------------------------------------
struct SubpassParameter
{
    VkPipelineBindPoint pipelineType;
    std::vector<VkAttachmentReference> inputAttachments;
    std::vector<VkAttachmentReference> colorAttachments;
    std::vector<VkAttachmentReference> resolveAttachments;
    const VkAttachmentReference* depthStencilAttachment;
    std::vector<uint32_t> preserveAttachments;
};

//----------------------------------------------------------------
struct ShaderStageParameter
{
    VkShaderStageFlagBits shaderStage;
    VkShaderModule shaderModule;
    const char* entryPointName;
    const VkSpecializationInfo* specializationInfo;
};

//----------------------------------------------------------------
struct ViewportInfo
{
    std::vector<VkViewport> viewports;
    std::vector<VkRect2D> scissors;
};

//----------------------------------------------------------------
struct BufferTransition
{
    VkBuffer buffer;
    VkAccessFlags currentAccess;
    VkAccessFlags newAccess;
    uint32_t currentQueueFamily;
    uint32_t newQueueFamily;
};

//----------------------------------------------------------------
struct VertexBufferParameter
{
    VkBuffer buffer;
    VkDeviceSize memoryOffset;
};

#endif /*__VULKAN_COMMON__H__*/
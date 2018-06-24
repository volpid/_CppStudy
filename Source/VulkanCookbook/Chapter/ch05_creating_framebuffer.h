
#ifndef __CH05_CREATING_FRAMEBUFFER__H_
#define __CH05_CREATING_FRAMEBUFFER__H_

#include "../Common/vulkan_common.h"

namespace Cookbook
{
    bool CreateFramebuffer(VkDevice logicalDevice,
        VkRenderPass renderpass,
        const std::vector<VkImageView>& attachments,
        uint32_t width,
        uint32_t height,
        uint32_t layers,
        VkFramebuffer& framebuffer);
}

#endif /*__CH05_CREATING_FRAMEBUFFER__H_*/




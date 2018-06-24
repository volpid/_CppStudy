
#include "ch05_creating_framebuffer.h"

#include <iostream>

bool Cookbook::CreateFramebuffer(VkDevice logicalDevice,
    VkRenderPass renderpass,
    const std::vector<VkImageView>& attachments,
    uint32_t width,
    uint32_t height,
    uint32_t layers,
    VkFramebuffer& framebuffer)
{
    VkFramebufferCreateInfo framebufferCreateInfo = 
    {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        nullptr,
        0,
        renderpass,
        static_cast<uint32_t> (attachments.size()),
        attachments.data(),
        width,
        height,
        layers
    };

    VkResult result = vkCreateFramebuffer(logicalDevice, &framebufferCreateInfo, nullptr, &framebuffer);
    if (result != VK_SUCCESS)
    {
        std::cout << "Could not create a framebuffer." << std::endl;
        return false;
    }

    return true;
}
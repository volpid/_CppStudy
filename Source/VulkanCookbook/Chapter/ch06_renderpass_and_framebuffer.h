
#ifndef __CH06_RENDERPASS_AND_FRAMEBUFFER__H__
#define __CH06_RENDERPASS_AND_FRAMEBUFFER__H__

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
    bool CreateRenderpass(VkDevice logicalDevice,
        const std::vector<VkAttachmentDescription>& attachmentDescriptions,
        const std::vector<SubpassParameter>& subpassParameters,
        const std::vector<VkSubpassDependency>& subpassDependencies,
        VkRenderPass& renderpass);

    void SpecifyAttachmentDescription(const std::vector<VkAttachmentDescription>& attachmentDescriptions);
    void SpecifySubpassDescription(const std::vector<SubpassParameter>& subpassParameters, 
        std::vector<VkSubpassDescription>& subpassDescriptions);
    void SpecifyDependencyBetweenSubpass(const std::vector<VkSubpassDependency>& subpassDependencies);

    void BeginRenderPass(VkCommandBuffer commandBuffer,
        VkRenderPass renderpass,
        VkFramebuffer framebuffer,
        VkRect2D renderArea,
        const std::vector<VkClearValue>& clearValues,
        VkSubpassContents subpassContents);
    void EndRenderPass(VkCommandBuffer commandBuffer);

    void ProgressToTheNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents subpassContents);
}

#endif /*__CH06_RENDERPASS_AND_FRAMEBUFFER__H__*/
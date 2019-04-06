
#include "ch06_renderpass_and_framebuffer.h"

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

bool Cookbook::CreateRenderpass(VkDevice logicalDevice,
    const std::vector<VkAttachmentDescription>& attachmentDescriptions,
    const std::vector<SubpassParameter>& subpassParameters,
    const std::vector<VkSubpassDependency>& subpassDependencies,
    VkRenderPass& renderpass)
{
    SpecifyAttachmentDescription(attachmentDescriptions);

    std::vector<VkSubpassDescription> subpassDescriptions;
    SpecifySubpassDescription(subpassParameters, subpassDescriptions);
    
    SpecifyDependencyBetweenSubpass(subpassDependencies);

    VkRenderPassCreateInfo renderpassCreateInfo = 
    {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        nullptr,
        0,
        static_cast<uint32_t> (attachmentDescriptions.size()),
        attachmentDescriptions.data(),
        static_cast<uint32_t> (subpassDescriptions.size()),
        subpassDescriptions.data(),
        static_cast<uint32_t> (subpassDependencies.size()),
        subpassDependencies.data()
    };

    VkResult result = vkCreateRenderPass(logicalDevice, &renderpassCreateInfo, nullptr, &renderpass);
    if (result != VK_SUCCESS)
    {
        std::cout << "Could not create a render pass." << std::endl;
        return false;
    }

    return true;
}

void Cookbook::SpecifyAttachmentDescription(const std::vector<VkAttachmentDescription>& attachmentDescriptions)
{
    attachmentDescriptions;
    //typedef struct VkAttachmentDescription {
    //    VkAttachmentDescriptionFlags    flags;
    //    VkFormat                        format;
    //    VkSampleCountFlagBits           samples;
    //    VkAttachmentLoadOp              loadOp;
    //    VkAttachmentStoreOp             storeOp;
    //    VkAttachmentLoadOp              stencilLoadOp;
    //    VkAttachmentStoreOp             stencilStoreOp;
    //    VkImageLayout                   initialLayout;
    //    VkImageLayout                   finalLayout;
    //} VkAttachmentDescription;    
}

void Cookbook::SpecifySubpassDescription(const std::vector<SubpassParameter>& subpassParameters, 
    std::vector<VkSubpassDescription>& subpassDescriptions)
{
    subpassDescriptions.clear();

    for (auto& subpassDescription : subpassParameters)
    {
        subpassDescriptions.push_back(
        {
            0,
            subpassDescription.pipelineType,
            static_cast<uint32_t> (subpassDescription.inputAttachments.size()),
            subpassDescription.inputAttachments.data(),
            static_cast<uint32_t> (subpassDescription.colorAttachments.size()),
            subpassDescription.colorAttachments.data(),
            subpassDescription.resolveAttachments.data(),
            subpassDescription.depthStencilAttachment,
            static_cast<uint32_t> (subpassDescription.preserveAttachments.size()),
            subpassDescription.preserveAttachments.data()
        });
    }
}

void Cookbook::SpecifyDependencyBetweenSubpass(const std::vector<VkSubpassDependency>& subpassDependencies)
{
    subpassDependencies;
    //typedef struct VkSubpassDependency {
    //    uint32_t                srcSubpass;
    //    uint32_t                dstSubpass;
    //    VkPipelineStageFlags    srcStageMask;
    //    VkPipelineStageFlags    dstStageMask;
    //    VkAccessFlags           srcAccessMask;
    //    VkAccessFlags           dstAccessMask;
    //    VkDependencyFlags       dependencyFlags;
    //} VkSubpassDependency;
}
void Cookbook::BeginRenderPass(VkCommandBuffer commandBuffer,
    VkRenderPass renderpass,
    VkFramebuffer framebuffer,
    VkRect2D renderArea,
    const std::vector<VkClearValue>& clearValues,
    VkSubpassContents subpassContents)
{
    VkRenderPassBeginInfo renderpassBeginInfo = 
    {
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        nullptr,
        renderpass,
        framebuffer,
        renderArea,
        static_cast<uint32_t> (clearValues.size()),
        clearValues.data()
    };

    vkCmdBeginRenderPass(commandBuffer, &renderpassBeginInfo, subpassContents);
}

void Cookbook::EndRenderPass(VkCommandBuffer commandBuffer)
{
    vkCmdEndRenderPass(commandBuffer);
}

void Cookbook::ProgressToTheNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents subpassContents)
{
    vkCmdNextSubpass(commandBuffer, subpassContents);
}

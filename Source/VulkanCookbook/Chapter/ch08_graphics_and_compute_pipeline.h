
#ifndef __GRAPHICS_AND_COMPUTE_PIPELINE__H__
#define __GRAPHICS_AND_COMPUTE_PIPELINE__H__

#include "../Common/vulkan_common.h"

namespace Cookbook
{
    bool CreateShader(VkDevice logicalDevice, const std::vector<unsigned char>& sourcecode, VkShaderModule& shaderModule);
    void SpecifyPipelineShaderStage(const std::vector<ShaderStageParameter>& shaderStageParams,
        std::vector<VkPipelineShaderStageCreateInfo>& shaderStageCreateInfos);
    void SpecifyPipelineVertexInputState(const std::vector<VkVertexInputBindingDescription>& bindingDescriptions,
        const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions,
        VkPipelineVertexInputStateCreateInfo& vertexInputStateCreateInfo);
    void SpecifyPipelineInputAssemblyState(VkPrimitiveTopology topology,
        bool primitiveRestartEnable,
        VkPipelineInputAssemblyStateCreateInfo& inputAssemblyStateCreateInfo);
    void SpecifyPipelineViewportAndScissorTestState(const ViewportInfo& viewportInfo,
        VkPipelineViewportStateCreateInfo& viewportStateCreateInfo);
    void SpecifyPipelineRasterizationState(bool depthClampEnable,
        bool rasterizerDiscardEnable,
        VkPolygonMode polygonMode,
        VkCullModeFlags cullmode,
        VkFrontFace frontface,
        bool depthBiasEnable,
        float depthBiasConstantFactor,
        float depthBiasClamp,
        float depthBiasSlopeFactor,
        float lineWidth,
        VkPipelineRasterizationStateCreateInfo& rasterizationStateCreateInfo);
    void SpecifyPipelineMultisampleState(VkSampleCountFlagBits sampleCount,
        bool perSampleShadingEnable,
        float minSampleShading,
        const VkSampleMask* sampleMas,
        bool alphaToCoverageEnable,
        bool alphaToOneEnable,
        VkPipelineMultisampleStateCreateInfo& multisampleStateCreateInfo);
    void SpecifyPipelineBlendState(bool logicalOpEnable,
        VkLogicOp logicOp,
        const std::vector<VkPipelineColorBlendAttachmentState>& attachmentBlendStates,
        const std::array<float, 4>& blendConstraint,
        VkPipelineColorBlendStateCreateInfo& blendStateCreateInfo);
    void SpecifyPipelineDynamicState(const std::vector<VkDynamicState>& dynamicStates,
        VkPipelineDynamicStateCreateInfo& dynamicStateCreateInfo);
    bool CreatePipelineLayout(VkDevice logicalDevice, 
        const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
        const std::vector<VkPushConstantRange>& pushConstantRanges,
        VkPipelineLayout& pipelineLayout);
    void SpecifyGraphicsPipelineCreateParameter(VkPipelineCreateFlags additionalOptions,
        const std::vector<VkPipelineShaderStageCreateInfo>& shaderStageCreateInfos,
        const VkPipelineVertexInputStateCreateInfo& vertexInputStateCreateInfo,
        const VkPipelineInputAssemblyStateCreateInfo& inputAssemblyStateCreateInfo,
        const VkPipelineTessellationStateCreateInfo* tessellationStateCreateInfo,
        const VkPipelineViewportStateCreateInfo* viewportStateCreateInfo,
        const VkPipelineRasterizationStateCreateInfo& rasterizationStateCreateInfo,
        const VkPipelineMultisampleStateCreateInfo* multisampleStateCreateInfo,
        const VkPipelineDepthStencilStateCreateInfo* depthAndStencilStateCreateInfo,
        const VkPipelineColorBlendStateCreateInfo* blendStateCreateInfo,
        const VkPipelineDynamicStateCreateInfo* dynamicStateCreateInfo,
        VkPipelineLayout pipelineLayout,
        VkRenderPass renderpass,
        uint32_t subpass,
        VkPipeline basePipelineHandle,
        int32_t basePipelineIndex,
        VkGraphicsPipelineCreateInfo& graphicsPipelineCreateInfo);
    bool CreateGraphicsPipeline(VkDevice logicalDevice,
        const std::vector<VkGraphicsPipelineCreateInfo>& graphicsPipelineCreateInfos,
        VkPipelineCache pipelineCache,
        std::vector<VkPipeline>& graphicsPipelines);
    void BindPipelineObject(VkCommandBuffer commandBuffer,
        VkPipelineBindPoint pipelineType,
        VkPipeline pipeline);
}

#endif /*__GRAPHICS_AND_COMPUTE_PIPELINE__H__*/

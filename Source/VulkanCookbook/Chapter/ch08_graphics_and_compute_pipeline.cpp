
#include "ch08_graphics_and_compute_pipeline.h"

#include <array>
#include <iostream>

bool Cookbook::CreateShader(VkDevice logicalDevice, const std::vector<unsigned char>& sourcecode, VkShaderModule& shaderModule)
{
    VkShaderModuleCreateInfo shaderMoudleCreateInfo =
    {
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        nullptr,
        0, 
        sourcecode.size(),
        reinterpret_cast<const uint32_t*> (sourcecode.data())
    };

    VkResult result = vkCreateShaderModule(logicalDevice, &shaderMoudleCreateInfo, nullptr, &shaderModule);
    if (result != VK_SUCCESS)
    {
        std::cout << "Could not a shader module." << std::endl;
        return false;
    }

    return true;
}

void Cookbook::SpecifyPipelineShaderStage(const std::vector<ShaderStageParameter>& shaderStageParams,
    std::vector<VkPipelineShaderStageCreateInfo>& shaderStageCreateInfos)
{
    shaderStageCreateInfos.clear();
    for (auto& shaderStage : shaderStageParams)
    {
        shaderStageCreateInfos.push_back(
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            nullptr,
            0,
            shaderStage.shaderStage,
            shaderStage.shaderModule,
            shaderStage.entryPointName,
            shaderStage.specializationInfo
        });
    }
}

void Cookbook::SpecifyPipelineVertexInputState(const std::vector<VkVertexInputBindingDescription>& bindingDescriptions,
    const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions,
    VkPipelineVertexInputStateCreateInfo& vertexInputStateCreateInfo)
{
    vertexInputStateCreateInfo = 
    {
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        nullptr,
        0,
        static_cast<uint32_t> (bindingDescriptions.size()),
        bindingDescriptions.data(),
        static_cast<uint32_t> (attributeDescriptions.size()),
        attributeDescriptions.data(),
    };
}

void Cookbook::SpecifyPipelineInputAssemblyState(VkPrimitiveTopology topology,
    bool primitiveRestartEnable,
    VkPipelineInputAssemblyStateCreateInfo& inputAssemblyStateCreateInfo)
{
    inputAssemblyStateCreateInfo = 
    {
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        nullptr,
        0,
        topology,
        primitiveRestartEnable
    };
}

void Cookbook::SpecifyPipelineViewportAndScissorTestState(const ViewportInfo& viewportInfo,
    VkPipelineViewportStateCreateInfo& viewportStateCreateInfo)
{
    uint32_t viewportCount = static_cast<uint32_t> (viewportInfo.viewports.size());
    uint32_t scissorCount = static_cast<uint32_t> (viewportInfo.scissors.size());

    viewportStateCreateInfo = 
    {
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        nullptr,
        0,
        viewportCount,
        viewportInfo.viewports.data(),
        scissorCount,
        viewportInfo.scissors.data()
    };

}

void Cookbook::SpecifyPipelineRasterizationState(bool depthClampEnable,
    bool rasterizerDiscardEnable,
    VkPolygonMode polygonMode,
    VkCullModeFlags cullmode,
    VkFrontFace frontface,
    bool depthBiasEnable,
    float depthBiasConstantFactor,
    float depthBiasClamp,
    float depthBiasSlopeFactor,
    float lineWidth,
    VkPipelineRasterizationStateCreateInfo& rasterizationStateCreateInfo)
{
    rasterizationStateCreateInfo = 
    {
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        nullptr,
        0,
        depthClampEnable,
        rasterizerDiscardEnable,
        polygonMode,
        cullmode,
        frontface,
        depthBiasEnable,
        depthBiasConstantFactor,
        depthBiasClamp,
        depthBiasSlopeFactor,
        lineWidth
    };
}

void Cookbook::SpecifyPipelineMultisampleState(VkSampleCountFlagBits sampleCount,
    bool perSampleShadingEnable,
    float minSampleShading,
    const VkSampleMask* sampleMas,
    bool alphaToCoverageEnable,
    bool alphaToOneEnable,
    VkPipelineMultisampleStateCreateInfo& multisampleStateCreateInfo)
{
    multisampleStateCreateInfo =
    {
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        nullptr,
        0,
        sampleCount,
        perSampleShadingEnable,
        minSampleShading,
        sampleMas,
        alphaToCoverageEnable,
        alphaToOneEnable
    };
}

void Cookbook::SpecifyPipelineBlendState(bool logicalOpEnable,
    VkLogicOp logicOp,
    const std::vector<VkPipelineColorBlendAttachmentState>& attachmentBlendStates,
    const std::array<float, 4>& blendConstraint,
    VkPipelineColorBlendStateCreateInfo& blendStateCreateInfo)
{
    blendStateCreateInfo = 
    {
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        nullptr,
        0,
        logicalOpEnable,
        logicOp,
        static_cast<uint32_t> (attachmentBlendStates.size()),
        attachmentBlendStates.data(),
        {
            blendConstraint[0],
            blendConstraint[1],
            blendConstraint[2],
            blendConstraint[3],
        }
    };    
}

void Cookbook::SpecifyPipelineDynamicState(const std::vector<VkDynamicState>& dynamicStates,
    VkPipelineDynamicStateCreateInfo& dynamicStateCreateInfo)
{
    dynamicStateCreateInfo = 
    {
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        nullptr,
        0,
        static_cast<uint32_t> (dynamicStates.size()),
        dynamicStates.data()
    };
}

bool Cookbook::CreatePipelineLayout(VkDevice logicalDevice, 
    const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
    const std::vector<VkPushConstantRange>& pushConstantRanges,
    VkPipelineLayout& pipelineLayout)
{
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = 
    {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        nullptr,
        0,
        static_cast<uint32_t> (descriptorSetLayouts.size()),
        descriptorSetLayouts.data(),
        static_cast<uint32_t> (pushConstantRanges.size()),
        pushConstantRanges.data()
    };

    VkResult result = vkCreatePipelineLayout(logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
    if (result != VK_SUCCESS)
    {
        std::cout << "Could not create pipeline layout." << std::endl;
        return false;
    }

    return true;
}

void Cookbook::SpecifyGraphicsPipelineCreateParameter(VkPipelineCreateFlags additionalOptions,
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
    VkGraphicsPipelineCreateInfo& graphicsPipelineCreateInfo)
{
    graphicsPipelineCreateInfo = 
    {
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        nullptr,
        additionalOptions,
        static_cast<uint32_t> (shaderStageCreateInfos.size()),
        shaderStageCreateInfos.data(),
        &vertexInputStateCreateInfo,
        &inputAssemblyStateCreateInfo,        
        tessellationStateCreateInfo,
        viewportStateCreateInfo,
        &rasterizationStateCreateInfo,
        multisampleStateCreateInfo,
        depthAndStencilStateCreateInfo,
        blendStateCreateInfo,
        dynamicStateCreateInfo,
        pipelineLayout,
        renderpass,
        subpass,
        basePipelineHandle,
        basePipelineIndex,
    };
}

bool Cookbook::CreateGraphicsPipeline(VkDevice logicalDevice,
    const std::vector<VkGraphicsPipelineCreateInfo>& graphicsPipelineCreateInfos,
    VkPipelineCache pipelineCache,
    std::vector<VkPipeline>& graphicsPipelines)
{
    if (graphicsPipelineCreateInfos.size() > 0)
    {
        graphicsPipelines.resize(graphicsPipelineCreateInfos.size());
        VkResult result = vkCreateGraphicsPipelines(logicalDevice,
            pipelineCache,
            static_cast<uint32_t> (graphicsPipelineCreateInfos.size()),
            graphicsPipelineCreateInfos.data(),
            nullptr,
            graphicsPipelines.data());

        if (result != VK_SUCCESS)
        {
            std::cout << "Could not create a graphics pipeline." << std::endl;
            return false;
        }
        return true;
    }
    return false;
}

void Cookbook::BindPipelineObject(VkCommandBuffer commandBuffer,
    VkPipelineBindPoint pipelineType,
    VkPipeline pipeline)
{
    vkCmdBindPipeline(commandBuffer, pipelineType, pipeline);
}

void Cookbook::SpecifyPipelineTessellationState(uint32_t patchControlPointsCount, 
    VkPipelineTessellationStateCreateInfo&  tessellationStateCreateInfo)
{
    tessellationStateCreateInfo =
    {
        VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
        nullptr,
        0,
        patchControlPointsCount
    };
}

bool Cookbook::CreateComputePipeline(VkDevice logicalDevice,
    VkPipelineCreateFlags additionalOptions,
    const VkPipelineShaderStageCreateInfo& computeShaderStage,
    VkPipelineLayout pipelineLayout,
    VkPipeline basePipelineHandle,
    VkPipelineCache pipelineCache,
    VkPipeline& computePipeline)
{
    VkComputePipelineCreateInfo computePipelineCreateInfo = 
    {
        VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        nullptr,
        additionalOptions,
        computeShaderStage,
        pipelineLayout,
        basePipelineHandle,
        -1
    };

    VkResult result = vkCreateComputePipelines(logicalDevice, 
        pipelineCache, 
        1,
        &computePipelineCreateInfo,
        nullptr,
        &computePipeline);
    if (result != VK_SUCCESS)
    {
        std::cout << "Could not create compute pipeline." << std::endl;
        return false;
    }

    return true;
}

void Cookbook::SpecifyPipelineDepthAndStencilState(bool depthTestEnable,
    bool depthWriteEnable,
    VkCompareOp depthCompareOP,
    bool depthBoundsTestEnable,
    float minDepthBounds,
    float maxDepthBounds,
    bool stencilTestEnable,
    VkStencilOpState frontStencilTestParameter,
    VkStencilOpState backStencilTestParameter,
    VkPipelineDepthStencilStateCreateInfo& depthAndStencilStateCreateInfo)
{
    depthAndStencilStateCreateInfo =
    {
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        nullptr,
        0,
        depthTestEnable,
        depthWriteEnable,
        depthCompareOP,
        depthBoundsTestEnable,
        stencilTestEnable,
        frontStencilTestParameter,
        backStencilTestParameter,
        minDepthBounds,
        maxDepthBounds
    };
}
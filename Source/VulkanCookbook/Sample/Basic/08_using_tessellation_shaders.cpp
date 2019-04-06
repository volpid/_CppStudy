
#include "../../Chapter/ch03_command_buffer_and_synchronization.h"
#include "../../Chapter/ch04_resource_and_memory.h"
#include "../../Chapter/ch06_renderpass_and_framebuffer.h"
#include "../../Chapter/ch08_graphics_and_compute_pipeline.h"
#include "../../Chapter/ch09_command_recording_and_drawing.h"

#include "../../Common/sample_framework.h"
#include "../../Common/vulkan_tool.h"

#include <array>

//----------------------------------------------------------------
// SampleChapter08 : using tessellationg shader
//----------------------------------------------------------------
class SampleChapter08 : public VulkanSample
{
public:
    virtual bool Initialize(WindowParameters parameters) override;
    virtual bool Draw(void) override;
    virtual bool Resize(void) override;    

private:
    VkDestroyer(VkRenderPass) _renderPass;
    VkDestroyer(VkPipelineLayout) _pipelineLayout;
    VkDestroyer(VkPipeline) _graphicsPipeline;

    VkDestroyer(VkBuffer) _vertexBuffer;
    VkDestroyer(VkDeviceMemory) _bufferMemory;
};

//VULKAN_SAMPLE_FRAMEWORK("08_using_tessellation_shaders", 50, 25, 800, 600, SampleChapter08);

//----------------------------------------------------------------
bool SampleChapter08::Initialize(WindowParameters parameters)
{   
    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.fillModeNonSolid = true;
    deviceFeatures.tessellationShader = true;
    if (InitializeVulkan(parameters, &deviceFeatures, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, false) == false)
    {
        return false;
    }

    std::vector<VkAttachmentDescription> attachmentDescription =
    {
        {
            0,
            _swapchain.format,
            VK_SAMPLE_COUNT_1_BIT,
            VK_ATTACHMENT_LOAD_OP_CLEAR,
            VK_ATTACHMENT_STORE_OP_STORE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        }
    };

    std::vector<SubpassParameter> subpassParameters =
    {
        {
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            {},
            {
                {
                    0, 
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                }
            },
            {},
            nullptr,
            {}
        }
    };

    std::vector<VkSubpassDependency> subpassDependencies = 
    {
        {
            VK_SUBPASS_EXTERNAL,
            0, 
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_MEMORY_READ_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_DEPENDENCY_BY_REGION_BIT
        },
        {
            0, 
            VK_SUBPASS_EXTERNAL,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_ACCESS_MEMORY_READ_BIT,
            VK_DEPENDENCY_BY_REGION_BIT
        }
    };

    InitVkDestroyer(_logicalDevice, _renderPass);
    if (Cookbook::CreateRenderpass(*_logicalDevice,
        attachmentDescription, 
        subpassParameters,
        subpassDependencies,
        *_renderPass) == false)
    {
        return false;
    }

    std::vector<unsigned char> vertexShaderSpirv;
    if (GetBinaryFileContent("..\\..\\Source\\VulkanCookbook\\Shader\\Basic\\08_shader.vert.spv", vertexShaderSpirv) == false)
    {
        return false;
    }

    VkDestroyer(VkShaderModule) vertexShaderModule;
    InitVkDestroyer(_logicalDevice, vertexShaderModule);
    if (Cookbook::CreateShader(*_logicalDevice, vertexShaderSpirv, *vertexShaderModule) == false)
    {
        return false;
    }

    std::vector<unsigned char> tessellationControlShaderSpirv;
    if (GetBinaryFileContent("..\\..\\Source\\VulkanCookbook\\Shader\\Basic\\08_shader.tesc.spv", tessellationControlShaderSpirv) == false)
    {
        return false;
    }

    VkDestroyer(VkShaderModule) tesselationControlShaderModule;
    InitVkDestroyer(_logicalDevice, tesselationControlShaderModule);
    if (Cookbook::CreateShader(*_logicalDevice, tessellationControlShaderSpirv, *tesselationControlShaderModule) == false)
    {
        return false;
    }

    std::vector<unsigned char> tessellationEvaluationShaderSpirv;
    if (GetBinaryFileContent("..\\..\\Source\\VulkanCookbook\\Shader\\Basic\\08_shader.tese.spv", tessellationEvaluationShaderSpirv) == false)
    {
        return false;
    }

    VkDestroyer(VkShaderModule) tesselationEvaluationShaderModule;
    InitVkDestroyer(_logicalDevice, tesselationEvaluationShaderModule);
    if (Cookbook::CreateShader(*_logicalDevice, tessellationEvaluationShaderSpirv, *tesselationEvaluationShaderModule) == false)
    {
        return false;
    }

    std::vector<unsigned char> fragmentShaderSpirv;
    if (GetBinaryFileContent("..\\..\\Source\\VulkanCookbook\\Shader\\Basic\\08_shader.frag.spv", fragmentShaderSpirv) == false)
    {
        return false;
    }

    VkDestroyer(VkShaderModule) fagmentShaderModule;
    InitVkDestroyer(_logicalDevice, fagmentShaderModule);
    if (Cookbook::CreateShader(*_logicalDevice, fragmentShaderSpirv, *fagmentShaderModule) == false)
    {
        return false;
    }

    std::vector<ShaderStageParameter> shaderStageParams =
    {
        {
            VK_SHADER_STAGE_VERTEX_BIT,
            *vertexShaderModule,
            "main",
            nullptr
        },
        {
            VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
            *tesselationControlShaderModule,
            "main",
            nullptr
        },
        {
            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
            *tesselationEvaluationShaderModule,
            "main",
            nullptr
        },
        {
            VK_SHADER_STAGE_FRAGMENT_BIT,
            *fagmentShaderModule,
            "main",
            nullptr
        }
    };
    
    std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos;
    Cookbook::SpecifyPipelineShaderStage(shaderStageParams, shaderStageCreateInfos);

    std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions =
    {
        {
            0,
            3 * sizeof(float),
            VK_VERTEX_INPUT_RATE_VERTEX,
        }
    };

    std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions = 
    {
        {
            0,
            0,
            VK_FORMAT_R32G32B32_SFLOAT,
            0
        }
    };

    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo;
    Cookbook::SpecifyPipelineVertexInputState(vertexInputBindingDescriptions, 
        vertexAttributeDescriptions,
        vertexInputStateCreateInfo);

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo;
    Cookbook::SpecifyPipelineInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, false, inputAssemblyStateCreateInfo);
    
    VkPipelineTessellationStateCreateInfo tessellationStateCreateInfo;
    Cookbook::SpecifyPipelineTessellationState(3, tessellationStateCreateInfo);

    ViewportInfo viewportInfos =
    {
        {
            {
                0.0f,
                0.0f,
                500.0f,
                500.0f,
                0.0f,
                1.0f,
            }
        },
        {
            {
                {0, 0},
                {500, 500}
            }
        }
    };
    
    VkPipelineViewportStateCreateInfo viewportStateCreateInfo;
    Cookbook::SpecifyPipelineViewportAndScissorTestState(viewportInfos, viewportStateCreateInfo);

    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo;
    Cookbook::SpecifyPipelineRasterizationState(false, 
        false,
        VK_POLYGON_MODE_LINE,
        VK_CULL_MODE_BACK_BIT,
        VK_FRONT_FACE_COUNTER_CLOCKWISE,
        false,
        0.0f,
        0.0f,
        0.0f,
        1.0f,
        rasterizationStateCreateInfo);

    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo;
    Cookbook::SpecifyPipelineMultisampleState(VK_SAMPLE_COUNT_1_BIT, false, 0.0f, nullptr, false, false, multisampleStateCreateInfo);

    std::vector<VkPipelineColorBlendAttachmentState> attachmentBlendStates = 
    {
        {
            false,
            VK_BLEND_FACTOR_ONE,
            VK_BLEND_FACTOR_ONE,
            VK_BLEND_OP_ADD,
            VK_BLEND_FACTOR_ONE,
            VK_BLEND_FACTOR_ONE,
            VK_BLEND_OP_ADD,
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
        }
    };

    VkPipelineColorBlendStateCreateInfo blendStateCraeteInfo;
    Cookbook::SpecifyPipelineBlendState(false, 
        VK_LOGIC_OP_COPY, 
        attachmentBlendStates,
        {1.0f, 1.0f, 1.0f, 1.0f},
        blendStateCraeteInfo);

    std::vector<VkDynamicState> dynamicStates = 
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };
    
    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo;
    Cookbook::SpecifyPipelineDynamicState(dynamicStates, dynamicStateCreateInfo);

    InitVkDestroyer(_logicalDevice, _pipelineLayout);
    if (Cookbook::CreatePipelineLayout(*_logicalDevice, {}, {}, *_pipelineLayout) == false)
    {
        return false;
    }

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo;
    Cookbook::SpecifyGraphicsPipelineCreateParameter(0,
        shaderStageCreateInfos,
        vertexInputStateCreateInfo,
        inputAssemblyStateCreateInfo,
        &tessellationStateCreateInfo,
        &viewportStateCreateInfo,
        rasterizationStateCreateInfo,
        &multisampleStateCreateInfo,
        nullptr,
        &blendStateCraeteInfo,
        &dynamicStateCreateInfo,
        *_pipelineLayout,
        *_renderPass,
        0,
        VK_NULL_HANDLE,
        -1,
        graphicsPipelineCreateInfo);

    std::vector<VkPipeline> graphicsPipelines;
    if (Cookbook::CreateGraphicsPipeline(*_logicalDevice, {graphicsPipelineCreateInfo}, VK_NULL_HANDLE, graphicsPipelines) == false)
    {
        return false;
    }

    InitVkDestroyer(_logicalDevice, _graphicsPipeline);
    *_graphicsPipeline = graphicsPipelines[0];

    std::vector<float> vertices = 
    {
        0.0f, -0.7f, 0.0f,
        0.25f, 0.75f, 0.0f,
        -0.75f, 0.75f, 0.0f,
    };
    
    InitVkDestroyer(_logicalDevice, _vertexBuffer);
    if (Cookbook::CreateBuffer(*_logicalDevice, 
        sizeof(vertices[0]) * vertices.size(),
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        *_vertexBuffer) == false)
    {
        return false;
    }
        
    InitVkDestroyer(_logicalDevice, _bufferMemory);
    if (Cookbook::AllocateAndBindMemoryObjectToBuffer(_physicalDevice,
        *_logicalDevice,
        *_vertexBuffer,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        *_bufferMemory) == false)
    {
        return false;
    }

    if (Cookbook::UseStagingBufferToUpdateBufferWithDeviceLocalMemoryBound(_physicalDevice,
        *_logicalDevice,
        sizeof(vertices[0]) * vertices.size(),
        &vertices[0],
        *_vertexBuffer,
        0,
        0,
        VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
        _graphicsQueue.handle,
        _frameResources.front().commandBuffer,
        {}) == false)
    {
        return false;
    }

    return true;
}

bool SampleChapter08::Draw(void)
{
    auto prepareFrame = [this](VkCommandBuffer commandBuffer, uint32_t swapchainImageIndex, VkFramebuffer framebuffer) -> bool
    {
        if (Cookbook::BeginCommandBufferRecordingOperation(commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr) == false)
        {
            return false;
        }

        if (_presentQueue.familyIndex != _graphicsQueue.familyIndex)
        {
            ImageTransition imageTrasitionBeforeDrawing =
            {
                _swapchain.images[swapchainImageIndex],
                VK_ACCESS_MEMORY_READ_BIT,
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                _presentQueue.familyIndex,
                _graphicsQueue.familyIndex,
                VK_IMAGE_ASPECT_COLOR_BIT
            };

            Cookbook::SetImageMemoryBarrier(commandBuffer, 
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                {imageTrasitionBeforeDrawing});
        }

        Cookbook::BeginRenderPass(commandBuffer, 
            *_renderPass, 
            framebuffer, 
            {{0, 0}, _swapchain.size},
            {{0.1f, 0.2f, 0.3f, 1.0f}},
            VK_SUBPASS_CONTENTS_INLINE);

        Cookbook::BindPipelineObject(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *_graphicsPipeline);

        VkViewport viewport = 
        {
            0.0f, 
            0.0f, 
            static_cast<float>(_swapchain.size.width),
            static_cast<float>(_swapchain.size.height),
            0.0f,
            1.0f
        };
        Cookbook::SetViewportStateDynamically(commandBuffer, 0, {viewport});

        VkRect2D sissor = 
        {
            {0, 0},
            {_swapchain.size.width, _swapchain.size.height}
        };
        Cookbook::SetScissorStateDynamically(commandBuffer, 0, {sissor});

        Cookbook::BindVertexBuffer(commandBuffer, 0, {{*_vertexBuffer, 0}});

        Cookbook::DrawGeometry(commandBuffer, 3, 1, 0, 0);

        Cookbook::EndRenderPass(commandBuffer);

        if (_presentQueue.familyIndex != _graphicsQueue.familyIndex)
        {
            ImageTransition imageTrasitionBeforePresent=
            {
                _swapchain.images[swapchainImageIndex],
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                VK_ACCESS_MEMORY_READ_BIT,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                _graphicsQueue.familyIndex,
                _presentQueue.familyIndex,
                VK_IMAGE_ASPECT_COLOR_BIT
            };

            Cookbook::SetImageMemoryBarrier(commandBuffer, 
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                {imageTrasitionBeforePresent});
        }

        if (Cookbook::EndCommandBufferRecordingOperation(commandBuffer) == false)
        {
            return false;
        }

        return true;
    };
    
    return Cookbook::IncreasePerformanceThroughIncreasingTheNumberOfSeparatelyRenderedFrames(*_logicalDevice,
        _graphicsQueue.handle,
        _presentQueue.handle,
        *_swapchain.handle,
        _swapchain.size,
        _swapchain.imageViewsRaw,
        *_renderPass,
        {},
        prepareFrame,
        _frameResources);
}

bool SampleChapter08::Resize(void)
{
    if (CreateSwapchain(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, false) == false)
    {
        return false;
    }

    return true;
}
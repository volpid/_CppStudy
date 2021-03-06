
#include "../../Chapter/ch01_instance_and_device.h"
#include "../../Chapter/ch02_image_presentation.h"
#include "../../Chapter/ch03_command_buffer_and_synchronization.h"
#include "../../Chapter/ch04_resource_and_memory.h"
#include "../../Chapter/ch05_descriptor_sets.h"
#include "../../Chapter/ch06_renderpass_and_framebuffer.h"
#include "../../Chapter/ch08_graphics_and_compute_pipeline.h"
#include "../../Chapter/ch09_command_recording_and_drawing.h"

#include "../../Common/sample_framework.h"
#include "../../Common/vulkan_tool.h"

#include <array>

//----------------------------------------------------------------
// SampleChapter04 : graphics pipeline
//----------------------------------------------------------------
class SampleChapter04 : public VulkanSample
{
public:
    virtual bool Initialize(WindowParameters parameters) override;
    virtual bool Draw(void) override;
    virtual bool Resize(void) override;    

private:
    VkCommandBuffer commandBuffer_;
    VkDestroyer(VkRenderPass) renderpass_;
    VkDestroyer(VkFramebuffer) framebuffer_;
    VkDestroyer(VkFence) drawingFence_;
    VkDestroyer(VkSemaphore) imageAcquiredSemaphore_;
    VkDestroyer(VkSemaphore) readyToPresentSemaphore_;
    VkDestroyer(VkPipeline) graphicsPipeline_;
    VkDestroyer(VkPipelineLayout) pipelineLayout_;
    VkDestroyer(VkBuffer) vertexBuffer_;
    VkDestroyer(VkDeviceMemory) bufferMemory_;
};

//VULKAN_SAMPLE_FRAMEWORK("04_UseGraphicsPipeline", 50, 25, 800, 600, SampleChapter04);

//----------------------------------------------------------------
bool SampleChapter04::Initialize(WindowParameters parameters)
{   
    if (InitializeVulkan(parameters) == false)
    {
        return false;
    }

    std::vector<VkCommandBuffer> commandBuffers;
    if (Cookbook::AllocateCommandBuffer(*_logicalDevice, *_commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, commandBuffers) == false)
    {
        return false;
    }
    commandBuffer_ = commandBuffers[0];

    InitVkDestroyer(_logicalDevice, drawingFence_);
    if (Cookbook::CreateFence(*_logicalDevice, true, *drawingFence_) == false)
    {
        return false;
    }

    InitVkDestroyer(_logicalDevice, imageAcquiredSemaphore_);
    if (Cookbook::CreateSemaphore(*_logicalDevice, *imageAcquiredSemaphore_) == false)
    {
        return false;
    }

    InitVkDestroyer(_logicalDevice, readyToPresentSemaphore_);
    if (Cookbook::CreateSemaphore(*_logicalDevice, *readyToPresentSemaphore_) == false)
    {
        return false;
    }
    
    std::vector<VkAttachmentDescription> attachmentDescriptions = 
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
    
    std::vector<SubpassParameter> subpassParameter = 
    {
        {
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            {},
            {   
                {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
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
            VK_SUBPASS_EXTERNAL,
            0, 
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_ACCESS_MEMORY_READ_BIT,
            VK_DEPENDENCY_BY_REGION_BIT
        },
    };

    InitVkDestroyer(_logicalDevice, renderpass_);
    if (Cookbook::CreateRenderpass(*_logicalDevice, attachmentDescriptions, subpassParameter, subpassDependencies, *renderpass_) == false)
    {
        return false;
    }
    
    std::vector<unsigned char> vertexShaderSpirv;
    if (GetBinaryFileContent("..\\..\\Source\\VulkanCookbook\\Shader\\Basic\\04_shader.vert.spv", vertexShaderSpirv) == false)
    {
        return false;
    }

    VkDestroyer(VkShaderModule) vertexShaderModule;
    InitVkDestroyer(_logicalDevice, vertexShaderModule);
    if (Cookbook::CreateShader(*_logicalDevice, vertexShaderSpirv, *vertexShaderModule) == false)
    {
        return false;
    }

    std::vector<unsigned char> fragmentShaderSpirv;
    if (GetBinaryFileContent("..\\..\\Source\\VulkanCookbook\\Shader\\Basic\\04_shader.frag.spv", fragmentShaderSpirv) == false)
    {
        return false;
    }

    VkDestroyer(VkShaderModule) fragmentShaderModule;
    InitVkDestroyer(_logicalDevice, fragmentShaderModule);
    if (Cookbook::CreateShader(*_logicalDevice, fragmentShaderSpirv, *fragmentShaderModule) == false)
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
            VK_SHADER_STAGE_FRAGMENT_BIT,
            *fragmentShaderModule,
            "main",
            nullptr
        }
    };

    std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo;
    Cookbook::SpecifyPipelineShaderStage(shaderStageParams, shaderStageCreateInfo);
    
    std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions = 
    {
        {
            0, 
            3 * sizeof(float),
            VK_VERTEX_INPUT_RATE_VERTEX
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
    Cookbook::SpecifyPipelineInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
        false,
        inputAssemblyStateCreateInfo);

    ViewportInfo viewportInfo = 
    {
        {{0.0f, 0.0f, 500.0f, 500.0f, 0.0f, 1.0f}},
        {{{0, 0}, {500, 500}}},
    };
    VkPipelineViewportStateCreateInfo viewportStateCreateInfo;
    Cookbook::SpecifyPipelineViewportAndScissorTestState(viewportInfo, viewportStateCreateInfo);

    VkPipelineRasterizationStateCreateInfo resterizationStateCreateInfo;
    Cookbook::SpecifyPipelineRasterizationState(false,
        false,
        VK_POLYGON_MODE_FILL,
        VK_CULL_MODE_BACK_BIT,
        VK_FRONT_FACE_COUNTER_CLOCKWISE,
        false,
        0.0f,
        0.0f,
        0.0f,
        1.0f,
        resterizationStateCreateInfo);

    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo;
    Cookbook::SpecifyPipelineMultisampleState(VK_SAMPLE_COUNT_1_BIT,
        false,
        0.0f,
        nullptr, 
        false,
        false,
        multisampleStateCreateInfo);

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
    VkPipelineColorBlendStateCreateInfo blendStateCreateInfo;
    Cookbook::SpecifyPipelineBlendState(false,
        VK_LOGIC_OP_COPY,
        attachmentBlendStates,
        {1.0f, 1.0f, 1.0f, 1.0f},
        blendStateCreateInfo);

    std::vector<VkDynamicState> dynamicStates = 
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };
    VkPipelineDynamicStateCreateInfo dynamicStateCreteInfo;
    Cookbook::SpecifyPipelineDynamicState(dynamicStates, dynamicStateCreteInfo);

    InitVkDestroyer(_logicalDevice, pipelineLayout_);
    if (Cookbook::CreatePipelineLayout(*_logicalDevice, {}, {}, *pipelineLayout_) == false)
    {
        return false;
    }

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo;
    Cookbook::SpecifyGraphicsPipelineCreateParameter(0,
        shaderStageCreateInfo,
        vertexInputStateCreateInfo,
        inputAssemblyStateCreateInfo,
        nullptr,
        &viewportStateCreateInfo,
        resterizationStateCreateInfo,
        &multisampleStateCreateInfo,
        nullptr,
        &blendStateCreateInfo, 
        &dynamicStateCreteInfo,
        *pipelineLayout_,
        *renderpass_,
        0,
        VK_NULL_HANDLE,
        -1,
        graphicsPipelineCreateInfo);

    std::vector<VkPipeline> graphicsPipelines;
    if (Cookbook::CreateGraphicsPipeline(*_logicalDevice, {graphicsPipelineCreateInfo}, VK_NULL_HANDLE, graphicsPipelines) == false)
    {
        return false;
    }

    InitVkDestroyer(_logicalDevice, graphicsPipeline_);
    *graphicsPipeline_ = graphicsPipelines[0];

    std::vector<float> vertices = 
    {
        0.0f, -0.75f, 0.0f,
        -0.75f, 0.75f, 0.0f,
        0.75f, 0.75f, 0.0f
    };
    
    InitVkDestroyer(_logicalDevice, vertexBuffer_);
    if (Cookbook::CreateBuffer(*_logicalDevice, 
        sizeof(vertices[0]) * vertices.size(), 
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        *vertexBuffer_) == false)
    {
        return false;
    }

    InitVkDestroyer(_logicalDevice, bufferMemory_);
    if (Cookbook::AllocateAndBindMemoryObjectToBuffer(_physicalDevice, 
        *_logicalDevice, 
        *vertexBuffer_, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        *bufferMemory_) == false)
    {
        return false;
    }

    if (Cookbook::UseStagingBufferToUpdateBufferWithDeviceLocalMemoryBound(_physicalDevice,
        *_logicalDevice,
        sizeof(vertices[0]) * vertices.size(),
        &vertices[0],
        *vertexBuffer_,
        0,
        0,
        VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
        _graphicsQueue.handle,
        commandBuffer_,
        {}) == false)
    {
        return false;
    }

    return true;
}

bool SampleChapter04::Draw(void)
{
    if (Cookbook::WaitForFence(*_logicalDevice, {*drawingFence_}, false, 5'000'000'000) == false)
    {
        return false;
    }

    if (Cookbook::ResetFence(*_logicalDevice, {*drawingFence_}) == false)
    {
        return false;
    }

    uint32_t imageIndex;
    if (Cookbook::AcquireSwapchainImage(*_logicalDevice, *_swapchain.handle, *imageAcquiredSemaphore_, VK_NULL_HANDLE, imageIndex) == false)
    {
        return false;
    }

    InitVkDestroyer(_logicalDevice, framebuffer_);
    if (Cookbook::CreateFramebuffer(*_logicalDevice, 
        *renderpass_, 
        {*_swapchain.imageViews[imageIndex]}, 
        _swapchain.size.width,
        _swapchain.size.height,
        1,
        *framebuffer_) == false)
    {
        return false;
    }

    if (Cookbook::BeginCommandBufferRecordingOperation(commandBuffer_, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr) == false)
    {
        return false;
    }

    if (_presentQueue.familyIndex != _graphicsQueue.familyIndex)
    {
        ImageTransition image_transition_before_drawing =
        {
            _swapchain.images[imageIndex],
            VK_ACCESS_MEMORY_READ_BIT,
            VK_ACCESS_MEMORY_READ_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            _presentQueue.familyIndex,
            _graphicsQueue.familyIndex,
            VK_IMAGE_ASPECT_COLOR_BIT
        };

        Cookbook::SetImageMemoryBarrier(commandBuffer_, 
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            {image_transition_before_drawing});
    }

    Cookbook::BeginRenderPass(commandBuffer_, 
        *renderpass_, 
        *framebuffer_, 
        {{0, 0}, _swapchain.size},
        {{0.2f, 0.5f, 0.8f, 1.0f}},
        VK_SUBPASS_CONTENTS_INLINE);

    Cookbook::BindPipelineObject(commandBuffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, *graphicsPipeline_);

    VkViewport viewport = 
    {
        0.0f,
        0.0f,
        static_cast<float> (_swapchain.size.width),
        static_cast<float> (_swapchain.size.height),
        0.0f,
        1.0f,
    };
    Cookbook::SetViewportStateDynamically(commandBuffer_, 0, {viewport});

    VkRect2D scissor = 
    {
        {0, 0},
        {_swapchain.size.width, _swapchain.size.height}
    };
    Cookbook::SetScissorStateDynamically(commandBuffer_, 0, {scissor});

    Cookbook::BindVertexBuffer(commandBuffer_, 0, {{*vertexBuffer_, 0}});
    Cookbook::DrawGeometry(commandBuffer_, 3, 1, 0, 0);

    Cookbook::EndRenderPass(commandBuffer_);

    if (_presentQueue.familyIndex != _graphicsQueue.familyIndex)
    {
        ImageTransition image_transition_before_present =
        {
            _swapchain.images[imageIndex],
            VK_ACCESS_MEMORY_READ_BIT,
            VK_ACCESS_MEMORY_READ_BIT,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            _graphicsQueue.familyIndex,
            _presentQueue.familyIndex,
            VK_IMAGE_ASPECT_COLOR_BIT
        };

        Cookbook::SetImageMemoryBarrier(commandBuffer_, 
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            {image_transition_before_present});
    }

    if (Cookbook::EndCommandBufferRecordingOperation(commandBuffer_) == false)
    {
        return false;
    }

    WaitSemaphoreInfo waitSemaphoreInfo = 
    {
        *imageAcquiredSemaphore_,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
    };
    if (Cookbook::SubmitCommandBufferToQueue(_graphicsQueue.handle, 
        {waitSemaphoreInfo},
        {commandBuffer_},
        {*readyToPresentSemaphore_},
        *drawingFence_) == false)
    {
        return false;
    }

    PresentInfo presentInfo =
    {
        *_swapchain.handle,
        imageIndex
    };

    if (Cookbook::PresentImage(_presentQueue.handle, {*readyToPresentSemaphore_}, {presentInfo}) == false)
    {
        return false;
    }

    return true;
}

bool SampleChapter04::Resize(void)
{
    return CreateSwapchain();
}

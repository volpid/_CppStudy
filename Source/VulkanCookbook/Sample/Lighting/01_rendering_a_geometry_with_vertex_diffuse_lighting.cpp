
#include "../../Chapter/ch03_command_buffer_and_synchronization.h"
#include "../../Chapter/ch04_resource_and_memory.h"
#include "../../Chapter/ch05_descriptor_sets.h"
#include "../../Chapter/ch06_renderpass_and_framebuffer.h"
#include "../../Chapter/ch08_graphics_and_compute_pipeline.h"
#include "../../Chapter/ch09_command_recording_and_drawing.h"
#include "../../Chapter/ch10_helper_recipes.h"

#include "../../Common/sample_framework.h"
#include "../../Common/vulkan_tool.h"

#include <array>

//----------------------------------------------------------------
// SampleLighting01 : rendering a geometry with vertex diffuse ligting
//----------------------------------------------------------------
class SampleLighting01 : public VulkanSample
{
public:
    virtual bool Initialize(WindowParameters parameters) override;
    virtual bool Draw(void) override;
    virtual bool Resize(void) override;    

    virtual void OnMouseEvent(void);

private:
    bool _UpdateUniformbuffer(bool force);

private: 
    Cookbook::Mesh  _model;

    VkDestroyer(VkBuffer) _vertexBuffer;
    VkDestroyer(VkDeviceMemory) _bufferMemory;

    VkDestroyer(VkBuffer) _stagingBuffer;
    VkDestroyer(VkDeviceMemory) _stagingBufferMemory;
    bool _updateUniformBuffer;
    VkDestroyer(VkBuffer) _uniformBuffer;
    VkDestroyer(VkDeviceMemory) _uniformBufferMemory;

    VkDestroyer(VkDescriptorSetLayout) _descriptorSetLayout;
    VkDestroyer(VkDescriptorPool) _descriptorPool;
    std::vector<VkDescriptorSet> _descriptorSets;

    VkDestroyer(VkRenderPass) _renderPass;
    VkDestroyer(VkPipeline) _graphicsPipeline;
    VkDestroyer(VkPipelineLayout) _graphicsPipelineLayout;
};

//VULKAN_SAMPLE_FRAMEWORK("01_rendering_a_geometry_with_vertex_diffuse_lighting", 50, 25, 800, 600, SampleLighting01);

//----------------------------------------------------------------
bool SampleLighting01::Initialize(WindowParameters parameters)
{   
    if (InitializeVulkan(parameters, nullptr, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, false) == false)
    {
        return false;
    }

    if (Cookbook::Load3DModelFromObjFile("../../Resource/VulkanCookbook/Model/knot.obj", true, false, false, true, _model) == false)
    {
        return false;
    }

    InitVkDestroyer(_logicalDevice, _vertexBuffer);
    if (Cookbook::CreateBuffer(*_logicalDevice, 
        sizeof(_model.data[0]) * _model.data.size(),
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
        sizeof(_model.data[0]) * _model.data.size(),
        &_model.data[0],
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
    
    InitVkDestroyer(_logicalDevice, _stagingBuffer);
    if (Cookbook::CreateBuffer(*_logicalDevice, 
        2 * 16 * sizeof(float),
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        *_stagingBuffer) == false)
    {
        return false;
    }
    
    InitVkDestroyer(_logicalDevice, _stagingBufferMemory);
    if (Cookbook::AllocateAndBindMemoryObjectToBuffer(_physicalDevice,
        *_logicalDevice,
        *_stagingBuffer,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,        
        *_stagingBufferMemory) == false)
    {
        return false;
    }
        
    InitVkDestroyer(_logicalDevice, _uniformBuffer);
    InitVkDestroyer(_logicalDevice, _uniformBufferMemory);
    if (Cookbook::CreateUniformBuffer(_physicalDevice,
        *_logicalDevice, 
        2 * 16 * sizeof(float),
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        *_uniformBuffer,
        *_uniformBufferMemory) == false)
    {
        return false;
    }

    if (_UpdateUniformbuffer(true) == false)
    {
        return false;
    }

    
    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = 
    {
        0,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1,
        VK_SHADER_STAGE_VERTEX_BIT,
        nullptr
    };

    InitVkDestroyer(_logicalDevice, _descriptorSetLayout);
    if (Cookbook::CreateDescriptorSetLayout(*_logicalDevice, {descriptorSetLayoutBinding}, *_descriptorSetLayout) == false)
    {
        return false;
    }
    
    VkDescriptorPoolSize descriptorPoolSize = 
    {
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1
    };

    InitVkDestroyer(_logicalDevice, _descriptorPool);
    if (Cookbook::CreateDescriptorPool(*_logicalDevice, false, 1, {descriptorPoolSize}, *_descriptorPool) == false)
    {
        return false;
    }

    if (Cookbook::AllocateDescriptorSets(*_logicalDevice, *_descriptorPool, {*_descriptorSetLayout}, _descriptorSets) == false)
    {
        return false;
    }

    Cookbook::BufferDescriptorInfo bufferDescriptorUpdate =
    {
        _descriptorSets[0],
        0,
        0,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        {
            {
                *_uniformBuffer,
                0,
                VK_WHOLE_SIZE
            }
        }
    };
    
    Cookbook::UpdateDescriptorSets(*_logicalDevice, {}, {bufferDescriptorUpdate}, {}, {});

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
        },
        {
            0,
            DepthFormat,
            VK_SAMPLE_COUNT_1_BIT,
            VK_ATTACHMENT_LOAD_OP_CLEAR,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        }
    };

    VkAttachmentReference depthAttachment = 
    {
        1,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
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
            &depthAttachment,
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
    
    InitVkDestroyer(_logicalDevice, _graphicsPipelineLayout);
    if (Cookbook::CreatePipelineLayout(*_logicalDevice, {*_descriptorSetLayout}, {}, *_graphicsPipelineLayout) == false)
    {
        return false;
    }

    std::vector<unsigned char> vertexShaderSpirv;
    if (GetBinaryFileContent("..\\..\\Source\\VulkanCookbook\\Shader\\Lighting\\01_shader.vert.spv", vertexShaderSpirv) == false)
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
    if (GetBinaryFileContent("..\\..\\Source\\VulkanCookbook\\Shader\\Lighting\\01_shader.frag.spv", fragmentShaderSpirv) == false)
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
            6 * sizeof(float),
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
        },
        {
            1,
            0,
            VK_FORMAT_R32G32B32_SFLOAT,
            3 * sizeof(float)
        }
    };

    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo;
    Cookbook::SpecifyPipelineVertexInputState(vertexInputBindingDescriptions, 
        vertexAttributeDescriptions,
        vertexInputStateCreateInfo);

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo;
    Cookbook::SpecifyPipelineInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, false, inputAssemblyStateCreateInfo);

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
        VK_POLYGON_MODE_FILL,
        VK_CULL_MODE_NONE,
        VK_FRONT_FACE_COUNTER_CLOCKWISE,
        false,
        0.0f,
        0.0f,
        0.0f,
        1.0f,
        rasterizationStateCreateInfo);

    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo;
    Cookbook::SpecifyPipelineMultisampleState(VK_SAMPLE_COUNT_1_BIT, false, 0.0f, nullptr, false, false, multisampleStateCreateInfo);

    VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo;
    Cookbook::SpecifyPipelineDepthAndStencilState(true,
        true,
        VK_COMPARE_OP_LESS_OR_EQUAL,
        false,
        0.0f,
        1.0f,
        false,
        {},
        {},
        depthStencilStateCreateInfo);

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

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo;
    Cookbook::SpecifyGraphicsPipelineCreateParameter(0,
        shaderStageCreateInfos,
        vertexInputStateCreateInfo,
        inputAssemblyStateCreateInfo,
        nullptr,
        &viewportStateCreateInfo,
        rasterizationStateCreateInfo,
        &multisampleStateCreateInfo,
        &depthStencilStateCreateInfo,
        &blendStateCraeteInfo,
        &dynamicStateCreateInfo,
        *_graphicsPipelineLayout,
        *_renderPass,
        0,
        VK_NULL_HANDLE,
        -1,
        graphicsPipelineCreateInfo);

    std::vector<VkPipeline> graphicsPipeline;
    if (Cookbook::CreateGraphicsPipeline(*_logicalDevice, {graphicsPipelineCreateInfo}, VK_NULL_HANDLE, graphicsPipeline) == false)
    {
        return false;
    }

    InitVkDestroyer(_logicalDevice, _graphicsPipeline);
    *_graphicsPipeline = graphicsPipeline[0];

    return true;
}

bool SampleLighting01::Draw(void)
{
    auto prepareFrame = [this](VkCommandBuffer commandBuffer, uint32_t swapchainImageIndex, VkFramebuffer framebuffer) -> bool
    {
        if (Cookbook::BeginCommandBufferRecordingOperation(commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr) == false)
        {
            return false;
        }

        if (_updateUniformBuffer == true)
        {
            _updateUniformBuffer = false;

            BufferTransition preTransferTransition = 
            {
                *_uniformBuffer,
                VK_ACCESS_UNIFORM_READ_BIT,
                VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_QUEUE_FAMILY_IGNORED,
                VK_QUEUE_FAMILY_IGNORED,
            };

            Cookbook::SetBufferMemoryBarrier(commandBuffer, 
                VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                {preTransferTransition});

            std::vector<VkBufferCopy> regions = 
            {
                {0, 0, 2 * 16 * sizeof(float)}
            };
            Cookbook::CopyDataBetweenBuffer(commandBuffer, *_stagingBuffer, *_uniformBuffer, regions);

            BufferTransition postTransferTransition = 
            {
                *_uniformBuffer,                
                VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_ACCESS_UNIFORM_READ_BIT,
                VK_QUEUE_FAMILY_IGNORED,
                VK_QUEUE_FAMILY_IGNORED,
            };
            Cookbook::SetBufferMemoryBarrier(commandBuffer, 
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
                {postTransferTransition});
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
            {{0.1f, 0.2f, 0.3f, 1.0f}, {1.0f, 0.0f}},
            VK_SUBPASS_CONTENTS_INLINE);

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

        Cookbook::BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *_graphicsPipelineLayout, 0, _descriptorSets, {});
        Cookbook::BindVertexBuffer(commandBuffer, 0, {{*_vertexBuffer, 0}});        
        Cookbook::BindPipelineObject(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *_graphicsPipeline);

        for (size_t idx = 0; idx < _model.parts.size(); ++idx)
        {
            Cookbook::DrawGeometry(commandBuffer, _model.parts[idx].vertexCount, 1, _model.parts[idx].vertexOffset, 0);
        }

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

bool SampleLighting01::Resize(void)
{
    if (CreateSwapchain(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) == false)
    {
        return false;
    }

    if (IsReady() == true)
    {
        if (_UpdateUniformbuffer(true) == false)
        {
            return false;
        }
    }
    
    return true;
}

void SampleLighting01::OnMouseEvent(void)
{
    _UpdateUniformbuffer(false);
}

bool SampleLighting01::_UpdateUniformbuffer(bool force)
{
    _updateUniformBuffer = true;
    static float horizontalAngle = 0.0f;
    static float verticalAngle = 0.0f;

    if (_mouseState.buttons[0].isPressed == true || force)
    {
        horizontalAngle += 0.5f * _mouseState.position.delta.x;
        verticalAngle -= 0.5f * _mouseState.position.delta.y;

        if (verticalAngle > 90.0f)
        {
            verticalAngle = 90.0f;
        }
        if (verticalAngle < -90.0f)
        {
            verticalAngle = -90.0f;
        }
    }

    Matrix4x4 rotationMatrix = Cookbook::PrepareRotationMatrix(verticalAngle, {1.0f, 0.0f, 0.0f})
        * Cookbook::PrepareRotationMatrix(horizontalAngle, {0.0f, -1.0f, 0.0f});
    Matrix4x4 translationMatrix = Cookbook::PrepareTranslationMatrix(0.0f, 0.0f, -4.0f);
    Matrix4x4 movelViewMatrix = translationMatrix * rotationMatrix;

    if (Cookbook::MapUpdateAndUnmapHostVisibleMemory(*_logicalDevice, 
        *_stagingBufferMemory, 
        0, 
        sizeof(movelViewMatrix[0]) *  movelViewMatrix.size(),
        &movelViewMatrix[0],
        true,
        nullptr) == false)
    {
        return false;
    }

    Matrix4x4 perspectiveMatrix = Cookbook::PreparePerspectiveProjectionMatrix(static_cast<float>(_swapchain.size.width) / static_cast<float>(_swapchain.size.height), 50.0f, 0.5f, 10.0f);
    if (Cookbook::MapUpdateAndUnmapHostVisibleMemory(*_logicalDevice, 
        *_stagingBufferMemory, 
        sizeof(movelViewMatrix[0]) *  movelViewMatrix.size(),
        sizeof(perspectiveMatrix[0]) *  perspectiveMatrix.size(),
        &perspectiveMatrix[0],
        true,
        nullptr) == false)
    {
        return false;
    }

    return true;
}
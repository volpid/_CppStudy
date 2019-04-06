
#include "../../Chapter/ch03_command_buffer_and_synchronization.h"
#include "../../Chapter/ch04_resource_and_memory.h"
#include "../../Chapter/ch05_descriptor_sets.h"
#include "../../Chapter/ch06_renderpass_and_framebuffer.h"
#include "../../Chapter/ch08_graphics_and_compute_pipeline.h"
#include "../../Chapter/ch09_command_recording_and_drawing.h"
#include "../../Chapter/ch10_helper_recipes.h"

#include "../../Common/sample_framework.h"
#include "../../Common/sample_orbiting_camera.h"
#include "../../Common/vulkan_tool.h"

#include <array>

//----------------------------------------------------------------
// SampleAdvance03 : drawing particle using compute and graphics pipeline
//----------------------------------------------------------------
class SampleAdvance03 : public VulkanSample
{
public:
    virtual bool Initialize(WindowParameters parameters) override;
    virtual bool Draw(void) override;
    virtual bool Resize(void) override;    

    virtual void OnMouseEvent(void);

private:
    bool _UpdateUniformbuffer(bool force);

private: 
    const uint32_t ParticleCount = 2000;

    OrbitingCamera _camera;

    VkDestroyer(VkCommandPool) _computeCommandPool;
    VkCommandBuffer _computeCommandBuffer;

    VkDestroyer(VkBuffer) _vertexBuffer;
    VkDestroyer(VkDeviceMemory) _vertexBufferMemory;
    VkDestroyer(VkBufferView) _vertexBufferView;
    
    VkDestroyer(VkBuffer) _stagingBuffer;
    VkDestroyer(VkDeviceMemory) _stagingBufferMemory;
    bool _updateUniformBuffer;
    VkDestroyer(VkBuffer) _uniformBuffer;
    VkDestroyer(VkDeviceMemory) _uniformBufferMemory;
    
    std::vector<VkDestroyer(VkDescriptorSetLayout)> _descriptorSetLayout;
    VkDestroyer(VkDescriptorPool) _descriptorPool;
    std::vector<VkDescriptorSet> _descriptorSets;

    VkDestroyer(VkRenderPass) _renderPass;    

    VkDestroyer(VkPipelineLayout) _computePipelineLayout;        
    VkDestroyer(VkPipeline) _computePipeline;
    VkDestroyer(VkSemaphore) _computeSemaphore;
    VkDestroyer(VkFence) _computeFence;

    VkDestroyer(VkPipelineLayout) _graphicsPipelineLayout;        
    VkDestroyer(VkPipeline) _graphicsPipeline;
};

//VULKAN_SAMPLE_FRAMEWORK("03_drawing_particles_using_compute_and_graphics_pipeline", 50, 25, 800, 600, SampleAdvance03);

//----------------------------------------------------------------
bool SampleAdvance03::Initialize(WindowParameters parameters)
{   
    VkPhysicalDeviceFeatures deviceFeature = {};
    deviceFeature.geometryShader = true;
    if (InitializeVulkan(parameters, &deviceFeature) == false)
    {
        return false;
    }
    
    _camera = OrbitingCamera(Vector3{0.0f, 0.0f, 0.0f}, 3.0f);

    InitVkDestroyer(_logicalDevice, _computeCommandPool);
    if (Cookbook::CreateCommandPool(*_logicalDevice, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, _computeQueue.familyIndex, *_computeCommandPool) == false)
    {
        return false;
    }

    std::vector<VkCommandBuffer> commandBuffers;
    if (Cookbook::AllocateCommandBuffer(*_logicalDevice, *_computeCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, commandBuffers) == false)
    {
        return false;
    }
    _computeCommandBuffer = commandBuffers[0];

    {
        std::vector<float> particles;
        for (uint32_t idx = 0; idx < ParticleCount; ++idx)
        {
            OrbitingCamera particle({0.0f, 0.0f, 0.0f}, 1.5f, static_cast<float>((std::rand() % 181) - 90), static_cast<float>((std::rand() % 51) - 25));
            
            Vector3 position = particle.GetPosition();
            Vector3 randColor = 
            {
                250.0f - std::abs(particle.GetVerticalAngle() * 10.0f),
                static_cast<float>(std::rand() % 61 + 40),
                static_cast<float>(std::rand() % 61 + 40)
            };
            Vector3 color = 0.0075f * randColor;

            float speed = 0.5f + 0.01f * static_cast<float>(std::rand() % 101) + color[0] * 0.5f;
            particles.insert(particles.end(), position.begin(), position.end());
            particles.push_back(1.0f);
            particles.insert(particles.end(), color.begin(), color.end());
            particles.push_back(speed);
        }

        
        InitVkDestroyer(_logicalDevice, _vertexBuffer);
        InitVkDestroyer(_logicalDevice, _vertexBufferMemory);
        InitVkDestroyer(_logicalDevice, _vertexBufferView);
        if (Cookbook::CreateStorageTexelBuffer(_physicalDevice,
            *_logicalDevice,
            VK_FORMAT_R32G32B32A32_SFLOAT,
            sizeof(particles[0]) * particles.size(),
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT,
            false,
            *_vertexBuffer,
            *_vertexBufferMemory,
            *_vertexBufferView) == false)
        {
            return false;
        }

        if (Cookbook::UseStagingBufferToUpdateBufferWithDeviceLocalMemoryBound(_physicalDevice,
            *_logicalDevice,
            sizeof(particles[0]) * particles.size(),
            particles.data(),
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

    std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBinding = 
    {
        {
            0,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            1,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT,
            nullptr
        },
        {
            0,
            VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
            1,
            VK_SHADER_STAGE_COMPUTE_BIT,
            nullptr
        }
    };

    _descriptorSetLayout.resize(2);
    InitVkDestroyer(_logicalDevice, _descriptorSetLayout[0]);
    InitVkDestroyer(_logicalDevice, _descriptorSetLayout[1]);
    if (Cookbook::CreateDescriptorSetLayout(*_logicalDevice, {descriptorSetLayoutBinding[0]}, *_descriptorSetLayout[0]) == false)
    {
        return false;
    }
    if (Cookbook::CreateDescriptorSetLayout(*_logicalDevice, {descriptorSetLayoutBinding[1]}, *_descriptorSetLayout[1]) == false)
    {
        return false;
    }
    
    std::vector<VkDescriptorPoolSize> descriptorPoolSize = 
    {
        {
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            1
        },
        {
            VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
            1
        },
    };

    InitVkDestroyer(_logicalDevice, _descriptorPool);
    if (Cookbook::CreateDescriptorPool(*_logicalDevice, false, 2, descriptorPoolSize, *_descriptorPool) == false)
    {
        return false;
    }

    if (Cookbook::AllocateDescriptorSets(*_logicalDevice, 
        *_descriptorPool, 
        {*_descriptorSetLayout[0], *_descriptorSetLayout[1]}, 
        _descriptorSets) == false)
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

    Cookbook::TexelBufferDescriptorInfo storageTexelBufferDescriptorUpdate =
    {
        _descriptorSets[1],
        0,
        0,
        VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
        {
            {
                *_vertexBufferView
            }
        }
    };

    Cookbook::UpdateDescriptorSets(*_logicalDevice, {}, {bufferDescriptorUpdate}, {storageTexelBufferDescriptorUpdate}, {});

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

    std::vector<unsigned char> computeShaderSpirv;
    if (GetBinaryFileContent("..\\..\\Source\\VulkanCookbook\\Shader\\Advance\\03_shader.comp.spv", computeShaderSpirv) == false)
    {
        return false;
    }

    VkDestroyer(VkShaderModule) computeShaderModule;
    InitVkDestroyer(_logicalDevice, computeShaderModule);
    if (Cookbook::CreateShader(*_logicalDevice, computeShaderSpirv, *computeShaderModule) == false)
    {
        return false;
    }

    std::vector<ShaderStageParameter> comnputeShaderStageParams =
    {
        {
            VK_SHADER_STAGE_COMPUTE_BIT,
            *computeShaderModule,
            "main",
            nullptr
        }
    };
    
    std::vector<VkPipelineShaderStageCreateInfo> computeShaderStageCreateInfos;
    Cookbook::SpecifyPipelineShaderStage(comnputeShaderStageParams, computeShaderStageCreateInfos);

    std::vector<VkPushConstantRange> pushConstantRanges =
    {
        {
            VK_SHADER_STAGE_COMPUTE_BIT,
            0,
            sizeof(float)
        }
    };

    InitVkDestroyer(_logicalDevice, _computePipelineLayout);
    if (Cookbook::CreatePipelineLayout(*_logicalDevice, {*_descriptorSetLayout[1]}, pushConstantRanges, *_computePipelineLayout) == false)
    {
        return false;
    }

    InitVkDestroyer(_logicalDevice, _computePipeline);
    if (Cookbook::CreateComputePipeline(*_logicalDevice,
        0,
        computeShaderStageCreateInfos[0],
        *_computePipelineLayout,
        VK_NULL_HANDLE,
        VK_NULL_HANDLE,
        *_computePipeline) == false)
    {
        return false;
    }
            
    std::vector<unsigned char> vertexShaderSpirv;
    if (GetBinaryFileContent("..\\..\\Source\\VulkanCookbook\\Shader\\Advance\\03_shader.vert.spv", vertexShaderSpirv) == false)
    {
        return false;
    }

    VkDestroyer(VkShaderModule) vertexShaderModule;
    InitVkDestroyer(_logicalDevice, vertexShaderModule);
    if (Cookbook::CreateShader(*_logicalDevice, vertexShaderSpirv, *vertexShaderModule) == false)
    {
        return false;
    }

    std::vector<unsigned char> geometryShaderSpirv;
    if (GetBinaryFileContent("..\\..\\Source\\VulkanCookbook\\Shader\\Advance\\03_shader.geom.spv", geometryShaderSpirv) == false)
    {
        return false;
    }

    VkDestroyer(VkShaderModule) geometryShaderModule;
    InitVkDestroyer(_logicalDevice, geometryShaderModule);
    if (Cookbook::CreateShader(*_logicalDevice, geometryShaderSpirv, *geometryShaderModule) == false)
    {
        return false;
    }

    std::vector<unsigned char> fragmentShaderSpirv;
    if (GetBinaryFileContent("..\\..\\Source\\VulkanCookbook\\Shader\\Advance\\03_shader.frag.spv", fragmentShaderSpirv) == false)
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
            VK_SHADER_STAGE_GEOMETRY_BIT,
            *geometryShaderModule,
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
            8 * sizeof(float),
            VK_VERTEX_INPUT_RATE_VERTEX,
        }
    };

    std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions = 
    {
        {
            0,
            0,
            VK_FORMAT_R32G32B32A32_SFLOAT,
            0
        },
        {
            1,
            0,
            VK_FORMAT_R32G32B32A32_SFLOAT,
            4 * sizeof(float)
        }
    };

    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo;
    Cookbook::SpecifyPipelineVertexInputState(vertexInputBindingDescriptions, 
        vertexAttributeDescriptions,
        vertexInputStateCreateInfo);
        
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo;
    Cookbook::SpecifyPipelineInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_POINT_LIST, false, inputAssemblyStateCreateInfo);

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

    InitVkDestroyer(_logicalDevice, _graphicsPipelineLayout);
    if (Cookbook::CreatePipelineLayout(*_logicalDevice, {*_descriptorSetLayout[0]}, {}, *_graphicsPipelineLayout) == false)
    {
        return false;
    }

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

    InitVkDestroyer(_logicalDevice, _computeSemaphore);
    if (Cookbook::CreateSemaphore(*_logicalDevice, *_computeSemaphore) == false)
    {
        return false;
    }

    InitVkDestroyer(_logicalDevice, _computeFence);
    if (Cookbook::CreateFence(*_logicalDevice, true, *_computeFence) == false)
    {
        return false;
    }

    return true;
}

bool SampleAdvance03::Draw(void)
{
    WaitSemaphoreInfo waitSemaphoreInfo =
    {
        *_computeSemaphore,
        VK_PIPELINE_STAGE_VERTEX_INPUT_BIT
    };

    if (Cookbook::WaitForFence(*_logicalDevice, {*_computeFence}, VK_FALSE, 2'000'000'000) == false)
    {
        return false;
    }

    if (Cookbook::ResetFence(*_logicalDevice, {*_computeFence}) == false)
    {
        return false;
    }

    if (Cookbook::BeginCommandBufferRecordingOperation(_computeCommandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr) == false)
    {
        return false;
    }
    
    Cookbook::BindPipelineObject(_computeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, *_computePipeline);
    Cookbook::BindDescriptorSets(_computeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, *_computePipelineLayout, 0, {_descriptorSets[1]}, {});
    
    float time = _timerState.GetDeltaTime();
    Cookbook::ProvideDataToShaderThroughPushConstants(_computeCommandBuffer, 
        *_computePipelineLayout,
        VK_SHADER_STAGE_COMPUTE_BIT,
        0, 
        sizeof(float),
        &time);

    Cookbook::DispatchComputeWork(_computeCommandBuffer, ParticleCount / 32 + 1, 1, 1);

    if (Cookbook::EndCommandBufferRecordingOperation(_computeCommandBuffer) == false)
    {
        return false;
    }

    if (Cookbook::SubmitCommandBufferToQueue(_computeQueue.handle, {}, {_computeCommandBuffer}, {*_computeSemaphore}, {*_computeFence}) == false)
    {
        return false;
    }

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

        Cookbook::BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *_graphicsPipelineLayout, 0, {_descriptorSets[0]}, {});
        Cookbook::BindVertexBuffer(commandBuffer, 0, {{*_vertexBuffer, 0}});        

        Cookbook::DrawGeometry(commandBuffer, ParticleCount, 1, 0, 0);        

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
        {waitSemaphoreInfo},
        prepareFrame,
        _frameResources);
}

bool SampleAdvance03::Resize(void)
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

void SampleAdvance03::OnMouseEvent(void)
{
    _UpdateUniformbuffer(false);
}

bool SampleAdvance03::_UpdateUniformbuffer(bool force)
{
    if (_mouseState.buttons[0].isPressed == true)
    {
        _camera.RotateHorizontally(0.5f * _mouseState.position.delta.x);
        _camera.RotateVertically(-0.5f * _mouseState.position.delta.y);

        force = true;
    }

    if (force == true)
    {
        _updateUniformBuffer = true;
        Matrix4x4 viewMatrix = _camera.GetMatrix();
        if (Cookbook::MapUpdateAndUnmapHostVisibleMemory(*_logicalDevice, 
            *_stagingBufferMemory, 
            0, 
            sizeof(viewMatrix[0]) *  viewMatrix.size(),
            &viewMatrix[0],
            true,
            nullptr) == false)
        {
            return false;
        }

        Matrix4x4 perspectiveMatrix = Cookbook::PreparePerspectiveProjectionMatrix(static_cast<float>(_swapchain.size.width) / static_cast<float>(_swapchain.size.height), 50.0f, 0.5f, 10.0f);
        if (Cookbook::MapUpdateAndUnmapHostVisibleMemory(*_logicalDevice, 
            *_stagingBufferMemory, 
            sizeof(viewMatrix[0]) *  viewMatrix.size(),
            sizeof(perspectiveMatrix[0]) *  perspectiveMatrix.size(),
            &perspectiveMatrix[0],
            true,
            nullptr) == false)
        {
            return false;
        }
    }

    return true;
}
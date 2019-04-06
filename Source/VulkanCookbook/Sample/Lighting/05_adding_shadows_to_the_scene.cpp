
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
// SampleLighting05 : adding shadows to the scene
//----------------------------------------------------------------
class SampleLighting05 : public VulkanSample
{
public:
    virtual bool Initialize(WindowParameters parameters) override;
    virtual bool Draw(void) override;
    virtual bool Resize(void) override;    

    virtual void OnMouseEvent(void);

private:
    bool _UpdateUniformbuffer(bool force);

private: 
    OrbitingCamera _camera;
    OrbitingCamera _lightSource;

    std::array<Cookbook::Mesh, 2> _scene;

    VkDestroyer(VkBuffer) _vertexBuffer;
    VkDestroyer(VkDeviceMemory) _vertexBufferMemory;

    VkDestroyer(VkBuffer) _stagingBuffer;
    VkDestroyer(VkDeviceMemory) _stagingBufferMemory;
    bool _updateUniformBuffer;
    VkDestroyer(VkBuffer) _uniformBuffer;
    VkDestroyer(VkDeviceMemory) _uniformBufferMemory;

    struct ShadowMapParameter
    {
        VkDestroyer(VkImage) image;
        VkDestroyer(VkDeviceMemory) imageMemory;
        VkDestroyer(VkImageView) imageView;
        VkDestroyer(VkFramebuffer) frameBuffer;
        
    };
    ShadowMapParameter _shadowMap;
    VkDestroyer(VkSampler) _shadowMapSampler;

    VkDestroyer(VkDescriptorSetLayout) _descriptorSetLayout;
    VkDestroyer(VkDescriptorPool) _descriptorPool;
    std::vector<VkDescriptorSet> _descriptorSets;

    VkDestroyer(VkPipelineLayout) _pipelineLayout;
    
    VkDestroyer(VkRenderPass) _shadowMapRenderPass;    
    VkDestroyer(VkPipeline) _shadowMapPipeline;
    VkDestroyer(VkRenderPass) _sceneRenderPass;    
    VkDestroyer(VkPipeline) _scenePipeline;
};

//VULKAN_SAMPLE_FRAMEWORK("05_adding_shadows_to_the_scene", 50, 25, 800, 600, SampleLighting05);

//----------------------------------------------------------------
bool SampleLighting05::Initialize(WindowParameters parameters)
{   
    if (InitializeVulkan(parameters, nullptr, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, false) == false)
    {
        return false;
    }

    _camera = OrbitingCamera(Vector3{0.0f, 0.0f, 0.0f}, 4.0f);
    _lightSource = OrbitingCamera(Vector3{0.0f, 0.0f, 0.0f}, 4.0f, 0.0f, -80.0f);

    if (Cookbook::Load3DModelFromObjFile("../../Resource/VulkanCookbook/Model/knot.obj", true, false, false, true, _scene[0]) == false)
    {
        return false;
    }

    if (Cookbook::Load3DModelFromObjFile("../../Resource/VulkanCookbook/Model/plane.obj", true, false, false, false, _scene[1]) == false)
    {
        return false;
    }

    std::vector<float> vertexData(_scene[0].data);
    vertexData.insert(vertexData.end(), _scene[1].data.begin(), _scene[1].data.end());

    InitVkDestroyer(_logicalDevice, _vertexBuffer);
    if (Cookbook::CreateBuffer(*_logicalDevice, 
        sizeof(vertexData[0]) * vertexData.size(),
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        *_vertexBuffer) == false)
    {
        return false;
    }
        
    InitVkDestroyer(_logicalDevice, _vertexBufferMemory);
    if (Cookbook::AllocateAndBindMemoryObjectToBuffer(_physicalDevice,
        *_logicalDevice,
        *_vertexBuffer,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        *_vertexBufferMemory) == false)
    {
        return false;
    }

    if (Cookbook::UseStagingBufferToUpdateBufferWithDeviceLocalMemoryBound(_physicalDevice,
        *_logicalDevice,
        sizeof(vertexData[0]) * vertexData.size(),
        &vertexData[0],
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
        3 * 16 * sizeof(float),
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
        3 * 16 * sizeof(float),
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

    InitVkDestroyer(_logicalDevice, _shadowMap.image);
    InitVkDestroyer(_logicalDevice, _shadowMap.imageView);
    InitVkDestroyer(_logicalDevice, _shadowMap.imageMemory);
    InitVkDestroyer(_logicalDevice, _shadowMapSampler);
    if (Cookbook::CreateCombinedImageSampler(_physicalDevice,
        *_logicalDevice,
        VK_IMAGE_TYPE_2D,
        DepthFormat, 
        {(uint32_t) 512, (uint32_t) 512, 1},
        1,
        1,
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        false,
        VK_IMAGE_VIEW_TYPE_2D,
        VK_IMAGE_ASPECT_DEPTH_BIT,
        VK_FILTER_LINEAR,
        VK_FILTER_LINEAR,
        VK_SAMPLER_MIPMAP_MODE_NEAREST,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        0.0f, 
        false,
        1.0f, 
        false,
        VK_COMPARE_OP_ALWAYS,
        0.0f, 
        1.0f,
        VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
        false,
        *_shadowMapSampler,
        *_shadowMap.image,
        *_shadowMap.imageMemory,
        *_shadowMap.imageView) == false)
    {
        return false;
    }

    std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBinding = 
    {
        {
            0,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            1,
            VK_SHADER_STAGE_VERTEX_BIT,
            nullptr
        },
        {
            1,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            1,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            nullptr
        }
    };

    InitVkDestroyer(_logicalDevice, _descriptorSetLayout);
    if (Cookbook::CreateDescriptorSetLayout(*_logicalDevice, descriptorSetLayoutBinding, *_descriptorSetLayout) == false)
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
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            1
        },
    };

    InitVkDestroyer(_logicalDevice, _descriptorPool);
    if (Cookbook::CreateDescriptorPool(*_logicalDevice, false, 1, descriptorPoolSize, *_descriptorPool) == false)
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

    Cookbook::ImageDesciptorInfo imageDescriptorUpdate = 
    {
        _descriptorSets[0],
        1,
        0,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        {
            {
                *_shadowMapSampler,
                *_shadowMap.imageView,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
            }
        }
    };
    
    Cookbook::UpdateDescriptorSets(*_logicalDevice, {imageDescriptorUpdate}, {bufferDescriptorUpdate}, {}, {});

    std::vector<VkAttachmentDescription> shadowMapAttachmentDescription =
    {
        {
            0,
            DepthFormat,
            VK_SAMPLE_COUNT_1_BIT,
            VK_ATTACHMENT_LOAD_OP_CLEAR,
            VK_ATTACHMENT_STORE_OP_STORE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
        }
    };

    VkAttachmentReference shadowMapDepthAttachment = 
    {
        0,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    std::vector<SubpassParameter> shadowMapSubpassParameters =
    {
        {
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            {},
            {},
            {},
            &shadowMapDepthAttachment,
            {}
        }
    };

    std::vector<VkSubpassDependency> shadowMapSubpassDependencies = 
    {
        {
            VK_SUBPASS_EXTERNAL,
            0, 
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            VK_ACCESS_SHADER_READ_BIT,
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_DEPENDENCY_BY_REGION_BIT
        },
        {
            0, 
            VK_SUBPASS_EXTERNAL,
            VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_ACCESS_SHADER_READ_BIT,
            VK_DEPENDENCY_BY_REGION_BIT
        }
    };

    InitVkDestroyer(_logicalDevice, _shadowMapRenderPass);
    if (Cookbook::CreateRenderpass(*_logicalDevice,
        shadowMapAttachmentDescription, 
        shadowMapSubpassParameters,
        shadowMapSubpassDependencies,
        *_shadowMapRenderPass) == false)
    {
        return false;
    }

    InitVkDestroyer(_logicalDevice, _shadowMap.frameBuffer);
    if (Cookbook::CreateFramebuffer(*_logicalDevice, *_shadowMapRenderPass, {*_shadowMap.imageView}, 512, 512, 1, *_shadowMap.frameBuffer) == false)
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
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
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
            VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_ACCESS_SHADER_READ_BIT,            
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

    InitVkDestroyer(_logicalDevice, _sceneRenderPass);
    if (Cookbook::CreateRenderpass(*_logicalDevice,
        attachmentDescription, 
        subpassParameters,
        subpassDependencies,
        *_sceneRenderPass) == false)
    {
        return false;
    }

    std::vector<VkPushConstantRange> pushConstantRanges =
    {
        {
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(float) * 4
        }
    };

    InitVkDestroyer(_logicalDevice, _pipelineLayout);
    if (Cookbook::CreatePipelineLayout(*_logicalDevice, {*_descriptorSetLayout}, pushConstantRanges, *_pipelineLayout) == false)
    {
        return false;
    }

    std::vector<unsigned char> sceneVertexShaderSpirv;
    if (GetBinaryFileContent("..\\..\\Source\\VulkanCookbook\\Shader\\Lighting\\05_scene.vert.spv", sceneVertexShaderSpirv) == false)
    {
        return false;
    }

    VkDestroyer(VkShaderModule) sceneVertexShaderModule;
    InitVkDestroyer(_logicalDevice, sceneVertexShaderModule);
    if (Cookbook::CreateShader(*_logicalDevice, sceneVertexShaderSpirv, *sceneVertexShaderModule) == false)
    {
        return false;
    }

    std::vector<unsigned char> sceneFragmentShaderSpirv;
    if (GetBinaryFileContent("..\\..\\Source\\VulkanCookbook\\Shader\\Lighting\\05_scene.frag.spv", sceneFragmentShaderSpirv) == false)
    {
        return false;
    }

    VkDestroyer(VkShaderModule) scenelFagmentShaderModule;
    InitVkDestroyer(_logicalDevice, scenelFagmentShaderModule);
    if (Cookbook::CreateShader(*_logicalDevice, sceneFragmentShaderSpirv, *scenelFagmentShaderModule) == false)
    {
        return false;
    }

    std::vector<ShaderStageParameter> sceneShaderStageParams =
    {
        {
            VK_SHADER_STAGE_VERTEX_BIT,
            *sceneVertexShaderModule,
            "main",
            nullptr
        },
        {
            VK_SHADER_STAGE_FRAGMENT_BIT,
            *scenelFagmentShaderModule,
            "main",
            nullptr
        }
    };
    
    std::vector<VkPipelineShaderStageCreateInfo> sceneShaderStageCreateInfos;
    Cookbook::SpecifyPipelineShaderStage(sceneShaderStageParams, sceneShaderStageCreateInfos);

    std::vector<VkVertexInputBindingDescription> sceneVertexInputBindingDescriptions =
    {
        {
            0,
            6 * sizeof(float),
            VK_VERTEX_INPUT_RATE_VERTEX,
        }
    };

    std::vector<VkVertexInputAttributeDescription> sceneVertexAttributeDescriptions = 
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

    VkPipelineVertexInputStateCreateInfo sceneVertexInputStateCreateInfo;
    Cookbook::SpecifyPipelineVertexInputState(sceneVertexInputBindingDescriptions, 
        sceneVertexAttributeDescriptions,
        sceneVertexInputStateCreateInfo);
        
    std::vector<unsigned char> shadowVertexShaderSpirv;
    if (GetBinaryFileContent("..\\..\\Source\\VulkanCookbook\\Shader\\Lighting\\05_shadow.vert.spv", shadowVertexShaderSpirv) == false)
    {
        return false;
    }

    VkDestroyer(VkShaderModule) shadowVertexShaderModule;
    InitVkDestroyer(_logicalDevice, shadowVertexShaderModule);
    if (Cookbook::CreateShader(*_logicalDevice, shadowVertexShaderSpirv, *shadowVertexShaderModule) == false)
    {
        return false;
    }

    std::vector<ShaderStageParameter> shadowShaderStageParams =
    {
        {
            VK_SHADER_STAGE_VERTEX_BIT,
            *shadowVertexShaderModule,
            "main",
            nullptr
        }
    };
    
    std::vector<VkPipelineShaderStageCreateInfo> shadowShaderStageCreateInfos;
    Cookbook::SpecifyPipelineShaderStage(shadowShaderStageParams, shadowShaderStageCreateInfos);

    std::vector<VkVertexInputBindingDescription> shadowVertexInputBindingDescriptions =
    {
        {
            0,
            6 * sizeof(float),
            VK_VERTEX_INPUT_RATE_VERTEX,
        }
    };

    std::vector<VkVertexInputAttributeDescription> shadowVertexAttributeDescriptions = 
    {
        {
            0,
            0,
            VK_FORMAT_R32G32B32_SFLOAT,
            0
        }
    };

    VkPipelineVertexInputStateCreateInfo shadowVertexInputStateCreateInfo;
    Cookbook::SpecifyPipelineVertexInputState(shadowVertexInputBindingDescriptions, 
        shadowVertexAttributeDescriptions,
        shadowVertexInputStateCreateInfo);

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo;
    Cookbook::SpecifyPipelineInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, false, inputAssemblyStateCreateInfo);

    ViewportInfo viewportInfos =
    {
        {
            {
                0.0f,
                0.0f,
                512.0f,
                512.0f,
                0.0f,
                1.0f,
            }
        },
        {
            {
                {0, 0},
                {512, 512}
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

    VkGraphicsPipelineCreateInfo scenePipelineCreateInfo;
    Cookbook::SpecifyGraphicsPipelineCreateParameter(0,
        sceneShaderStageCreateInfos,
        sceneVertexInputStateCreateInfo,
        inputAssemblyStateCreateInfo,
        nullptr,
        &viewportStateCreateInfo,
        rasterizationStateCreateInfo,
        &multisampleStateCreateInfo,
        &depthStencilStateCreateInfo,
        &blendStateCraeteInfo,
        &dynamicStateCreateInfo,
        *_pipelineLayout,
        *_sceneRenderPass,
        0,
        VK_NULL_HANDLE,
        -1,
        scenePipelineCreateInfo);
        
    std::vector<VkPipeline> scenePipeline;
    if (Cookbook::CreateGraphicsPipeline(*_logicalDevice, {scenePipelineCreateInfo}, VK_NULL_HANDLE, scenePipeline) == false)
    {
        return false;
    }

    InitVkDestroyer(_logicalDevice, _scenePipeline);
    *_scenePipeline = scenePipeline[0];

    VkGraphicsPipelineCreateInfo shadowPipelineCreateInfo;
    Cookbook::SpecifyGraphicsPipelineCreateParameter(0,
        shadowShaderStageCreateInfos,
        shadowVertexInputStateCreateInfo,
        inputAssemblyStateCreateInfo,
        nullptr,
        &viewportStateCreateInfo,
        rasterizationStateCreateInfo,
        &multisampleStateCreateInfo,
        &depthStencilStateCreateInfo,
        nullptr,
        nullptr,
        *_pipelineLayout,
        *_shadowMapRenderPass,
        0,
        VK_NULL_HANDLE,
        -1,
        shadowPipelineCreateInfo);
        
    std::vector<VkPipeline> shadowPipeline;
    if (Cookbook::CreateGraphicsPipeline(*_logicalDevice, {shadowPipelineCreateInfo}, VK_NULL_HANDLE, shadowPipeline) == false)
    {
        return false;
    }

    InitVkDestroyer(_logicalDevice, _shadowMapPipeline);
    *_shadowMapPipeline = shadowPipeline[0];

    return true;
}

bool SampleLighting05::Draw(void)
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
                {0, 0, 3 * 16 * sizeof(float)}
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

        Cookbook::BeginRenderPass(commandBuffer, 
            *_shadowMapRenderPass, 
            *_shadowMap.frameBuffer, 
            {{0, 0}, {512, 512}}, 
            {{1.0f, 0.0f}},
            VK_SUBPASS_CONTENTS_INLINE);
        
        Cookbook::BindVertexBuffer(commandBuffer, 0, {{*_vertexBuffer, 0}});        
        Cookbook::BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *_pipelineLayout, 0, _descriptorSets, {});        
        Cookbook::BindPipelineObject(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *_shadowMapPipeline);

        Cookbook::DrawGeometry(commandBuffer, _scene[0].parts[0].vertexCount + _scene[1].parts[0].vertexCount, 1, 0, 0);

        Cookbook::EndRenderPass(commandBuffer);

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
            *_sceneRenderPass, 
            framebuffer, 
            {{0, 0}, _swapchain.size},
            {{0.1f, 0.2f, 0.3f, 1.0f}, {1.0f, 0.0f}},
            VK_SUBPASS_CONTENTS_INLINE);

        Cookbook::BindPipelineObject(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *_scenePipeline);

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

        Cookbook::ProvideDataToShaderThroughPushConstants(commandBuffer, 
            *_pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(float) * 4,
            &_lightSource.GetPosition()[0]);

        Cookbook::DrawGeometry(commandBuffer, _scene[0].parts[0].vertexCount + _scene[1].parts[0].vertexCount, 1, 0, 0);
        
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

    Cookbook::WaitForAllSubmittedCommandToBeFinished(*_logicalDevice);
    
    return Cookbook::IncreasePerformanceThroughIncreasingTheNumberOfSeparatelyRenderedFrames(*_logicalDevice,
        _graphicsQueue.handle,
        _presentQueue.handle,
        *_swapchain.handle,
        _swapchain.size,
        _swapchain.imageViewsRaw,
        *_sceneRenderPass,
        {},
        prepareFrame,
        _frameResources);
}

bool SampleLighting05::Resize(void)
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

void SampleLighting05::OnMouseEvent(void)
{
    _UpdateUniformbuffer(false);
}

bool SampleLighting05::_UpdateUniformbuffer(bool force)
{
    if (_mouseState.buttons[0].isPressed == true)
    {
        _camera.RotateHorizontally(0.5f * _mouseState.position.delta.x);
        _camera.RotateVertically(-0.5f * _mouseState.position.delta.y);
        force = true;
    }

    if (_mouseState.buttons[1].isPressed == true)
    {
        _lightSource.RotateHorizontally(0.5f * _mouseState.position.delta.x);
        _lightSource.RotateVertically(-0.5f * _mouseState.position.delta.y);
        force = true;
    }

    if (force == true)
    {
        _updateUniformBuffer = true;

        Matrix4x4 lightViewMatrix = _lightSource.GetMatrix();
        Matrix4x4 sceneViewMatrix = _camera.GetMatrix();
        Matrix4x4 perspectiveMatrix = Cookbook::PreparePerspectiveProjectionMatrix(static_cast<float>(_swapchain.size.width) / static_cast<float>(_swapchain.size.height), 50.0f, 0.5f, 10.0f);

        std::vector<float> data;
        data.insert(data.end(), &lightViewMatrix[0], &lightViewMatrix[0] + 16);
        data.insert(data.end(), &sceneViewMatrix[0], &sceneViewMatrix[0] + 16);
        data.insert(data.end(), &perspectiveMatrix[0], &perspectiveMatrix[0] + 16);

        if (Cookbook::MapUpdateAndUnmapHostVisibleMemory(*_logicalDevice, 
            *_stagingBufferMemory, 
            0, 
            sizeof(data[0]) *  data.size(),
            &data[0],
            true,
            nullptr) == false)
        {
            return false;
        }
    }

    return true;
}
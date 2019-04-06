
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
// SampleLighting04 : rendering a reflective and refractive geometry using cubemap
//----------------------------------------------------------------
class SampleLighting04 : public VulkanSample
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

    Cookbook::Mesh  _model;
    VkDestroyer(VkBuffer) _modelVertexBuffer;
    VkDestroyer(VkDeviceMemory) _modelVertexBufferMemory;

    Cookbook::Mesh  _skybox;
    VkDestroyer(VkBuffer) _skyboxVertexBuffer;
    VkDestroyer(VkDeviceMemory) _skyboxVertexBufferMemory;
    
    VkDestroyer(VkImage) _cubemapImage;
    VkDestroyer(VkDeviceMemory) _cubemapImageMemory;
    VkDestroyer(VkImageView) _cubemapImageView;
    VkDestroyer(VkSampler) _cubemapSampler;
    
    VkDestroyer(VkBuffer) _stagingBuffer;
    VkDestroyer(VkDeviceMemory) _stagingBufferMemory;
    bool _updateUniformBuffer;
    VkDestroyer(VkBuffer) _uniformBuffer;
    VkDestroyer(VkDeviceMemory) _uniformBufferMemory;

    VkDestroyer(VkDescriptorSetLayout) _descriptorSetLayout;
    VkDestroyer(VkDescriptorPool) _descriptorPool;
    std::vector<VkDescriptorSet> _descriptorSets;

    VkDestroyer(VkRenderPass) _renderPass;    
    VkDestroyer(VkPipelineLayout) _graphicsPipelineLayout;
    VkDestroyer(VkPipeline) _modelPipeline;
    VkDestroyer(VkPipeline) _skyboxPipeline;
};

//VULKAN_SAMPLE_FRAMEWORK("04_rendering_a_reflective_and_refractive_geometry_using_cuebemap", 50, 25, 800, 600, SampleLighting04);

//----------------------------------------------------------------
bool SampleLighting04::Initialize(WindowParameters parameters)
{   
    if (InitializeVulkan(parameters, nullptr, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, false) == false)
    {
        return false;
    }

    _camera = OrbitingCamera(Vector3{0.0f, 0.0f, 0.0f}, 4.0f);

    if (Cookbook::Load3DModelFromObjFile("../../Resource/VulkanCookbook/Model/teapot.obj", true, false, false, true, _model) == false)
    {
        return false;
    }

    InitVkDestroyer(_logicalDevice, _modelVertexBuffer);
    if (Cookbook::CreateBuffer(*_logicalDevice, 
        sizeof(_model.data[0]) * _model.data.size(),
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        *_modelVertexBuffer) == false)
    {
        return false;
    }
        
    InitVkDestroyer(_logicalDevice, _modelVertexBufferMemory);
    if (Cookbook::AllocateAndBindMemoryObjectToBuffer(_physicalDevice,
        *_logicalDevice,
        *_modelVertexBuffer,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        *_modelVertexBufferMemory) == false)
    {
        return false;
    }

    if (Cookbook::UseStagingBufferToUpdateBufferWithDeviceLocalMemoryBound(_physicalDevice,
        *_logicalDevice,
        sizeof(_model.data[0]) * _model.data.size(),
        &_model.data[0],
        *_modelVertexBuffer,
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
    
    if (Cookbook::Load3DModelFromObjFile("../../Resource/VulkanCookbook/Model/cube.obj", false, false, false, false, _skybox) == false)
    {
        return false;
    }

    InitVkDestroyer(_logicalDevice, _skyboxVertexBuffer);
    if (Cookbook::CreateBuffer(*_logicalDevice, 
        sizeof(_skybox.data[0]) * _skybox.data.size(),
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        *_skyboxVertexBuffer) == false)
    {
        return false;
    }
        
    InitVkDestroyer(_logicalDevice, _skyboxVertexBufferMemory);
    if (Cookbook::AllocateAndBindMemoryObjectToBuffer(_physicalDevice,
        *_logicalDevice,
        *_skyboxVertexBuffer,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        *_skyboxVertexBufferMemory) == false)
    {
        return false;
    }

    if (Cookbook::UseStagingBufferToUpdateBufferWithDeviceLocalMemoryBound(_physicalDevice,
        *_logicalDevice,
        sizeof(_skybox.data[0]) * _skybox.data.size(),
        &_skybox.data[0],
        *_skyboxVertexBuffer,
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

    InitVkDestroyer(_logicalDevice, _cubemapSampler);
    InitVkDestroyer(_logicalDevice, _cubemapImage);
    InitVkDestroyer(_logicalDevice, _cubemapImageMemory);
    InitVkDestroyer(_logicalDevice, _cubemapImageView);

    if (Cookbook::CreateCombinedImageSampler(_physicalDevice,
        *_logicalDevice,
        VK_IMAGE_TYPE_2D,
        VK_FORMAT_R8G8B8A8_UNORM, 
        {(uint32_t) 1024, (uint32_t) 1024, 1},
        1,
        6,
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        true,
        VK_IMAGE_VIEW_TYPE_CUBE,
        VK_IMAGE_ASPECT_COLOR_BIT,
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
        *_cubemapSampler,
        *_cubemapImage,
        *_cubemapImageMemory,
        *_cubemapImageView) == false)
    {
        return false;
    }

    std::vector<std::string> cubemapImages =
    {
        "../../Resource/VulkanCookbook/Texture/posx.jpg",
        "../../Resource/VulkanCookbook/Texture/negx.jpg",
        "../../Resource/VulkanCookbook/Texture/posy.jpg",
        "../../Resource/VulkanCookbook/Texture/negy.jpg",
        "../../Resource/VulkanCookbook/Texture/posz.jpg",
        "../../Resource/VulkanCookbook/Texture/negz.jpg",
    };

    for (size_t idx = 0; idx < cubemapImages.size(); ++idx)
    {
        std::vector<unsigned char> cubemapImageData;
        int imageDataSize;
        if (Cookbook::LoadTextureDataFromFile(cubemapImages[idx].c_str(), 
            4, 
            cubemapImageData, 
            nullptr,
            nullptr,
            nullptr,
            &imageDataSize) == false)
        {
            return false;
        }

        VkImageSubresourceLayers imageSubresourceLayer = 
        {
            VK_IMAGE_ASPECT_COLOR_BIT,
            0,
            static_cast<uint32_t>(idx),
            1
        };
        
        if (Cookbook::UseStagingBufferToUpdateImageWithDeviceLocalMemoryBound(_physicalDevice,
            *_logicalDevice,
            imageDataSize,
            &cubemapImageData[0], //imageData.data(),
            *_cubemapImage,
            imageSubresourceLayer,
            {0, 0, 0},
            {(uint32_t) 1024, (uint32_t) 1024, 1},
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            0,
            VK_ACCESS_SHADER_READ_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            _graphicsQueue.handle,
            _frameResources.front().commandBuffer,
            {}) == false)
        {
            return false;
        }
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
                *_cubemapSampler,
                *_cubemapImageView,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            }
        }
    };
    
    Cookbook::UpdateDescriptorSets(*_logicalDevice, {imageDescriptorUpdate}, {bufferDescriptorUpdate}, {}, {});

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

    std::vector<VkPushConstantRange> pushConstantRanges =
    {
        {
            VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(float) * 4
        }
    };

    InitVkDestroyer(_logicalDevice, _graphicsPipelineLayout);
    if (Cookbook::CreatePipelineLayout(*_logicalDevice, {*_descriptorSetLayout}, pushConstantRanges, *_graphicsPipelineLayout) == false)
    {
        return false;
    }

    std::vector<unsigned char> modelVertexShaderSpirv;
    if (GetBinaryFileContent("..\\..\\Source\\VulkanCookbook\\Shader\\Lighting\\04_model.vert.spv", modelVertexShaderSpirv) == false)
    {
        return false;
    }

    VkDestroyer(VkShaderModule) modelVertexShaderModule;
    InitVkDestroyer(_logicalDevice, modelVertexShaderModule);
    if (Cookbook::CreateShader(*_logicalDevice, modelVertexShaderSpirv, *modelVertexShaderModule) == false)
    {
        return false;
    }

    std::vector<unsigned char> modelFragmentShaderSpirv;
    if (GetBinaryFileContent("..\\..\\Source\\VulkanCookbook\\Shader\\Lighting\\04_model.frag.spv", modelFragmentShaderSpirv) == false)
    {
        return false;
    }

    VkDestroyer(VkShaderModule) modelFagmentShaderModule;
    InitVkDestroyer(_logicalDevice, modelFagmentShaderModule);
    if (Cookbook::CreateShader(*_logicalDevice, modelFragmentShaderSpirv, *modelFagmentShaderModule) == false)
    {
        return false;
    }

    std::vector<ShaderStageParameter> modelShaderStageParams =
    {
        {
            VK_SHADER_STAGE_VERTEX_BIT,
            *modelVertexShaderModule,
            "main",
            nullptr
        },
        {
            VK_SHADER_STAGE_FRAGMENT_BIT,
            *modelFagmentShaderModule,
            "main",
            nullptr
        }
    };
    
    std::vector<VkPipelineShaderStageCreateInfo> modelShaderStageCreateInfos;
    Cookbook::SpecifyPipelineShaderStage(modelShaderStageParams, modelShaderStageCreateInfos);

    std::vector<VkVertexInputBindingDescription> modelVertexInputBindingDescriptions =
    {
        {
            0,
            6 * sizeof(float),
            VK_VERTEX_INPUT_RATE_VERTEX,
        }
    };

    std::vector<VkVertexInputAttributeDescription> modelVertexAttributeDescriptions = 
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

    VkPipelineVertexInputStateCreateInfo modelVertexInputStateCreateInfo;
    Cookbook::SpecifyPipelineVertexInputState(modelVertexInputBindingDescriptions, 
        modelVertexAttributeDescriptions,
        modelVertexInputStateCreateInfo);
        
    std::vector<unsigned char> skyboxVertexShaderSpirv;
    if (GetBinaryFileContent("..\\..\\Source\\VulkanCookbook\\Shader\\Lighting\\04_skybox.vert.spv", skyboxVertexShaderSpirv) == false)
    {
        return false;
    }

    VkDestroyer(VkShaderModule) skyboxVertexShaderModule;
    InitVkDestroyer(_logicalDevice, skyboxVertexShaderModule);
    if (Cookbook::CreateShader(*_logicalDevice, skyboxVertexShaderSpirv, *skyboxVertexShaderModule) == false)
    {
        return false;
    }

    std::vector<unsigned char> skyboxFragmentShaderSpirv;
    if (GetBinaryFileContent("..\\..\\Source\\VulkanCookbook\\Shader\\Lighting\\04_skybox.frag.spv", skyboxFragmentShaderSpirv) == false)
    {
        return false;
    }

    VkDestroyer(VkShaderModule) skyboxFagmentShaderModule;
    InitVkDestroyer(_logicalDevice, skyboxFagmentShaderModule);
    if (Cookbook::CreateShader(*_logicalDevice, skyboxFragmentShaderSpirv, *skyboxFagmentShaderModule) == false)
    {
        return false;
    }

    std::vector<ShaderStageParameter> skyboxShaderStageParams =
    {
        {
            VK_SHADER_STAGE_VERTEX_BIT,
            *skyboxVertexShaderModule,
            "main",
            nullptr
        },
        {
            VK_SHADER_STAGE_FRAGMENT_BIT,
            *skyboxFagmentShaderModule,
            "main",
            nullptr
        }
    };
    
    std::vector<VkPipelineShaderStageCreateInfo> skyboxShaderStageCreateInfos;
    Cookbook::SpecifyPipelineShaderStage(skyboxShaderStageParams, skyboxShaderStageCreateInfos);

    std::vector<VkVertexInputBindingDescription> skyboxVertexInputBindingDescriptions =
    {
        {
            0,
            3 * sizeof(float),
            VK_VERTEX_INPUT_RATE_VERTEX,
        }
    };

    std::vector<VkVertexInputAttributeDescription> skyboxVertexAttributeDescriptions = 
    {
        {
            0,
            0,
            VK_FORMAT_R32G32B32_SFLOAT,
            0
        }
    };

    VkPipelineVertexInputStateCreateInfo skyboxVertexInputStateCreateInfo;
    Cookbook::SpecifyPipelineVertexInputState(skyboxVertexInputBindingDescriptions, 
        skyboxVertexAttributeDescriptions,
        skyboxVertexInputStateCreateInfo);

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
    
    VkPipelineRasterizationStateCreateInfo modelRasterizationStateCreateInfo;
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
        modelRasterizationStateCreateInfo);

    VkPipelineRasterizationStateCreateInfo skyboxRasterizationStateCreateInfo;
    Cookbook::SpecifyPipelineRasterizationState(false, 
        false,
        VK_POLYGON_MODE_FILL,
        VK_CULL_MODE_FRONT_BIT,
        VK_FRONT_FACE_COUNTER_CLOCKWISE,
        false,
        0.0f,
        0.0f,
        0.0f,
        1.0f,
        skyboxRasterizationStateCreateInfo);

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

    VkGraphicsPipelineCreateInfo modelPipelineCreateInfo;
    Cookbook::SpecifyGraphicsPipelineCreateParameter(0,
        modelShaderStageCreateInfos,
        modelVertexInputStateCreateInfo,
        inputAssemblyStateCreateInfo,
        nullptr,
        &viewportStateCreateInfo,
        modelRasterizationStateCreateInfo,
        &multisampleStateCreateInfo,
        &depthStencilStateCreateInfo,
        &blendStateCraeteInfo,
        &dynamicStateCreateInfo,
        *_graphicsPipelineLayout,
        *_renderPass,
        0,
        VK_NULL_HANDLE,
        -1,
        modelPipelineCreateInfo);
        
    std::vector<VkPipeline> modelPipeline;
    if (Cookbook::CreateGraphicsPipeline(*_logicalDevice, {modelPipelineCreateInfo}, VK_NULL_HANDLE, modelPipeline) == false)
    {
        return false;
    }

    InitVkDestroyer(_logicalDevice, _modelPipeline);
    *_modelPipeline = modelPipeline[0];

    VkGraphicsPipelineCreateInfo skyboxPipelineCreateInfo;
    Cookbook::SpecifyGraphicsPipelineCreateParameter(0,
        skyboxShaderStageCreateInfos,
        skyboxVertexInputStateCreateInfo,
        inputAssemblyStateCreateInfo,
        nullptr,
        &viewportStateCreateInfo,
        skyboxRasterizationStateCreateInfo,
        &multisampleStateCreateInfo,
        &depthStencilStateCreateInfo,
        &blendStateCraeteInfo,
        &dynamicStateCreateInfo,
        *_graphicsPipelineLayout,
        *_renderPass,
        0,
        VK_NULL_HANDLE,
        -1,
        skyboxPipelineCreateInfo);
        
    std::vector<VkPipeline> skyboxPipeline;
    if (Cookbook::CreateGraphicsPipeline(*_logicalDevice, {skyboxPipelineCreateInfo}, VK_NULL_HANDLE, skyboxPipeline) == false)
    {
        return false;
    }

    InitVkDestroyer(_logicalDevice, _skyboxPipeline);
    *_skyboxPipeline = skyboxPipeline[0];

    return true;
}

bool SampleLighting04::Draw(void)
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

        Cookbook::ProvideDataToShaderThroughPushConstants(commandBuffer, 
            *_graphicsPipelineLayout,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(float) * 4,
            &_camera.GetPosition()[0]);

        Cookbook::BindVertexBuffer(commandBuffer, 0, {{*_modelVertexBuffer, 0}});        
        Cookbook::BindPipelineObject(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *_modelPipeline);
        for (size_t idx = 0; idx < _model.parts.size(); ++idx)
        {
            Cookbook::DrawGeometry(commandBuffer, _model.parts[idx].vertexCount, 1, _model.parts[idx].vertexOffset, 0);
        }

        Cookbook::BindVertexBuffer(commandBuffer, 0, {{*_skyboxVertexBuffer, 0}});        
        Cookbook::BindPipelineObject(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *_skyboxPipeline);
        for (size_t idx = 0; idx < _skybox.parts.size(); ++idx)
        {
            Cookbook::DrawGeometry(commandBuffer, _skybox.parts[idx].vertexCount, 1, _skybox.parts[idx].vertexOffset, 0);
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

bool SampleLighting04::Resize(void)
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

void SampleLighting04::OnMouseEvent(void)
{
    _UpdateUniformbuffer(false);
}

bool SampleLighting04::_UpdateUniformbuffer(bool force)
{
    _updateUniformBuffer = true;
    if (_mouseState.buttons[0].isPressed == true || force)
    {
        _camera.RotateHorizontally(0.5f * _mouseState.position.delta.x);
        _camera.RotateVertically(-0.5f * _mouseState.position.delta.y);

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
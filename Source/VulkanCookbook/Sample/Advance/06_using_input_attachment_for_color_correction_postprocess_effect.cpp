
#include "../../Chapter/ch02_image_presentation.h"
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
// SampleAdvance07 : rendering a tesselated terrain
//----------------------------------------------------------------
class SampleAdvance07 : public VulkanSample
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

    VkDestroyer(VkImage) _sceneImage;
    VkDestroyer(VkDeviceMemory) _sceneImageMemory;
    VkDestroyer(VkImageView) _sceneImageView;
    VkDestroyer(VkFence) _sceneFence;
    
    Cookbook::Mesh _model;
    VkDestroyer(VkBuffer) _modelVertexBuffer;
    VkDestroyer(VkDeviceMemory) _modelVertexBufferMemory;
    VkDestroyer(VkBufferView) _modelVertexBufferView;

    Cookbook::Mesh _skybox;
    VkDestroyer(VkBuffer) _skyboxVertexBuffer;
    VkDestroyer(VkDeviceMemory) _skyboxVertexBufferMemory;
    VkDestroyer(VkBufferView) _skyboxVertexBufferView;

    VkDestroyer(VkBuffer) _postprocessVertexBuffer;
    VkDestroyer(VkDeviceMemory) _postprocessVertexBufferMemory;
    VkDestroyer(VkBufferView) _postprocessVertexBufferView;
        
    VkDestroyer(VkBuffer) _stagingBuffer;
    VkDestroyer(VkDeviceMemory) _stagingBufferMemory;
    bool _updateUniformBuffer;
    VkDestroyer(VkBuffer) _uniformBuffer;
    VkDestroyer(VkDeviceMemory) _uniformBufferMemory;
    
    VkDestroyer(VkImage) _cubemapImage;
    VkDestroyer(VkDeviceMemory) _cubemapImageMemory;
    VkDestroyer(VkImageView) _cubemapImageView;
    VkDestroyer(VkSampler) _cubemapSampler;

    VkDestroyer(VkDescriptorSetLayout) _descriptorSetLayout;
    VkDestroyer(VkDescriptorPool) _descriptorPool;
    std::vector<VkDescriptorSet> _descriptorSets;

    VkDestroyer(VkDescriptorSetLayout) _postprocessDescriptorSetLayout;
    VkDestroyer(VkDescriptorPool) _postprocessDescriptorPool;
    std::vector<VkDescriptorSet> _postprocessDescriptorSets;

    VkDestroyer(VkRenderPass) _renderPass;    
    VkDestroyer(VkPipelineLayout) _graphicsPipelineLayout;        
    VkDestroyer(VkPipeline) _skyboxPipeline;
    VkDestroyer(VkPipeline) _modelPipeline;
    VkDestroyer(VkPipelineLayout) _postprocessPipelineLayout;        
    VkDestroyer(VkPipeline) _postprocessPipeline;
};

VULKAN_SAMPLE_FRAMEWORK("06_using_input_attachment_for_color_correction_postprocess_effect", 50, 25, 800, 600, SampleAdvance07);

//----------------------------------------------------------------
bool SampleAdvance07::Initialize(WindowParameters parameters)
{   
    if (InitializeVulkan(parameters) == false)
    {
        return false;
    }

    _camera = OrbitingCamera(Vector3{0.0f, 0.0f, 0.0f}, 4.0f);

    InitVkDestroyer(_logicalDevice, _sceneFence);
    if (Cookbook::CreateFence(*_logicalDevice, true, *_sceneFence) == false)
    {
        return false;
    }

    if (Cookbook::Load3DModelFromObjFile("../../Resource/VulkanCookbook/Model/sphere.obj", true, false, false, true, _model) == false)
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

    std::vector<float> vertices = 
    {
        -1.0f, -1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,        
    };

    InitVkDestroyer(_logicalDevice, _postprocessVertexBuffer);
    if (Cookbook::CreateBuffer(*_logicalDevice, 
        sizeof(vertices[0]) * vertices.size(),
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        *_postprocessVertexBuffer) == false)
    {
        return false;
    }
        
    InitVkDestroyer(_logicalDevice, _postprocessVertexBufferMemory);
    if (Cookbook::AllocateAndBindMemoryObjectToBuffer(_physicalDevice,
        *_logicalDevice,
        *_postprocessVertexBuffer,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        *_postprocessVertexBufferMemory) == false)
    {
        return false;
    }

    if (Cookbook::UseStagingBufferToUpdateBufferWithDeviceLocalMemoryBound(_physicalDevice,
        *_logicalDevice,
        sizeof(vertices[0]) * _skybox.data.size(),
        &vertices[0],
        *_postprocessVertexBuffer,
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

    if (Cookbook::AllocateDescriptorSets(*_logicalDevice, 
        *_descriptorPool, 
        {*_descriptorSetLayout},
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

    std::vector<Cookbook::ImageDesciptorInfo> imageDescriptorUpdate = 
    {
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
        }
    };
    
    Cookbook::UpdateDescriptorSets(*_logicalDevice, imageDescriptorUpdate, {bufferDescriptorUpdate}, {}, {});
 
    std::vector<VkDescriptorSetLayoutBinding> sceneDescriptorSetLayoutBinding = 
    {
        {
            0,
            VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
            1,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            nullptr
        }
    };

    InitVkDestroyer(_logicalDevice, _postprocessDescriptorSetLayout);
    if (Cookbook::CreateDescriptorSetLayout(*_logicalDevice, sceneDescriptorSetLayoutBinding, *_postprocessDescriptorSetLayout) == false)
    {
        return false;
    }

    std::vector<VkDescriptorPoolSize> sceneDescriptorPoolSize = 
    {
        {
            VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
            1
        }
    };

    InitVkDestroyer(_logicalDevice, _postprocessDescriptorPool);
    if (Cookbook::CreateDescriptorPool(*_logicalDevice, false, 1, sceneDescriptorPoolSize, *_postprocessDescriptorPool) == false)
    {
        return false;
    }

    if (Cookbook::AllocateDescriptorSets(*_logicalDevice, 
        *_postprocessDescriptorPool, 
        {*_postprocessDescriptorSetLayout},
        _postprocessDescriptorSets) == false)
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
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
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
        },
        {
            0,
            _swapchain.format,
            VK_SAMPLE_COUNT_1_BIT,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VK_ATTACHMENT_STORE_OP_STORE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        },
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
        },
        {
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            {
                {
                    0, 
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                }
            },  
            {
                {
                    2, 
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
            0,
            1, 
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
            VK_DEPENDENCY_BY_REGION_BIT
        },
        {
            VK_SUBPASS_EXTERNAL,
            1, 
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_MEMORY_READ_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_DEPENDENCY_BY_REGION_BIT
        },
        {
            1, 
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

    InitVkDestroyer(_logicalDevice, _postprocessPipelineLayout);
    if (Cookbook::CreatePipelineLayout(*_logicalDevice, {*_postprocessDescriptorSetLayout}, pushConstantRanges, *_postprocessPipelineLayout) == false)
    {
        return false;
    }


    std::vector<unsigned char> modelVertexShaderSpirv;
    if (GetBinaryFileContent("..\\..\\Source\\VulkanCookbook\\Shader\\Advance\\06_model.vert.spv", modelVertexShaderSpirv) == false)
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
    if (GetBinaryFileContent("..\\..\\Source\\VulkanCookbook\\Shader\\Advance\\06_model.frag.spv", modelFragmentShaderSpirv) == false)
    {
        return false;
    }

    VkDestroyer(VkShaderModule) modelFragmentShaderModule;
    InitVkDestroyer(_logicalDevice, modelFragmentShaderModule);
    if (Cookbook::CreateShader(*_logicalDevice, modelFragmentShaderSpirv, *modelFragmentShaderModule) == false)
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
            *modelFragmentShaderModule,
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
    if (GetBinaryFileContent("..\\..\\Source\\VulkanCookbook\\Shader\\Advance\\06_skybox.vert.spv", skyboxVertexShaderSpirv) == false)
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
    if (GetBinaryFileContent("..\\..\\Source\\VulkanCookbook\\Shader\\Advance\\06_skybox.frag.spv", skyboxFragmentShaderSpirv) == false)
    {
        return false;
    }

    VkDestroyer(VkShaderModule) skyboxFragmentShaderModule;
    InitVkDestroyer(_logicalDevice, skyboxFragmentShaderModule);
    if (Cookbook::CreateShader(*_logicalDevice, skyboxFragmentShaderSpirv, *skyboxFragmentShaderModule) == false)
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
            *skyboxFragmentShaderModule,
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

    std::vector<unsigned char> postprocessVertexShaderSpirv;
    if (GetBinaryFileContent("..\\..\\Source\\VulkanCookbook\\Shader\\Advance\\06_postprocess.vert.spv", postprocessVertexShaderSpirv) == false)
    {
        return false;
    }

    VkDestroyer(VkShaderModule) postprocessVertexShaderModule;
    InitVkDestroyer(_logicalDevice, postprocessVertexShaderModule);
    if (Cookbook::CreateShader(*_logicalDevice, postprocessVertexShaderSpirv, *postprocessVertexShaderModule) == false)
    {
        return false;
    }

    std::vector<unsigned char> postprocessFragmentShaderSpirv;
    if (GetBinaryFileContent("..\\..\\Source\\VulkanCookbook\\Shader\\Advance\\06_postprocess.frag.spv", postprocessFragmentShaderSpirv) == false)
    {
        return false;
    }

    VkDestroyer(VkShaderModule) postprocessFragmentShaderModule;
    InitVkDestroyer(_logicalDevice, postprocessFragmentShaderModule);
    if (Cookbook::CreateShader(*_logicalDevice, postprocessFragmentShaderSpirv, *postprocessFragmentShaderModule) == false)
    {
        return false;
    }

    std::vector<ShaderStageParameter> postprocessShaderStageParams =
    {
        {
            VK_SHADER_STAGE_VERTEX_BIT,
            *postprocessVertexShaderModule,
            "main",
            nullptr
        },
        {
            VK_SHADER_STAGE_FRAGMENT_BIT,
            *postprocessFragmentShaderModule,
            "main",
            nullptr
        }
    };
    
    
    std::vector<VkPipelineShaderStageCreateInfo> postprocessShaderStageCreateInfos;
    Cookbook::SpecifyPipelineShaderStage(postprocessShaderStageParams, postprocessShaderStageCreateInfos);

    std::vector<VkVertexInputBindingDescription> postprocessVertexInputBindingDescriptions =
    {
        {
            0,
            3 * sizeof(float),
            VK_VERTEX_INPUT_RATE_VERTEX,
        }
    };

    std::vector<VkVertexInputAttributeDescription> postprocessVertexAttributeDescriptions = 
    {
        {
            0,
            0,
            VK_FORMAT_R32G32B32_SFLOAT,
            0
        }
    };

    VkPipelineVertexInputStateCreateInfo postprocessVertexInputStateCreateInfo;
    Cookbook::SpecifyPipelineVertexInputState(postprocessVertexInputBindingDescriptions, 
        postprocessVertexAttributeDescriptions,
        postprocessVertexInputStateCreateInfo);
        
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

    VkPipelineRasterizationStateCreateInfo postprocessRasterizationStateCreateInfo;
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
        postprocessRasterizationStateCreateInfo);

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
    
    VkGraphicsPipelineCreateInfo postprocessPipelineCreateInfo;
    Cookbook::SpecifyGraphicsPipelineCreateParameter(0,
        postprocessShaderStageCreateInfos,
        postprocessVertexInputStateCreateInfo,
        inputAssemblyStateCreateInfo,
        nullptr,
        &viewportStateCreateInfo,
        postprocessRasterizationStateCreateInfo,
        &multisampleStateCreateInfo,
        nullptr,
        &blendStateCraeteInfo,
        &dynamicStateCreateInfo,
        *_postprocessPipelineLayout,
        *_renderPass,
        1,
        VK_NULL_HANDLE,
        -1,
        postprocessPipelineCreateInfo);
        
    std::vector<VkPipeline> postprocessPipeline;
    if (Cookbook::CreateGraphicsPipeline(*_logicalDevice, {postprocessPipelineCreateInfo}, VK_NULL_HANDLE, postprocessPipeline) == false)
    {
        return false;
    }

    InitVkDestroyer(_logicalDevice, _postprocessPipeline);
    *_postprocessPipeline = postprocessPipeline[0];

    return true;
}

bool SampleAdvance07::Draw(void)
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

        /* #0 subpass */
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

        Cookbook::BindPipelineObject(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *_modelPipeline);
        Cookbook::BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *_graphicsPipelineLayout, 0, _descriptorSets, {});
        Cookbook::BindVertexBuffer(commandBuffer, 0, {{*_modelVertexBuffer, 0}});        

        Cookbook::ProvideDataToShaderThroughPushConstants(commandBuffer, *_graphicsPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float) * 4, &_camera.GetPosition()[0]);

        for (size_t idx = 0; idx < _model.parts.size(); ++idx)
        {
            Cookbook::DrawGeometry(commandBuffer, _model.parts[idx].vertexCount, 1, 0, 0);
        }

        Cookbook::BindPipelineObject(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *_skyboxPipeline);
        Cookbook::BindVertexBuffer(commandBuffer, 0, {{*_skyboxVertexBuffer, 0}});    

        for (size_t idx = 0; idx < _skybox.parts.size(); ++idx)
        {
            Cookbook::DrawGeometry(commandBuffer, _skybox.parts[idx].vertexCount, 1, 0, 0);
        }

        /* #1 subpass */
        Cookbook::ProgressToTheNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);

        Cookbook::BindPipelineObject(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *_postprocessPipeline);
        Cookbook::BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *_postprocessPipelineLayout, 0, _postprocessDescriptorSets, {});
        Cookbook::BindVertexBuffer(commandBuffer, 0, {{*_postprocessVertexBuffer, 0}});        

        float time = _timerState.GetTime();
        Cookbook::ProvideDataToShaderThroughPushConstants(commandBuffer, *_postprocessPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float) , &time);
        
        Cookbook::DrawGeometry(commandBuffer, 6, 1, 0, 0);
        
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

    static uint32_t frameIndex = 0;
    auto& currentFrame = _frameResources[frameIndex];

    if (Cookbook::WaitForFence(*_logicalDevice, {*_sceneFence}, false, 2'000'000'000) == false)
    {
        return false;
    }

    if (Cookbook::ResetFence(*_logicalDevice, {*_sceneFence}) == false)
    {
        return false;
    }

    InitVkDestroyer(_logicalDevice, currentFrame.frameBuffer);

    uint32_t imageIndex = 0;
    if (Cookbook::AcquireSwapchainImage(*_logicalDevice, *_swapchain.handle, *currentFrame.imageAcquiredSemaphore, VK_NULL_HANDLE, imageIndex) == false)
    {
        return false;
    }

    std::vector<VkImageView> attachments = {*_sceneImageView, *currentFrame.depthAttachment, *_swapchain.imageViews[imageIndex]};
    if (Cookbook::CreateFramebuffer(*_logicalDevice, *_renderPass, attachments, _swapchain.size.width, _swapchain.size.height, 1, *currentFrame.frameBuffer) == false)
    {
        return false;
    }

    if (prepareFrame(currentFrame.commandBuffer, imageIndex, *currentFrame.frameBuffer) == false)
    {
        return false;
    }

    std::vector<WaitSemaphoreInfo> waitSemaphoreInfos = 
    {
        {
            *currentFrame.imageAcquiredSemaphore,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        }
    };

    if (Cookbook::SubmitCommandBufferToQueue(_graphicsQueue.handle,
        waitSemaphoreInfos, 
        {currentFrame.commandBuffer},
        {*currentFrame.readyToPresentSemaphore},
        *_sceneFence) == false)
    {
        return false;
    }

    PresentInfo presentInfo = 
    {
        *_swapchain.handle,
        imageIndex
    };

    if (Cookbook::PresentImage(_graphicsQueue.handle, {*currentFrame.readyToPresentSemaphore}, {presentInfo}) == false)
    {
        return false;
    }

    frameIndex = (frameIndex + 1) % _frameResources.size();

    return true;
}

bool SampleAdvance07::Resize(void)
{
    if (CreateSwapchain() == false)
    {
        return false;
    }

    if (IsReady() == true)
    {
        InitVkDestroyer(_logicalDevice, _sceneImage);
        InitVkDestroyer(_logicalDevice, _sceneImageMemory);
        InitVkDestroyer(_logicalDevice, _sceneImageView);
        if (Cookbook::CreateInputAttachment(_physicalDevice,
            *_logicalDevice,
            VK_IMAGE_TYPE_2D,
            _swapchain.format,
            {_swapchain.size.width, _swapchain.size.height, 1},
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
            VK_IMAGE_VIEW_TYPE_2D,
            VK_IMAGE_ASPECT_COLOR_BIT,
            *_sceneImage,
            *_sceneImageMemory,
            *_sceneImageView) == false)
        {
            return false;
        }

        Cookbook::ImageDesciptorInfo sceneImageDescriptorUpdate = 
        {   
            _postprocessDescriptorSets[0],
            0,
            0,
            VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
            {
                {
                    VK_NULL_HANDLE,
                    *_sceneImageView,
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                }
            }
        };

        Cookbook::UpdateDescriptorSets(*_logicalDevice, {sceneImageDescriptorUpdate}, {}, {}, {});

        if (_UpdateUniformbuffer(true) == false)
        {
            return false;
        }
    }
    
    return true;
}

void SampleAdvance07::OnMouseEvent(void)
{
    _UpdateUniformbuffer(false);
}

bool SampleAdvance07::_UpdateUniformbuffer(bool force)
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
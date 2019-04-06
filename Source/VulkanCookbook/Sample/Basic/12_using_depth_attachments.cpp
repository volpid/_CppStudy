
#include "../../Chapter/ch02_image_presentation.h"
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
// SampleChapter12 : using depth attachment
//----------------------------------------------------------------
class SampleChapter12 : public VulkanSample
{
public:
    virtual bool Initialize(WindowParameters parameters) override;
    virtual bool Draw(void) override;
    virtual bool Resize(void) override;    
    
private:
    bool _UpdateStagingBuffer(void);
    bool _CreateDepthImageAndView(void);

private:
    static const VkFormat DepthFormat = VK_FORMAT_D16_UNORM;

    VkDestroyer(VkCommandPool) _commandPool;
    VkCommandBuffer _commandBuffer;

    Cookbook::Mesh _model;

    VkDestroyer(VkBuffer) _vertexBuffer;
    VkDestroyer(VkDeviceMemory) _bufferMemory;
    
    VkDestroyer(VkBuffer) _uniformBuffer;
    VkDestroyer(VkDeviceMemory) _uniformBufferMemory;
    VkDestroyer(VkDescriptorSetLayout) _descriptorSetLayout;
    VkDestroyer(VkDescriptorPool) _descriptorPool;
    std::vector<VkDescriptorSet> _descriptorSets;

    VkDestroyer(VkFence) _drawingFence;
    VkDestroyer(VkSemaphore) _imageAcquiredSemaphore;
    VkDestroyer(VkSemaphore) _readyToPresentSemaphore;

    VkDestroyer(VkRenderPass) _renderPass;
    VkDestroyer(VkPipelineLayout) _pipelineLayout;
    VkDestroyer(VkPipeline) _graphicsPipeline;
    VkDestroyer(VkFramebuffer) _framebuffer;

    VkDestroyer(VkImage) _depthAttachment;
    VkDestroyer(VkDeviceMemory) _depthAttachmentMemory;
    VkDestroyer(VkImageView) _depthAttachmentView;
};

//VULKAN_SAMPLE_FRAMEWORK("12_using_depth_attachments", 50, 25, 800, 600, SampleChapter12);

//----------------------------------------------------------------
bool SampleChapter12::Initialize(WindowParameters parameters)
{   
    if (InitializeVulkan(parameters, 
        nullptr, 
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 
        false) == false)
    {
        return false;
    }

    InitVkDestroyer(_logicalDevice, _commandPool);
    if (Cookbook::CreateCommandPool(*_logicalDevice, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, _graphicsQueue.familyIndex, *_commandPool) == false)
    {
        return false;
    }

    std::vector<VkCommandBuffer> commandBuffers;
    if (Cookbook::AllocateCommandBuffer(*_logicalDevice, *_commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, commandBuffers) == false)
    {
        return false;
    }
    _commandBuffer = commandBuffers[0];

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

    if (_UpdateStagingBuffer() == false)
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

    InitVkDestroyer(_logicalDevice, _drawingFence);
    if (Cookbook::CreateFence(*_logicalDevice, true, *_drawingFence) == false)
    {
        return false;
    }

    InitVkDestroyer(_logicalDevice, _imageAcquiredSemaphore);
    if (Cookbook::CreateSemaphore(*_logicalDevice, *_imageAcquiredSemaphore) == false)
    {
        return false;
    }

    InitVkDestroyer(_logicalDevice, _readyToPresentSemaphore);
    if (Cookbook::CreateSemaphore(*_logicalDevice, *_readyToPresentSemaphore) == false)
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

    std::vector<unsigned char> vertexShaderSpirv;
    if (GetBinaryFileContent("..\\..\\Source\\VulkanCookbook\\Shader\\Basic\\12_shader.vert.spv", vertexShaderSpirv) == false)
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
    if (GetBinaryFileContent("..\\..\\Source\\VulkanCookbook\\Shader\\Basic\\12_shader.frag.spv", fragmentShaderSpirv) == false)
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

    InitVkDestroyer(_logicalDevice, _pipelineLayout);
    if (Cookbook::CreatePipelineLayout(*_logicalDevice, {*_descriptorSetLayout}, {}, *_pipelineLayout) == false)
    {
        return false;
    }
        
    VkGraphicsPipelineCreateInfo pipelineCreateInfo;
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
        *_pipelineLayout,
        *_renderPass,
        0,
        VK_NULL_HANDLE,
        -1,
        pipelineCreateInfo);

    std::vector<VkPipeline> pipeline;
    if (Cookbook::CreateGraphicsPipeline(*_logicalDevice, {pipelineCreateInfo}, VK_NULL_HANDLE, pipeline) == false)
    {
        return false;
    }

    InitVkDestroyer(_logicalDevice, _graphicsPipeline);
    *_graphicsPipeline = pipeline[0];

    return true;
}

bool SampleChapter12::Draw(void)
{
    if (Cookbook::WaitForFence(*_logicalDevice, {*_drawingFence}, false, 5'000'000'000) == false)
    {
        return false;
    }

    if (Cookbook::ResetFence(*_logicalDevice, {*_drawingFence}) == false)
    {
        return false;
    }

    uint32_t imageIndex;
    if (Cookbook::AcquireSwapchainImage(*_logicalDevice,
        *_swapchain.handle,
        *_imageAcquiredSemaphore,
        VK_NULL_HANDLE,
        imageIndex) == false)
    {
        return false;
    }

    InitVkDestroyer(_logicalDevice, _framebuffer);
    if (Cookbook::CreateFramebuffer(*_logicalDevice,
        *_renderPass,
        {*_swapchain.imageViews[imageIndex], *_depthAttachmentView},
        _swapchain.size.width,
        _swapchain.size.height,
        1,
        *_framebuffer) == false)
    {
        return false;
    }

    if (Cookbook::BeginCommandBufferRecordingOperation(_commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr) == false)
    {
        return false;
    }

    if (_presentQueue.familyIndex != _graphicsQueue.familyIndex)
    {
        ImageTransition imageTrasitionBeforeDrawing =
        {
            _swapchain.images[imageIndex],
            VK_ACCESS_MEMORY_READ_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            _presentQueue.familyIndex,
            _graphicsQueue.familyIndex,
            VK_IMAGE_ASPECT_COLOR_BIT
        };
        
        Cookbook::SetImageMemoryBarrier(_commandBuffer, 
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            {imageTrasitionBeforeDrawing});
    }

    Cookbook::BeginRenderPass(_commandBuffer, 
        *_renderPass, 
        *_framebuffer, 
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
    Cookbook::SetViewportStateDynamically(_commandBuffer, 0, {viewport});

    VkRect2D sissor = 
    {
        {0, 0},
        {_swapchain.size.width, _swapchain.size.height}
    };
    Cookbook::SetScissorStateDynamically(_commandBuffer, 0, {sissor});

    Cookbook::BindVertexBuffer(_commandBuffer, 0, {{*_vertexBuffer, 0}});
    Cookbook::BindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *_pipelineLayout, 0, _descriptorSets, {});

    Cookbook::BindPipelineObject(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *_graphicsPipeline);
    for (auto& part : _model.parts)
    {
        Cookbook::DrawGeometry(_commandBuffer, part.vertexCount, 1, part.vertexOffset, 0);
    }
    Cookbook::EndRenderPass(_commandBuffer);

    if (_presentQueue.familyIndex != _graphicsQueue.familyIndex)
    {
        ImageTransition imageTrasitionBeforePresent=
        {
            _swapchain.images[imageIndex],
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_ACCESS_MEMORY_READ_BIT,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            _graphicsQueue.familyIndex,
            _presentQueue.familyIndex,
            VK_IMAGE_ASPECT_COLOR_BIT
        };

        Cookbook::SetImageMemoryBarrier(_commandBuffer, 
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            {imageTrasitionBeforePresent});
    }

    if (Cookbook::EndCommandBufferRecordingOperation(_commandBuffer) == false)
    {
        return false;
    }

    WaitSemaphoreInfo waitSemaphoreInfo = 
    {
        *_imageAcquiredSemaphore,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
    };
    if (Cookbook::SubmitCommandBufferToQueue(_computeQueue.handle, 
        {waitSemaphoreInfo},
        {_commandBuffer},
        {*_readyToPresentSemaphore},
        *_drawingFence) == false)
    {
        return false;
    }

    PresentInfo presentInfo = 
    {
        *_swapchain.handle,
        imageIndex
    };
    if (Cookbook::PresentImage(_presentQueue.handle, {*_readyToPresentSemaphore}, {presentInfo}) == false)
    {
        return false;
    }

    return true;
}

bool SampleChapter12::Resize(void)
{
    if (CreateSwapchain(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, false) == false)
    {
        return false;
    }
    
    if (_CreateDepthImageAndView() == false)
    {
        return false;
    }

    if (_UpdateStagingBuffer() == false)
    {
        return false;
    }

    return true;
}

bool SampleChapter12::_UpdateStagingBuffer(void)
{
    Matrix4x4 rotationMatrix = Cookbook::PrepareRotationMatrix(40.0f, {0.0f, -1.0f, 0.0f});
    Matrix4x4 translationMatrix = Cookbook::PrepareTranslationMatrix(0.0f, 0.0f, -3.0f);
    Matrix4x4 movelViewMatrix = translationMatrix * rotationMatrix;
    Matrix4x4 perspectiveMatrix = Cookbook::PreparePerspectiveProjectionMatrix(static_cast<float>(_swapchain.size.width) / static_cast<float>(_swapchain.size.height), 50.0f, 1.0f, 20.0f);
    
    if (Cookbook::UseStagingBufferToUpdateBufferWithDeviceLocalMemoryBound(_physicalDevice,
       *_logicalDevice,
       sizeof(translationMatrix[0]) * translationMatrix.size(),
       &translationMatrix[0], 
       *_uniformBuffer, 
       0,
       0,
       VK_ACCESS_UNIFORM_READ_BIT,
       VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
       VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
       _graphicsQueue.handle,
       _frameResources.front().commandBuffer,
       {}) == false)
    {
        return false;
    }

    if (Cookbook::UseStagingBufferToUpdateBufferWithDeviceLocalMemoryBound(_physicalDevice,
       *_logicalDevice,
       sizeof(perspectiveMatrix[0]) * perspectiveMatrix.size(),
       &perspectiveMatrix[0], 
       *_uniformBuffer, 
       16 * sizeof(float),
       0,
       VK_ACCESS_UNIFORM_READ_BIT,
       VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
       VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
       _graphicsQueue.handle,
       _frameResources.front().commandBuffer,
       {}) == false)
    {
        return false;
    }

    return true;
}

bool SampleChapter12::_CreateDepthImageAndView(void)
{
    InitVkDestroyer(_logicalDevice, _depthAttachment);
    if (Cookbook::CreateImage(*_logicalDevice, 
        VK_IMAGE_TYPE_2D, 
        DepthFormat, 
        {_swapchain.size.width, _swapchain.size.height, 1}, 
        1,
        1,
        VK_SAMPLE_COUNT_1_BIT,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
        false,
        *_depthAttachment) == false)
    {
        return false;
    }
    
    InitVkDestroyer(_logicalDevice, _depthAttachmentMemory);
    if (Cookbook::AllocateAndBindMemoryObjectToImage(_physicalDevice, 
        *_logicalDevice, 
        *_depthAttachment, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
        *_depthAttachmentMemory) == false)
    {
        return false;
    }
    
    InitVkDestroyer(_logicalDevice, _depthAttachmentView);
    if (Cookbook::CreateImageView(*_logicalDevice, 
        *_depthAttachment, 
        VK_IMAGE_VIEW_TYPE_2D,
        DepthFormat,
        VK_IMAGE_ASPECT_DEPTH_BIT,
        *_depthAttachmentView) == false)
    {
        return false;
    }

    return true;
}
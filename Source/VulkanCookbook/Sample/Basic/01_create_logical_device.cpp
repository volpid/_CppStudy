
#include "../../Chapter/ch01_instance_and_device.h"
#include "../../Chapter/ch03_command_buffer_and_synchronization.h"
#include "../../Common/sample_framework.h"

//----------------------------------------------------------------
// SampleChapter01 : create logical device
//----------------------------------------------------------------
class SampleChapter01 : public VulkanSampleBase
{
public:
    virtual bool Initialize(WindowParameters parameters) override;
    virtual bool Draw(void) override;
    virtual bool Resize(void) override;
    virtual void Deinitialize(void) override;

private:
    LIBRARY_TYPE _vulkanLibrary;
    VkDestroyer(VkInstance) _instance;
    VkDestroyer(VkDevice) _logicalDevice;
    VkQueue _graphicsQueue;
    VkQueue _computeQueue;
};

//VULKAN_SAMPLE_FRAMEWORK("01_CreateLogicalDevice", 50, 25, 800, 600, SampleChapter01);

//----------------------------------------------------------------
bool SampleChapter01::Initialize(WindowParameters parameters)
{
    parameters;

    if (Cookbook::ConnectWithVulkanLoaderLibrary(_vulkanLibrary) == false)
    {
        return false;
    }

    if (Cookbook::LoadFunctionExportedFromVulkanLoaderLibrary(_vulkanLibrary) == false)
    {
        return false;
    }

    if (Cookbook::LoadGlobalLevelFunctions() == false)
    {
        return false;
    }

    if (Cookbook::CreateVulkanInstance({}, "vulkanCookbook_ch01", *_instance) == false)
    {
        return false;
    }

    if (Cookbook::LoadInstanceLevelFunction(*_instance, {}) == false)
    {
        return false;
    }

    if (Cookbook::CreateLogicalDeviceWithGeometryShaderAndGraphicAndComputeQueue(*_instance, 
        *_logicalDevice, 
        _graphicsQueue, 
        _computeQueue) == false)
    {
        return false;
    }

    return true;
}

bool SampleChapter01::Draw(void)
{
    return true;
}

bool SampleChapter01::Resize(void)
{
    return true;
}

void SampleChapter01::Deinitialize(void)
{
    if (_logicalDevice == true)
    {
        Cookbook::WaitForAllSubmittedCommandToBeFinished(*_logicalDevice);
    }
}
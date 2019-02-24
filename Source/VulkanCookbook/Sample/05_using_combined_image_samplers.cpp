
#include "../Chapter/ch10_helper_recipes.h"

#include "../Common/sample_framework.h"
#include "../Common/vulkan_tool.h"

#include <array>

//----------------------------------------------------------------
// SampleChapter04 : graphics pipeline
//----------------------------------------------------------------
class SampleChapter05 : public VulkanSample
{
public:
    virtual bool Initialize(WindowParameters parameters) override;
    virtual bool Draw(void) override;
    virtual bool Resize(void) override;    

private:
};

VULKAN_SAMPLE_FRAMEWORK("05_UsingCombinedImageSamplers", 50, 25, 800, 600, SampleChapter05);

//----------------------------------------------------------------
bool SampleChapter05::Initialize(WindowParameters parameters)
{   
    if (InitializeVulkan(parameters, nullptr, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, false) == false)
    {
        return false;
    }

    int width = 1;
    int height = 1;
    std::vector<unsigned char> imageData;
    if (Cookbook::LoadTextureDataFromFile("../../Resource/VulkanCookbook/Texture/sunset.jpg", 4, imageData, &width, &height) == false)
    {
        return false;
    }

    //D:\+Github\_CppStudy\Resource\VulkanCookbook\Texture



    return true;
}

bool SampleChapter05::Draw(void)
{
    return true;
}

bool SampleChapter05::Resize(void)
{
    return CreateSwapchain();
}

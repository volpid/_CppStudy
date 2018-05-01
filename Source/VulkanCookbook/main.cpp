
#include "Common/sample_framework.h"

//----------------------------------------------------------------
// Sample
//----------------------------------------------------------------
class Sample : public VulkanSample
{
public:
    virtual bool Initialize(WindowParameters parameters) override;
    virtual bool Draw(void) override;
    virtual bool Resize(void) override;
};

//----------------------------------------------------------------
bool Sample::Initialize(WindowParameters parameters)
{
    return true;
}

bool Sample::Draw(void)
{
    return true;
}

bool Sample::Resize(void)
{
    return true;
}

//VULKAN_SAMPLE_FRAMEWORK("VulkanSample", 50, 25, 1280, 800, Sample);
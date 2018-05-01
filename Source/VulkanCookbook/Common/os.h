
#ifndef __VULKAN_OS__H__
#define __VULKAN_OS__H__

#include "vulkan_common.h"

class VulkanSampleBase;

//----------------------------------------------------------------
// WindowFramework
//----------------------------------------------------------------
class WindowFramework
{
public:
    WindowFramework(const char* title, int x, int y, int width, int height, VulkanSampleBase& sample);
    virtual ~WindowFramework(void);

    virtual void Render(void) final;
    
private:
    WindowParameters windowParams_;
    VulkanSampleBase& sample_;
    
    bool created_;
};

#endif /*__VULKAN_OS__H__*/
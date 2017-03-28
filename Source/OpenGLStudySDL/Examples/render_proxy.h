
#ifndef __RENDER_PROXY__H__
#define __RENDER_PROXY__H__

#include "../Core/display.h"

class RenderProxy
{
public:
    static const int WIDTH = 800;
    static const int HEIGHT = 800;

    static inline void Init(void);
    static inline void CleanUp(void);
    static inline void Draw(Display& display);

    static RenderProxy* intance;

public:
    RenderProxy(void);
    virtual ~RenderProxy(void) = default;

protected:
    virtual void InitInternal(void) = 0;
    virtual void CleanUpInternal(void) = 0;
    virtual void DrawInternal(Display& display) = 0;
};

//-----------------------------------------------------------------------------------
// inline section
//-----------------------------------------------------------------------------------
inline RenderProxy::RenderProxy(void)
{
    intance = this;
}

inline void RenderProxy::Init(void)
{
    if (intance == nullptr)
    {
        exit(0);
    }
    intance->InitInternal();
}

inline void RenderProxy::CleanUp(void)
{
    intance->CleanUpInternal();
}

inline void RenderProxy::Draw(Display& display)
{
    intance->DrawInternal(display);
}


#endif /*__RENDER_PROXY__H__*/

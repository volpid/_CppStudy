
#include "Core\display.h"
#include "Examples\render_proxy.h"

#include <iostream>
#include <string>

/*import library*/
#ifdef _DEBUG
#pragma comment(lib, "glew32d.lib")
#else
#pragma comment(lib, "glew32.lib")
#endif

#pragma comment(lib, "SDL3.lib")
#pragma comment(lib, "OpenGL32.lib")

RenderProxy* RenderProxy::intance = nullptr;

int main(int argc, char** argv)
{   
    Display display(RenderProxy::WIDTH, RenderProxy::HEIGHT, std::string("title"));

    RenderProxy::Init();

    SDL_Event e;
    while (display.IsClosed() == false)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_EVENT_QUIT)
            {
                display.SetClosed();
            }
        }

        RenderProxy::Draw(display);
    }

    RenderProxy::CleanUp();

    return 0;
}
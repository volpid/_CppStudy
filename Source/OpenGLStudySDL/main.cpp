
#include "Core\display.h"
#include "Examples\render_proxy.h"

#include <iostream>
#include <string>

/*import library*/
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2main.lib")
#pragma comment(lib, "glew32.lib")
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
            if (e.type == SDL_QUIT)
            {
                display.SetClosed();
            }
        }

        RenderProxy::Draw(display);
    }

    RenderProxy::CleanUp();

    return 0;
}
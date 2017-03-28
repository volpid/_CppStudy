
#include "display.h"
#include <iostream>

//---------------------------------------------------------------------------
// define - Display
//---------------------------------------------------------------------------
Display::Display(int width, int height, std::string& title)
{
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    pWindow_ = SDL_CreateWindow(title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width, 
        height,
        SDL_WINDOW_OPENGL);

    pOpenGLContext_ = SDL_GL_CreateContext(pWindow_);

    GLenum status = glewInit();
    if (status != GLEW_OK)
    {
        std::cerr << "Glew fail to initialize!"  << std::endl;
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

Display::~Display(void)
{
    SDL_GL_DeleteContext(pOpenGLContext_);
    SDL_DestroyWindow(pWindow_);
    SDL_Quit();
}

void Display::Update(void)
{
    SDL_GL_SwapWindow(pWindow_);
}

void Display::Clear(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
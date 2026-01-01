/*
 * Copyright � 2012-2013 Graham Sellers
 *
 * This code is part of the OpenGL SuperBible, 6th Edition.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef __SB6_H__
#define __SB6_H__

#ifdef WIN32
    #pragma once
    #define _CRT_SECURE_NO_WARNINGS 1

    #define WIN32_LEAN_AND_MEAN 1
    #include <Windows.h>

    #ifdef _DEBUG
        #ifdef _WIN64
            #pragma comment (lib, "GLFW_d64.lib")
            #ifndef IN_SB6_LIB
                #pragma comment (lib, "sb6_d64.lib")
            #endif
        #else
            #pragma comment (lib, "GLFW_d32.lib")
            #ifndef IN_SB6_LIB
                #pragma comment (lib, "sb6_d32.lib")
            #endif
        #endif
    #else
        #ifdef _WIN64
            #pragma comment (lib, "GLFW_r64.lib")
            #ifndef IN_SB6_LIB
                #pragma comment (lib, "sb6_r64.lib")
            #endif
        #else
            #pragma comment (lib, "GLFW_r32.lib")
            #ifndef IN_SB6_LIB
                #pragma comment (lib, "sb6_r32.lib")
            #endif
        #endif
    #endif

    #pragma comment (lib, "OpenGL32.lib")
#endif

#include "gl3w/gl/gl3w.h"
#include "gl3w/gl/glcorearb.h"

#define GLFW_NO_GLU 1
#define GLFW_INCLUDE_GLCOREARB 1
#include "glfw/glfw3.h"

#include <stdio.h>
#include <string.h>

namespace sb6
{

class application
{
public:
    application() {}
    virtual ~application() {}
    virtual void run(sb6::application* the_app)
    {
        bool running = true;
        app = the_app;

        if (!glfwInit())
        {
            fprintf(stderr, "Failed to initialize GLFW\n");
            return;
        }

        init();

        // 2. 윈도우 힌트 설정 (하드웨어 요구사항 정의)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#ifdef _DEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif /* _DEBUG */
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
        glfwWindowHint(GLFW_SAMPLES, info.samples);
        glfwWindowHint(GLFW_STEREO, info.flags.stereo ? GLFW_TRUE : GLFW_FALSE);

        // 3. 윈도우 생성 (glfwOpenWindow의 현대적 대체)
        GLFWwindow* window = glfwCreateWindow(info.windowWidth, info.windowHeight, info.title, NULL, NULL);
        if (window == nullptr) 
        {
            glfwTerminate();
            return;
        }

        // 4. 생성한 윈도우의 컨텍스트를 현재 스레드의 작업대로 설정
        glfwMakeContextCurrent(window);


        glfwSetWindowSizeCallback(window, glfw_onResize);
        glfwSetKeyCallback(window, glfw_onKey);
        glfwSetMouseButtonCallback(window, glfw_onMouseButton);
        glfwSetCursorPosCallback(window, glfw_onMouseMove);
        glfwSetScrollCallback(window, glfw_onMouseWheel);
                
        // 5. gl3w 초기화 (드라이버에서 OpenGL 함수 주소를 가져옴)
        gl3wInit();

        if (info.flags.debug)
        {
            if (gl3wIsSupported(4, 3))
            {
                glDebugMessageCallback(debug_callback, this);
                glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            }
        }

        startup();
        onResize(info.windowWidth, info.windowHeight);

        do
        {
            render(glfwGetTime());

            glfwSwapBuffers(window);

            running &= (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE);
            running &= (glfwWindowShouldClose(window) == false);

            glfwPollEvents();
            
        } 
        while (running);

        shutdown();

        glfwTerminate();
    }

    virtual void init()
    {
        strcpy_s(info.title, "SuperBible6 Example");
        info.windowWidth = 800;
        info.windowHeight = 600;
#ifdef __APPLE__
        info.majorVersion = 3;
        info.minorVersion = 2;
#else
        info.majorVersion = 4;
        info.minorVersion = 3;
#endif
        info.samples = 0;
        info.flags.all = 0;
        info.flags.cursor = 1;
#ifdef _DEBUG
        info.flags.debug = 1;
#endif
    }

    virtual void startup()
    {
    }

    virtual void render(double currentTime)
    {
    }

    virtual void shutdown()
    {
    }

    virtual void onResize(int w, int h)
    {
        info.windowWidth = w;
        info.windowHeight = h;
    }

    virtual void onKey(int key, int action)
    {
    }

    virtual void onMouseButton(int button, int action)
    {
    }

    virtual void onMouseMove(int x, int y)
    {
    }

    virtual void onMouseWheel(int pos)
    {
    }

    virtual void onDebugMessage(GLenum source,
                                GLenum type,
                                GLuint id,
                                GLenum severity,
                                GLsizei length,
                                const GLchar* message)
    {
#ifdef _WIN32
        OutputDebugStringA(message);
        OutputDebugStringA("\n");
#endif /* _WIN32 */
    }

public:
    struct APPINFO
    {
        char title[128];
        int windowWidth;
        int windowHeight;
        int majorVersion;
        int minorVersion;
        int samples;
        union
        {
            struct
            {
                unsigned int    fullscreen  : 1;
                unsigned int    vsync       : 1;
                unsigned int    cursor      : 1;
                unsigned int    stereo      : 1;
                unsigned int    debug       : 1;
            };
            unsigned int        all;
        } flags;
    };

    
protected:
    APPINFO     info;
    static      sb6::application* app;

    static void glfw_onResize(GLFWwindow* window, int width, int height)
    {
        app->onResize(width, height);
    }

    static void glfw_onKey(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        app->onKey(key, action);
    }

    static void glfw_onMouseButton(GLFWwindow* window, int button, int action, int mods)
    {
        app->onMouseButton(button, action);
    }

    static void glfw_onMouseMove(GLFWwindow* window, double xpos, double ypos)
    {
        app->onMouseMove(xpos, ypos);
    }

    static void glfw_onMouseWheel(GLFWwindow* window, double xoffset, double yoffset)
    {
        app->onMouseWheel(yoffset);
    }

    void setVsync(bool enable)
    {
        info.flags.vsync = enable ? 1 : 0;
        glfwSwapInterval((int)info.flags.vsync);
    }

    static void APIENTRY debug_callback(GLenum source,
                                        GLenum type,
                                        GLuint id,
                                        GLenum severity,
                                        GLsizei length,
                                        const GLchar* message,
                                        const void* userParam)
    {
        reinterpret_cast<application*>(const_cast<void*>(userParam))->onDebugMessage(source, type, id, severity, length, message);
    }
};

};

#if defined _WIN32
#define DECLARE_MAIN(a)                             \
sb6::application* sb6::application::app = 0;        \
int CALLBACK WinMain(HINSTANCE hInstance,           \
                     HINSTANCE hPrevInstance,       \
                     LPSTR lpCmdLine,               \
                     int nCmdShow)                  \
{                                                   \
    a *app = new a;                                 \
    app->run(app);                                  \
    delete app;                                     \
    return 0;                                       \
}
#elif defined _LINUX || defined __APPLE__
#define DECLARE_MAIN(a)                             \
int main(int argc, const char ** argv)              \
{                                                   \
    a *app = new a;                                 \
    app->run(app);                                  \
    delete app;                                     \
    return 0;                                       \
}
#else
#error Undefined platform!
#endif

#endif /* __SB6_H__ */


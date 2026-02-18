
#include "../Book/object.h"
#include "../Book/sb6.h"
#include "../Book/sb6ktx.h"
#include "../Book/shader.h"
#include "../Book/vmath.h"

#include <cstdio>
#include <math.h>


enum
{
    NUM_ELEMENTS = 2048,
};

class Chapter12_Rimlight : public sb6::application
{
public:
    GLuint program = 0;

    struct
    {
        GLint mv_matrix;
        GLint proj_matrix;
        GLint rim_color;
        GLint rim_power;
    } uniforms;

    vmath::mat4 mat_rotation;

    sb6::object object;
    bool paused = false;
    vmath::vec3 rim_color = vmath::vec3(0.3f, 0.3f, 0.3f);
    float rim_power = 2.5;
    bool rim_enable = true;

public:
    Chapter12_Rimlight()
    {
    }

    virtual void init() override
    {
        sb6::application::init();

        info.samples = 1;
    }

    virtual void startup() override
    {   
        LoadShader();

        object.load("../Media/objects/dragon.sbm");

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
    }

    void LoadShader()
    {   
        if (program != 0)
        {
            glDeleteProgram(program);
        }

        GLuint vs = sb6::shader::load("../Media/shaders/rimlight/render.vs.glsl", GL_VERTEX_SHADER);
        GLuint fs = sb6::shader::load("../Media/shaders/rimlight/render.fs.glsl", GL_FRAGMENT_SHADER);

        program = glCreateProgram();
        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glLinkProgram(program);

        uniforms.mv_matrix = glGetUniformLocation(program, "mv_matrix");
        uniforms.proj_matrix = glGetUniformLocation(program, "proj_matrix");
        uniforms.rim_color = glGetUniformLocation(program, "rim_color");
        uniforms.rim_power = glGetUniformLocation(program, "rim_power");
    }

    virtual void shutdown() override
    {
        glDeleteProgram(program);
    }

    virtual void render(double currentTime) override
    {
        static const GLfloat green[] = { 0.0f, 0.25f, 0.0f, 1.0f };
        static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        static const GLfloat one = 1.0f;
        static double last_timne = 0.0;
        static double total_time = 0.0;

        if (paused == false)
        {
            total_time += (currentTime - last_timne);
        }
        last_timne = currentTime;
        float f = (float) total_time;

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, black);
        glClearBufferfv(GL_DEPTH, 0, &one);

        glUseProgram(program);

        vmath::mat4 mv_matrix = vmath::translate(0.0f, -5.0f, -20.0f)
            * vmath::rotate(f * 5.0f, 0.0f, 1.0f, 0.0f);

        vmath::mat4 prj_matrix = vmath::perspective(50.0f, (float)info.windowWidth / (float)info.windowHeight, 0.1f, 1000.0f);

        glUniformMatrix4fv(uniforms.proj_matrix, 1, GL_FALSE, prj_matrix);
        glUniformMatrix4fv(uniforms.mv_matrix, 1, GL_FALSE, mv_matrix);
        glUniform3fv(uniforms.rim_color, 1, rim_enable ? rim_color : vmath::vec3(0.0f));
        glUniform1f(uniforms.rim_power, rim_power);

        object.render();
    }

    virtual void onResize(int w, int h) override
    {
        sb6::application::onResize(w, h);
    }

    virtual void onKey(int key, int action) override 
    {
        if (action != 0)
        {
            switch (key)
            {   
                case 'Q': { rim_color[0] += 0.1f; } break;
                case 'W': { rim_color[1] += 0.1f; } break;
                case 'E': { rim_color[2] += 0.1f; } break;
                case 'R': { rim_power *= 1.5f; } break;

                case 'A': { rim_color[0] -= 0.1f; } break;
                case 'S': { rim_color[1] -= 0.1f; } break;
                case 'D': { rim_color[2] -= 0.1f; } break;
                case 'F': { rim_power /= 1.5f; } break;

                case 'Z': { rim_enable = !rim_enable; } break;
            }
        }
    }
};

//DECLARE_MAIN(Chapter12_Rimlight);
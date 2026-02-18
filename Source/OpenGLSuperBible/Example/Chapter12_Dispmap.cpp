
#include "../Book/object.h"
#include "../Book/sb6.h"
#include "../Book/sb6ktx.h"
#include "../Book/shader.h"
#include "../Book/vmath.h"

#include <cstdio>
#include <math.h>


class Chapter12_Dispmap : public sb6::application
{
public:
    GLuint program = 0;
    GLuint vao = 0;
    GLuint tex_displacement = 0;
    GLuint tex_color = 0;

    float dmap_depth = 0.0f;
    bool enable_displacement = true;
    bool wireframe = false;
    bool enable_fog = true;
    bool paused = false;

    struct
    {
        GLint mvp_matrix;
        GLint mv_matrix;
        GLint proj_matrix;
        GLint dmap_depth;
        GLint enable_fog;
    } uniforms;

public:
    Chapter12_Dispmap()
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

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glPatchParameteri(GL_PATCH_VERTICES, 4);

        glEnable(GL_CULL_FACE);

        tex_displacement = sb6::ktx::file::load("../Media/textures/terragen1.ktx");
        glActiveTexture(GL_TEXTURE1);
        tex_color = sb6::ktx::file::load("../Media/textures/terragen_color.ktx");
    }

    void LoadShader()
    {   
        if (program != 0)
        {
            glDeleteProgram(program);
        }

        GLuint vs = sb6::shader::load("../Media/shaders/dispmap/dispmap.vs.glsl", GL_VERTEX_SHADER);
        GLuint tcs = sb6::shader::load("../Media/shaders/dispmap/dispmap.tcs.glsl", GL_TESS_CONTROL_SHADER);
        GLuint tes = sb6::shader::load("../Media/shaders/dispmap/dispmap.tes.glsl", GL_TESS_EVALUATION_SHADER);
        GLuint fs = sb6::shader::load("../Media/shaders/dispmap/dispmap.fs.glsl", GL_FRAGMENT_SHADER);

        program = glCreateProgram();
        glAttachShader(program, vs);
        glAttachShader(program, tcs);
        glAttachShader(program, tes);
        glAttachShader(program, fs);
        glLinkProgram(program);

        uniforms.mv_matrix = glGetUniformLocation(program, "mv_matrix");
        uniforms.mvp_matrix = glGetUniformLocation(program, "mvp_matrix");
        uniforms.proj_matrix = glGetUniformLocation(program, "proj_matrix");
        uniforms.dmap_depth = glGetUniformLocation(program, "dmap_depth");
        uniforms.enable_fog = glGetUniformLocation(program, "enable_fog");

        glDeleteShader(vs);
        glDeleteShader(tcs);
        glDeleteShader(tes);
        glDeleteShader(fs);

        dmap_depth = 6.0f;
    }

    virtual void shutdown() override
    {
        glDeleteProgram(program);
    }

    virtual void render(double currentTime) override
    {
        static const GLfloat black[] = { 0.85f, 0.95f, 1.0f, 1.0f };
        static const GLfloat one = 1.0f;
        static double last_time = 0.0;
        static double total_time = 0.0;

        if (paused == false)
        {
            total_time += (currentTime - last_time);
        }
        last_time = currentTime;

        float t = (float) total_time * 0.03f;
        float r = sinf(t * 5.37f) * 15.0f + 16.0f;
        float h = cosf(t * 4.79f) * 2.0f + 3.2f;

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, black);
        glClearBufferfv(GL_DEPTH, 0, &one);

        vmath::mat4 mv_matrix = vmath::lookat(vmath::vec3(sinf(t) * r, h, cosf(t) * r), vmath::vec3(0.0f), vmath::vec3(0.0f, 1.0f, 0.0f));
        vmath::mat4 proj_matrix = vmath::perspective(60.0f, 
            (float) info.windowHeight / (float) info.windowHeight,
            0.1f, 1000.0f);

        glUseProgram(program);

        glUniformMatrix4fv(uniforms.mv_matrix, 1, GL_FALSE, mv_matrix);
        glUniformMatrix4fv(uniforms.proj_matrix, 1, GL_FALSE, proj_matrix);
        glUniformMatrix4fv(uniforms.mvp_matrix, 1, GL_FALSE, proj_matrix * mv_matrix);
        glUniform1f(uniforms.dmap_depth, enable_displacement ? dmap_depth : 0.0f);
        glUniform1f(uniforms.enable_fog, enable_fog ? dmap_depth : 0.0f);
        
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        if (wireframe == true)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        glDrawArraysInstanced(GL_PATCHES, 0, 4, 64 * 64);
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
                case GLFW_KEY_KP_ADD: { dmap_depth += 0.1f; } break;
                case GLFW_KEY_KP_SUBTRACT: { dmap_depth -= 0.1f; } break;
                case 'F': { enable_fog = !enable_fog; } break;
                case 'D': { enable_displacement = !enable_displacement; } break;
                case 'W': { wireframe = !wireframe; } break;
                case 'p': { paused = !paused; } break;
            }
        }
    }
};

//DECLARE_MAIN(Chapter12_Dispmap);

#include "../Book/object.h"
#include "../Book/sb6.h"
#include "../Book/sb6ktx.h"
#include "../Book/shader.h"
#include "../Book/vmath.h"

#include <cstdio>
#include <math.h>


class Chapter12_Equirectangular : public sb6::application
{
public:
    GLuint render_prog = 0;
    GLuint tex_envmap = 0;
    GLuint envmap[3];
    int envmap_index = 0;

    struct
    {
        GLint mv_matrix;
        GLint proj_matrix;
    } uniforms;

    sb6::object object;

public:
    Chapter12_Equirectangular()
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

        envmap[0] = sb6::ktx::file::load("../Media/textures/envmaps/equirectangularmap1.ktx");
        tex_envmap = envmap[envmap_index];
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        object.load("../Media/objects/dragon.sbm");

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
    }

    void LoadShader()
    {   
        if (render_prog != 0)
        {
            glDeleteProgram(render_prog);
        }

        GLuint vs = sb6::shader::load("../Media/shaders/equirectangular/render.vs.glsl", GL_VERTEX_SHADER);
        GLuint fs = sb6::shader::load("../Media/shaders/equirectangular/render.fs.glsl", GL_FRAGMENT_SHADER);

        render_prog = glCreateProgram();
        glAttachShader(render_prog, vs);
        glAttachShader(render_prog, fs);
        glLinkProgram(render_prog);

        uniforms.mv_matrix = glGetUniformLocation(render_prog, "mv_matrix");
        uniforms.proj_matrix = glGetUniformLocation(render_prog, "proj_matrix");
    }

    virtual void shutdown() override
    {
        glDeleteProgram(render_prog);
    }

    virtual void render(double currentTime) override
    {
        static const GLfloat green[] = { 0.0f, 0.25f, 0.0f, 1.0f };
        static const GLfloat one = 1.0f;
        static double last_timne = 0.0;
        static double total_time = 0.0;

        glClearBufferfv(GL_COLOR, 0, green);
        glClearBufferfv(GL_DEPTH, 0, &one);
        glBindTexture(GL_TEXTURE_2D, tex_envmap);

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glUseProgram(render_prog);

        vmath::mat4 mv_matrix = vmath::translate(0.0f, 0.0f, -15.0f)
            * vmath::rotate((float) currentTime, 1.0f, 0.0f, 0.0f)
            * vmath::rotate((float) currentTime * 5.1f, 0.0f, 1.0f, 0.0f)
            * vmath::translate(0.0f, -4.0f, 0.0f);

        vmath::mat4 prj_matrix = vmath::perspective(60.0f, (float)info.windowWidth / (float)info.windowHeight, 0.1f, 1000.0f);

        glUniformMatrix4fv(uniforms.proj_matrix, 1, GL_FALSE, prj_matrix);
        glUniformMatrix4fv(uniforms.mv_matrix, 1, GL_FALSE, mv_matrix);

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
                case 'E': 
                {
                    envmap_index = (envmap_index + 1) % 1;
                    tex_envmap = envmap[envmap_index];
                } 
                break;
            }
        }
    }
};

//DECLARE_MAIN(Chapter12_Equirectangular);
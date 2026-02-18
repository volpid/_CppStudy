
#include "../Book/object.h"
#include "../Book/sb6.h"
#include "../Book/sb6ktx.h"
#include "../Book/shader.h"
#include "../Book/vmath.h"

#include <cstdio>
#include <math.h>


class Chapter12_Prepixelgloss : public sb6::application
{
public:
    GLuint render_prog = 0;

    GLuint tex_envmap = 0;
    GLuint tex_glossmap = 0;

    struct
    {
        GLint mv_matrix;
        GLint proj_matrix;        
        
    } uniforms;

    sb6::object object;

public:
    Chapter12_Prepixelgloss()
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

        glActiveTexture(GL_TEXTURE0);
        tex_envmap = sb6::ktx::file::load("../Media/textures/envmaps/mountains3d.ktx");

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glActiveTexture(GL_TEXTURE1);
        tex_glossmap = sb6::ktx::file::load("../Media/textures/pattern1.ktx");

        object.load("../Media/objects/torus_nrms_tc.sbm");
    }

    void LoadShader()
    {   
        if (render_prog != 0)
        {
            glDeleteProgram(render_prog);
        }

        GLuint vs = sb6::shader::load("../Media/shaders/perpixelgloss/perpixelgloss.vs.glsl", GL_VERTEX_SHADER);
        GLuint fs = sb6::shader::load("../Media/shaders/perpixelgloss/perpixelgloss.fs.glsl", GL_FRAGMENT_SHADER);

        render_prog = glCreateProgram();
        glAttachShader(render_prog, vs);
        glAttachShader(render_prog, fs);
        glLinkProgram(render_prog);

        uniforms.mv_matrix = glGetUniformLocation(render_prog, "mv_matrix");
        uniforms.proj_matrix = glGetUniformLocation(render_prog, "proj_matrix");

        glDeleteShader(vs);
        glDeleteShader(fs);
    }

    virtual void shutdown() override
    {
        glDeleteProgram(render_prog);
    }

    virtual void render(double currentTime) override
    {
        static const GLfloat green[] = { 0.2f, 0.2f, 0.2f, 1.0f };
        static const GLfloat one = 1.0f;

        vmath::mat4 mv_matrix = vmath::translate(0.0f, 0.0f, -3.0f)
            * vmath::rotate((float) currentTime * 13.75f, 0.0f, 1.0f, 0.0f)
            * vmath::rotate((float) currentTime * 7.75f, 0.0f, 0.0f, 1.0f)
            * vmath::rotate((float) currentTime * 15.75F, 1.0f, 0.0f, 0.0f);

        vmath::mat4 prj_matrix = vmath::perspective(60.0f, (float)info.windowWidth / (float)info.windowHeight, 0.1f, 1000.0f);

        glClearBufferfv(GL_COLOR, 0, green);
        glClearBufferfv(GL_DEPTH, 0, &one);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, tex_envmap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, tex_glossmap);

        glViewport(0, 0, info.windowWidth, info.windowHeight);

        glUseProgram(render_prog);

        glUniformMatrix4fv(uniforms.proj_matrix, 1, GL_FALSE, prj_matrix);
        glUniformMatrix4fv(uniforms.mv_matrix, 1, GL_FALSE, mv_matrix);

        glEnable(GL_DEPTH_TEST);
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
                
            }
        }
    }
};

//DECLARE_MAIN(Chapter12_Prepixelgloss);
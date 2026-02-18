
#include "../Book/object.h"
#include "../Book/sb6.h"
#include "../Book/sb6ktx.h"
#include "../Book/shader.h"
#include "../Book/vmath.h"

#include <cstdio>
#include <math.h>


class Chapter12_CubemapEnv : public sb6::application
{
public:
    GLuint render_prog = 0;
    GLuint skybox_prog = 0;
    GLuint skybox_vao = 0;
    GLuint tex_envmap = 0;
    GLuint envmap[3];
    int envmap_index = 0;

    struct
    {
        struct
        {
            GLint mv_matrix;
            GLint proj_matrix;
        } render;
        struct
        {
            GLint view_matrix;
        } skybox;
        
    } uniforms;

    sb6::object object;

public:
    Chapter12_CubemapEnv()
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

        envmap[0] = sb6::ktx::file::load("../Media/textures/envmaps/mountaincube.ktx");
        tex_envmap = envmap[envmap_index];
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        object.load("../Media/objects/dragon.sbm");

        glGenVertexArrays(1, &skybox_vao);
        glBindVertexArray(skybox_vao);
    }

    void LoadShader()
    {   
        if (render_prog != 0)
        {
            glDeleteProgram(render_prog);
        }

        GLuint vs = sb6::shader::load("../Media/shaders/cubemapenv/render.vs.glsl", GL_VERTEX_SHADER);
        GLuint fs = sb6::shader::load("../Media/shaders/cubemapenv/render.fs.glsl", GL_FRAGMENT_SHADER);

        render_prog = glCreateProgram();
        glAttachShader(render_prog, vs);
        glAttachShader(render_prog, fs);
        glLinkProgram(render_prog);

        uniforms.render.mv_matrix = glGetUniformLocation(render_prog, "mv_matrix");
        uniforms.render.proj_matrix = glGetUniformLocation(render_prog, "proj_matrix");

        glDeleteShader(vs);
        glDeleteShader(fs);

        if (skybox_prog != 0)
        {
            glDeleteProgram(skybox_prog);
        }

        vs = sb6::shader::load("../Media/shaders/cubemapenv/skybox.vs.glsl", GL_VERTEX_SHADER);
        fs = sb6::shader::load("../Media/shaders/cubemapenv/skybox.fs.glsl", GL_FRAGMENT_SHADER);

        skybox_prog = glCreateProgram();
        glAttachShader(skybox_prog, vs);
        glAttachShader(skybox_prog, fs);
        glLinkProgram(skybox_prog);

        uniforms.skybox.view_matrix = glGetUniformLocation(skybox_prog, "view_matrix");
    }

    virtual void shutdown() override
    {
        glDeleteProgram(render_prog);
    }

    virtual void render(double currentTime) override
    {
        static const GLfloat green[] = { 0.2f, 0.2f, 0.2f, 1.0f };
        static const GLfloat one = 1.0f;        
        const float f = (float) currentTime;

        vmath::mat4 view_matrix = vmath::lookat(vmath::vec3(15.0f * sinf(f), 0.0f, 15.0f * cosf(f)),
            vmath::vec3(0.0f, 0.0f, 0.0f),
            vmath::vec3(0.0f, 1.0f, 0.0f));

        vmath::mat4 mv_matrix = view_matrix
            * vmath::rotate(f, 1.0f, 0.0f, 0.0f)
            * vmath::rotate((float) f * 13.1f, 0.0f, 1.0f, 0.0f)
            * vmath::translate(0.0f, -4.0f, 0.0f);

        vmath::mat4 prj_matrix = vmath::perspective(60.0f, (float)info.windowWidth / (float)info.windowHeight, 0.1f, 1000.0f);

        glClearBufferfv(GL_COLOR, 0, green);
        glClearBufferfv(GL_DEPTH, 0, &one);
        glBindTexture(GL_TEXTURE_CUBE_MAP, tex_envmap);

        glViewport(0, 0, info.windowWidth, info.windowHeight);

        glUseProgram(skybox_prog);
        glBindVertexArray(skybox_vao);

        glUniformMatrix4fv(uniforms.skybox.view_matrix, 1, GL_FALSE, view_matrix);

        glDisable(GL_DEPTH_TEST);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glUseProgram(render_prog);

        glUniformMatrix4fv(uniforms.render.proj_matrix, 1, GL_FALSE, prj_matrix);
        glUniformMatrix4fv(uniforms.render.mv_matrix, 1, GL_FALSE, mv_matrix);

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

//DECLARE_MAIN(Chapter12_CubemapEnv);
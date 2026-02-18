
#include "../Book/object.h"
#include "../Book/sb6.h"
#include "../Book/sb6ktx.h"
#include "../Book/shader.h"
#include "../Book/vmath.h"

#include <math.h>

class Chapter9_GSLayered : public sb6::application
{
public:
    GLuint program_gslayers = 0;
    GLuint program_showlayers = 0;
    GLuint vao;

    int mode;

    GLuint transform_ubo;
    GLuint layered_fbo;
    GLuint array_texture;
    GLuint array_depth;

    sb6::object object;

    bool wireframeMode;

public:
    Chapter9_GSLayered()
    {
        wireframeMode = false;
    }

    void CompileShader()
    {
        GLuint vs;
        GLuint gs;
        GLuint fs;

        if (program_showlayers)
        {
            glDeleteProgram(program_showlayers);
        }
        
        program_showlayers = glCreateProgram();
        vs = sb6::shader::load("../Media/shaders/gslayers/showlayers.vs.glsl", GL_VERTEX_SHADER);
        fs = sb6::shader::load("../Media/shaders/gslayers/showlayers.fs.glsl", GL_FRAGMENT_SHADER);

        glAttachShader(program_showlayers, vs);
        glAttachShader(program_showlayers, fs);
        glLinkProgram(program_showlayers);

        glDeleteShader(vs);
        glDeleteShader(fs);

        if (program_gslayers)
        {
            glDeleteProgram(program_gslayers);
        }

        program_gslayers = glCreateProgram();
        vs = sb6::shader::load("../Media/shaders/gslayers/gslayers.vs.glsl", GL_VERTEX_SHADER);
        gs = sb6::shader::load("../Media/shaders/gslayers/gslayers.gs.glsl", GL_GEOMETRY_SHADER);
        fs = sb6::shader::load("../Media/shaders/gslayers/gslayers.fs.glsl", GL_FRAGMENT_SHADER);

        glAttachShader(program_gslayers, vs);
        glAttachShader(program_gslayers, gs);
        glAttachShader(program_gslayers, fs);
        glLinkProgram(program_gslayers);

        glDeleteShader(vs);
        glDeleteShader(gs);
        glDeleteShader(fs);
    }

    virtual void startup() override
    {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        CompileShader();
        object.load("../Media/objects/torus.sbm");

        glGenBuffers(1, &transform_ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, transform_ubo);
        glBufferData(GL_UNIFORM_BUFFER, 17 * sizeof(vmath::mat4), nullptr, GL_DYNAMIC_DRAW);

        glGenTextures(1, &array_texture);
        glBindTexture(GL_TEXTURE_2D_ARRAY, array_texture);
        glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, 256, 256, 16);

        glGenTextures(1, &array_depth);
        glBindTexture(GL_TEXTURE_2D_ARRAY, array_depth);
        glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT32, 256, 256, 16);

        glGenFramebuffers(1, &layered_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, layered_fbo);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, array_texture, 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, array_depth, 0);
    }

    virtual void shutdown() override
    {
        glDeleteProgram(program_gslayers);
        glDeleteProgram(program_showlayers);
        glDeleteVertexArrays(1, &vao);
    }

    virtual void render(double currentTime) override
    {
        static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        static const GLfloat gray[] = { 0.1f, 0.1f, 0.1f, 1.0f };
        static const GLfloat one = 1.0f;

        vmath::mat4 proj_matrix = vmath::perspective(50.0f, (float) info.windowWidth / (float) info.windowHeight, 0.1f, 1000.0f);
        vmath::mat4 mv_matrix = vmath::translate(0.0f, 0.0f, -4.0f)
            * vmath::rotate((float) currentTime * 5.0f, 0.0f, 0.0f, 1.0f)
            * vmath::rotate((float) currentTime * 30.0f, 1.0f, 0.0f, 0.0f);

        vmath::mat4 mvp = proj_matrix * mv_matrix;

        struct TRANSFORM_BUFFER
        {
            vmath::mat4 proj_matrix;
            vmath::mat4 mv_matgrix[16];
        };

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, transform_ubo);
        TRANSFORM_BUFFER* buffer = (TRANSFORM_BUFFER*) glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(TRANSFORM_BUFFER), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        buffer->proj_matrix = vmath::perspective(50.0f, 1.0f, 0.1f, 1000.0f);

        for (int idx = 0; idx < 16; ++idx)
        {
            float fi = (float) (idx + 12) / 16.0f;
            buffer->mv_matgrix[idx] = vmath::translate(0.0f, 0.0f, -4.0f)
                * vmath::rotate((float) currentTime * 25.0f * fi, 0.0f, 0.0f, 1.0f)
                * vmath::rotate((float) currentTime * 35.0f * fi, 1.0f, 0.0f, 0.0f);
        }

        glUnmapBuffer(GL_UNIFORM_BUFFER);

        static const GLenum ca0 = GL_COLOR_ATTACHMENT0;

        glBindFramebuffer(GL_FRAMEBUFFER, layered_fbo);
        glDrawBuffers(1, &ca0);
        glViewport(0, 0, 256, 256);
        glClearBufferfv(GL_COLOR, 0, black);
        glClearBufferfv(GL_DEPTH, 0, &one);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glUseProgram(program_gslayers);

        object.render();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDrawBuffer(GL_BACK);
        glUseProgram(program_showlayers);

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, gray);

        glBindTexture(GL_TEXTURE_2D_ARRAY, array_texture);
        glDisable(GL_DEPTH_TEST);

        glBindVertexArray(vao);
        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, 16);

        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
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
                case 'W':
                {
                    wireframeMode = !wireframeMode;
                }
                break;
                case '1':
                case '2':
                {
                    mode = key - '1';
                }
                break;
                case 'M':
                {
                    mode = (mode + 1) % 2;
                }
                break;
            }
        }
    }
};

//DECLARE_MAIN(Chapter9_GSLayered);
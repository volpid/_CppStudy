
#include "../Book/object.h"
#include "../Book/sb6.h"
#include "../Book/sb6ktx.h"
#include "../Book/shader.h"
#include "../Book/vmath.h"

#include <math.h>

class Chapter9_HdrTonemap : public sb6::application
{
public:
    GLuint tex_src;
    GLuint tex_lut;

    GLuint program_naive;
    GLuint program_exposure;
    GLuint program_adaptive;
    GLuint vao;

    float exposure;
    int mode;

    struct
    {
        struct
        {
            int exposure;
        } exposure;
    } uniforms;

    bool wireframeMode;    

public:
    Chapter9_HdrTonemap()
    {
        wireframeMode = false;
        exposure = 1.0f;
        mode = 0;
    }

    virtual void init() override
    {
        sb6::application::init();

        info.samples = 8;
    }

    void CompileShader()
    {
        GLuint vs;
        GLuint fs;

        if (program_naive != 0)
        {
            glDeleteShader(program_naive);
        }
        program_naive = glCreateProgram();

        vs = sb6::shader::load("../Media/shaders/hdrtonemap/tonemap.vs.glsl", GL_VERTEX_SHADER);
        fs = sb6::shader::load("../Media/shaders/hdrtonemap/tonemap_naive.fs.glsl", GL_FRAGMENT_SHADER);

        glAttachShader(program_naive, vs);
        glAttachShader(program_naive, fs);
        glLinkProgram(program_naive);

        if (program_adaptive != 0)
        {
            glDeleteShader(program_adaptive);
        }
        program_adaptive = glCreateProgram();

        glDeleteShader(fs);
        fs = sb6::shader::load("../Media/shaders/hdrtonemap/tonemap_adaptive.fs.glsl", GL_FRAGMENT_SHADER);

        glAttachShader(program_adaptive, vs);
        glAttachShader(program_adaptive, fs);
        glLinkProgram(program_adaptive);

        if (program_exposure != 0)
        {
            glDeleteShader(program_exposure);
        }
        program_exposure = glCreateProgram();

        glDeleteShader(fs);
        fs = sb6::shader::load("../Media/shaders/hdrtonemap/tonemap_exposure.fs.glsl", GL_FRAGMENT_SHADER);

        glAttachShader(program_exposure, vs);
        glAttachShader(program_exposure, fs);
        glLinkProgram(program_exposure);

        uniforms.exposure.exposure = glGetUniformLocation(program_exposure, "exposure");

        glDeleteShader(vs);
        glDeleteShader(fs);
    }

    virtual void startup() override
    {
        tex_src = sb6::ktx::file::load("../Media/textures/treelights_2k.ktx");

        glBindTexture(GL_TEXTURE_2D, tex_src);

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        CompileShader();

        static const GLfloat exposureLUT[20] =
        {
            11.0f, 6.0f, 3.2f, 2.8f, 2.2f, 1.90f, 1.80f, 1.80f, 1.70f, 1.70f,  1.60f, 1.60f, 1.50f, 1.50f, 1.40f, 1.40f, 1.30f, 1.20f, 1.10f, 1.00f
        };

        glGenTextures(1, &tex_lut);
        glBindTexture(GL_TEXTURE_1D, tex_lut);
        glTexStorage1D(GL_TEXTURE_1D, 1, GL_R32F, 20);
        glTexSubImage1D(GL_TEXTURE_1D, 0, 0, 20, GL_RED, GL_FLOAT, exposureLUT);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINES);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINES);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    }

    virtual void shutdown() override
    {
        glDeleteProgram(program_adaptive);
        glDeleteProgram(program_exposure);
        glDeleteProgram(program_naive);
        glDeleteVertexArrays(1, &vao);
        glDeleteTextures(1, &tex_src);
        glDeleteTextures(1, &tex_lut);
    }

    virtual void render(double currentTime) override
    {
        static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, black);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_1D, tex_lut);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex_src);

        switch (mode)
        {
            case 0:
            {
                glUseProgram(program_naive);
            }
            break;
            case 1:
            {
                glUseProgram(program_exposure);
                glUniform1f(uniforms.exposure.exposure, exposure);
            }
            break;
            case 2:
            {
                glUseProgram(program_adaptive);
            }
            break;
        }

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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
                case '1':
                case '2':
                case '3':
                {
                    mode = key - '1';
                }
                break;
                case 'W': 
                {
                    wireframeMode = !wireframeMode;
                }
                break;
                case 'M':
                {
                    mode = (mode + 1) % 3;
                }
                break;
                case GLFW_KEY_KP_ADD:
                {
                    exposure *= 1.1f;
                }
                break;
                case GLFW_KEY_KP_SUBTRACT:
                {
                    exposure /= 1.1f;
                }
                break;
            }
        }
    }
};

//DECLARE_MAIN(Chapter9_HdrTonemap);

#include "../Book/object.h"
#include "../Book/sb6.h"
#include "../Book/sb6ktx.h"
#include "../Book/shader.h"
#include "../Book/vmath.h"

#include <math.h>

enum
{
    MAX_SCENE_WIDTH = 2048,
    MAX_SCENE_HEIGHT = 2048,
    SPHERE_COUNT = 32,
};

class Chapter9_HdrExposure : public sb6::application
{
public:
    GLuint texture;
    GLuint program;
    GLuint vao;

    float exposure;
    bool wireframeMode;    

public:
    Chapter9_HdrExposure()
    {
        wireframeMode = false;
        exposure = 1.0f;
        program = 0;
    }

    virtual void init() override
    {
        sb6::application::init();

        info.samples = 8;
    }

    void CompileShader()
    {   
        static const GLchar* vertShader[] =
        {
            R"vs(

            #version 420 core

            void main(void)
            {
                const vec4 vertices[] = vec4[]
                (
                    vec4(-1.0, -1.0, 0.5, 1.0),
                    vec4(1.0, -1.0, 0.5, 1.0),
                    vec4(-1.0, 1.0, 0.5, 1.0),
                    vec4(1.0, 1.0, 0.5, 1.0)
                );
                
                gl_Position = vertices[gl_VertexID];
            }

            )vs"
        };

        static const GLchar* fragShader[] =
        {
            R"fs(

            #version 420 core

            uniform sampler2D s;
            uniform float exposure;

            out vec4 color;

            void main()
            {
                vec4 c = texture(s, 2.0 * gl_FragCoord.xy / textureSize(s, 0));
                c.xyz = vec3(1.0) - exp(-c.xyz * exposure);
                color = c;
            }
                
            )fs"
        };


        char logBuffer[1024];
        GLint success;

        program = glCreateProgram();

        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, vertShader, nullptr);
        glCompileShader(vs);

        glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
        glGetShaderInfoLog(vs, 1024, NULL, logBuffer);

        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, fragShader, nullptr);
        glCompileShader(fs);
        glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
        glGetShaderInfoLog(fs, 1024, NULL, logBuffer);

        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glLinkProgram(program);
        
        glDeleteShader(vs);
        glDeleteShader(fs);
    }

    virtual void startup() override
    {        
        glGenTextures(1, &texture);

        sb6::ktx::file::load("../Media/textures/treelights_2k.ktx", texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        CompileShader();
    }

    virtual void shutdown() override
    {
        glDeleteProgram(program);
        glDeleteVertexArrays(1, &vao);
    }

    virtual void render(double currentTime) override
    {
        static const GLfloat green[] = { 0.0f, 0.25f, 0.0f, 1.0f };
        
        glClearBufferfv(GL_COLOR, 0, green);

        glUseProgram(program);
        glViewport(0, 0, info.windowWidth, info.windowHeight);

        glUniform1f(0, exposure);
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

//DECLARE_MAIN(Chapter9_HdrExposure);
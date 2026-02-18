
#include "../Book/object.h"
#include "../Book/sb6.h"
#include "../Book/sb6ktx.h"
#include "../Book/shader.h"
#include "../Book/vmath.h"

#include <math.h>

static unsigned int seed = 0x13371337;

static inline float random_float()
{
    float res;
    unsigned int tmp;

    seed *= 16807;

    tmp = seed ^ (seed >> 4) ^ (seed << 15);

    *((unsigned int*)&res) = (tmp >> 9) | 0x3F800000;

    return (res - 1.0f);
}

enum
{
    NUM_STARS = 2000,
};


class Chapter9_ShapedPoints : public sb6::application
{
public:
    GLuint program_prog;
    GLuint render_vao;

public:
    Chapter9_ShapedPoints()
    {
        program_prog = 0;
        render_vao = 0;
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

            flat out int shape;

            void main(void)
            {
                const vec4[4] position = vec4[4]
                (
                    vec4(-0.4, -0.4, 0.5, 1.0),
                    vec4(0.4, -0.4, 0.5, 1.0),
                    vec4(-0.4, 0.4, 0.5, 1.0),
                    vec4(0.4, 0.4, 0.5, 1.0)
                );

                gl_Position = position[gl_VertexID];
                shape = gl_VertexID;
            }

            )vs"
        };

        static const GLchar* fragShader[] =
        {
            R"fs(

            #version 420 core

            flat in int shape;

            layout (location = 0) out vec4 color;

            void main()
            {
                color = vec4(1.0);
                vec2 p = gl_PointCoord * 2.0 - vec2(1.0);
                if (shape == 0)
                {
                    if (dot(p, p) > 1.0)
                    {
                        discard;
                    }
                }
                else if (shape == 1)
                {
                    if (dot(p, p) > sin(atan(p.y, p.x) * 5.0))
                    {
                        discard;
                    }
                }
                else if (shape == 2)
                {
                    if (abs(0.8 - dot(p, p)) > 0.2)
                    {
                        discard;
                    }
                }
                else if (shape == 3)
                {
                    if (abs(p.x) < abs(p.y))
                    {
                        discard;
                    }
                }
            }
                
            )fs"
        };


        char logBuffer[1024];
        GLint success;

        program_prog = glCreateProgram();

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

        glAttachShader(program_prog, vs);
        glAttachShader(program_prog, fs);
        glLinkProgram(program_prog);

        glDeleteShader(vs);
        glDeleteShader(fs);
    }

    virtual void startup() override
    {   
        CompileShader();

        glGenVertexArrays(1, &render_vao);
        glBindVertexArray(render_vao);
    }

    virtual void shutdown() override
    {
        glDeleteProgram(program_prog);
        glDeleteVertexArrays(1, &render_vao);
    }

    virtual void render(double currentTime) override
    {
        static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        static const GLfloat one = 1.0f;

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, black);
        glClearBufferfv(GL_DEPTH, 0, &one);

        float t = (float)currentTime;

        glUseProgram(program_prog);;

        glEnable(GL_PROGRAM_POINT_SIZE);
        glPointSize(200.0f);        
        glDrawArrays(GL_POINTS, 0, 4);
    }

    virtual void onResize(int w, int h) override
    {
        sb6::application::onResize(w, h);
    }

    virtual void onKey(int key, int action) override 
    {
        if (action != 0)
        {
        }
    }
};

//DECLARE_MAIN(Chapter9_ShapedPoints);
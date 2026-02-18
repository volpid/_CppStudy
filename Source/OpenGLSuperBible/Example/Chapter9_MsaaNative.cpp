
#include "../Book/object.h"
#include "../Book/sb6.h"
#include "../Book/sb6ktx.h"
#include "../Book/shader.h"
#include "../Book/vmath.h"

#include <math.h>

class Chapter9_MSAANative : public sb6::application
{
public:
    GLuint program;
    GLuint vao;
    GLuint position_buffer;
    GLuint index_buffer;
    GLuint mv_location;
    GLuint proj_location;;

    bool wireframeMode;

public:
    Chapter9_MSAANative()
    {
        wireframeMode = false;
    }

    virtual void init() override
    {
        sb6::application::init();

        info.samples = 4;
    }

    void CompileShader()
    {
        static const GLchar* vertShader[] =
        {
            R"vs(

            #version 420 core

            in vec4  position;

            uniform mat4 mv_matrix;
            uniform mat4 proj_matrix;

            void main(void)
           {
                gl_Position = proj_matrix * mv_matrix * position;
            }

            )vs"
        };

        static const GLchar* fragShader[] =
        {
            R"fs(

            #version 420 core

            out vec4 color;

            void main()
            {
                color = vec4(1.0);
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

        mv_location = glGetUniformLocation(program, "mv_matrix");
        proj_location = glGetUniformLocation(program, "proj_matrix");

        glDeleteShader(vs);
        glDeleteShader(fs);
    }

    virtual void startup() override
    {
        CompileShader();
        static const GLushort vertex_indices[] =
        {
            0, 1, 2,
            2, 1, 3,
            2, 3, 4,
            4, 3, 5,
            4, 5, 6,
            6, 5, 7,
            6, 7, 0,
            0, 7, 1,
            6, 0, 2,
            2, 4, 6,
            7, 5, 3,
            7, 3, 1
        };

        static const GLfloat vertex_positions[] =
        {
            -0.25f, -0.25f, -0.25f,
            -0.25f,  0.25f, -0.25f,
             0.25f, -0.25f, -0.25f,
             0.25f,  0.25f, -0.25f,
             0.25f, -0.25f,  0.25f,
             0.25f,  0.25f,  0.25f,
            -0.25f, -0.25f,  0.25f,
            -0.25f,  0.25f,  0.25f,
        };

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &position_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_positions), vertex_positions, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
        glEnableVertexAttribArray(0);

        glGenBuffers(1, &index_buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vertex_indices), vertex_indices, GL_STATIC_DRAW);

        glEnable(GL_CULL_FACE);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
    }

    virtual void shutdown() override
    {
        glDeleteProgram(program);
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &position_buffer);
        glDeleteBuffers(1, &index_buffer);
    }

    virtual void render(double currentTime) override
    {
        static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        static const GLfloat one = 1.0f;

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, black);
        glClearBufferfv(GL_DEPTH, 0, &one);

        glUseProgram(program);

        vmath::mat4 proj_matrix = vmath::perspective(50.0f, (float)info.windowWidth / (float)info.windowHeight, 0.1f, 1000.0f);
        glUniformMatrix4fv(proj_location, 1, GL_FALSE, proj_matrix);

        //#define MANY_CUBE
#ifdef MANY_CUBE
        for (int idx = 0; idx < 24; ++idx)
        {
            float f = (float) idx + (float) currentTime * 0.3;
            vmath::mat4 mv_matrix = vmath::translate(0.0f, 0.0f, -20.0f)
                * vmath::rotate((float) currentTime * 45.0f, 0.0f, 1.0f, 0.0f)
                * vmath::rotate((float) currentTime * 21.0f, 1.0f, 0.0f, 0.0f)
                * vmath::translate(sinf(2.1f *f) * 2.0f, cosf(1.7 * f) * 2.0f, sinf(1.3f * f) * cosf(1.5f * f) * 2.0f);

            glUniformMatrix4fv(mv_location, 1, GL_FALSE, mv_matrix);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
        }
#else
        
        float f = (float) currentTime * 0.3;
        currentTime = 3.15;
        vmath::mat4 mv_matrix = vmath::translate(0.0f, 0.0f, -4.0f)
            * vmath::rotate((float) currentTime * 45.0f, 0.0f, 1.0f, 0.0f)
            * vmath::rotate((float) currentTime * 21.0f, 1.0f, 0.0f, 0.0f);

        glUniformMatrix4fv(mv_location, 1, GL_FALSE, mv_matrix);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
#endif //MANY_CUBE
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
            }
        }
    }
};

//DECLARE_MAIN(Chapter9_MSAANative);
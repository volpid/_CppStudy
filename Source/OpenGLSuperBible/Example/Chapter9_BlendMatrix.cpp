
#include "../Book/object.h"
#include "../Book/sb6.h"
#include "../Book/sb6ktx.h"
#include "../Book/shader.h"
#include "../Book/vmath.h"

#include <math.h>

class Chapter9_BlendMatrix : public sb6::application
{
public:
    GLuint program;
    GLuint vao;
    GLuint position_buffer;
    GLuint index_buffer;

    GLuint mv_location;
    GLuint proj_location;

    bool wireframeMode;

public:
    Chapter9_BlendMatrix()
    {
        wireframeMode = false;
    }

    void CompileShader()
    {
        static const GLchar* vertShader[] =
        {
            R"vs(

            #version 420 core

            in vec4 position;

            out VS_OUT
            {
                vec4 color0;
                vec4 color1;
            } vs_out;

            uniform mat4 mv_matrix;
            uniform mat4 proj_matrix;

            void main(void)
            {
                gl_Position = proj_matrix * mv_matrix * position;
                vs_out.color0 = position * 2.0 + vec4(0.5, 0.5, 0.5, 0.0);
                vs_out.color1 = vec4(0.5, 0.5, 0.5, 0.0) - position * 2.0;
            }

            )vs"
        };

        static const GLchar* fragShader[] =
        {
            R"fs(

            #version 420 core

            in VS_OUT
            {
                vec4 color0;
                vec4 color1;
            } fs_in;

            layout (location = 0, index = 0) out vec4 color0;
            layout (location = 0, index = 1) out vec4 color1;

            void main()
            {
                color0 = vec4(fs_in.color0.xzy, 1.0);
                color1 = vec4(fs_in.color1.xzy, 1.0);
            }
                
            )fs"
        };
        
        program = glCreateProgram();

        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, vertShader, nullptr);
        glCompileShader(vs);

        char logBuffer[1024];
        GLint success;
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
    }


    virtual void startup() override
    {
        CompileShader();

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

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

        glGenBuffers(1, &position_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_positions), vertex_positions, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
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
        static const GLfloat orange[] = { 0.6f, 0.4f, 0.1f, 1.0f };
        static const GLfloat one = 1.0f;
        static const GLenum blend_func[] =
        {
            GL_ZERO,
            GL_ONE,
            GL_SRC_COLOR,
            GL_ONE_MINUS_SRC_COLOR,
            GL_DST_COLOR,
            GL_ONE_MINUS_DST_COLOR,
            GL_SRC_ALPHA,
            GL_ONE_MINUS_SRC_ALPHA,
            GL_DST_ALPHA,
            GL_ONE_MINUS_DST_ALPHA,
            GL_CONSTANT_COLOR,
            GL_ONE_MINUS_CONSTANT_COLOR,
            GL_CONSTANT_ALPHA,
            GL_ONE_MINUS_CONSTANT_ALPHA,
            GL_SRC_ALPHA_SATURATE,
            GL_SRC1_COLOR,
            GL_ONE_MINUS_SRC1_COLOR,
            GL_SRC1_ALPHA,
            GL_ONE_MINUS_SRC1_ALPHA
        };

        static const int num_blend_funcs = sizeof(blend_func) / sizeof(blend_func[0]);
        static const float x_scale = 20.0f / float(num_blend_funcs);
        static const float y_scale = 16.0f / float(num_blend_funcs);
        const float t = (float) currentTime;

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, orange);
        glClearBufferfv(GL_DEPTH, 0, &one);

        glUseProgram(program);

        vmath::mat4 proj_matrix = vmath::perspective(50.0f, (float)info.windowWidth / (float)info.windowHeight, 0.1f, 1000.0f);
        
        glUniformMatrix4fv(proj_location, 1, GL_FALSE, proj_matrix);

        glEnable(GL_BLEND);
        glBlendColor(0.2f, 0.5f, 0.7f, 0.5f);
        for (int idx = 0; idx < num_blend_funcs; ++idx)
        {
            for (int sub = 0; sub < num_blend_funcs; ++sub)
            {
                vmath::mat4 mv_matrix = vmath::translate(9.5f - x_scale * (float) sub, 7.5f - y_scale * (float) idx, - 18.0f)
                    * vmath::rotate((float)currentTime * -45.0f, 0.0f, 1.0f, 0.0f)
                    * vmath::rotate((float)currentTime * -21.0f, 1.0f, 0.0f, 0.0f);

                glUniformMatrix4fv(mv_location, 1, GL_FALSE, mv_matrix);
                glBlendFunc(blend_func[sub], blend_func[idx]);
                glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
            }
        }
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

//DECLARE_MAIN(Chapter9_BlendMatrix);

#include "../Book/sb6.h"
#include "../Book/sb6ktx.h"
#include "../Book/vmath.h"

#include <math.h>

class Chapter7_Instancing : public sb6::application
{
public:
    virtual void render(double currentTime) override
    {
        static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        glClearBufferfv(GL_COLOR, 0, black);

        glUseProgram(renderingProgram);

        glBindVertexArray(vertexArrayObject);
        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, 4);
    }

    virtual void startup() override
    {
        static const GLfloat square_vertices[] =
        {
            -1.0f, -1.0f, 0.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f,
             1.0f,  1.0f, 0.0f, 1.0f,
            -1.0f,  1.0f, 0.0f, 1.0f
        };

        static const GLfloat instance_colors[] =
        {
            1.0f, 0.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 0.0f, 1.0f
        };

        static const GLfloat instance_positions[] =
        {
            -2.0f, -2.0f, 0.0f, 0.0f,
             2.0f, -2.0f, 0.0f, 0.0f,
             2.0f,  2.0f, 0.0f, 0.0f,
            -2.0f,  2.0f, 0.0f, 0.0f
        };

        GLuint offset = 0;

        glGenVertexArrays(1, &vertexArrayObject);        
        glGenBuffers(1, &buffer);

        glBindVertexArray(vertexArrayObject);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, 
            sizeof(square_vertices) + sizeof(instance_colors) + sizeof(instance_positions),
            nullptr, 
            GL_STATIC_DRAW);

        glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(square_vertices), square_vertices);
        offset += sizeof(square_vertices);
        glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(instance_colors), instance_colors);
        offset += sizeof(instance_colors);
        glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(instance_positions), instance_positions);
        offset += sizeof(instance_positions);

        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*) sizeof(square_vertices));
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*) (sizeof(square_vertices) + sizeof(instance_colors)));

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);

        glVertexAttribDivisor(1, 1);
        glVertexAttribDivisor(2, 1);

        renderingProgram = CompileShader();
    }

    virtual void onResize(int w, int h) override
    {
        sb6::application::onResize(w, h);
    }


    GLuint CompileShader()
    {
        GLuint vertexShader;
        GLuint fragmentShader;
        GLuint program;

        
        static const GLchar* vertShader[] =
        {
            R"vs(
                #version 430 core

                layout (location = 0) in vec4 position;
                layout (location = 1) in vec4 instance_color;
                layout (location = 2) in vec4 instance_position;

                out Fragment
                {
                    vec4 color;
                } fragment;

                void main()
                {
                    gl_Position = (position + instance_position) * vec4(0.25, 0.25, 1.0, 1.0);
                    fragment.color = instance_color;
                }
            )vs"
        };

        static const GLchar* fragShader[] =
        {
            R"fs(
                #version 430 core

                in Fragment
                {
                    vec4 color;
                } fragment;

                out vec4 output_color;

                void main()
                {
                    output_color = fragment.color;
                }
            )fs"
        };

        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, vertShader, nullptr);
        glCompileShader(vertexShader);

        char logBuffer[1024];
        GLint success;
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        glGetShaderInfoLog(vertexShader, 1024, NULL, logBuffer);

        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, fragShader, nullptr);
        glCompileShader(fragmentShader);

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        glGetShaderInfoLog(fragmentShader, 1024, NULL, logBuffer);

        program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        // 유니폼 확인용 코드
        GLint count;
        glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count);
        for (GLint idx = 0; idx < count; ++idx)
        {
            char uniformName[256] = {};
            GLsizei length;
            GLint size;
            GLenum type;

            glGetActiveUniform(program, idx, sizeof(uniformName), &length, &size, &type, uniformName);

            GLint location = glGetUniformLocation(program, uniformName);
        }

        // 프로그램에 링크 후 기존 쉐이더 삭제
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return program;
    }

    virtual void shutdown()
    {
        glDeleteVertexArrays(1, &vertexArrayObject);
        glDeleteProgram(renderingProgram);
    }

private:
    GLuint renderingProgram;
    GLuint vertexArrayObject;
    GLuint buffer;

};

//DECLARE_MAIN(Chapter7_Instancing);
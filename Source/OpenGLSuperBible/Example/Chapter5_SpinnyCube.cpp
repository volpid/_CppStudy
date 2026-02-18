
#include "../Book/sb6.h"
#include "../Book/vmath.h"

#include <math.h>


class Chapter5_SpinnyCube : public sb6::application
{
public:
    virtual void render(double currentTime) override
    {
        // 한점 렌더링
        glViewport(0, 0, info.windowWidth, info.windowHeight);

        const GLfloat color[] = { 0.0f, 0.2f, 0.0f, 1.0f };
        const GLfloat one = 1.0f;

        glClearBufferfv(GL_COLOR, 0, color);
        glClearBufferfv(GL_DEPTH, 0, &one);

//#define DRAW_ONE
#ifdef DRAW_ONE
        float f = (float) currentTime * (float) M_PI * 0.3f;
        vmath::mat4 mv_matrix = vmath::translate(0.0f, 0.0f, -4.0f) 
            * vmath::translate(sinf(2.1f * f) * 0.5f, cosf(1.7f * f) * 0.5f, sinf(1.3f * f) * cosf(1.5f * f) * 2.0f)
            * vmath::rotate((float) currentTime * 45.0f, 0.0f, 1.0f, 0.0f)
            * vmath::rotate((float) currentTime * 81.0f, 1.0f, 0.0f, 0.0f);

        glUseProgram(renderingProgram);

        glUniformMatrix4fv(mv_location, 1, GL_FALSE, mv_matrix);
        glUniformMatrix4fv(proj_location, 1, GL_FALSE, proj_matrix);

        glDrawArrays(GL_TRIANGLES, 0, 36);
#else
        for (int32_t idx = 0; idx < 24; ++idx)
        {
            float f = (float)currentTime * 0.3f + idx;
            vmath::mat4 mv_matrix = vmath::translate(0.0f, 0.0f, -20.0f)
                * vmath::rotate((float)currentTime * 45.0f, 0.0f, 1.0f, 0.0f)
                * vmath::rotate((float)currentTime * 21.0f, 1.0f, 0.0f, 0.0f)
                * vmath::translate(sinf(2.1f * f) * 2.0f, cosf(1.7f * f) * 2.0f, sinf(1.3f * f) * cosf(1.5f * f) * 2.0f);

            glUseProgram(renderingProgram);

            glUniformMatrix4fv(mv_location, 1, GL_FALSE, mv_matrix);
            glUniformMatrix4fv(proj_location, 1, GL_FALSE, proj_matrix);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        
#endif //DRAW_ONE
    }

    virtual void startup() override
    {
        renderingProgram = CompileShader();

        mv_location = glGetUniformLocation(renderingProgram, "mv_matrix");
        proj_location = glGetUniformLocation(renderingProgram, "proj_matrix");
        
        glGenVertexArrays(1, &vertexArrayObject);
        glBindVertexArray(vertexArrayObject);
        
        static const GLfloat vertex_position[] =
        {
            -0.25f,  0.25f, -0.25f,
            -0.25f, -0.25f, -0.25f,
             0.25f, -0.25f, -0.25f,

             0.25f, -0.25f, -0.25f,
             0.25f,  0.25f, -0.25f,
            -0.25f,  0.25f, -0.25f,

             0.25f, -0.25f, -0.25f,
             0.25f, -0.25f,  0.25f,
             0.25f,  0.25f, -0.25f,

             0.25f, -0.25f,  0.25f,
             0.25f,  0.25f,  0.25f,
             0.25f,  0.25f, -0.25f,

             0.25f, -0.25f,  0.25f,
            -0.25f, -0.25f,  0.25f,
             0.25f,  0.25f,  0.25f,

            -0.25f, -0.25f,  0.25f,
            -0.25f,  0.25f,  0.25f,
             0.25f,  0.25f,  0.25f,

            -0.25f, -0.25f,  0.25f,
            -0.25f, -0.25f, -0.25f,
            -0.25f,  0.25f,  0.25f,

            -0.25f, -0.25f, -0.25f,
            -0.25f,  0.25f, -0.25f,
            -0.25f,  0.25f,  0.25f,

            -0.25f, -0.25f,  0.25f,
             0.25f, -0.25f,  0.25f,
             0.25f, -0.25f, -0.25f,

             0.25f, -0.25f, -0.25f,
            -0.25f, -0.25f, -0.25f,
            -0.25f, -0.25f,  0.25f,

            -0.25f,  0.25f, -0.25f,
             0.25f,  0.25f, -0.25f,
             0.25f,  0.25f,  0.25f,

             0.25f,  0.25f,  0.25f,
            -0.25f,  0.25f,  0.25f,
            -0.25f,  0.25f, -0.25f,
        };

        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CW);

        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_position), vertex_position, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);
    }

    virtual void onResize(int w, int h) override
    {
        sb6::application::onResize(w, h);

        aspect = (float) info.windowWidth / (float) info.windowHeight;
        proj_matrix = vmath::perspective(50.0f, aspect, 0.1f, 1000.0f);
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

            layout(location = 0) in vec4 position;

            out VS_OUT
            {
                vec4 color;
            } vs_out;

            uniform mat4 mv_matrix;
            uniform mat4 proj_matrix;

            layout (location = 17) uniform vec4 myUniform;

            void main()
            {
                gl_Position = proj_matrix * mv_matrix * position;
                vs_out.color = position * 2.0 + vec4(0.5, 0.5, 0.5, 0.0);
            }

            )vs"
        };

        static const GLchar* fragShader[] =
        {
            R"fs(

            #version 430 core

            in VS_OUT
            {
                vec4 color;
            } fs_in;

            layout (location = 0) out vec4 color;

            void main()
            {
                color = fs_in.color;
            }
                
            )fs"
        };

        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, vertShader, nullptr);
        glCompileShader(vertexShader);

        GLint success;
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, fragShader, nullptr);
        glCompileShader(fragmentShader);

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

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
        //glDeleteVertexArrays(1, &vertexArrayObject);
    }

private:
    GLuint renderingProgram;
    GLuint vertexArrayObject;

    GLuint buffer;

    GLint mv_location;
    GLint proj_location;

    float aspect;
    vmath::mat4 proj_matrix;
};

//DECLARE_MAIN(Chapter5_SpinnyCube);
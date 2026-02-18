
#include "../Book/sb6.h"

#include <math.h>

class SimpleTestApp : public sb6::application
{
public:
    virtual void render(double currentTime) override
    {
        // RED 그리기
        /*            
        const GLfloat color[] = { 1.0f, 0.0f, 0.0f, 1.0f };
        glClearBufferfv(GL_COLOR, 0, color);
        */

        // 변경된는 색
        /*
        const GLfloat color[] = { (float)sin(currentTime) * 0.5 + 0.5, cos(currentTime) * 0.5 + 0.5, 0.0f, 1.0f };
        glClearBufferfv(GL_COLOR, 0, color);
        */

        // 한점 렌더링
        const GLfloat color[] = { 0.0f, 0.2f, 0.0f, 1.0f };
        glClearBufferfv(GL_COLOR, 0, color);

        glUseProgram(renderingProgram);

        GLfloat attrib[] = 
        {
            (float) sin(currentTime) * 0.5f,
            (float) cos(currentTime) * 0.6f,
            0.0f, 
            0.0f,
        };

        glVertexAttrib4fv(0, attrib);

        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    virtual void startup() override
    {
        renderingProgram = CompileShader();
        
        glGenVertexArrays(1, &vertexArrayObject);
        glBindVertexArray(vertexArrayObject);
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

            layout(location = 0) in vec4 offset;
            layout(location = 1) in vec4 color;

            out VS_OUT
            {
                //layout(location = 0) vec4 color;
                vec4 color;
            } vs_out;

            void main()
            {
                const vec4 vertices[3] = vec4[3]
                (
                    vec4(0.25, -0.25, 0.5, 1.0f),
                    vec4(-0.25, -0.25, 0.5, 1.0f),
                    vec4(0.25, 0.25, 0.5, 1.0f)
                );

                gl_Position = vertices[gl_VertexID] + offset;
                vs_out.color = color;
            }

            )vs"
        };

        static const GLchar* fragShader[] =
        {
            R"fs(

            #version 430 core

            in VS_OUT
            {
                //layout(location = 0) vec4 color;
                vec4 color;
            } fs_in;

            layout(location = 0) out vec4 color;

            void main()
            {
                color = fs_in.color;
            }
                
            )fs"
        };

        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, vertShader, nullptr);
        glCompileShader(vertexShader);

        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, fragShader, nullptr);
        glCompileShader(vertexShader);

        program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

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
};

//DECLARE_MAIN(SimpleTestApp);

#include "../Book/object.h"
#include "../Book/sb6.h"
#include "../Book/sb6ktx.h"
#include "../Book/shader.h"
#include "../Book/vmath.h"

#include <math.h>

class Chapter8_TessellatedGSTri : public sb6::application
{
public:
    GLuint program;
    GLuint vao;

    bool wireframeMode;

public:
    Chapter8_TessellatedGSTri()
    {
        wireframeMode = false;
    }

    void CompileShader()
    {
        static const GLchar* vertShader[] =
        {
            R"vs(

            #version 420 core

            void main(void)
            {
                const vec4 vertices[] = vec4[](vec4(0.25, -0.25, 0.5, 1.0),
                    vec4(-0.25, -0.25, 0.5, 1.0),
                    vec4(0.25, 0.25, 0.5, 1.0));
                gl_Position = vertices[gl_VertexID];
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
                color = vec4(0.0,  0.8, 1.0, 1.0);
            }
                
            )fs"
        };

        static const GLchar* tscSource[] =
        {
            R"tsc(

            #version 420 core

            layout (vertices  = 3) out;

            void main(void)
            {
                if (gl_InvocationID == 0)
                {
                    gl_TessLevelInner[0] = 5.0;
                    gl_TessLevelOuter[0] = 5.0;
                    gl_TessLevelOuter[1] = 5.0;
                    gl_TessLevelOuter[2] = 5.0;

                }

                gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
            }

            )tsc"
        };

        static const GLchar* tesSource[] =
        {
            R"tes(
                
                #version 420 core

                layout (triangles, equal_spacing, cw) in;

                void main(void)
                {
                    gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position)
                        + (gl_TessCoord.y * gl_in[1].gl_Position)
                        + (gl_TessCoord.z * gl_in[2].gl_Position);
                }
            )tes"
        };

        static const GLchar* gsSource[] =
        {
            R"gs(
                
                #version 420 core

                layout (triangles) in;
                layout (points, max_vertices = 3) out;

                void main(void)
                {
                    int i = 0;
                    for (i = 0; i < gl_in.length(); ++i)
                    {
                        gl_Position = gl_in[i].gl_Position;
                        EmitVertex();
                    }
                }
            )gs"
        };


        char logBuffer[1024];
        GLint success;

        program = glCreateProgram();
        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, fragShader, nullptr);
        glCompileShader(fs);
        
        glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
        glGetShaderInfoLog(fs, 1024, NULL, logBuffer);

        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, vertShader, nullptr);
        glCompileShader(vs);

        glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
        glGetShaderInfoLog(vs, 1024, NULL, logBuffer);

        GLuint tcs = glCreateShader(GL_TESS_CONTROL_SHADER);
        glShaderSource(tcs, 1, tscSource, nullptr);
        glCompileShader(tcs);

        glGetShaderiv(tcs, GL_COMPILE_STATUS, &success);
        glGetShaderInfoLog(tcs, 1024, NULL, logBuffer);

        GLuint tes = glCreateShader(GL_TESS_EVALUATION_SHADER);
        glShaderSource(tes, 1, tesSource, nullptr);
        glCompileShader(tes);

        glGetShaderiv(tes, GL_COMPILE_STATUS, &success);
        glGetShaderInfoLog(tes, 1024, NULL, logBuffer);

        GLuint gs = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(gs, 1, gsSource, nullptr);
        glCompileShader(gs);

        glGetShaderiv(gs, GL_COMPILE_STATUS, &success);
        glGetShaderInfoLog(gs, 1024, NULL, logBuffer);

        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glAttachShader(program, tcs);
        glAttachShader(program, tes);
        glAttachShader(program, gs);

        glLinkProgram(program);

        GLint status;
        glGetProgramiv(program, GL_LINK_STATUS, &status);

        if (!status)
        {
            char buffer[4096];
            glGetProgramInfoLog(program, 4096, NULL, buffer);
            glDeleteProgram(program);
        }

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
    }


    virtual void startup() override
    {
        CompileShader();

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glEnable(GL_CULL_FACE);
    }

    virtual void shutdown() override
    {
        glDeleteProgram(program);
        glDeleteVertexArrays(1, &vao);
    }

    virtual void render(double currentTime) override
    {
        static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        
        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, black);
        
        glUseProgram(program);

        glPolygonMode(GL_FRONT_AND_BACK, wireframeMode ? GL_LINE : GL_FILL);

        glDrawArrays(GL_PATCHES, 0, 3);
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

//DECLARE_MAIN(Chapter8_TessellatedGSTri);
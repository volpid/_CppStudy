
#include "../Book/object.h"
#include "../Book/sb6.h"
#include "../Book/sb6ktx.h"
#include "../Book/shader.h"
#include "../Book/vmath.h"

#include <math.h>

class Chapter8_TessellatedQuad : public sb6::application
{
public:
    GLuint program[4];
    GLuint vao;

    int programIndex = 0;

    bool wireframeMode;

public:
    Chapter8_TessellatedQuad()
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
                const vec4 vertices[] = vec4[](vec4(0.4, -0.4, 0.5, 1.0),
                    vec4(-0.4, -0.4, 0.5, 1.0),
                    vec4(0.4, 0.4, 0.5, 1.0),
                    vec4(-0.4, 0.4, 0.5, 1.0));

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
                color = vec4(1.0);
            }
                
            )fs"
        };

        
        static const GLchar* tcsTriangles[] =
        {
            R"tcs(

            #version 420 core

            layout (vertices = 3) out;

            void main(void)
            {
                if (gl_InvocationID == 0)
                {
                    gl_TessLevelInner[0] = 5.0;
                    gl_TessLevelOuter[0] = 8.0;
                    gl_TessLevelOuter[1] = 8.0;
                    gl_TessLevelOuter[2] = 8.0;
                }

                gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
            }

            )tcs"
        };


        static const GLchar* tesTriangles[] =
        {
            R"tes(

            #version 420 core

            layout (triangles) in;

            void main(void)
            {
                gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position)
                    + (gl_TessCoord.y * gl_in[1].gl_Position)
                    + (gl_TessCoord.z * gl_in[2].gl_Position);
            }

            )tes"
        };

        static const GLchar* tesTrianglesAsPoints[] =
        {
            R"tes(
                
                #version 420 core

                layout (triangles, point_mode) in;

                void main(void)
                {
                    gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position)
                        + (gl_TessCoord.y * gl_in[1].gl_Position)
                        + (gl_TessCoord.z * gl_in[2].gl_Position);
                }
            )tes"
        };

        static const GLchar* tcsSourceQuads[] =
        {
            R"tcs(

            #version 420 core

            layout (vertices = 4) out;

            void main(void)
            {
                if (gl_InvocationID == 0)
                {
                    gl_TessLevelInner[0] = 9.0;
                    gl_TessLevelInner[1] = 7.0;
                    gl_TessLevelOuter[0] = 3.0;
                    gl_TessLevelOuter[1] = 5.0;
                    gl_TessLevelOuter[2] = 3.0;
                    gl_TessLevelOuter[3] = 5.0;
                }

                gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
            }

            )tcs"
        };

        static const GLchar* tesSourceQuads[] =
        {
            R"tes(

            #version 420 core

            layout (quads) in;

            void main(void)
            {
                vec4 p1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
                vec4 p2 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x);
                gl_Position = mix(p1, p2, gl_TessCoord.y);
            }

            )tes"
        };

        static const GLchar* tcsSourceIsolines[] =
        {
            R"tcs(

            #version 420 core

            layout (vertices = 4) out;

            void main(void)
            {
                if (gl_InvocationID == 0)
                {
                    gl_TessLevelOuter[0] = 5.0;
                    gl_TessLevelOuter[1] = 5.0;
                }

                gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
            }

            )tcs"
        };

        static const GLchar* tesSourceIsolines[] =
        {
            R"tes(

            #version 420 core

            layout (isolines) in;

            void main(void)
            {
                vec4 r = (gl_TessCoord.y + gl_TessCoord.x / gl_TessLevelOuter[0]);
                vec4 t = gl_TessCoord.x * 2.0 * 3.14159;
                gl_Position = vec4(sin(t) * r, cos(t) * r, 0.5, 1.0);
            }

            )tes"
        };

        static const char* const* vs_sources[] = { vertShader, vertShader, vertShader, vertShader };
        static const char* const* fs_sources[] = { fragShader, fragShader, fragShader, fragShader };
        static const char* const* tcs_sources[] = { tcsSourceQuads, tcsTriangles, tcsTriangles, tcsSourceIsolines };
        static const char* const* tec_sources[] = { tesSourceQuads, tesTriangles, tesTrianglesAsPoints, tesSourceIsolines };

        for (int idx = 0; idx < 4; ++idx)
        {
            char logBuffer[1024];
            GLint success;

            program[idx] = glCreateProgram();

            GLuint vs = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vs, 1, vs_sources[idx], nullptr);
            glCompileShader(vs);

            glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
            glGetShaderInfoLog(vs, 1024, NULL, logBuffer);

            GLuint tcs = glCreateShader(GL_TESS_CONTROL_SHADER);
            glShaderSource(tcs, 1, tcs_sources[idx], nullptr);
            glCompileShader(tcs);

            glGetShaderiv(tcs, GL_COMPILE_STATUS, &success);
            glGetShaderInfoLog(tcs, 1024, NULL, logBuffer);
            
            GLuint tes = glCreateShader(GL_TESS_EVALUATION_SHADER);
            glShaderSource(tes, 1, tec_sources[idx], nullptr);
            glCompileShader(tes);

            glGetShaderiv(tes, GL_COMPILE_STATUS, &success);
            glGetShaderInfoLog(tes, 1024, NULL, logBuffer);
            
            GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fs, 1, fs_sources[idx], nullptr);
            glCompileShader(fs);

            glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
            glGetShaderInfoLog(fs, 1024, NULL, logBuffer);

            glAttachShader(program[idx], vs);
            glAttachShader(program[idx], tcs);
            glAttachShader(program[idx], tes);
            glAttachShader(program[idx], fs);
            glLinkProgram(program[idx]);

            glDeleteShader(vs);
            glDeleteShader(tcs);
            glDeleteShader(tes);
            glDeleteShader(fs);
        }

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glPatchParameteri(GL_PATCH_VERTICES, 4);
    }


    virtual void startup() override
    {
        CompileShader();
    }

    virtual void shutdown() override
    {
        for (int32_t idx = 0; idx < 4; ++idx)
        {
            glDeleteProgram(program[idx]);
        }

        glDeleteVertexArrays(1, &vao);
    }

    virtual void render(double currentTime) override
    {
        static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        
        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, black);
        
        glPolygonMode(GL_FRONT_AND_BACK, wireframeMode ? GL_LINE : GL_FILL);

        glUseProgram(program[programIndex]);
        glDrawArrays(GL_PATCHES, 0, 4);
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
                case 'M':
                {
                    programIndex = (++programIndex) % 4;
                }
                break;
            }
        }
    }
};

//DECLARE_MAIN(Chapter8_TessellatedQuad);

#include "../Book/object.h"
#include "../Book/sb6.h"
#include "../Book/sb6ktx.h"
#include "../Book/shader.h"
#include "../Book/vmath.h"

#include <math.h>

class Chapter8_TessSubDivModes : public sb6::application
{
public:
    GLuint program[3];
    GLuint vao;

    int programIndex = 0;

    bool wireframeMode;

public:
    Chapter8_TessSubDivModes()
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
                const vec4 vertices[] = vec4[]
                (
                    vec4(0.8, -0.8, 0.5, 1.0),
                    vec4(-0.8, -0.8, 0.5, 1.0),
                    vec4(0.8, 0.8, 0.5, 1.0),
                    vec4(-0.8, 0.8, 0.5, 1.0)
                );

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


        static const GLchar* tcs_source_triangles[] =
        {
            R"tcs(

            #version 420 core

            layout (vertices = 3) out;

            uniform float tess_level = 2.7;

            void main(void)
            {
                if (gl_InvocationID == 0)
                {
                    gl_TessLevelInner[0] = tess_level;
                    gl_TessLevelOuter[0] = tess_level;
                    gl_TessLevelOuter[1] = tess_level;
                    gl_TessLevelOuter[2] = tess_level;
                }

                gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
            }

            )tcs"
        };


        static const GLchar* tes_source_equal[] =
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

        static const GLchar* tes_source_fract_even[] =
        {
            R"tes(
                
                #version 420 core

                layout (triangles, fractional_even_spacing) in;

                void main(void)
                {
                    gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position)
                        + (gl_TessCoord.y * gl_in[1].gl_Position)
                        + (gl_TessCoord.z * gl_in[2].gl_Position);
                }
            )tes"
        };

        static const GLchar* tes_source_fract_odd[] =
        {
            R"tes(
                
                #version 420 core

                layout (triangles, fractional_odd_spacing) in;

                void main(void)
                {
                    gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position)
                        + (gl_TessCoord.y * gl_in[1].gl_Position)
                        + (gl_TessCoord.z * gl_in[2].gl_Position);
                }
            )tes"
        };

        static const char* const* vs_sources[] = { vertShader, vertShader, vertShader };
        static const char* const* fs_sources[] = { fragShader, fragShader, fragShader };
        static const char* const* tcs_sources[] = { tcs_source_triangles, tcs_source_triangles, tcs_source_triangles };
        static const char* const* tec_sources[] = { tes_source_equal, tes_source_fract_even, tes_source_fract_odd };

        for (int idx = 0; idx < 3; ++idx)
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

        glUniform1f(0, sinf((float) currentTime * 0.5f) * 5.0f + 6.0f);


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
                programIndex = (++programIndex) % 3;
            }
            break;
            }
        }
    }
};

//DECLARE_MAIN(Chapter8_TessSubDivModes);
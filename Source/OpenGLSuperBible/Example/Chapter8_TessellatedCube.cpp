
#include "../Book/object.h"
#include "../Book/sb6.h"
#include "../Book/sb6ktx.h"
#include "../Book/shader.h"
#include "../Book/vmath.h"

#include <math.h>

class Chapter8_TessellatedCube : public sb6::application
{
public:
    GLuint program;
    GLuint vao;
    GLuint positionBuffer;
    GLuint indexBuffer;
    GLuint mvLocation;
    GLuint projLocation;

    bool wireframeMode;

public:
    Chapter8_TessellatedCube()
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

            void main(void)
            {
                gl_Position = position;
            }

            )vs"
        };

        static const GLchar* fragShader[] =
        {
            R"fs(

            #version 420 core

            in vec3 normal;
            out vec4 color;

            void main()
            {
                color = vec4(abs(normal), 1.0);
            }
                
            )fs"
        };

#if 1
        static const GLchar* tscSource[] =
        {
            R"tsc(

            #version 420 core

            layout (vertices  = 4) out;

            void main(void)
            {
                if (gl_InvocationID == 0)
                {
                    gl_TessLevelInner[0] = 4.0;
                    gl_TessLevelInner[1] = 4.0;
                    gl_TessLevelOuter[0] = 4.0;
                    gl_TessLevelOuter[1] = 4.0;
                    gl_TessLevelOuter[2] = 4.0;
                    gl_TessLevelOuter[3] = 4.0;

                    //gl_TessLevelInner[0] = 9.0;
                    //gl_TessLevelInner[1] = 7.0;
                    //gl_TessLevelOuter[0] = 3.0;
                    //gl_TessLevelOuter[1] = 5.0;
                    //gl_TessLevelOuter[2] = 3.0;
                    //gl_TessLevelOuter[3] = 5.0;

                }

                gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
            }

            )tsc"
        };

#else

        static const GLchar* tscSource[] =
        {
            R"tsc(

            #version 420 core

            layout (vertices = 4) out

            uniform mat4 mv_matrix;
            uniform mat4 proj_matrix;

            void main(void)
            {
                vec4 pos[4];
                float tf[4];
                float t = 0.0;

                int i;

                if (gl_InvocationID == 0)
                {
                    for (i = 0; i < 4; ++i)
                    {
                        pos[i] = proj_matrix * mv_matrix * gl_in[i].gl_Position;
                    }

                    tf[0] = max(2.0, distance(pos[0].xy / pos[0].w, pos[1].xy / pos[1].w) * 16.0;
                    tf[1] = max(2.0, distance(pos[1].xy / pos[1].w, pos[3].xy / pos[3].w) * 16.0;
                    tf[2] = max(2.0, distance(pos[2].xy / pos[2].w, pos[3].xy / pos[3].w) * 16.0;
                    tf[3] = max(2.0, distance(pos[2].xy / pos[2].w, pos[0].xy / pos[0].w) * 16.0;

                    for (i = 0; i < 4; ++i)
                    {
                        t = max(t, tf[i]);
                    }

                    gl_TessLevelInner[0] = t;
                    gl_TessLevelInner[1] = t;
                    gl_TessLevelOuter[0] = tf[0];
                    gl_TessLevelOuter[1] = tf[1];
                    gl_TessLevelOuter[2] = tf[2];
                    gl_TessLevelOuter[3] = tf[3];
                }

                gl_out[gl_InvocationID].gl_Position = gl_in(gl_InvocationID].gl_Position;
            }
            
            )tsc"
        };

#endif

        static const GLchar* tesSource[] =
        {
            R"tes(
                
                #version 420 core

                layout (quads, fractional_odd_spacing, ccw) in;

                uniform mat4 mv_matrix;
                uniform mat4 proj_matrix;

                out vec3 normal;

                void main(void)
                {
                    vec4 mid1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
                    vec4 mid2 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x);
                    vec4 pos = mix(mid1, mid2, gl_TessCoord.y);
                    pos.xyz = (pos.xyz) * 0.25;
                    normal = normalize(mat3(mv_matrix) *  pos.xyz);
                    gl_Position = proj_matrix * mv_matrix * pos;
                }

            )tes"
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

        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glAttachShader(program, tcs);
        glAttachShader(program, tes);

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

        mvLocation = glGetUniformLocation(program, "mv_matrix");
        projLocation = glGetUniformLocation(program, "proj_matrix");

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        static const GLushort vertex_indices[] =
        {
            0, 1, 2, 3,
            2, 3, 4, 5,
            4, 5, 6, 7,
            6, 7, 0, 1,
            0, 2, 6, 4,
            1, 7, 3, 5
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

        glGenBuffers(1, &positionBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_positions), vertex_positions, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);

        glGenBuffers(1, &indexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vertex_indices), vertex_indices, GL_STATIC_DRAW);

        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
    }

    virtual void shutdown() override
    {
        glDeleteProgram(program);
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &positionBuffer);
        glDeleteBuffers(1, &indexBuffer);
    }

    virtual void render(double currentTime) override
    {
        static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        static const GLfloat One = 1.0f;

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, black);
        glClearBufferfv(GL_DEPTH, 0, &One);

        glUseProgram(program);

        vmath::mat4 projMatrix = vmath::perspective(50.0f, 
            (float) info.windowWidth / (float) info.windowHeight,
            0.1f,
            1000.0f);

        glUniformMatrix4fv(projLocation, 1, GL_FALSE, projMatrix);
        glPatchParameteri(GL_PATCH_VERTICES, 4);

        glPolygonMode(GL_FRONT_AND_BACK, wireframeMode ? GL_LINE : GL_FILL);

//#define MANY_CUBE
#ifdef MANY_CUBE
        for (int i = 0; i < 100; ++i)
        {
            float f = (float) i + (float) currentTime * 0.03f;
            vmath::mat4 mv_matrix = vmath::translate(0.0f, 0.0f, -10.0f)
                * vmath::translate(sinf(2.1f * f) * 4.0f, 
                    cosf(1.7f * f) * 4.0f, 
                    sinf(4.3f * f) * cosf(3.5f * f) * 30.0f)
                * vmath::rotate((float) currentTime * 3.0f, 1.0f, 0.0f, 0.0f)
                * vmath::rotate((float) currentTime * 5.0f, 0.0f, 1.0f, 0.0f);

            glUniformMatrix4fv(mvLocation, 1, GL_FALSE, mv_matrix);
            glDrawElements(GL_PATCHES, 24, GL_UNSIGNED_SHORT, 0);
        }
#else
        float f = (float) currentTime * 0.3f;

        //vmath::mat4 mv_matrix = vmath::translate(0.0f, 0.0f, -3.0f)
        //    * vmath::translate(0.0f, 0.0f, sinf(1.3f * f) * cosf(1.5f * f) * 15.0f)
        //    * vmath::rotate((float) currentTime * 81.0f, 1.0f, 0.0f, 0.0f);


        vmath::mat4 mv_matrix = vmath::translate(0.0f, 0.0f, -3.0f)
            * vmath::rotate((float) currentTime * 5.1f, 1.0f, 0.0f, 0.0f)
            * vmath::scale(10.0f, 10.0f, 10.0f);

        glUniformMatrix4fv(mvLocation, 1, GL_FALSE, mv_matrix);
        glDrawElements(GL_PATCHES, 24, GL_UNSIGNED_SHORT, 0);
#endif /**/

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

//DECLARE_MAIN(Chapter8_TessellatedCube);
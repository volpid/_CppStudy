
#include "../Book/object.h"
#include "../Book/sb6.h"
#include "../Book/sb6ktx.h"
#include "../Book/shader.h"
#include "../Book/vmath.h"

#include <math.h>

class Chapter9_MultiScissor : public sb6::application
{
public:
    GLuint program;
    GLuint vao;
    GLuint position_buffer;
    GLuint index_buffer;
    GLuint uniform_buffer;

    bool wireframeMode;

public:
    Chapter9_MultiScissor()
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
                vec4 color;
            } vs_out;

            void main(void)
            {
                gl_Position = position;
                vs_out.color = position * 2.0 + vec4(0.5, 0.5, 0.5, 0.0);
            }

            )vs"
        };

        static const GLchar* gsShader[] =
        {
            R"gs(

            #version 420 core

            layout (triangles, invocations = 4) in;
            layout (triangle_strip, max_vertices = 3) out;

            layout (std140, binding = 0) 
            uniform transform_block
            {
                mat4 mvp_matrix[4];
            };

            in VS_OUT
            {
                vec4 color;
            } gs_in[];

            out GS_OUT
            {
                vec4 color;
            } gs_out;

            void main(void)
            {
                for (int i = 0; i < gl_in.length(); ++i)
                {
                    gs_out.color = gs_in[i].color;
                    gl_Position = mvp_matrix[gl_InvocationID] * gl_in[i].gl_Position;
                    gl_ViewportIndex = gl_InvocationID;
                    
                    EmitVertex();
                }
                EndPrimitive();
            }
            
            )gs"
        };

        static const GLchar* fragShader[] =
        {
            R"fs(

            #version 420 core
            
            in GS_OUT
            {
                vec4 color;
            } fs_in;

            out vec4 color;

            void main()
            {
                color = fs_in.color;
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

        GLuint gs = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(gs, 1, gsShader, nullptr);
        glCompileShader(gs);
        glGetShaderiv(gs, GL_COMPILE_STATUS, &success);
        glGetShaderInfoLog(gs, 1024, NULL, logBuffer);

        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, fragShader, nullptr);
        glCompileShader(fs);
        glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
        glGetShaderInfoLog(fs, 1024, NULL, logBuffer);

        glAttachShader(program, vs);
        glAttachShader(program, gs);
        glAttachShader(program, fs);
        glLinkProgram(program);

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

        glGenBuffers(1, &uniform_buffer);
        glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
        glBufferData(GL_UNIFORM_BUFFER, 4 * sizeof(vmath::mat4), nullptr, GL_DYNAMIC_DRAW);

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
        glDeleteBuffers(1, &uniform_buffer);
    }

    virtual void render(double currentTime) override
    {
        static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        static const GLfloat one = 1.0f;

        glDisable(GL_SCISSOR_TEST);

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, black);
        glClearBufferfv(GL_DEPTH, 0, &one);

        glEnable(GL_SCISSOR_TEST);

        int scissor_width = (7 * info.windowWidth) / 16;
        int scissor_height = (7 * info.windowHeight) / 16;

        glScissorIndexed(0, 0, 0, scissor_width, scissor_height);        
        glScissorIndexed(1, info.windowWidth - scissor_width, 0, scissor_width, scissor_height);
        glScissorIndexed(2, 0, info.windowHeight - scissor_height, scissor_width, scissor_height);
        glScissorIndexed(3, info.windowWidth - scissor_width, info.windowHeight - scissor_height, scissor_width, scissor_height);

        glUseProgram(program);

        vmath::mat4 proj_matrix = vmath::perspective(50.0f, (float)info.windowWidth / (float)info.windowHeight, 0.1f, 1000.0f);

        float f = (float) currentTime * 0.3f;

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniform_buffer);
        vmath::mat4* mv_matrix_array = (vmath::mat4*) glMapBufferRange(GL_UNIFORM_BUFFER, 
            0, 
            4 * sizeof(vmath::mat4),
            GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

        for (int idx = 0; idx < 4; ++idx)
        {
            mv_matrix_array[idx] = proj_matrix 
                * vmath::translate(0.0f, 0.0f, -1.0f)
                * vmath::rotate((float) currentTime * 45.0f * (float)(idx + 1), 0.0f, 1.0f, 0.0f)
                * vmath::rotate((float) currentTime * 81.0f * (float)(idx + 1), 1.0f, 0.0f, 0.0f);
        }

        glUnmapBuffer(GL_UNIFORM_BUFFER);

        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
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

//DECLARE_MAIN(Chapter9_MultiScissor);
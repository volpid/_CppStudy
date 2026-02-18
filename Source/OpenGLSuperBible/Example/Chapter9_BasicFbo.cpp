
#include "../Book/object.h"
#include "../Book/sb6.h"
#include "../Book/sb6ktx.h"
#include "../Book/shader.h"
#include "../Book/vmath.h"

#include <math.h>

class Chapter9_BasicFBO : public sb6::application
{
public:
    GLuint program1;
    GLuint program2;
    GLuint vao;

    GLuint position_buffer;
    GLuint index_buffer;

    GLuint fbo;
    GLuint color_texture;
    GLuint depth_texture;

    GLuint mv_location;
    GLuint proj_location;
    GLuint mv_location2;
    GLuint proj_location2;

    bool wireframeMode;

public:
    Chapter9_BasicFBO()
    {
        wireframeMode = false;
    }

    void CompileShader()
    {
        static const GLchar* vertShader[] =
        {
            R"vs(

            #version 420 core

            layout (location = 0) in vec4 position;
            layout (location = 1) in vec2 texcoord;

            out VS_OUT
            {
                vec4 color;
                vec2 texcoord;
            } vs_out;

            uniform mat4 mv_matrix;
            uniform mat4 proj_matrix;

            void main(void)
            {
                gl_Position = proj_matrix * mv_matrix * position;
                vs_out.color = position * 2.0 + vec4(0.5, 0.5, 0.5, 0.0);
                vs_out.texcoord = texcoord;
            }

            )vs"
        };

        static const GLchar* fragShader1[] =
        {
            R"fs(

            #version 420 core

            in VS_OUT
            {
                vec4 color;
                vec2 texcoord;
            } fs_in;

            out vec4 color;

            void main()
            {
                color = sin(fs_in.color * vec4(40.0, 20.0, 30.0, 1.0)) * 0.5 + vec4(0.5);
            }
                
            )fs"
        };

        static const GLchar* fragShader2[] =
        {
            R"fs(

            #version 420 core

            in VS_OUT
            {
                vec4 color;
                vec2 texcoord;
            } fs_in;

            out vec4 color;

            uniform sampler2D tex;

            void main()
            {
                color = mix(fs_in.color, texture(tex, fs_in.texcoord), 0.7);
            }
                
            )fs"
        };

        char logBuffer[1024];
        GLint success;

        program1 = glCreateProgram();

        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, vertShader, nullptr);
        glCompileShader(vs);

        glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
        glGetShaderInfoLog(vs, 1024, NULL, logBuffer);

        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, fragShader1, nullptr);
        glCompileShader(fs);
        glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
        glGetShaderInfoLog(fs, 1024, NULL, logBuffer);

        glAttachShader(program1, vs);
        glAttachShader(program1, fs);
        glLinkProgram(program1);

        glDeleteShader(vs);
        glDeleteShader(fs);

        program2 = glCreateProgram();

        vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, vertShader, nullptr);
        glCompileShader(vs);

        glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
        glGetShaderInfoLog(vs, 1024, NULL, logBuffer);

        fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, fragShader2, nullptr);
        glCompileShader(fs);
        glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
        glGetShaderInfoLog(fs, 1024, NULL, logBuffer);

        glAttachShader(program2, vs);
        glAttachShader(program2, fs);
        glLinkProgram(program2);

        glDeleteShader(vs);
        glDeleteShader(fs);

        mv_location = glGetUniformLocation(program1, "mv_matrix");
        proj_location = glGetUniformLocation(program1, "proj_matrix");
        mv_location2 = glGetUniformLocation(program2, "mv_matrix");
        proj_location2 = glGetUniformLocation(program2, "proj_matrix");
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

        static const GLfloat vertex_data[] =
        {
            // Position                 Tex Coord
           -0.25f, -0.25f,  0.25f,      0.0f, 1.0f,
           -0.25f, -0.25f, -0.25f,      0.0f, 0.0f,
            0.25f, -0.25f, -0.25f,      1.0f, 0.0f,

            0.25f, -0.25f, -0.25f,      1.0f, 0.0f,
            0.25f, -0.25f,  0.25f,      1.0f, 1.0f,
           -0.25f, -0.25f,  0.25f,      0.0f, 1.0f,

            0.25f, -0.25f, -0.25f,      0.0f, 0.0f,
            0.25f,  0.25f, -0.25f,      1.0f, 0.0f,
            0.25f, -0.25f,  0.25f,      0.0f, 1.0f,

            0.25f,  0.25f, -0.25f,      1.0f, 0.0f,
            0.25f,  0.25f,  0.25f,      1.0f, 1.0f,
            0.25f, -0.25f,  0.25f,      0.0f, 1.0f,

            0.25f,  0.25f, -0.25f,      1.0f, 0.0f,
           -0.25f,  0.25f, -0.25f,      0.0f, 0.0f,
            0.25f,  0.25f,  0.25f,      1.0f, 1.0f,

           -0.25f,  0.25f, -0.25f,      0.0f, 0.0f,
           -0.25f,  0.25f,  0.25f,      0.0f, 1.0f,
            0.25f,  0.25f,  0.25f,      1.0f, 1.0f,

           -0.25f,  0.25f, -0.25f,      1.0f, 0.0f,
           -0.25f, -0.25f, -0.25f,      0.0f, 0.0f,
           -0.25f,  0.25f,  0.25f,      1.0f, 1.0f,

           -0.25f, -0.25f, -0.25f,      0.0f, 0.0f,
           -0.25f, -0.25f,  0.25f,      0.0f, 1.0f,
           -0.25f,  0.25f,  0.25f,      1.0f, 1.0f,

           -0.25f,  0.25f, -0.25f,      0.0f, 1.0f,
            0.25f,  0.25f, -0.25f,      1.0f, 1.0f,
            0.25f, -0.25f, -0.25f,      1.0f, 0.0f,

            0.25f, -0.25f, -0.25f,      1.0f, 0.0f,
           -0.25f, -0.25f, -0.25f,      0.0f, 0.0f,
           -0.25f,  0.25f, -0.25f,      0.0f, 1.0f,

           -0.25f, -0.25f,  0.25f,      0.0f, 0.0f,
            0.25f, -0.25f,  0.25f,      1.0f, 0.0f,
            0.25f,  0.25f,  0.25f,      1.0f, 1.0f,

            0.25f,  0.25f,  0.25f,      1.0f, 1.0f,
           -0.25f,  0.25f,  0.25f,      0.0f, 1.0f,
           -0.25f, -0.25f,  0.25f,      0.0f, 0.0f,
        };

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &position_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*) (3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);

        glGenBuffers(1, &index_buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vertex_indices), vertex_indices, GL_STATIC_DRAW);

        glEnable(GL_CULL_FACE);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glGenTextures(1, &color_texture);
        glBindTexture(GL_TEXTURE_2D, color_texture);
        glTexStorage2D(GL_TEXTURE_2D, 9, GL_RGBA8, 512, 512);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glGenTextures(1, &depth_texture);
        glBindTexture(GL_TEXTURE_2D, depth_texture);
        glTexStorage2D(GL_TEXTURE_2D, 9, GL_DEPTH_COMPONENT32F, 512, 512);

        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color_texture, 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_texture, 0);

        static const GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, draw_buffers);
    }

    virtual void shutdown() override
    {
        glDeleteProgram(program1);
        glDeleteProgram(program2);
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &position_buffer);
        glDeleteBuffers(1, &index_buffer);

        glDeleteFramebuffers(1, &fbo);
        glDeleteTextures(1, &color_texture);

    }

    virtual void render(double currentTime) override
    {
        static const GLfloat green[] = { 0.0f, 0.1f, 0.0f, 1.0f };
        static const GLfloat blue[] = { 0.0f, 0.0f, 0.3f, 1.0f };
        static const GLfloat one = 1.0f;

        vmath::mat4 proj_matrix = vmath::perspective(50.0f, (float)info.windowWidth / (float)info.windowHeight, 0.1f, 1000.0f);
        float f = (float) currentTime * 0.3f;

        vmath::mat4 mv_matrix = vmath::translate(0.0f, 0.0f, -4.0f)
            * vmath::translate(sinf(2.1f * f) * 0.5f, cosf(1.7f * f) * 0.5f, sinf(1.3f * f) * cosf(1.5f * f) * 2.0f)
            * vmath::rotate((float)currentTime * 45.0f, 0.0f, 1.0f, 0.0f)
            * vmath::rotate((float)currentTime * 81.0f, 1.0f, 0.0f, 0.0f);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glViewport(0, 0, 512, 512);
        glClearBufferfv(GL_COLOR, 0, green);
        glClearBufferfv(GL_DEPTH, 0, &one);

        glUseProgram(program1);

        glUniformMatrix4fv(proj_location, 1, GL_FALSE, proj_matrix);
        glUniformMatrix4fv(mv_location, 1, GL_FALSE, mv_matrix);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, blue);
        glClearBufferfv(GL_DEPTH, 0, &one);

        glBindTexture(GL_TEXTURE_2D, color_texture);

        glUseProgram(program2);

        glUniformMatrix4fv(proj_location2, 1, GL_FALSE, proj_matrix);
        glUniformMatrix4fv(mv_location2, 1, GL_FALSE, mv_matrix);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glBindTexture(GL_TEXTURE_2D, 0);
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

//DECLARE_MAIN(Chapter9_BasicFBO);
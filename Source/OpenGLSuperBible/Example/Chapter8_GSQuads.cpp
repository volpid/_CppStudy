
#include "../Book/object.h"
#include "../Book/sb6.h"
#include "../Book/sb6ktx.h"
#include "../Book/shader.h"
#include "../Book/vmath.h"

#include <math.h>

class Chapter8_GSQuads : public sb6::application
{
public:    
    GLuint program_fans;
    GLuint program_linesadjacency;
    GLuint vao;

    int mode;
    int mvp_loc_fans;
    int mvp_loc_linesadj;
    int vid_offset_loc_fans;
    int vid_offset_loc_linesadj;
    int vid_offset;

    bool paused;
    bool wireframeMode;

public:
    Chapter8_GSQuads()
    {
        program_fans = 0;
        program_linesadjacency = 0;
        vao = 0;

        mode = 0;
        mvp_loc_fans = 0;
        mvp_loc_linesadj = 0;
        vid_offset_loc_fans = 0;
        vid_offset_loc_linesadj = 0;
        vid_offset = 0;

        paused = false;
        wireframeMode = false;
    }

    void CompileShader()
    {
        GLuint vs = 0;
        GLuint gs = 0;
        GLuint fs = 0;

        if (program_fans != 0)
        {
            glDeleteProgram(program_fans);
        }

        program_fans = glCreateProgram();
        vs = sb6::shader::load("../Media/shaders/gsquads/quadsasfans.vs.glsl", GL_VERTEX_SHADER);
        fs = sb6::shader::load("../Media/shaders/gsquads/quadsasfans.fs.glsl", GL_FRAGMENT_SHADER);

        glAttachShader(program_fans, vs);
        glAttachShader(program_fans, fs);
        glLinkProgram(program_fans);

        // 유니폼 확인용 코드
        GLint count;
        glGetProgramiv(program_fans, GL_ACTIVE_UNIFORMS, &count);
        for (GLint idx = 0; idx < count; ++idx)
        {
            char uniformName[256] = {};
            GLsizei length;
            GLint size;
            GLenum type;

            glGetActiveUniform(program_fans, idx, sizeof(uniformName), &length, &size, &type, uniformName);

            GLint location = glGetUniformLocation(program_fans, uniformName);
        }

        mvp_loc_fans = glGetUniformLocation(program_fans, "mvp");
        vid_offset_loc_fans = glGetUniformLocation(program_fans, "vid_offset");

        glDeleteShader(vs);
        glDeleteShader(fs);

        if (program_linesadjacency != 0)
        {
            glDeleteProgram(program_linesadjacency);
        }

        program_linesadjacency = glCreateProgram();
        vs = sb6::shader::load("../Media/shaders/gsquads/quadsaslinesadj.vs.glsl", GL_VERTEX_SHADER);
        gs = sb6::shader::load("../Media/shaders/gsquads/quadsaslinesadj.gs.glsl", GL_GEOMETRY_SHADER);
        fs = sb6::shader::load("../Media/shaders/gsquads/quadsaslinesadj.fs.glsl", GL_FRAGMENT_SHADER);
        glAttachShader(program_linesadjacency, vs);
        glAttachShader(program_linesadjacency, gs);
        glAttachShader(program_linesadjacency, fs);
        glLinkProgram(program_linesadjacency);

        // 유니폼 확인용 코드
        glGetProgramiv(program_fans, GL_ACTIVE_UNIFORMS, &count);
        for (GLint idx = 0; idx < count; ++idx)
        {
            char uniformName[256] = {};
            GLsizei length;
            GLint size;
            GLenum type;

            glGetActiveUniform(program_linesadjacency, idx, sizeof(uniformName), &length, &size, &type, uniformName);

            GLint location = glGetUniformLocation(program_linesadjacency, uniformName);
        }

        mvp_loc_linesadj = glGetUniformLocation(program_linesadjacency, "mvp");
        vid_offset_loc_linesadj = glGetUniformLocation(program_linesadjacency, "vid_offset");

        glDeleteShader(vs);
        glDeleteShader(gs);
        glDeleteShader(fs);
    }

    virtual void startup() override
    {
        CompileShader();

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        //glEnable(GL_CULL_FACE);
    }

    virtual void shutdown() override
    {
        glDeleteProgram(program_fans);
        glDeleteProgram(program_linesadjacency);
        glDeleteVertexArrays(1, &vao);
    }

    virtual void render(double currentTime) override
    {
        static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };        
        glViewport(0, 0, info.windowWidth, info.windowHeight);

        static double last_time = 0.0;
        static double total_time = 0.0;

        if (paused == false)
        {
            total_time += (currentTime - last_time);
        }
        last_time = currentTime;
        float t = (float) total_time;

        glClearBufferfv(GL_COLOR, 0, black);

        vmath::mat4 mv_matrix = vmath::translate(0.0f, 0.0f, -2.0f)
            * vmath::rotate((float) t * 5.0f, 0.0f, 0.0f, 1.0f)
            * vmath::rotate((float) t * 30.0f, 1.0f, 0.0f, 0.0f);
        vmath::mat4 proj_matrix = vmath::perspective(50.0f, (float) info.windowWidth / (float) info.windowHeight, 0.1f, 1000.0f);
        vmath::mat4 mvp = proj_matrix * mv_matrix;

        glPolygonMode(GL_FRONT_AND_BACK, wireframeMode ? GL_LINE : GL_FILL);

        switch (mode)
        {
            case 0:
            {
                glUseProgram(program_fans);
                glUniformMatrix4fv(mvp_loc_fans, 1, GL_FALSE, mvp);
                glUniform1i(vid_offset_loc_fans, vid_offset);
                glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            }
            break;
            case 1:
            {
                glUseProgram(program_linesadjacency);
                glUniformMatrix4fv(mvp_loc_linesadj, 1, GL_FALSE, mvp);
                glUniform1i(vid_offset_loc_linesadj, vid_offset);
                glDrawArrays(GL_LINES_ADJACENCY, 0, 4);
            }
            break;
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
                case '1':
                case '2':
                {
                    mode = key - '1';
                }
                break;
                case GLFW_KEY_KP_ADD:
                {
                    vid_offset++;
                }
                break;
                case GLFW_KEY_KP_SUBTRACT:
                {
                    vid_offset--;
                }
                break;
                case 'M':
                {
                    mode = (mode + 1) % 2;
                }
                break;
            }
        }
    }
};

//DECLARE_MAIN(Chapter8_GSQuads);
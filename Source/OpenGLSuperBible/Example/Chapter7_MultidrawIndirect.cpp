
#include "../Book/object.h"
#include "../Book/sb6.h"
#include "../Book/sb6ktx.h"
#include "../Book/shader.h"
#include "../Book/vmath.h"

#include <math.h>

enum
{
    NUM_DRAWS = 50000
};

struct DrawArraysIndirectCommand
{
    GLuint count;
    GLuint primeCount;
    GLuint first;
    GLuint baseInstance;
};

class Chapter7_MultidrawIndirect : public sb6::application
{
public:
    enum Mode
    {
        Mode_First,
        Mode_Multidraw,
        Mode_SeperateDraw,
        Mode_Max,
    };

    struct Uniform
    {
        GLint time;
        GLint viewMatrix;
        GLint projMatrix;
        GLint viewProjMatrix;
    };

    Uniform uniforms;

    GLuint renderProgram;
    sb6::object object;
    GLuint indirectDrawBuffer;
    GLuint drawIndexBuffer;
    Mode mode;
    bool paused;
    bool vsync;

public:
    Chapter7_MultidrawIndirect()
        : renderProgram(0),
        mode(Mode_Multidraw),
        paused(false),
        vsync(false)
    {

    }

    virtual void startup() override
    {
        loadShader();

        object.load("../Media/objects/asteroids.sbm");

        glGenBuffers(1, &indirectDrawBuffer);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectDrawBuffer);
        glBufferData(GL_DRAW_INDIRECT_BUFFER, 
            NUM_DRAWS * sizeof(DrawArraysIndirectCommand), 
            nullptr,
            GL_STATIC_DRAW);

        DrawArraysIndirectCommand* cmd = (DrawArraysIndirectCommand*) glMapBufferRange(GL_DRAW_INDIRECT_BUFFER, 
            0, 
            NUM_DRAWS * sizeof(DrawArraysIndirectCommand),
            GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

        for (int idx = 0; idx < NUM_DRAWS; ++idx)
        {
            object.get_sub_object_info(idx % object.get_sub_object_count(), cmd[idx].first, cmd[idx].count);
            cmd[idx].primeCount = 1;
            cmd[idx].baseInstance = idx;
        }

        glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);

        glBindVertexArray(object.get_vao());

        glGenBuffers(1, &drawIndexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, drawIndexBuffer);
        glBufferData(GL_ARRAY_BUFFER, NUM_DRAWS * sizeof(GLuint), nullptr, GL_STATIC_DRAW);

        GLuint* drawIndex = (GLuint*) glMapBufferRange(GL_ARRAY_BUFFER, 0, NUM_DRAWS * sizeof(GLuint), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

        for (int idx = 0; idx < NUM_DRAWS; ++idx)
        {
            drawIndex[idx] = idx;
        }

        glUnmapBuffer(GL_ARRAY_BUFFER);

        glVertexAttribIPointer(10, 1, GL_UNSIGNED_INT, 0, nullptr);
        glVertexAttribDivisor(10, 1);
        glEnableVertexAttribArray(10);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glEnable(GL_CULL_FACE);
    }

    void loadShader()
    {
        GLuint shaders[2];

        shaders[0] = sb6::shader::load("../Media/shaders/multidrawindirect/render.vs.glsl", GL_VERTEX_SHADER);
        shaders[1] = sb6::shader::load("../Media/shaders/multidrawindirect/render.fs.glsl", GL_FRAGMENT_SHADER);

        if (renderProgram != 0)
        {
            glDeleteProgram(renderProgram);
        }

        renderProgram = sb6::program::link_from_shaders(shaders, 2, true);

        // 유니폼 확인용 코드
        GLint count;
        glGetProgramiv(renderProgram, GL_ACTIVE_UNIFORMS, &count);
        for (GLint idx = 0; idx < count; ++idx)
        {
            char uniformName[256] = {};
            GLsizei length;
            GLint size;
            GLenum type;

            glGetActiveUniform(renderProgram, idx, sizeof(uniformName), &length, &size, &type, uniformName);

            GLint location = glGetUniformLocation(renderProgram, uniformName);
        }

        uniforms.time = glGetUniformLocation(renderProgram, "time");
        uniforms.viewMatrix = glGetUniformLocation(renderProgram, "view_matrix");
        uniforms.projMatrix = glGetUniformLocation(renderProgram, "proj_matrix");
        uniforms.viewProjMatrix = glGetUniformLocation(renderProgram, "viewproj_matrix");
    }

    virtual void render(double currentTime) override
    {
        static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        static const GLfloat One = 1.0f;

        static double lastTime = 0.0;
        static double totalTime = 0.0;

        if (paused == false)
        {
            totalTime += (currentTime - lastTime);
        }
        lastTime = currentTime;

        float t = (float) totalTime;
        int i = (int) (totalTime * 3.0f);

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, black);
        glClearBufferfv(GL_DEPTH, 0, &One);

        const vmath::mat4 view_matrix = vmath::lookat(vmath::vec3(100.0f * cosf(t * 0.023f), 100.0 * cosf(t * 0.023f), 300.0f * sinf(t * 0.037f) - 600.0f),
            vmath::vec3(0.0f, 0.0f, 260.0f),
            vmath::normalize(vmath::vec3(0.1f - cosf(t * 0.1f) * 0.3f, 1.0f, 0.0f)));

        const vmath::mat4 proj_matrix = vmath::perspective(50.0f,
            (float) info.windowWidth / (float) info.windowHeight,
            1.0f,
            2000.0f);

        glUseProgram(renderProgram);

        glUniform1f(uniforms.time, t);
        glUniformMatrix4fv(uniforms.viewMatrix, 1, GL_FALSE, view_matrix);
        glUniformMatrix4fv(uniforms.projMatrix, 1, GL_FALSE, proj_matrix);
        glUniformMatrix4fv(uniforms.viewProjMatrix, 1, GL_FALSE, proj_matrix * view_matrix);

        glBindVertexArray(object.get_vao());

        if (mode == Mode_Multidraw)
        {
            glMultiDrawArraysIndirect(GL_TRIANGLES, nullptr, NUM_DRAWS, 0);
        }
        else
        {
            for (int j = 0; j < NUM_DRAWS; ++j)
            {
                GLuint first, count;
                object.get_sub_object_info(j % object.get_sub_object_count(), first, count);
                glDrawArraysInstancedBaseInstance(GL_TRIANGLES, first, count, 1, j);
            }
        }
    }

    virtual void onResize(int w, int h) override
    {
        sb6::application::onResize(w, h);
    }

    virtual void shutdown()
    {
        glDeleteProgram(renderProgram);
    }

    virtual void onKey(int key, int action) override 
    {
        if (action != 0)
        {
            switch (key)
            {
                case 'P': 
                {
                    paused = !paused;
                }
                break;
                case 'V':
                {
                    vsync = !vsync;
                }
                break;
                case 'D':
                {
                    mode = Mode(mode + 1);
                    if (mode > Mode::Mode_Max)
                    {
                        mode = Mode::Mode_First;
                    }
                }
                break;
                case 'R':
                {
                    loadShader();
                }
                break;
            }
        }
    }
};

//DECLARE_MAIN(Chapter7_MultidrawIndirect);
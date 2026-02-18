
#include "../Book/object.h"
#include "../Book/sb6.h"
#include "../Book/sb6ktx.h"
#include "../Book/shader.h"
#include "../Book/vmath.h"

#include <cstdio>
#include <math.h>


enum
{
    WORKGROUP_SIZE = 256,
    NUM_WORKGROUPS = 64,
    FLOCK_SIZE = (NUM_WORKGROUPS * WORKGROUP_SIZE),
};


class Chapter10_CsFlocking : public sb6::application
{
public:
    GLuint flock_update_program = 0;
    GLuint flock_render_program = 0;

    GLuint flock_buffer[2];    
    GLuint flock_render_vao[2];
    GLuint geometry_buffer;

    struct FlockMember
    {
        vmath::vec3 position;
        unsigned int : 32;
        vmath::vec3 velocity;
        unsigned int : 32;
    };

    struct
    {
        struct
        {
            GLint goal;
        } update;
        struct
        {
            GLint mvp;
        } render;
    } uniforms;

    GLuint frame_index = 0;

public:
    Chapter10_CsFlocking()
    {
        
    }

    virtual void init() override
    {
        sb6::application::init();

        info.samples = 1;
    }

    virtual void startup() override
    {   
         LoadShader();

        static const vmath::vec3 geometry[] =
        {
            // Positions
            vmath::vec3(-5.0f, 1.0f, 0.0f),
            vmath::vec3(-1.0f, 1.5f, 0.0f),
            vmath::vec3(-1.0f, 1.5f, 7.0f),
            vmath::vec3(0.0f, 0.0f, 0.0f),
            vmath::vec3(0.0f, 0.0f, 10.0f),
            vmath::vec3(1.0f, 1.5f, 0.0f),
            vmath::vec3(1.0f, 1.5f, 7.0f),
            vmath::vec3(5.0f, 1.0f, 0.0f),

            // Normals
            vmath::vec3(0.0f),
            vmath::vec3(0.0f),
            vmath::vec3(0.107f, -0.859f, 0.00f),
            vmath::vec3(0.832f, 0.554f, 0.00f),
            vmath::vec3(-0.59f, -0.395f, 0.00f),
            vmath::vec3(-0.832f, 0.554f, 0.00f),
            vmath::vec3(0.295f, -0.196f, 0.00f),
            vmath::vec3(0.124f, 0.992f, 0.00f),
        };

        glGenBuffers(2, flock_buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, flock_buffer[0]);
        glBufferData(GL_SHADER_STORAGE_BUFFER, FLOCK_SIZE * sizeof(FlockMember), nullptr, GL_DYNAMIC_COPY);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, flock_buffer[1]);
        glBufferData(GL_SHADER_STORAGE_BUFFER, FLOCK_SIZE * sizeof(FlockMember), nullptr, GL_DYNAMIC_COPY);

        glGenBuffers(1, &geometry_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, geometry_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(geometry), geometry, GL_STATIC_DRAW);

        glGenVertexArrays(2, flock_render_vao);

        for (int idx = 0; idx < 2; ++idx)
        {
            glBindVertexArray(flock_render_vao[idx]);
            glBindBuffer(GL_ARRAY_BUFFER, geometry_buffer);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*) (8 * sizeof(vmath::vec3)));

            glBindBuffer(GL_ARRAY_BUFFER, flock_buffer[idx]);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(FlockMember), nullptr);
            glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(FlockMember), (void*) sizeof(vmath::uvec4));
            glVertexAttribDivisor(2, 1);
            glVertexAttribDivisor(3, 1);

            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            glEnableVertexAttribArray(2);
            glEnableVertexAttribArray(3);
        }

        glBindBuffer(GL_ARRAY_BUFFER, flock_buffer[0]);
        FlockMember* ptr = reinterpret_cast<FlockMember*>(glMapBufferRange(GL_ARRAY_BUFFER, 0, FLOCK_SIZE * sizeof(FlockMember), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
        for (int idx = 0; idx < FLOCK_SIZE; ++idx)
        {
            ptr[idx].position = (vmath::vec3::random() - vmath::vec3(0.5f)) * 300.0f;
            ptr[idx].velocity = (vmath::vec3::random() - vmath::vec3(0.5f));
        }
        glUnmapBuffer(GL_ARRAY_BUFFER);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
    }

    void LoadShader()
    {
        if (flock_update_program != 0)
        {
            glDeleteProgram(flock_update_program);
        }
        if (flock_render_program != 0)
        {
            glDeleteProgram(flock_render_program);
        }

        struct
        {
            GLuint vs;
            GLuint fs;
            GLuint cs;
        } shaders;

        shaders.cs = sb6::shader::load("../Media/shaders/flocking/flocking.cs.glsl", GL_COMPUTE_SHADER);
        flock_update_program = sb6::program::link_from_shaders(&shaders.cs, 1, true);

        uniforms.update.goal = glGetUniformLocation(flock_update_program, "goal");

        shaders.vs = sb6::shader::load("../Media/shaders/flocking/render.vs.glsl", GL_VERTEX_SHADER);
        shaders.fs = sb6::shader::load("../Media/shaders/flocking/render.fs.glsl", GL_FRAGMENT_SHADER);
        flock_render_program = sb6::program::link_from_shaders(&shaders.vs, 2, true);

        uniforms.render.mvp = glGetUniformLocation(flock_render_program, "mvp");
    }

    virtual void render(double currentTime) override
    {
        float t = (float) currentTime;
        static const float black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        static const float ones = 1.0f;

        glUseProgram(flock_update_program);

        vmath::vec3 goal = vmath::vec3(sinf(t * 0.34f), cosf(t * 0.29f), sinf(t * 0.12f) * cosf(t * 0.5f));
        goal = goal * vmath::vec3(35.0f, 25.0f, 60.0f);

        glUniform3fv(uniforms.update.goal, 1, goal);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, flock_buffer[frame_index]);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, flock_buffer[frame_index ^ 0x1]);

        glDispatchCompute(NUM_WORKGROUPS, 1, 1);

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, black);
        glClearBufferfv(GL_DEPTH, 0, &ones);

        glUseProgram(flock_render_program);

        vmath::mat4 mv_matrix = vmath::lookat(vmath::vec3(0.0f, 0.0f, -400.0f), 
            vmath::vec3(0.0f, 0.0f, 0.0f),
            vmath::vec3(0.0f, 1.0f, 0.0f));

        vmath::mat4 proj_matrix = vmath::perspective(60.0f,
            (float)info.windowWidth / (float)info.windowHeight,
            0.1f,
            3000.0f);

        vmath::mat4 mvp = proj_matrix * mv_matrix;
        glUniformMatrix4fv(uniforms.render.mvp, 1, GL_FALSE, mvp);

        glBindVertexArray(flock_render_vao[frame_index]);

        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 8, FLOCK_SIZE);

        frame_index ^= 1;
    }

    virtual void shutdown() override
    {
        glDeleteProgram(flock_render_program);
        glDeleteProgram(flock_update_program);
    }

    virtual void onResize(int w, int h) override
    {
        sb6::application::onResize(w, h);
    }

    virtual void onKey(int key, int action) override 
    {
        if (action != 0)
        {
        }
    }
};

//DECLARE_MAIN(Chapter10_CsFlocking);
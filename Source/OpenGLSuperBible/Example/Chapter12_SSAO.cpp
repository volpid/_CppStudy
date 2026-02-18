
#include "../Book/object.h"
#include "../Book/sb6.h"
#include "../Book/sb6ktx.h"
#include "../Book/shader.h"
#include "../Book/vmath.h"

#include <cstdio>
#include <math.h>

enum
{
    MAX_DISPLAY_WIDTH = 2048,
    MAX_DISPLAY_HEIGHT = 2048,
};
static unsigned int seed = 0x13371337;

static inline float random_float()
{
    float res;
    unsigned int tmp;

    seed *= 16807;

    tmp = seed ^ (seed >> 4) ^ (seed << 15);

    *((unsigned int*)&res) = (tmp >> 9) | 0x3F800000;

    return (res - 1.0f);
}

class Chapter12_SSAO : public sb6::application
{
public:
    GLuint render_program = 0;
    GLuint ssao_program = 0;

    bool paused = false;

    GLuint render_fbo = 0;
    GLuint fbo_texture[3];
    GLuint quad_vao = 0;
    GLuint points_buffer = 0;

    sb6::object object;
    sb6::object cube;

    struct
    {
        struct
        {
            GLint mv_matrix;
            GLint proj_matrix;
            GLint shading_level;
        } render;
        struct
        {
            GLint ssao_level;
            GLint object_level;
            GLint ssao_radius;
            GLint weight_by_angle;
            GLint randomize_points;
            GLint point_count;
        } ssao;
    } uniforms;

    bool show_shadow = true;
    bool show_ao = true;
    float ssao_level = 1.0f;
    float ssao_radius = 0.05f;
    bool weight_by_angle = true;
    bool randomize_points = true;
    unsigned int point_count = 10;

    struct SAMPLE_POINTS
    {
        vmath::vec4 points[256];
        vmath::vec4 random_vectors[256];
    };

public:
    Chapter12_SSAO()
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

        glGenFramebuffers(1, &render_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, render_fbo);

        glGenTextures(3, fbo_texture);
        glBindTexture(GL_TEXTURE_2D, fbo_texture[0]);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB16F, MAX_DISPLAY_WIDTH, MAX_DISPLAY_HEIGHT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, fbo_texture[1]);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, MAX_DISPLAY_WIDTH, MAX_DISPLAY_HEIGHT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, fbo_texture[2]);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, MAX_DISPLAY_WIDTH, MAX_DISPLAY_HEIGHT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fbo_texture[0], 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, fbo_texture[1], 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, fbo_texture[2], 0);

        static GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
        glDrawBuffers(2, draw_buffers);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glGenVertexArrays(1, &quad_vao);
        glBindVertexArray(quad_vao);

        object.load("../Media/objects/dragon.sbm");
        cube.load("../Media/objects/cube.sbm");

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        SAMPLE_POINTS point_data;
        for (int idx = 0; idx < 256; ++idx)
        {
            do
            {
                point_data.points[idx][0] = random_float() * 2.0f - 1.0f;
                point_data.points[idx][1] = random_float() * 2.0f - 1.0f;
                point_data.points[idx][2] = random_float();
                point_data.points[idx][3] = 0.0f;
            } while (vmath::length(point_data.points[idx]) > 1.0f);            
            vmath::normalize(point_data.points[idx]);

            point_data.random_vectors[idx][0] = random_float();
            point_data.random_vectors[idx][1] = random_float();
            point_data.random_vectors[idx][2] = random_float();
            point_data.random_vectors[idx][3] = random_float();
        }

        glGenBuffers(1, &points_buffer);
        glBindBuffer(GL_UNIFORM_BUFFER, points_buffer);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(SAMPLE_POINTS), &point_data, GL_STATIC_DRAW);
    }

    void LoadShader()
    {   
        GLuint shaders[2];

        if (render_program != 0)
        {
            glDeleteProgram(render_program);
        }

        shaders[0] = sb6::shader::load("../Media/shaders/ssao/render.vs.glsl", GL_VERTEX_SHADER);
        shaders[1] = sb6::shader::load("../Media/shaders/ssao/render.fs.glsl", GL_FRAGMENT_SHADER);
        render_program = sb6::program::link_from_shaders(shaders, 2, true);

        uniforms.render.mv_matrix = glGetUniformLocation(render_program, "mv_matrix");
        uniforms.render.proj_matrix = glGetUniformLocation(render_program, "proj_matrix");
        uniforms.render.shading_level = glGetUniformLocation(render_program, "shading_level");

        if (ssao_program != 0)
        {
            glDeleteProgram(ssao_program);
        }
        shaders[0] = sb6::shader::load("../Media/shaders/ssao/ssao.vs.glsl", GL_VERTEX_SHADER);
        shaders[1] = sb6::shader::load("../Media/shaders/ssao/ssao.fs.glsl", GL_FRAGMENT_SHADER);
        ssao_program = sb6::program::link_from_shaders(shaders, 2, true);

        uniforms.ssao.ssao_radius = glGetUniformLocation(ssao_program, "ssao_radius");
        uniforms.ssao.ssao_level  = glGetUniformLocation(ssao_program, "ssao_level");
        uniforms.ssao.object_level = glGetUniformLocation(ssao_program, "object_level");
        uniforms.ssao.weight_by_angle = glGetUniformLocation(ssao_program, "weight_by_angle");
        uniforms.ssao.randomize_points = glGetUniformLocation(ssao_program, "randomize_points");
        uniforms.ssao.point_count = glGetUniformLocation(ssao_program, "point_count");
    }

    virtual void shutdown() override
    {
        glDeleteProgram(render_program);
        glDeleteProgram(ssao_program);
    }

    virtual void render(double currentTime) override
    {
        static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        static const GLfloat one = 1.0f;
        static double last_time = 0.0;
        static double total_time = 0.0;

        if (paused == false)
        {
            total_time += (currentTime - last_time);
        }
        last_time = currentTime;

        float f = (float) total_time;

        glViewport(0, 0, info.windowWidth, info.windowHeight);

        glBindFramebuffer(GL_FRAMEBUFFER, render_fbo);
        glEnable(GL_DEPTH_TEST);

        glClearBufferfv(GL_COLOR, 0, black);
        glClearBufferfv(GL_COLOR, 1, black);
        glClearBufferfv(GL_DEPTH, 0, &one);

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, points_buffer);
        glUseProgram(render_program);

        const vmath::mat4 lookat_matrix = vmath::lookat(vmath::vec3(0.0f, 3.0f, 15.0f),
            vmath::vec3(0.0f, 0.0f, 0.0f),
            vmath::vec3(0.0f, 1.0f, 0.0f));

        vmath::mat4 proj_matrix = vmath::perspective(50.0f, (float) info.windowHeight / (float) info.windowHeight, 0.1f, 1000.0f);
        glUniformMatrix4fv(uniforms.render.proj_matrix, 1, GL_FALSE, proj_matrix);

        vmath::mat4 mv_matrix = vmath::translate(0.0f, -5.0f, 0.0f)
            * vmath::rotate(f * 5.0f, 0.0f, 1.0f, 0.0f);
        glUniformMatrix4fv(uniforms.render.mv_matrix, 1, GL_FALSE, lookat_matrix * mv_matrix);

        glUniform1f(uniforms.render.shading_level, show_shadow ? (show_ao ? 0.7f : 1.0f) : 0.0f);

        object.render();

        mv_matrix = vmath::translate(0.0f, -4.5f, 0.0f)
            * vmath::rotate(f * 5.0f, 0.0f, 1.0f, 0.0f)
            * vmath::scale(4000.0f, 0.1f, 4000.0f);
        glUniformMatrix4fv(uniforms.render.mv_matrix, 1, GL_FALSE, lookat_matrix * mv_matrix);
        
        cube.render();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glUseProgram(ssao_program);

        glUniform1f(uniforms.ssao.ssao_radius, ssao_radius * float(info.windowWidth) / 1000.0f);
        glUniform1f(uniforms.ssao.ssao_level, show_ao ? (show_shadow ? 0.3f : 1.0f) : 0.0f);
        glUniform1i(uniforms.ssao.randomize_points, randomize_points ? 1 : 0);
        glUniform1ui(uniforms.ssao.point_count, point_count);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fbo_texture[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, fbo_texture[1]);

        glDisable(GL_DEPTH_TEST);
        glBindVertexArray(quad_vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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
                case 'P': { paused = !paused; } break;
                case 'N': { weight_by_angle = !weight_by_angle; } break;
                case 'R': { randomize_points = !randomize_points; } break;                
                case 'S': { ++point_count; } break;
                case 'X': { --point_count; } break;
                case 'Q': { show_shadow = !show_shadow; } break;
                case 'W': { show_ao = !show_ao; } break;
                case 'A': { ssao_radius += 0.01f; } break;
                case 'Z': { ssao_radius -= 0.01f; } break;
            }
        }
    }
};

//DECLARE_MAIN(Chapter12_SSAO);
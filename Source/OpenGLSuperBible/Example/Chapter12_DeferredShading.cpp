
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
    NUM_LIGHTS = 64,
    NUM_INSTANCES = (15 * 15)
};

class Chapter12_DeferredShading : public sb6::application
{
public:
    GLuint gbuffer = 0;
    GLuint gbuffer_tex[3];
    GLuint fs_quad_vao = 0;

    sb6::object object;

    GLuint render_program = 0;
    GLuint render_program_nm = 0;
    GLuint render_transform_ubo = 0;

    GLuint light_program = 0;
    GLuint light_ubo = 0;

    GLuint vis_program = 0;
    GLint loc_vis_mode = 0;

    GLuint tex_diffuse = 0;
    GLuint tex_nm = 0;

    bool use_nm = true;
    bool paused = false;

    enum
    {
        VIS_OFF,
        VIS_NORMALS,
        VIS_WS_COORDS,
        VIS_DIFFUSE,
        VIS_META,
    } vis_mode = VIS_OFF;

#pragma pack(push, 1)
    struct light_t
    {
        vmath::vec3 position;
        unsigned int : 32;
        vmath::vec3 color;
        unsigned int : 32;
    };
#pragma pack(pop)

public:
    Chapter12_DeferredShading()
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

        glGenTextures(3, gbuffer_tex);
        glBindTexture(GL_TEXTURE_2D, gbuffer_tex[0]);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32UI, MAX_DISPLAY_WIDTH, MAX_DISPLAY_HEIGHT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, gbuffer_tex[1]);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, MAX_DISPLAY_WIDTH, MAX_DISPLAY_HEIGHT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, gbuffer_tex[2]);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, MAX_DISPLAY_WIDTH, MAX_DISPLAY_HEIGHT);

        glGenFramebuffers(1, &gbuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, gbuffer);

        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, gbuffer_tex[0], 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, gbuffer_tex[1], 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, gbuffer_tex[2], 0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glGenVertexArrays(1, &fs_quad_vao);
        glBindVertexArray(fs_quad_vao);

        object.load("../Media/objects/ladybug.sbm");
        tex_nm = sb6::ktx::file::load("../Media/textures/ladybug_nm.ktx");
        tex_diffuse = sb6::ktx::file::load("../Media/textures/ladybug_co.ktx");

        glGenBuffers(1, &light_ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, light_ubo);
        glBufferData(GL_UNIFORM_BUFFER, NUM_LIGHTS * sizeof(light_t), nullptr, GL_DYNAMIC_DRAW);

        glGenBuffers(1, &render_transform_ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, render_transform_ubo);
        glBufferData(GL_UNIFORM_BUFFER, (2 + NUM_INSTANCES) * sizeof(vmath::mat4), nullptr, GL_DYNAMIC_DRAW);
    }

    void LoadShader()
    {   
        if (render_program != 0)
        {
            glDeleteProgram(render_program);
        }
        if (light_program!= 0)
        {
            glDeleteProgram(light_program);
        }

        GLuint vs = sb6::shader::load("../Media/shaders/deferredshading/render.vs.glsl", GL_VERTEX_SHADER);
        GLuint fs = sb6::shader::load("../Media/shaders/deferredshading/render.fs.glsl", GL_FRAGMENT_SHADER);

        render_program = glCreateProgram();
        glAttachShader(render_program, vs);;
        glAttachShader(render_program, fs);
        glLinkProgram(render_program);
        glDeleteShader(vs);
        glDeleteShader(fs);

        vs = sb6::shader::load("../Media/shaders/deferredshading/render-nm.vs.glsl", GL_VERTEX_SHADER);
        fs = sb6::shader::load("../Media/shaders/deferredshading/render-nm.fs.glsl", GL_FRAGMENT_SHADER);

        render_program_nm = glCreateProgram();
        glAttachShader(render_program_nm, vs);;
        glAttachShader(render_program_nm, fs);
        glLinkProgram(render_program_nm);
        glDeleteShader(vs);
        glDeleteShader(fs);

        vs = sb6::shader::load("../Media/shaders/deferredshading/light.vs.glsl", GL_VERTEX_SHADER);
        fs = sb6::shader::load("../Media/shaders/deferredshading/light.fs.glsl", GL_FRAGMENT_SHADER);

        light_program = glCreateProgram();
        glAttachShader(light_program, vs);;
        glAttachShader(light_program, fs);
        glLinkProgram(light_program);
        glDeleteShader(fs);

        fs = sb6::shader::load("../Media/shaders/deferredshading/render-vis.fs.glsl", GL_FRAGMENT_SHADER);

        vis_program = glCreateProgram();
        glAttachShader(vis_program, vs);;
        glAttachShader(vis_program, fs);
        glLinkProgram(vis_program);
        glDeleteShader(vs);
        glDeleteShader(fs);

        loc_vis_mode = glGetUniformLocation(vis_program, "vis_mode");
    }

    virtual void shutdown() override
    {
        glDeleteProgram(render_program);
        glDeleteProgram(render_program_nm);
        glDeleteProgram(light_program);
        glDeleteProgram(vis_program);
    }

    virtual void render(double currentTime) override
    {
        static const GLuint uint_zeros[] = { 0, 0, 0, 0 };
        static const GLfloat float_zeros[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        static const float float_ones[] = { 1.0, 1.0, 1.0, 1.0 };
        static GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
        static double last_time = 0.0;
        static double total_time = 0.0;

        if (paused == false)
        {
            total_time += (currentTime - last_time);
        }
        last_time = currentTime;

        float t = (float) total_time;

        glBindFramebuffer(GL_FRAMEBUFFER, gbuffer);
        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glDrawBuffers(2, draw_buffers);
        glClearBufferuiv(GL_COLOR, 0, uint_zeros);
        glClearBufferuiv(GL_COLOR, 1, uint_zeros);
        glClearBufferfv(GL_DEPTH, 0, float_ones);

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, render_transform_ubo);
        vmath::mat4* matrices = reinterpret_cast<vmath::mat4*>(glMapBufferRange(GL_UNIFORM_BUFFER, 
            0, 
            (2 + NUM_INSTANCES) * sizeof(vmath::mat4),
            GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));

        matrices[0] = vmath::perspective(50.0f, (float)info.windowHeight / (float)info.windowHeight, 0.1f, 1000.0f);

        float d = (sinf(t * 0.13f) + 2.0f) * 0.15f;
        vmath::vec3 eye_pos = vmath::vec3(d * 120.0f * sinf(t * 0.11f), 5.5f, d * 120.0f * cosf(t * 0.01f));
        matrices[1] = vmath::lookat(eye_pos, vmath::vec3(0.0f, -20.0f, 0.0f), vmath::vec3(0.0f, 1.0f, 0.0f));

        for (int idx = 0; idx < 15; ++idx)
        {
            float idx_f = (float) idx;
            for (int sub = 0; sub < 15; ++sub)
            {
                float sub_f = (float) sub;
                matrices[idx * 15 + sub + 2] = vmath::translate((sub_f - 7.5f) * 7.0f, 0.0f, (idx_f - 7.5f) * 11.0f);
            }
        }

        glUnmapBuffer(GL_UNIFORM_BUFFER);

        glUseProgram(use_nm ? render_program_nm : render_program);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex_diffuse);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, tex_nm);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        object.render(NUM_INSTANCES);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glDrawBuffer(GL_BACK);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gbuffer_tex[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gbuffer_tex[1]);

        if (vis_mode == VIS_OFF)
        {
            glUseProgram(light_program);
        }
        else
        {
            glUseProgram(vis_program);
            glUniform1i(loc_vis_mode, vis_mode);
        }

        glDisable(GL_DEPTH_TEST);

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, light_ubo);
        light_t* lights = reinterpret_cast<light_t*>(glMapBufferRange(GL_UNIFORM_BUFFER,
            0,
            NUM_LIGHTS * sizeof(light_t),
            GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));

        for (int idx = 0; idx < NUM_LIGHTS; ++idx)
        {
            float idx_f = ((float) idx - 7.5f) * 0.01f + 0.3f;
            lights[idx].position = vmath::vec3(100.0f * sinf(t * 1.1f + (5.0f * idx_f)) * cosf(t * 2.4f + (9.0 * idx_f)),
                15.0f,
                100.0f * sinf(t * 1.1f + (6.0f * idx_f)) * cosf(t * 1.9f + (11.0 * idx_f)));
            lights[idx].color = vmath::vec3(cosf(idx_f * 14.0f) * 0.5f + 0.8f,            
                sinf(idx_f * 17.0f) * 0.5f + 0.8f,
                sinf(idx_f * 13.0f) * cosf(idx_f * 19.0f) * 0.5f + 0.8f);
        }

        glUnmapBuffer(GL_UNIFORM_BUFFER);

        glBindVertexArray(fs_quad_vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0);
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
                case 'P': { paused = !paused; } break;
                case 'N': { use_nm = !use_nm; } break;
                case '1': { vis_mode = VIS_OFF; } break;
                case '2': { vis_mode = VIS_NORMALS; } break;
                case '3': { vis_mode = VIS_WS_COORDS; } break;
                case '4': { vis_mode = VIS_DIFFUSE; } break;
                case '5': { vis_mode = VIS_META; } break;
            }
        }
    }
};

//DECLARE_MAIN(Chapter12_DeferredShading);
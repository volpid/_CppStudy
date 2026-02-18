
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

class Chapter12_Raytracer : public sb6::application
{
public:
    GLuint prepare_program = 0;
    GLuint trace_program = 0;
    GLuint blit_program = 0;

    struct UniformsBlock
    {
        vmath::mat4 mv_matrix;
        vmath::mat4 view_matrix;
        vmath::mat4 proj_matrix;
    };

    GLuint uniforms_buffer;
    GLuint sphere_buffer;
    GLuint plane_buffer;
    GLuint light_buffer;

    struct
    {
        GLint ray_origin;
        GLint ray_lookat;
        GLint aspect;
    } uniforms;

    GLuint vao = 0;

    struct Sphere
    {
        vmath::vec3 center;
        float radius;
        vmath::vec4 color;
    };

    struct Plane
    {
        vmath::vec3 normal;
        float d;
    };

    struct Light
    {
        vmath::vec3 position;
        unsigned int : 32;
    };

    enum
    {
        MAX_RECURSION_DEPTH = 5,
        MAX_FB_WIDTH = 2048,
        MAX_FB_HEIGHT = 1024,
    };

    enum DEBUG_MODE
    {
        DEBUG_NONE,
        DEBUG_REFLECTED,
        DEBUG_REFRACTED,
        DEBUG_REFLECTED_COLOR,
        DEBUG_REFRACTED_COLOR,
    };

    GLuint tex_composite;
    GLuint ray_fbo[MAX_RECURSION_DEPTH];
    GLuint tex_position[MAX_RECURSION_DEPTH];
    GLuint tex_reflected[MAX_RECURSION_DEPTH];
    GLuint tex_reflected_intensity[MAX_RECURSION_DEPTH];
    GLuint tex_refracted[MAX_RECURSION_DEPTH];
    GLuint tex_refracted_intensity[MAX_RECURSION_DEPTH];

    int max_depth = 1;
    int debug_depth = 0;
    DEBUG_MODE debug_mode = DEBUG_NONE;
    bool paused = false;

public:
    Chapter12_Raytracer()
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

        glGenBuffers(1, &uniforms_buffer);
        glBindBuffer(GL_UNIFORM_BUFFER, uniforms_buffer);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformsBlock), nullptr, GL_DYNAMIC_DRAW);

        glGenBuffers(1, &sphere_buffer);
        glBindBuffer(GL_UNIFORM_BUFFER, sphere_buffer);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(Sphere) * 128, nullptr, GL_DYNAMIC_DRAW);

        glGenBuffers(1, &plane_buffer);
        glBindBuffer(GL_UNIFORM_BUFFER, plane_buffer);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(Plane) * 128, nullptr, GL_DYNAMIC_DRAW);

        glGenBuffers(1, &light_buffer);
        glBindBuffer(GL_UNIFORM_BUFFER, light_buffer);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(Light) * 128, nullptr, GL_DYNAMIC_DRAW);

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenFramebuffers(MAX_RECURSION_DEPTH, ray_fbo);
        glGenTextures(1, &tex_composite);
        glGenTextures(MAX_RECURSION_DEPTH, tex_position);
        glGenTextures(MAX_RECURSION_DEPTH, tex_reflected);
        glGenTextures(MAX_RECURSION_DEPTH, tex_refracted);
        glGenTextures(MAX_RECURSION_DEPTH, tex_reflected_intensity);
        glGenTextures(MAX_RECURSION_DEPTH, tex_refracted_intensity);

        glBindTexture(GL_TEXTURE_2D, tex_composite);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB16F, MAX_FB_WIDTH, MAX_FB_HEIGHT);

        for (int idx = 0; idx < MAX_RECURSION_DEPTH; ++idx)
        { 
            glBindFramebuffer(GL_FRAMEBUFFER, ray_fbo[idx]);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex_composite, 0);

            glBindTexture(GL_TEXTURE_2D, tex_position[idx]);
            glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, MAX_FB_WIDTH, MAX_FB_HEIGHT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);            
            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, tex_position[idx], 0);

            glBindTexture(GL_TEXTURE_2D, tex_reflected[idx]);
            glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB16F, MAX_FB_WIDTH, MAX_FB_HEIGHT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, tex_reflected[idx], 0);

            glBindTexture(GL_TEXTURE_2D, tex_refracted[idx]);
            glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB16F, MAX_FB_WIDTH, MAX_FB_HEIGHT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, tex_refracted[idx], 0);

            glBindTexture(GL_TEXTURE_2D, tex_reflected_intensity[idx]);
            glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB16F, MAX_FB_WIDTH, MAX_FB_HEIGHT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, tex_reflected_intensity[idx], 0);

            glBindTexture(GL_TEXTURE_2D, tex_refracted_intensity[idx]);
            glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB16F, MAX_FB_WIDTH, MAX_FB_HEIGHT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, tex_refracted_intensity[idx], 0);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void LoadShader()
    {   
        GLuint shaders[2];

        if (prepare_program != 0)
        {
            glDeleteProgram(prepare_program);
        }

        shaders[0] = sb6::shader::load("../Media/shaders/raytracer/trace-prepare.vs.glsl", GL_VERTEX_SHADER);
        shaders[1] = sb6::shader::load("../Media/shaders/raytracer/trace-prepare.fs.glsl", GL_FRAGMENT_SHADER);

        prepare_program = sb6::program::link_from_shaders(shaders, 2, true);

        uniforms.ray_origin = glGetUniformLocation(prepare_program, "ray_origin");
        uniforms.ray_lookat = glGetUniformLocation(prepare_program, "ray_lookat");
        uniforms.aspect = glGetUniformLocation(prepare_program, "aspect");

        if (trace_program != 0)
        {
            glDeleteProgram(trace_program);
        }

        shaders[0] = sb6::shader::load("../Media/shaders/raytracer/raytracer.vs.glsl", GL_VERTEX_SHADER);
        shaders[1] = sb6::shader::load("../Media/shaders/raytracer/raytracer.fs.glsl", GL_FRAGMENT_SHADER);
        trace_program = sb6::program::link_from_shaders(shaders, 2, true);

        if (blit_program != 0)
        {
            glDeleteProgram(blit_program);
        }

        shaders[0] = sb6::shader::load("../Media/shaders/raytracer/blit.vs.glsl", GL_VERTEX_SHADER);
        shaders[1] = sb6::shader::load("../Media/shaders/raytracer/blit.fs.glsl", GL_FRAGMENT_SHADER);
        blit_program = sb6::program::link_from_shaders(shaders, 2, true);
    }

    virtual void shutdown() override
    {
        glDeleteProgram(prepare_program);
        glDeleteProgram(trace_program);
        glDeleteProgram(blit_program);
    }

    virtual void render(double currentTime) override
    {
        static const GLfloat zeors[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        static const GLfloat gray[] = { 0.1f, 0.1f, 0.1f, 0.0f };
        static const GLfloat ones[] = { 1.0f };
        static double last_time = 0.0;
        static double total_time = 0.0;

        if (paused == false)
        {
            total_time += (currentTime - last_time);
        }
        last_time = currentTime;
        float f = (float) total_time;

        vmath::vec3 view_position = vmath::vec3(sinf(f * 0.32f) * 28.0f, cosf(f * 0.42f) * 28.0f, cosf(f * 0.12f) * 28.0f);
        vmath::vec3 lookat_point = vmath::vec3(sinf(f * 0.21f) * 8.0f, cosf(f * 0.15f) * 8.0f, sinf(f * 0.73f) * 8.0f);
        vmath::mat4 view_matrix = vmath::lookat(view_position, lookat_point, vmath::vec3(0.0f, 1.0f, 0.0f));

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniforms_buffer);
        UniformsBlock* block = (UniformsBlock*) glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(UniformsBlock), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

        vmath::mat4 model_matrix = vmath::scale(7.0f);

        block->mv_matrix = view_matrix * model_matrix;
        block->view_matrix = view_matrix;
        block->proj_matrix = vmath::perspective(50.0f, (float)info.windowHeight / (float)info.windowHeight, 0.1f, 1000.0f);

        glUnmapBuffer(GL_UNIFORM_BUFFER);

        glBindBufferBase(GL_UNIFORM_BUFFER, 1, sphere_buffer);
        Sphere* s = (Sphere*) glMapBufferRange(GL_UNIFORM_BUFFER, 0, 128 * sizeof(Sphere), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

        for (int idx = 0; idx < 128; ++idx)
        {
            float fi = (float) idx / 128.0f;
            s[idx].center = vmath::vec3(sinf(fi * 123.0f + f) * 15.75f, cosf(fi * 456.0f + f) * 15.75f, sinf(fi * 300.0f + f) * cosf(fi * 200.0f + f) * 20.0f);
            s[idx].radius = fi * 2.3f + 3.5f;

            float r = fi * 61.0f;
            float g = r + 0.25f;
            float b = g + 0.25f;
            r = (r - floorf(r)) * 0.8f + 0.2f;
            g = (g - floorf(g)) * 0.8f + 0.2f;
            b = (b - floorf(b)) * 0.8f + 0.2f;
            s[idx].color = vmath::vec4(r, g, b, 1.0f);
        }

        glUnmapBuffer(GL_UNIFORM_BUFFER);

        glBindBufferBase(GL_UNIFORM_BUFFER, 2, plane_buffer);

        Plane* p = (Plane*) glMapBufferRange(GL_UNIFORM_BUFFER, 0, 128 * sizeof(Plane), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        {
            p[0].normal = vmath::vec3(0.0f, 0.0f, -1.0f);
            p[0].d = 30.0f;
            p[1].normal = vmath::vec3(0.0f, 0.0f, 1.0f);
            p[1].d = 30.0f;
            p[2].normal = vmath::vec3(-1.0f, 0.0f, 0.0f);
            p[2].d = 30.0f;
            p[3].normal = vmath::vec3(1.0f, 0.0f, 0.0f);
            p[3].d = 30.0f;
            p[4].normal = vmath::vec3(0.0f, -1.0f, 0.0f);
            p[4].d = 30.0f;
            p[5].normal = vmath::vec3(0.0f, 1.0f, 0.0f);
            p[5].d = 30.0f;
        }

        glUnmapBuffer(GL_UNIFORM_BUFFER);

        glBindBufferBase(GL_UNIFORM_BUFFER, 3, light_buffer);
        Light* l = (Light*) glMapBufferRange(GL_UNIFORM_BUFFER, 0, 128 * sizeof(Light), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

        for (int idx = 0; idx < 128; ++idx)
        {
            float fi = 3.33f - (float) idx;
            l[idx].position = vmath::vec3(sinf(fi * 2.0f - f) * 15.0f, 
                cosf(fi * 5.0f - f) * 5.0f,
                sinf(fi * 3.0f - f) * cosf(fi * 2.5f - f) * 19.0f);
        }
          
        glUnmapBuffer(GL_UNIFORM_BUFFER);

        glBindVertexArray(vao);
        glViewport(0, 0, info.windowWidth, info.windowHeight);

        glUseProgram(prepare_program);
        glUniformMatrix4fv(uniforms.ray_lookat, 1, GL_FALSE, view_matrix);
        glUniform3fv(uniforms.ray_origin, 1, view_position);
        glUniform1f(uniforms.aspect, (float) info.windowWidth / (float) info.windowHeight);
        
        glBindFramebuffer(GL_FRAMEBUFFER, ray_fbo[0]);
        static const GLenum draw_buffers[] =
        {
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2,
            GL_COLOR_ATTACHMENT3,
            GL_COLOR_ATTACHMENT4,
            GL_COLOR_ATTACHMENT5,
        };
        glDrawBuffers(6, draw_buffers);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glUseProgram(trace_program);
        Recurse(0);

        glUseProgram(blit_program);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDrawBuffer(GL_BACK);

        glActiveTexture(GL_TEXTURE0);
        switch (debug_mode)
        {
            case DEBUG_NONE:
            {
                glBindTexture(GL_TEXTURE_2D, tex_composite);
            }
            break;
            case DEBUG_REFLECTED:
            {
                glBindTexture(GL_TEXTURE_2D, tex_reflected[debug_depth]);
            }
            break;
            case DEBUG_REFRACTED:
            {
                glBindTexture(GL_TEXTURE_2D, tex_refracted[debug_depth]);
            }
            break;
            case DEBUG_REFLECTED_COLOR:
            {
                glBindTexture(GL_TEXTURE_2D, tex_reflected_intensity[debug_depth]);
            }
            break;
            case DEBUG_REFRACTED_COLOR:
            {
                glBindTexture(GL_TEXTURE_2D, tex_refracted_intensity[debug_depth]);
            }
            break;
        }
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    void Recurse(int depth)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, ray_fbo[depth + 1]);

        static const GLenum draw_buffers[] =
        {
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2,
            GL_COLOR_ATTACHMENT3,
            GL_COLOR_ATTACHMENT4,
            GL_COLOR_ATTACHMENT5,
        };
        glDrawBuffers(6, draw_buffers);

        glEnablei(GL_BLEND, 0);
        glBlendFunci(0, GL_ONE, GL_ONE);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex_position[depth]);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, tex_reflected[depth]);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, tex_reflected_intensity[depth]);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        if (depth != (max_depth - 1))
        {
            Recurse(depth + 1);
        }

        glDisablei(GL_BLEND, 0);
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
                case GLFW_KEY_KP_ADD:
                {
                    ++max_depth;
                    if (max_depth > MAX_RECURSION_DEPTH)
                    {
                        max_depth = MAX_RECURSION_DEPTH;
                    }
                }
                break;
                case GLFW_KEY_KP_SUBTRACT:
                {
                    --max_depth;
                    if (max_depth < 1)
                    {
                        max_depth = 1;
                    }
                }
                break;
                case 'Q': { debug_mode = DEBUG_NONE; } break;
                case 'W': { debug_mode = DEBUG_REFLECTED; } break;
                case 'E': { debug_mode = DEBUG_REFRACTED; } break;
                case 'S': { debug_mode = DEBUG_REFLECTED_COLOR; } break;
                case 'D': { debug_mode = DEBUG_REFRACTED_COLOR; } break;
                case 'A':
                {
                    ++debug_depth;
                    if (debug_depth > MAX_RECURSION_DEPTH)
                    {
                        debug_depth = MAX_RECURSION_DEPTH;
                    }
                }
                break;
                case 'Z':
                {
                    --debug_depth;
                    if (debug_depth < 0)
                    {
                        debug_depth = 0;
                    }
                }
                break;
            }
        }
    }
};

DECLARE_MAIN(Chapter12_Raytracer);
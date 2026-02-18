
#include "../Book/object.h"
#include "../Book/sb6.h"
#include "../Book/sb6ktx.h"
#include "../Book/shader.h"
#include "../Book/vmath.h"

#include <math.h>

enum
{
    MAX_SCENE_WIDTH = 2048,
    MAX_SCENE_HEIGHT = 2048,
    SPHERE_COUNT = 32,
};

class Chapter9_HdrBloom : public sb6::application
{
public:
    GLuint tex_src;
    GLuint tex_lut;

    GLuint render_fbo;
    GLuint filter_fbo[2];

    GLuint tex_scene;
    GLuint tex_brightpass;
    GLuint tex_depth;
    GLuint tex_filter[2];

    GLuint program_render;
    GLuint program_filter;
    GLuint program_resolve;

    GLuint vao;

    float exposure;
    int mode;
    bool paused;
    float bloom_factor;
    bool show_bloom;
    bool show_scene;
    bool show_prefilter;
    float bloom_thresh_min;
    float bloom_thresh_max;

    struct
    {
        struct
        {
            int bloom_thresh_min;
            int bloom_thresh_max;
        } scene;
        struct
        {
            int exposure;
            int bloom_factor;
            int scene_factor;
        } resolve;
    } uniforms;

    GLuint ubo_transform;
    GLuint ubo_material;

    sb6::object object;

    bool wireframeMode;    

public:
    Chapter9_HdrBloom()
    {
        wireframeMode = false;
        exposure = 1.0f;
        mode = 0;
        paused = false;
        bloom_factor = 1.0f;
        show_bloom = true;
        show_scene = true;
        bloom_thresh_min = 0.8f;
        bloom_thresh_max = 1.2f;
        show_prefilter = false;

        program_render = 0;
        program_filter = 0;
        program_resolve = 0;
    }

    virtual void init() override
    {
        sb6::application::init();

        info.samples = 8;
    }

    void CompileShader()
    {
        struct
        {
            GLuint vs;
            GLuint fs;
        } shaders;

        if (program_render != 0)
        {
            glDeleteProgram(program_render);
        }
        
        shaders.vs = sb6::shader::load("../Media/shaders/hdrbloom/hdrbloom-scene.vs.glsl", GL_VERTEX_SHADER);
        shaders.fs = sb6::shader::load("../Media/shaders/hdrbloom/hdrbloom-scene.fs.glsl", GL_FRAGMENT_SHADER);
        program_render = sb6::program::link_from_shaders(&shaders.vs, 2, true);

        uniforms.scene.bloom_thresh_min = glGetUniformLocation(program_render, "bloom_thresh_min");
        uniforms.scene.bloom_thresh_max = glGetUniformLocation(program_render, "bloom_thresh_max");

        if (program_filter != 0)
        {
            glDeleteProgram(program_filter);
        }

        shaders.vs = sb6::shader::load("../Media/shaders/hdrbloom/hdrbloom-filter.vs.glsl", GL_VERTEX_SHADER);
        shaders.fs = sb6::shader::load("../Media/shaders/hdrbloom/hdrbloom-filter.fs.glsl", GL_FRAGMENT_SHADER);
        program_filter = sb6::program::link_from_shaders(&shaders.vs, 2, true);

        if (program_resolve != 0)
        {
            glDeleteProgram(program_resolve);
        }

        shaders.vs = sb6::shader::load("../Media/shaders/hdrbloom/hdrbloom-resolve.vs.glsl", GL_VERTEX_SHADER);
        shaders.fs = sb6::shader::load("../Media/shaders/hdrbloom/hdrbloom-resolve.fs.glsl", GL_FRAGMENT_SHADER);
        program_resolve = sb6::program::link_from_shaders(&shaders.vs, 2, true);

        uniforms.resolve.exposure = glGetUniformLocation(program_resolve, "exposure");
        uniforms.resolve.bloom_factor = glGetUniformLocation(program_resolve, "bloom_factor");
        uniforms.resolve.scene_factor = glGetUniformLocation(program_resolve, "scene_factor");
    }

    virtual void startup() override
    {
        static const GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        CompileShader();

        static const GLfloat exposureLUT[20] = 
        {
            11.0f, 6.0f, 3.2f, 2.8f, 2.2f, 1.90f, 1.80f, 1.80f, 1.70f, 1.70f,  1.60f, 1.60f, 1.50f, 1.50f, 1.40f, 1.40f, 1.30f, 1.20f, 1.10f, 1.00f 
        };

        glGenFramebuffers(1, &render_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, render_fbo);

        glGenTextures(1, &tex_scene);
        glBindTexture(GL_TEXTURE_2D, tex_scene);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, MAX_SCENE_WIDTH, MAX_SCENE_HEIGHT);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex_scene, 0);

        glGenTextures(1, &tex_brightpass);
        glBindTexture(GL_TEXTURE_2D, tex_brightpass);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, MAX_SCENE_WIDTH, MAX_SCENE_HEIGHT);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, tex_brightpass, 0);

        glGenTextures(1, &tex_depth);
        glBindTexture(GL_TEXTURE_2D, tex_depth);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32, MAX_SCENE_WIDTH, MAX_SCENE_HEIGHT);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, tex_depth, 0);
        glDrawBuffers(2, buffers);

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

        glGenFramebuffers(2, &filter_fbo[0]);
        glGenTextures(2, &tex_filter[0]);

        for (int idx = 0; idx < 2; ++idx)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, filter_fbo[idx]);
            glBindTexture(GL_TEXTURE_2D, tex_filter[idx]);
            glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, idx ? MAX_SCENE_WIDTH : MAX_SCENE_HEIGHT, idx ? MAX_SCENE_HEIGHT : MAX_SCENE_WIDTH);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex_filter[idx], 0);
            glDrawBuffers(1, buffers);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glGenTextures(1, &tex_lut);
        glBindTexture(GL_TEXTURE_1D, tex_lut);
        glTexStorage1D(GL_TEXTURE_1D, 1, GL_R32F, 20);
        glTexSubImage1D(GL_TEXTURE_1D, 0, 0, 20, GL_RED, GL_FLOAT, exposureLUT);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

        object.load("../Media/objects/sphere.sbm");

        glGenBuffers(1, &ubo_transform);
        glBindBuffer(GL_UNIFORM_BUFFER, ubo_transform);
        glBufferData(GL_UNIFORM_BUFFER, (2 + SPHERE_COUNT) * sizeof(vmath::mat4), nullptr, GL_DYNAMIC_DRAW);

        struct material
        {
            vmath::vec3 diffuse_color;
            unsigned int :32;
            vmath::vec3 specular_color;
            float specular_power;
            vmath::vec3 ambient_color;
            unsigned int : 32;
        };

        glGenBuffers(1, &ubo_material);
        glBindBuffer(GL_UNIFORM_BUFFER, ubo_material);
        glBufferData(GL_UNIFORM_BUFFER, SPHERE_COUNT * sizeof(material), nullptr, GL_STATIC_DRAW);

        material* m = (material*) glMapBufferRange(GL_UNIFORM_BUFFER, 0, SPHERE_COUNT * sizeof(material), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        float ambient = 0.002f;
        for (int idx = 0; idx < SPHERE_COUNT; ++idx)
        {
            float fi = 3.145 * (float) idx / 8.0f;
            m[idx].diffuse_color = vmath::vec3(sinf(fi) * 0.5 + 0.5f, sinf(fi + 1.345f) * 0.5 + 0.5f, sinf(fi + 2.567f) * 0.5 + 0.5f);
            m[idx].specular_color = vmath::vec3(2.8f, 2.8f, 2.9f);
            m[idx].specular_power = 30.0f;
            m[idx].ambient_color = vmath::vec3(ambient * 0.025f);
            ambient *= 1.5f;
        }
        glUnmapBuffer(GL_UNIFORM_BUFFER);
    }

    virtual void shutdown() override
    {
        glDeleteProgram(program_filter);
        glDeleteProgram(program_render);
        glDeleteProgram(program_resolve);
        glDeleteVertexArrays(1, &vao);
        glDeleteTextures(1, &tex_src);
        glDeleteTextures(1, &tex_lut);
    }

    virtual void render(double currentTime) override
    {
        static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        static const GLfloat one = 1.0f;

        static double last_time = 0.0f;
        static double total_time = 0.0f;

        if (paused == false)
        {
            total_time += (currentTime - last_time);
        }
        last_time = currentTime;
        float t = (float) total_time;

        glViewport(0, 0, info.windowWidth, info.windowHeight);

        glBindFramebuffer(GL_FRAMEBUFFER, render_fbo);
        glClearBufferfv(GL_COLOR, 0, black);
        glClearBufferfv(GL_COLOR, 1, black);
        glClearBufferfv(GL_DEPTH, 0, &one);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glUseProgram(program_render);

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo_transform);
        struct transforms_t
        {
            vmath::mat4 mat_proj;
            vmath::mat4 mat_view;
            vmath::mat4 mat_model[SPHERE_COUNT];
        };

        transforms_t* transform = (transforms_t*) glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(transforms_t), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        transform->mat_proj = vmath::perspective(50.0f, (float)info.windowWidth / (float)info.windowHeight, 0.1f, 1000.0f);
        transform->mat_view = vmath::translate(0.0f, 0.0f, -20.0f);
        for (int idx = 0; idx < SPHERE_COUNT; ++idx)
        {
            float fi = 3.1415f * (float) idx / 16.0f;
            float r = (idx & 0x2) ? 0.6f : 1.5f;
            transform->mat_model[idx] = vmath::translate(cosf(t + fi) * 5.0f * r, sinf(t + fi) * 4.0f, sinf(t + fi) * 5.0f * r);
        }
        glUnmapBuffer(GL_UNIFORM_BUFFER);        
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo_material);

        glUniform1f(uniforms.scene.bloom_thresh_min, bloom_thresh_min);
        glUniform1f(uniforms.scene.bloom_thresh_max, bloom_thresh_max);
        
        object.render(SPHERE_COUNT);

        glDisable(GL_DEPTH_TEST);

        glUseProgram(program_filter);
        glBindVertexArray(vao);

        glBindFramebuffer(GL_FRAMEBUFFER, filter_fbo[0]);
        glBindTexture(GL_TEXTURE_2D, tex_brightpass);
        glViewport(0, 0, info.windowHeight, info.windowWidth);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glBindFramebuffer(GL_FRAMEBUFFER, filter_fbo[1]);
        glBindTexture(GL_TEXTURE_2D, tex_filter[0]);
        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glUseProgram(program_resolve);

        glUniform1f(uniforms.resolve.exposure, exposure);
        if (show_prefilter == true)
        {
            glUniform1f(uniforms.resolve.bloom_factor, 0.0f);
            glUniform1f(uniforms.resolve.scene_factor, 1.0f);
        }
        else
        {
            glUniform1f(uniforms.resolve.bloom_factor, show_bloom ? bloom_factor : 0.0f);
            glUniform1f(uniforms.resolve.scene_factor, show_scene ? 1.0f : 0.0f);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, tex_filter[1]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, show_prefilter ? tex_brightpass : tex_scene);

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
                case '1':
                case '2':
                case '3':
                {
                    mode = key - '1';
                }
                break;
                case 'W': 
                {
                    wireframeMode = !wireframeMode;
                }
                break;
                case 'M':
                {
                    mode = (mode + 1) % 3;
                }
                break;
                case GLFW_KEY_KP_ADD:
                {
                    exposure *= 1.1f;
                }
                break;
                case GLFW_KEY_KP_SUBTRACT:
                {
                    exposure /= 1.1f;
                }
                break;
                case 'B':
                {
                    show_bloom = !show_bloom;
                }
                break;
                case 'V':
                {
                    show_scene = !show_scene;
                }
                break;
                case 'A':
                {
                    bloom_factor += 0.1f;
                }
                break;
                case 'Z':
                {
                    bloom_factor -= 0.1f;
                }
                break;
                case 'S':
                {
                    bloom_thresh_min += 0.1f;
                }
                break;
                case 'X':
                {
                    bloom_thresh_min -= 0.1f;
                }
                break;
                case 'D':
                {
                    bloom_thresh_max += 0.1f;
                }
                break;
                case 'C':
                {
                    bloom_thresh_max -= 0.1f;
                }
                break;
                case 'N':
                {
                    show_prefilter = !show_prefilter;
                }
                break;
                case 'P':
                {
                    paused = !paused;
                }
                break;
            }
        }
    }
};

//DECLARE_MAIN(Chapter9_HdrBloom);

#include "../Book/object.h"
#include "../Book/sb6.h"
#include "../Book/sb6ktx.h"
#include "../Book/shader.h"
#include "../Book/vmath.h"

#include <cstdio>
#include <math.h>

#define DEPTH_TEXTURE_SIZE 4096
#define FRUSTUM_DEPTH 1000

class Chapter12_Shadowmapping : public sb6::application
{
public:
    GLuint light_program = 0;
    GLuint view_program = 0;

    GLint show_light_depth_program = 0;

    struct
    {
        struct
        {
            GLint mv_matrix;
            GLint proj_matrix;
            GLint shadow_matrix;
            GLint full_shading;
        } view;
        struct
        {
            GLint mvp;
        } light;
        
    } uniforms;

    GLuint depth_fbo = 0;
    GLuint depth_tex= 0;
    GLuint depth_debug_tex = 0;

    enum
    {
        OBJECT_COUNT = 4,
    };

    struct
    {
        sb6::object obj;
        vmath::mat4 model_matrix;
    } objects[OBJECT_COUNT];

    vmath::mat4 light_view_matrix;
    vmath::mat4 light_proj_matrix;

    vmath::mat4 camera_view_matrix;
    vmath::mat4 camera_proj_matrix;

    GLuint quad_vao = 0;

    enum
    {
        RENDER_FULL,
        RENDER_LIGHT,
        RENDER_DEPTH,
    } mode;
    bool paused = false;

public:
    Chapter12_Shadowmapping()
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

        static const char* const object_names[] =
        {
            "../Media/objects/dragon.sbm",
            "../Media/objects/sphere.sbm",
            "../Media/objects/cube.sbm",
            "../Media/objects/torus.sbm",
        };

        for (int idx = 0; idx < OBJECT_COUNT; ++idx)
        {
            objects[idx].obj.load(object_names[idx]);
        }

        glGenFramebuffers(1, &depth_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);

        glGenTextures(1, &depth_tex);
        glBindTexture(GL_TEXTURE_2D, depth_tex);
        glTexStorage2D(GL_TEXTURE_2D, 11, GL_DEPTH_COMPONENT32F, DEPTH_TEXTURE_SIZE, DEPTH_TEXTURE_SIZE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_tex, 0);

        glGenTextures(1, &depth_debug_tex);
        glBindTexture(GL_TEXTURE_2D, depth_debug_tex);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, DEPTH_TEXTURE_SIZE, DEPTH_TEXTURE_SIZE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, depth_debug_tex, 0);

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glEnable(GL_DEPTH_TEST);

        glGenVertexArrays(1, &quad_vao);
    }

    void LoadShader()
    {   
        if (light_program != 0)
        {
            glDeleteProgram(light_program);
        }

        GLuint vs = sb6::shader::load("../Media/shaders/shadowmapping/shadowmapping-light.vs.glsl", GL_VERTEX_SHADER);
        GLuint fs = sb6::shader::load("../Media/shaders/shadowmapping/shadowmapping-light.fs.glsl", GL_FRAGMENT_SHADER);

        light_program = glCreateProgram();
        glAttachShader(light_program, vs);
        glAttachShader(light_program, fs);
        glLinkProgram(light_program);

        uniforms.light.mvp = glGetUniformLocation(light_program, "mvp");

        glDeleteShader(vs);
        glDeleteShader(fs);

        if (view_program != 0)
        {
            glDeleteProgram(view_program);
        }

        vs = sb6::shader::load("../Media/shaders/shadowmapping/shadowmapping-camera.vs.glsl", GL_VERTEX_SHADER);
        fs = sb6::shader::load("../Media/shaders/shadowmapping/shadowmapping-camera.fs.glsl", GL_FRAGMENT_SHADER);

        view_program = glCreateProgram();
        glAttachShader(view_program, vs);
        glAttachShader(view_program, fs);
        glLinkProgram(view_program);

        uniforms.view.mv_matrix = glGetUniformLocation(view_program, "mv_matrix");
        uniforms.view.proj_matrix = glGetUniformLocation(view_program, "proj_matrix");
        uniforms.view.shadow_matrix = glGetUniformLocation(view_program, "shadow_matrix");
        uniforms.view.full_shading = glGetUniformLocation(view_program, "full_shading");

        glDeleteShader(vs);
        glDeleteShader(fs);

        if (show_light_depth_program != 0)
        {
            glDeleteProgram(show_light_depth_program);
        }

        vs = sb6::shader::load("../Media/shaders/shadowmapping/shadowmapping-light-view.vs.glsl", GL_VERTEX_SHADER);
        fs = sb6::shader::load("../Media/shaders/shadowmapping/shadowmapping-light-view.fs.glsl", GL_FRAGMENT_SHADER);

        show_light_depth_program = glCreateProgram();
        glAttachShader(show_light_depth_program, vs);
        glAttachShader(show_light_depth_program, fs);
        glLinkProgram(show_light_depth_program);

        glDeleteShader(vs);
        glDeleteShader(fs);
    }

    virtual void shutdown() override
    {
        glDeleteProgram(light_program);
        glDeleteProgram(view_program);
        glDeleteProgram(show_light_depth_program);
    }

    virtual void render(double currentTime) override
    {
        static const GLfloat green[] = { 0.2f, 0.2f, 0.2f, 1.0f };
        static const GLfloat one = 1.0f;

        static double last_time = 0.0;
        static double total_time = 0.0;

        if (paused == false)
        {
            total_time += (currentTime - last_time);
        }
        last_time = currentTime;

        const float f = (float) total_time + 30.0f;

        vmath::vec3 light_position = vmath::vec3(20.0f, 20.0f, 20.0f);
        vmath::vec3 view_position = vmath::vec3(0.0f, 0.0f, 40.0f);

        light_proj_matrix = vmath::frustum(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1000.0f);
        light_view_matrix = vmath::lookat(light_position, vmath::vec3(0.0f), vmath::vec3(0.0f, 1.0f, 0.0f));
        
        camera_proj_matrix = vmath::perspective(50.0f, (float) info.windowWidth / (float) info.windowHeight, 1.0f, 200.0f);
        camera_view_matrix = vmath::lookat(view_position, vmath::vec3(0.0f), vmath::vec3(0.0f, 1.0f, 0.0f));

        objects[0].model_matrix = vmath::rotate(f * 14.0f, 0.0f, 1.0f, 0.0f)
            * vmath::rotate(20.0f, 1.0f, 0.0f, 0.0f)
            * vmath::translate(0.0f, -4.0f, 0.0f);
        objects[1].model_matrix = vmath::rotate(f * 3.0f, 0.0f, 1.0f, 0.0f)            
            * vmath::translate(sinf(f * 0.37f) * 12.0f, cosf(f * 0.37f) * 12.0f, 0.0f)
            * vmath::scale(2.0f);
        objects[2].model_matrix = vmath::rotate(f * 6.0f, 0.0f, 1.0f, 0.0f)
            * vmath::rotate(20.0f, 1.0f, 0.0f, 0.0f)
            * vmath::translate(sinf(f * 0.25f) * 10.0f, cosf(f * 0.25f) * 10.0f, 0.0f)
            * vmath::rotate(f * 99.0f, 0.0f, 0.0f, 1.0f)
            * vmath::scale(2.0f);
        objects[3].model_matrix = vmath::rotate(f * 5.0f, 0.0f, 1.0f, 0.0f)
            * vmath::rotate(20.0f, 1.0f, 0.0f, 0.0f)
            * vmath::translate(sinf(f * 0.51f) * 14.0f, cosf(f * 0.51f) * 14.0f, 0.0f)
            * vmath::rotate(f * 120.0f, 0.7f, 0.0f, 0.7f)
            * vmath::scale(2.0f);

        glEnable(GL_DEPTH_TEST);        
        render_scene(total_time, true);

        if (mode == RENDER_DEPTH)
        {
            glDisable(GL_DEPTH_TEST);
            glBindVertexArray(quad_vao);
            glUseProgram(show_light_depth_program);
            glBindTexture(GL_TEXTURE_2D, depth_debug_tex);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
        else
        {
            render_scene(total_time, false);
        }
    }

    void render_scene(double time, bool from_light)
    {
        static const GLfloat ones[] = { 1.0f };
        static const GLfloat zero[] = { 0.0f };
        static const GLfloat gray[] = { 0.1f, 0.1f, 0.1f };
        static const vmath::mat4 scale_bias_matrix = vmath::mat4
        (
            vmath::vec4(0.5f, 0.0f, 0.0f, 0.0f),
            vmath::vec4(0.0f, 0.5f, 0.0f, 0.0f),
            vmath::vec4(0.0f, 0.0f, 0.5f, 0.0f),
            vmath::vec4(0.5f, 0.5f, 0.5f, 1.0f)
        );

        vmath::mat4 light_vp_matrix = light_proj_matrix * light_view_matrix;
        vmath::mat4 shadow_sbpv_matrix = scale_bias_matrix * light_proj_matrix * light_view_matrix;

        if (from_light == true)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
            glViewport(0, 0, DEPTH_TEXTURE_SIZE, DEPTH_TEXTURE_SIZE);
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(4.0f, 4.0f);
            glUseProgram(light_program);
            static const GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };
            glDrawBuffers(1, buffers);
            glClearBufferfv(GL_COLOR, 0, zero);
        }
        else
        {
            glViewport(0, 0, info.windowWidth, info.windowHeight);
            glClearBufferfv(GL_COLOR, 0, gray);
            glUseProgram(view_program);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, depth_tex);
            glUniformMatrix4fv(uniforms.view.proj_matrix, 1, GL_FALSE, camera_proj_matrix);
            glDrawBuffer(GL_BACK);
        }
        
        glClearBufferfv(GL_DEPTH, 0, ones);

        for (int idx = 0; idx < 4; ++idx)
        {
            vmath::mat4& model_matrix = objects[idx].model_matrix;
            if (from_light == true)
            {
                glUniformMatrix4fv(uniforms.light.mvp, 1, GL_FALSE, light_vp_matrix * objects[idx].model_matrix);
            }
            else
            {
                vmath::mat4 shadow_matrix = shadow_sbpv_matrix * model_matrix;
                glUniformMatrix4fv(uniforms.view.shadow_matrix, 1, GL_FALSE, shadow_matrix);
                glUniformMatrix4fv(uniforms.view.mv_matrix, 1, GL_FALSE, camera_view_matrix * objects[idx].model_matrix);
                glUniform1i(uniforms.view.full_shading, mode == RENDER_FULL ? 1 : 0);
            }

            objects[idx].obj.render();
        }

        if (from_light == true)
        {
            glDisable(GL_POLYGON_OFFSET_FILL);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        glClearBufferfv(GL_DEPTH, 0, ones);
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
                case '1': { mode = RENDER_FULL; } break;
                case '2': { mode = RENDER_LIGHT; } break;
                case '3': { mode = RENDER_DEPTH; } break;
                case 'p': { paused = !paused; } break;
            }
        }
    }
};

//DECLARE_MAIN(Chapter12_Shadowmapping);
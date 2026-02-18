
#include "../Book/object.h"
#include "../Book/sb6.h"
#include "../Book/sb6ktx.h"
#include "../Book/shader.h"
#include "../Book/vmath.h"

#include <cstdio>
#include <math.h>


#define FBO_SIZE                2048
#define FRUSTUM_DEPTH           1000

class Chapter10_Dof : public sb6::application
{
public:
    GLuint view_program = 0;
    GLuint filter_program = 0;
    GLuint display_program = 0;

    struct
    {
        struct
        {
            GLint focal_distance;
            GLint focal_depth;
        } dof;

        struct
        {
            GLint mv_matrix;
            GLint proj_matrix;
            GLint full_shading;
            GLint diffuse_albedo;
            

        } view;
    } uniforms;

    GLuint depth_fbo;
    GLuint depth_tex;
    GLuint color_tex;
    GLuint temp_tex;

    enum
    {
        OBJECT_COUNT = 5,
    };

    struct
    {
        sb6::object obj;
        vmath::mat4 model_matrix;
        vmath::vec4 diffuse_albedo;
    } objects[OBJECT_COUNT];

    vmath::mat4 camera_view_matrix;
    vmath::mat4 camera_proj_matrix;

    GLuint quad_vao;

    bool paused = false;
    float focal_distance = 40.0f;
    float focal_depth = 50.0f;

public:
    Chapter10_Dof()
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
            "../Media/objects/cube.sbm",
            "../Media/objects/cube.sbm",
        };

        static const vmath::vec4 object_colros[] =
        {
            vmath::vec4(1.0f, 0.7f, 0.8f, 1.0f),
            vmath::vec4(0.7f, 0.8f, 1.0f, 1.0f),
            vmath::vec4(0.3f, 0.9f, 0.4f, 1.0f),
            vmath::vec4(0.6f, 0.4f, 0.9f, 1.0f),
            vmath::vec4(0.8f, 0.2f, 0.1f, 1.0f),
        };

        for (int idx = 0; idx < OBJECT_COUNT; ++idx)
        {
            objects[idx].obj.load(object_names[idx]);
            objects[idx].diffuse_albedo = object_colros[idx];
        }

        glGenFramebuffers(1, &depth_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);

        glGenTextures(1, &depth_tex);
        glBindTexture(GL_TEXTURE_2D, depth_tex);
        glTexStorage2D(GL_TEXTURE_2D, 11, GL_DEPTH_COMPONENT32F, FBO_SIZE, FBO_SIZE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glGenTextures(1, &color_tex);
        glBindTexture(GL_TEXTURE_2D, color_tex);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, FBO_SIZE, FBO_SIZE);

        glGenTextures(1, &temp_tex);
        glBindTexture(GL_TEXTURE_2D, temp_tex);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, FBO_SIZE, FBO_SIZE);

        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_tex, 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color_tex, 0);

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glEnable(GL_DEPTH_TEST);

        glGenVertexArrays(1, &quad_vao);
        glBindVertexArray(quad_vao);
    }

    void LoadShader()
    {
        GLuint shaders[4];
        
        if (view_program != 0)
        {
            glDeleteProgram(view_program);
        }
        shaders[0] = sb6::shader::load("../Media/shaders/dof/render.vs.glsl", GL_VERTEX_SHADER);
        shaders[1] = sb6::shader::load("../Media/shaders/dof/render.fs.glsl", GL_FRAGMENT_SHADER);

        view_program = sb6::program::link_from_shaders(shaders, 2, true);

        // 유니폼 확인용 코드
        GLint count;
        glGetProgramiv(view_program, GL_ACTIVE_UNIFORMS, &count);
        for (GLint idx = 0; idx < count; ++idx)
        {
            char uniformName[256] = {};
            GLsizei length;
            GLint size;
            GLenum type;

            glGetActiveUniform(view_program, idx, sizeof(uniformName), &length, &size, &type, uniformName);

            GLint location = glGetUniformLocation(view_program, uniformName);
        }

        uniforms.view.proj_matrix = glGetUniformLocation(view_program, "proj_matrix");
        uniforms.view.mv_matrix = glGetUniformLocation(view_program, "mv_matrix");
        uniforms.view.full_shading = glGetUniformLocation(view_program, "full_shading");
        uniforms.view.diffuse_albedo = glGetUniformLocation(view_program, "diffuse_albedo");

        if (display_program != 0)
        {
            glDeleteProgram(display_program);
        }

        shaders[0] = sb6::shader::load("../Media/shaders/dof/display.vs.glsl", GL_VERTEX_SHADER);
        shaders[1] = sb6::shader::load("../Media/shaders/dof/display.fs.glsl", GL_FRAGMENT_SHADER);

        display_program = sb6::program::link_from_shaders(shaders, 2, true);

        uniforms.dof.focal_distance = glGetUniformLocation(display_program, "focal_distance");
        uniforms.dof.focal_depth = glGetUniformLocation(display_program, "focal_depth");

        if (filter_program != 0)
        {
            glDeleteProgram(filter_program);
        }

        shaders[0] = sb6::shader::load("../Media/shaders/dof/gensat.cs.glsl", GL_COMPUTE_SHADER);

        filter_program = sb6::program::link_from_shaders(shaders, 1, true);
    }

    void RenderScene(double currentTime)
    {
        static const GLfloat ones[] = { 1.0f };
        static const GLfloat zero[] = { 0.0f };
        static const GLfloat gray[] = { 0.1f, 0.1f, 0.1f, 0.0f };
        static const GLenum attachments[] = { GL_COLOR_ATTACHMENT0 };
        static const vmath::mat4 scale_bias_matrix = vmath::mat4(
            vmath::vec4(0.5f, 0.0f, 0.0f, 0.0f),
            vmath::vec4(0.0f, 0.5f, 0.0f, 0.0f),
            vmath::vec4(0.0f, 0.0f, 0.5f, 0.0f),
            vmath::vec4(0.5f, 0.5f, 0.5f, 1.0f));

        glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
        glDrawBuffers(1, attachments);
        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, gray);
        glClearBufferfv(GL_DEPTH, 0, ones);

        glUseProgram(view_program);
        glUniformMatrix4fv(uniforms.view.proj_matrix, 1, GL_FALSE, camera_proj_matrix);

        glClearBufferfv(GL_DEPTH, 0, ones);

        for (int idx = 0; idx < OBJECT_COUNT; ++idx)
        {
            vmath::mat4& model_matrix = objects[idx].model_matrix;
            glUniformMatrix4fv(uniforms.view.mv_matrix, 1, GL_FALSE, camera_view_matrix * objects[idx].model_matrix);
            glUniform3fv(uniforms.view.diffuse_albedo, 1, objects[idx].diffuse_albedo);
            objects[idx].obj.render();
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    virtual void shutdown() override
    {
        glDeleteProgram(view_program);
        glDeleteProgram(filter_program);
        glDeleteProgram(display_program);
    }

    virtual void render(double currentTime) override
    {   
        static const GLfloat zeros[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        static double last_time = 0.0;
        static double total_time = 0.0;

        if (paused == false)
        {
            total_time += (currentTime - last_time);
        }
        last_time = currentTime;

        const float f = (float) total_time + 30.0f;

        vmath::vec3 view_position = vmath::vec3(0.0f, 0.0f, 40.0f);

        camera_proj_matrix = vmath::perspective(50.0f, (float)info.windowWidth / (float)info.windowHeight, 0.1f, 1000.0f);
        camera_view_matrix = vmath::lookat(view_position, vmath::vec3(0.0f), vmath::vec3(0.0f, 1.0f, 0.0f));

        objects[0].model_matrix = vmath::translate(5.0F, 0.0F, 20.0F)
            * vmath::rotate(f * 14.5f, 0.0f, 1.0f, 0.0f)
            * vmath::rotate(20.0f, 1.0f, 0.0f, 0.0f)
            * vmath::translate(0.0F, -4.0F, 0.0f);

        objects[1].model_matrix = vmath::translate(-5.0F, 0.0F, 0.0F)
            * vmath::rotate(f * 14.5f, 0.0f, 1.0f, 0.0f)
            * vmath::rotate(20.0f, 1.0f, 0.0f, 0.0f)
            * vmath::translate(0.0F, -4.0F, 0.0f);

        objects[2].model_matrix = vmath::translate(-15.0F, 0.0F, -20.0F)
            * vmath::rotate(f * 14.5f, 0.0f, 1.0f, 0.0f)
            * vmath::rotate(20.0f, 1.0f, 0.0f, 0.0f)
            * vmath::translate(0.0F, -4.0F, 0.0f);

        objects[3].model_matrix = vmath::translate(-25.0F, 0.0F, -40.0F)
            * vmath::rotate(f * 14.5f, 0.0f, 1.0f, 0.0f)
            * vmath::rotate(20.0f, 1.0f, 0.0f, 0.0f)
            * vmath::translate(0.0F, -4.0F, 0.0f);

        objects[4].model_matrix = vmath::translate(-25.0F, 0.0F, -60.0F)
            * vmath::rotate(f * 14.5f, 0.0f, 1.0f, 0.0f)
            * vmath::rotate(20.0f, 1.0f, 0.0f, 0.0f)
            * vmath::translate(0.0F, -4.0F, 0.0f);
            * vmath::translate(0.0F, -4.0F, 0.0f);

        glEnable(GL_DEPTH_TEST);

        RenderScene(currentTime);

        glUseProgram(filter_program);

        glBindImageTexture(0, color_tex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        glBindImageTexture(1, temp_tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

        glDispatchCompute(info.windowHeight, 1, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glBindImageTexture(0, temp_tex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        glBindImageTexture(1, color_tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

        glDispatchCompute(info.windowWidth, 1, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glBindTexture(GL_TEXTURE_2D, color_tex);
        
        glDisable(GL_DEPTH_TEST);

        glUseProgram(display_program);
        glUniform1f(uniforms.dof.focal_distance, focal_distance);
        glUniform1f(uniforms.dof.focal_depth, focal_depth);

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
                case 'Q':
                {
                    focal_distance *= 1.1f;
                }
                break;
                case 'A':
                {
                    focal_distance /= 1.1f;
                }
                break;
                case 'W':
                {
                    focal_depth *= 1.1f;
                }
                break;
                case 'S':
                {
                    focal_depth /= 1.1f;
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

//DECLARE_MAIN(Chapter10_Dof);
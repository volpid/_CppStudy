
#include "../Book/object.h"
#include "../Book/sb6.h"
#include "../Book/sb6ktx.h"
#include "../Book/shader.h"
#include "../Book/vmath.h"

#include <cstdio>
#include <math.h>


enum
{
    NUM_ELEMENTS = 2048,
};

class Chapter10_Prefixsum2d : public sb6::application
{
public:
    GLint prefix_sum_prog = 0;
    GLint show_image_prog = 0;
    GLuint dummy_vao = 0;

    GLuint images[3];

public:
    Chapter10_Prefixsum2d()
    {
    }

    virtual void init() override
    {
        sb6::application::init();

        info.samples = 1;
    }

    virtual void startup() override
    {   
        glGenTextures(3, images);

        images[0] = sb6::ktx::file::load("../Media/textures/salad-gray.ktx");
        for (int idx = 1; idx < 3; ++idx)
        {
            glBindTexture(GL_TEXTURE_2D, images[idx]);
            glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, NUM_ELEMENTS, NUM_ELEMENTS);
        }

        glGenVertexArrays(1, &dummy_vao);
        glBindVertexArray(dummy_vao);

        LoadShader();
    }

    void LoadShader()
    {
        GLuint cs = sb6::shader::load("../Media/shaders/prefixsum2d/prefixsum2d.cs.glsl", GL_COMPUTE_SHADER);

        if (prefix_sum_prog != 0)
        {
            glDeleteProgram(prefix_sum_prog);
        }

        prefix_sum_prog = sb6::program::link_from_shaders(&cs, 1, true);

        struct
        {
            GLuint vs;
            GLuint fs;
        } show_image_shaders;

        show_image_shaders.vs = sb6::shader::load("../Media/shaders/prefixsum2d/showimage.vs.glsl", GL_VERTEX_SHADER);
        show_image_shaders.fs = sb6::shader::load("../Media/shaders/prefixsum2d/showimage.fs.glsl", GL_FRAGMENT_SHADER);

        show_image_prog = sb6::program::link_from_shaders(&show_image_shaders.vs, 2, true);
    }

    virtual void shutdown() override
    {
        glDeleteProgram(prefix_sum_prog);
        glDeleteProgram(show_image_prog);
    }


    virtual void render(double currentTime) override
    {
        glUseProgram(prefix_sum_prog);

        glBindImageTexture(0, images[0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
        glBindImageTexture(1, images[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

        glDispatchCompute(NUM_ELEMENTS, 1, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glBindImageTexture(0, images[1], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
        glBindImageTexture(1, images[2], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

        glDispatchCompute(NUM_ELEMENTS, 1, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);        
         
        glBindTexture(GL_TEXTURE_2D, images[2]);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, images[2]);

        glUseProgram(show_image_prog);

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glBindVertexArray(dummy_vao);

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
            }
        }
    }
};

//DECLARE_MAIN(Chapter10_Prefixsum2d);
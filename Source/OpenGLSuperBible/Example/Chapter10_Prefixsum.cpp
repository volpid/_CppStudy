
#include "../Book/object.h"
#include "../Book/sb6.h"
#include "../Book/sb6ktx.h"
#include "../Book/shader.h"
#include "../Book/vmath.h"

#include <cstdio>
#include <math.h>

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

enum
{
    NUM_ELEMENTS = 2048,
};


class Chapter10_Prefixsum : public sb6::application
{
public:
    GLint prefix_sum_prog = 0;
    GLuint data_buffer[2];

    float input_data[NUM_ELEMENTS] = { 0, };
    float out_data[NUM_ELEMENTS] = { 0, };

public:
    Chapter10_Prefixsum()
    {
    }

    virtual void init() override
    {
        sb6::application::init();

        info.samples = 1;
    }

    virtual void startup() override
    {   
        glGenBuffers(2, data_buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, data_buffer[0]);
        glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_ELEMENTS * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, data_buffer[1]);
        glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_ELEMENTS * sizeof(float), nullptr, GL_DYNAMIC_COPY);

        for (int idx = 0; idx < NUM_ELEMENTS; ++idx)
        {
            input_data[idx] = random_float();
        }

        PrefixSum(input_data, out_data, NUM_ELEMENTS);
        LoadShader();
    }

    void PrefixSum(float*, float*, int )
    {
        float f = 0.0f;
        for (int idx = 0; idx < NUM_ELEMENTS; ++idx)
        {
            f += input_data[idx];
            out_data[idx] = f;
        }
    }

    void LoadShader()
    {
        GLuint cs = sb6::shader::load("../Media/shaders/prefixsum/prefixsum.cs.glsl", GL_COMPUTE_SHADER);

        if (prefix_sum_prog != 0)
        {
            glDeleteProgram(prefix_sum_prog);
        }

        prefix_sum_prog = sb6::program::link_from_shaders(&cs, 1, true);

        glShaderStorageBlockBinding(prefix_sum_prog, 0, 0);
        glShaderStorageBlockBinding(prefix_sum_prog, 1, 1);
    }

    virtual void shutdown() override
    {
        glDeleteProgram(prefix_sum_prog);
    }

    virtual void render(double currentTime) override
    {
        float* ptr = nullptr;

        glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, data_buffer[0], 0, sizeof(float) * NUM_ELEMENTS);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(float) * NUM_ELEMENTS, input_data);

        glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, data_buffer[1], 0, sizeof(float) * NUM_ELEMENTS);

        glUseProgram(prefix_sum_prog);
        glDispatchCompute(1, 1, 1);

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glFinish();

        glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, data_buffer[1], 0, sizeof(float) * NUM_ELEMENTS);
        ptr = (float*) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(float) * NUM_ELEMENTS, GL_MAP_READ_BIT);

        char buffer[1024] = { 0, };

        sprintf_s(buffer, "%2.2f %2.2f %2.2f %2.2f %2.2f %2.2f %2.2f %2.2f "
            "%2.2f %2.2f %2.2f %2.2f %2.2f %2.2f %2.2f %2.2f\n",
            ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], ptr[6], ptr[7],
            ptr[8], ptr[9], ptr[10], ptr[11], ptr[12], ptr[13], ptr[14], ptr[15]);

        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
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

//DECLARE_MAIN(Chapter10_Prefixsum);

#include "../Book/object.h"
#include "../Book/sb6.h"
#include "../Book/sb6ktx.h"
#include "../Book/shader.h"
#include "../Book/vmath.h"

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
    NUM_STARS = 2000,
};


class Chapter9_Starfield : public sb6::application
{
public:
    GLuint program;
    GLuint star_texture;
    GLuint star_vao;
    GLuint star_buffer;

    struct
    {
        int time;
        int proj_matrix;
    } uniforms;

public:
    Chapter9_Starfield()
    {
        program = 0;
        star_texture = 0;
        star_vao = 0;
        star_buffer = 0;
    }

    virtual void init() override
    {
        sb6::application::init();

        info.samples = 8;
    }

    void CompileShader()
    {   
        static const GLchar* vertShader[] =
        {
            R"vs(

            #version 420 core

            layout (location = 0) in vec4 position;
            layout (location = 1) in vec4 color;

            uniform float time;
            uniform mat4 proj_matrix;

            flat out vec4 starColor;

            void main(void)
            {
                vec4 newVertex = position;
                newVertex.z += time;
                newVertex.z = fract(newVertex.z);

                float size = (20.0 * newVertex.z * newVertex.z);
                starColor = smoothstep(1.0, 7.0, size) * color;

                newVertex.z = (999.9 * newVertex.z) - 1000.0;
                gl_Position = proj_matrix * newVertex;
                gl_PointSize = size * 2.0;
            }

            )vs"
        };

        static const GLchar* fragShader[] =
        {
            R"fs(

            #version 420 core

            flat in vec4 starColor;

            layout (location = 0) out vec4 color;

            uniform sampler2D tex_star;

            void main()
            {
                color = starColor * texture(tex_star, gl_PointCoord);
            }
                
            )fs"
        };


        char logBuffer[1024];
        GLint success;

        program = glCreateProgram();

        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, vertShader, nullptr);
        glCompileShader(vs);

        glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
        glGetShaderInfoLog(vs, 1024, NULL, logBuffer);

        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, fragShader, nullptr);
        glCompileShader(fs);
        glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
        glGetShaderInfoLog(fs, 1024, NULL, logBuffer);

        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glLinkProgram(program);

        uniforms.time = glGetUniformLocation(program, "time");
        uniforms.proj_matrix = glGetUniformLocation(program, "proj_matrix");

        glDeleteShader(vs);
        glDeleteShader(fs);
    }

    virtual void startup() override
    {   
        CompileShader();

        glGenVertexArrays(1, &star_vao);
        glBindVertexArray(star_vao);

        star_texture = sb6::ktx::file::load("../Media/textures/star.ktx");

        struct Star
        {
            vmath::vec3 position;
            vmath::vec3 color;
        };

        glGenBuffers(1, &star_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, star_buffer);
        glBufferData(GL_ARRAY_BUFFER, NUM_STARS * sizeof(Star), nullptr, GL_STATIC_DRAW);

        Star* star = (Star*) glMapBufferRange(GL_ARRAY_BUFFER, 0, NUM_STARS * sizeof(Star), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        for (int idx = 0; idx < 1000; ++idx)
        {
            star[idx].position[0] = (random_float() * 2.0f - 1.0f) * 100.0f;
            star[idx].position[1] = (random_float() * 2.0f - 1.0f) * 100.0f;
            star[idx].position[2] = random_float();
            star[idx].color[0] = 0.8f + random_float() * 0.2f;
            star[idx].color[1] = 0.8f + random_float() * 0.2f;
            star[idx].color[2] = 0.8f + random_float() * 0.2f;
        }
        glUnmapBuffer(GL_ARRAY_BUFFER);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Star), nullptr);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Star), (void*) sizeof(vmath::vec3));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
    }

    virtual void shutdown() override
    {
        glDeleteProgram(program);
        glDeleteVertexArrays(1, &star_vao);
        glDeleteTextures(1, &star_texture);
    }

    virtual void render(double currentTime) override
    {
        static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        static const GLfloat one = 1.0f;

        float t = (float) currentTime;
        vmath::mat4 proj_matrix = vmath::perspective(50.0f, (float)info.windowWidth / (float)info.windowHeight, 0.1f, 1000.0f);
        t *= 0.1f;
        t -= floor(t);

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, black);
        glClearBufferfv(GL_DEPTH, 0, &one);

        glUseProgram(program);

        glUniform1f(uniforms.time, t);
        glUniformMatrix4fv(uniforms.proj_matrix, 1, GL_FALSE, proj_matrix);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        glBindVertexArray(star_vao);

        glEnable(GL_PROGRAM_POINT_SIZE);
        glDrawArrays(GL_POINTS, 0, NUM_STARS);
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

//DECLARE_MAIN(Chapter9_Starfield);

#include "../Book/sb6.h"
#include "../Book/sb6ktx.h"
#include "../Book/vmath.h"

#include <math.h>

//-------------------------------------------------------------------------------------------
// Random number generator
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
//-------------------------------------------------------------------------------------------

class Chapter5_AlienRain : public sb6::application
{
public:
    virtual void render(double currentTime) override
    {
        static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        float t = (float) currentTime;
                
        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, black);

        glUseProgram(renderingProgram);

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, buffer);
        vmath::vec4* droplet = (vmath::vec4*) glMapBufferRange(GL_UNIFORM_BUFFER, 
            0,
            256 * sizeof(vmath::vec4),
            GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

        for (int32_t idx = 0; idx < 256; ++idx)
        {
            droplet[idx][0] = droplet_x_offset[idx];
            droplet[idx][1] = 2.0f - fmodf(t + float(idx) * droplet_rot_speed[idx], 4.3f);
            droplet[idx][2] = t * droplet_rot_speed[idx];
            droplet[idx][3] = 0.0f;
        }

        glUnmapBuffer(GL_UNIFORM_BUFFER);

        for (int32_t alienIndex = 0; alienIndex < 256; ++alienIndex)
        {
            glVertexAttribI1i(0, alienIndex);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
    }

    virtual void startup() override
    {
        renderingProgram = CompileShader();

        tex_alien_array = sb6::ktx::file::load("../Media/textures/aliens.ktx");

        glBindTexture(GL_TEXTURE_2D_ARRAY, tex_alien_array);
        glTextureParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        glGenVertexArrays(1, &vertexArrayObject);
        glBindVertexArray(vertexArrayObject);

        //glEnable(GL_CULL_FACE);
        //glFrontFace(GL_CW);

        glGenBuffers(1, &buffer);
        glBindBuffer(GL_UNIFORM_BUFFER, buffer);
        glBufferData(GL_UNIFORM_BUFFER, 256 * sizeof(vmath::vec4), nullptr, GL_DYNAMIC_DRAW);

        for (int32_t idx = 0; idx < 256; ++idx)
        {
            droplet_x_offset[idx] = random_float() * 2.0f - 1.0f;
            droplet_rot_speed[idx] = (random_float() + 0.5f) * ((idx & 1) ? -3.0f : 3.0f);
            droplet_fall_speed[idx] = random_float() + 0.2f;
        }

        glBindVertexArray(vertexArrayObject);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    virtual void onResize(int w, int h) override
    {
        sb6::application::onResize(w, h);
    }


    GLuint CompileShader()
    {
        GLuint vertexShader;
        GLuint fragmentShader;
        GLuint program;

        
        static const GLchar* vertShader[] =
        {
            R"vs(
                #version 430 core

                layout (location = 0) in int alien_index;

                out VS_OUT
                {
                    flat int alien;
                    vec2 texCoord;
                } vs_out;

                struct droplet_t
                {
                    float x_offset;
                    float y_offset;
                    float orientation;
                    float unused;
                };

                layout (std140) uniform droplets
                {
                    droplet_t droplet[256];
                };

                void main()
                {
                    const vec2[4] position = vec2[4]
                    (
                        vec2(-0.5, -0.5),
                        vec2(0.5, -0.5),
                        vec2(-0.5, 0.5),
                        vec2(0.5, 0.5)
                    );

                    vs_out.texCoord = position[gl_VertexID].xy + vec2(0.5);
                    float co = cos(droplet[alien_index].orientation);
                    float si = sin(droplet[alien_index].orientation);
                    mat2 rot = mat2
                    (
                        vec2(co, si),
                        vec2(-si, co)
                    );

                    vec2 pos = 0.25 * rot * position[gl_VertexID];
                    gl_Position = vec4
                    (
                        pos.x + droplet[alien_index].x_offset,
                        pos.y + droplet[alien_index].y_offset,
                        0.5, 
                        1
                    );

                    vs_out.alien = alien_index % 64;
                }
            )vs"
        };

        static const GLchar* fragShader[] =
        {
            R"fs(
                #version 430 core

                in VS_OUT
                {
                    flat int alien;
                    vec2 texCoord;
                } fs_in;

                uniform sampler2DArray tex_alients;

                layout (location = 0) out vec4 color;

                void main()
                {
                    color = texture(tex_alients, vec3(fs_in.texCoord, float(fs_in.alien)));
                }
            )fs"
        };

        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, vertShader, nullptr);
        glCompileShader(vertexShader);

        char logBuffer[1024];
        GLint success;
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        glGetShaderInfoLog(vertexShader, 1024, NULL, logBuffer);

        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, fragShader, nullptr);
        glCompileShader(fragmentShader);

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        glGetShaderInfoLog(fragmentShader, 1024, NULL, logBuffer);

        program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        // 유니폼 확인용 코드
        GLint count;
        glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count);
        for (GLint idx = 0; idx < count; ++idx)
        {
            char uniformName[256] = {};
            GLsizei length;
            GLint size;
            GLenum type;

            glGetActiveUniform(program, idx, sizeof(uniformName), &length, &size, &type, uniformName);

            GLint location = glGetUniformLocation(program, uniformName);
        }

        // 프로그램에 링크 후 기존 쉐이더 삭제
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return program;
    }

    virtual void shutdown()
    {
        glDeleteVertexArrays(1, &vertexArrayObject);
        glDeleteProgram(renderingProgram);
        //glDeleteVertexArrays(1, &vertexArrayObject);
    }

private:
    GLuint renderingProgram;
    GLuint vertexArrayObject;
    GLuint buffer;
    GLuint tex_alien_array;

    float droplet_x_offset[256] = {};
    float droplet_rot_speed[256] = {};
    float droplet_fall_speed[256] = {};
};

//DECLARE_MAIN(Chapter5_AlienRain);
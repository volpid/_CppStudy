
#include "../Book/sb6.h"
#include "../Book/sb6ktx.h"
#include "../Book/vmath.h"

#include <math.h>

class Chapter7_Grass : public sb6::application
{
public:
    virtual void render(double currentTime) override
    {
        static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        static const GLfloat One = 1.0f;
                        
        float t = (float) currentTime * 0.02f;
        float r = 550.0f;

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, black);
        glClearBufferfv(GL_DEPTH, 0, &One);

        vmath::mat4 mv_matrix = vmath::lookat(vmath::vec3(sinf(t) * r, 25.0f, cosf(t) * r),
            vmath::vec3(0.0f, -50.0f, 0.0f),
            vmath::vec3(0.0f, 1.0f, 0.0f));

        vmath::mat4 prj_matrix = vmath::perspective(45.0f, (float) info.windowWidth / (float) info.windowHeight, 0.1f, 1000.0f);

        glUseProgram(renderingProgram);
        glUniformMatrix4fv(uniforms.mpvMatrix, 1, GL_FALSE, (prj_matrix * mv_matrix));

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glViewport(0, 0, info.windowWidth, info.windowHeight);

        glBindVertexArray(vertexArrayObject);

        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 6, 1024 * 1024);
    }

    virtual void startup() override
    {
        static const GLfloat grass_blade[] =
        {
            -0.3f, 0.0f,
             0.3f, 0.0f,
            -0.20f, 1.0f,
             0.1f, 1.3f,
            -0.05f, 2.3f,
             0.0f, 3.3f
        };

        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(grass_blade), grass_blade, GL_STATIC_DRAW);

        glGenVertexArrays(1, &vertexArrayObject);
        glBindVertexArray(vertexArrayObject);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);

        renderingProgram = CompileShader();

        uniforms.mpvMatrix = glGetUniformLocation(renderingProgram, "mvpMatrix");

        glActiveTexture(GL_TEXTURE1);
        tex_grass_length = sb6::ktx::file::load("../Media/textures/grass_length.ktx");
        glActiveTexture(GL_TEXTURE2);
        tex_grass_orientation = sb6::ktx::file::load("../Media/textures/grass_orientation.ktx");
        glActiveTexture(GL_TEXTURE3);
        tex_grass_color = sb6::ktx::file::load("../Media/textures/grass_color.ktx");        
        glActiveTexture(GL_TEXTURE4);
        tex_grass_bend = sb6::ktx::file::load("../Media/textures/grass_bend.ktx");
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

                in vec4 vVertex;
                out vec4 color;

                uniform mat4 mvpMatrix;

                layout (binding = 0)
                uniform sampler1D grassplatte_texture;
                layout (binding = 1)
                uniform sampler2D length_texture;
                layout (binding = 2)
                uniform sampler2D orientation_texture;
                layout (binding = 3)
                uniform sampler2D grasscolor_texture;
                layout (binding = 4)
                uniform sampler2D bend_texture;

                int random(int seed, int iterations)
                {
                    int value = seed;
                    int n;

                    for (n = 0; n < iterations; ++n)
                    {
                        value = ((value >> 7) ^ (value << 9)) * 15485863;
                    }

                    return value;
                }

                vec4 random_vector(int seed)
                {
                    int r = random(gl_InstanceID, 4);
                    int g = random(r, 2);
                    int b = random(g, 2);
                    int a = random(b, 2);

                    return vec4(float(r & 0x3ff) / 1024.0, 
                        float(g & 0x3ff) / 1024.0,
                        float(b & 0x3ff) / 1024.0,
                        float(a & 0x3ff) / 1024.0);
                }

                mat4 construct_rotation_matrix(float angle)
                {
                    float st = sin(angle);
                    float ct = cos(angle);

                    return mat4(vec4(ct, 0.0, st, 0.0),
                        vec4(0.0, 1.0, 0.0, 0.0),
                        vec4(-st, 0.0, ct, 0.0),
                        vec4(0.0, 0.0, 0.0, 1.0));
                }

                void main()
                {
                    vec4 offset = vec4(float(gl_InstanceID >> 10) - 512.0, 
                        0.0f,
                        float(gl_InstanceID & 0x3ff) - 512.0, 
                        0.0);

                    int number1 = random(gl_InstanceID, 3);
                    int number2 = random(number1, 2);
                    offset += vec4(float(number1 & 0x3ff) / 256.0,
                        0.0,
                        float(number2 & 0x3ff) / 256.0,
                        0.0);

                    // float angle = float(random(number2, 2) & 0x3ff) / 1024.0;

                    vec2 texcoord = offset.xz / 1024.0 + vec2(0.5);

                    // float bend_factor = float(random(number2, 7) & & 0x3ff) / 1024.0;
                    float bend_factor = texture(bend_texture, texcoord).r * 2.0;
                    float bend_amount = cos(vVertex.y);

                    float angle = texture(orientation_texture, texcoord).r * 2.0 * 3.141592;
                    mat4 rot = construct_rotation_matrix(angle);
                    vec4 position = (rot * (vVertex + vec4(0.0, 0.0, bend_amount * bend_factor, 0.0))) + offset;

                    position *= vec4(1.0, texture(length_texture, texcoord).r * 0.9 + 0.3, 1.0, 1.0);

                    gl_Position = mvpMatrix * position;

                    // color = vec4(random_vector(gl_InstanceID).xyz * vec3(0.1, 0.5, 0.1) + vec3(0.1, 0.4, 0.1), 1.0);
                    // color = texture(orientation_texture, texcoord);
                    color = texture(grassplatte_texture, texture(grasscolor_texture, texcoord).r) 
                        + vec4(random_vector(gl_InstanceID).xyz * vec3(0.1, 0.5, 0.1), 1.0);
                }
            )vs"
        };

        static const GLchar* fragShader[] =
        {
            R"fs(
                #version 430 core

                in vec4 color;
                out vec4 output_color;

                void main()
                {
                    output_color = color;
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
    }

private:
    GLuint renderingProgram;
    GLuint vertexArrayObject;
    GLuint buffer;

    GLuint tex_grass_color;
    GLuint tex_grass_length;
    GLuint tex_grass_orientation;
    GLuint tex_grass_bend;

    struct
    {
        GLuint mpvMatrix;
    } uniforms;
};

//DECLARE_MAIN(Chapter7_Grass);
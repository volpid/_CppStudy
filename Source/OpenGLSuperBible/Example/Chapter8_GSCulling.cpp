
#include "../Book/object.h"
#include "../Book/sb6.h"
#include "../Book/sb6ktx.h"
#include "../Book/shader.h"
#include "../Book/vmath.h"

#include <math.h>

class Chapter8_GSCulling : public sb6::application
{
public:
    GLuint program;
    GLint mv_location;
    GLint mvp_location;
    GLint viewport_location;

    sb6::object object;

    bool wireframeMode;

public:
    Chapter8_GSCulling()
    {
        wireframeMode = false;
    }

    void CompileShader()
    {
        static const GLchar* vertShader[] =
        {
            R"vs(

            #version 420 core

            in vec4 vVertex;
            in vec3 vNormal;

            out Vertex
            {
                vec3 normal;
                vec4 color;
            } vertex;

            uniform vec3 vLightPosition = vec3(-10.0, 40.0, 200.0);
            uniform mat4 mvMatrix;

            void main(void)
            {
                vec3 vEyeNormal = mat3(mvMatrix) * normalize(vNormal);

                vec4 vPosition4 = mvMatrix * vVertex;
                vec3 vPosition3 = vPosition4.xyz / vPosition4.w;

                vec3 vLightDir = normalize(vLightPosition - vPosition3);
                vertex.color = vec4(0.7, 0.6, 1.0, 1.0) * abs(dot(vEyeNormal, vLightDir));
                gl_Position = vVertex;
                vertex.normal = vNormal;
            }

            )vs"
        };

        static const GLchar* fragShader[] =
        {
            R"fs(

            #version 420 core
            
            in vec4 color;
            out vec4 out_color;

            void main()
            {
                out_color = color;
            }
                
            )fs"
        };

        static const GLchar* gsSource[] =
        {
            R"gs(
                
                #version 420 core

                layout (triangles) in;
                layout (triangle_strip, max_vertices = 3) out;

                in Vertex
                {
                    vec3 normal;
                    vec4 color;
                } vertex[];

                out vec4 color;

                uniform vec3 vLightPosition;
                uniform mat4 mvpMatrix;
                uniform mat4 mvMatrix;
                uniform vec3 viewpoint;

                void main(void)
                {
                    int n;
                    vec3 ab = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
                    vec3 ac = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
                    vec3 normal = normalize(cross(ab, ac));
                    vec3 transformed_normal = (mat3(mvMatrix) * normal);
                    vec4 worldspace = gl_in[0].gl_Position;
                    vec3 vt = normalize(viewpoint - worldspace.xyz);

                    if (dot(normal, vt) > 0.0)
                    {
                        for (n = 0; n < 3; ++n)
                        {
                            gl_Position = mvpMatrix * gl_in[n].gl_Position;
                            color = vertex[n].color;
                            EmitVertex();
                        }
                        EndPrimitive();
                    }
                }
            )gs"
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

        GLuint gs = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(gs, 1, gsSource, nullptr);
        glCompileShader(gs);

        glGetShaderiv(gs, GL_COMPILE_STATUS, &success);
        glGetShaderInfoLog(gs, 1024, NULL, logBuffer);

        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glAttachShader(program, gs);

        glLinkProgram(program);

        GLint status;
        glGetProgramiv(program, GL_LINK_STATUS, &status);

        if (!status)
        {
            char buffer[4096];
            glGetProgramInfoLog(program, 4096, NULL, buffer);
            glDeleteProgram(program);
        }

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
    }

    virtual void startup() override
    {
        CompileShader();

        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        mv_location = glGetUniformLocation(program, "mvMatrix");
        mvp_location = glGetUniformLocation(program, "mvpMatrix");
        viewport_location = glGetUniformLocation(program, "viewpoint");

        object.load("../Media/objects/dragon.sbm");
    }

    virtual void shutdown() override
    {
        glDeleteProgram(program);
    }

    virtual void render(double currentTime) override
    {
        static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        static const GLfloat one = 1.0f;

        float f = (float) currentTime;

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, black);
        glClearBufferfv(GL_DEPTH, 0, &one);

        vmath::mat4 proj_matrix = vmath::perspective(50.0f, (float)info.windowWidth / (float)info.windowHeight, 0.1f, 1000.0f);
        vmath::mat4 mv_matrix = vmath::translate(0.0f, -5.0f, -20.5f);
        vmath::mat4 mvp = proj_matrix * mv_matrix;

        GLfloat vViewport[] =
        {
            sinf(f * 2.1f) * 70.0f,
            cosf(f * 1.4f) * 70.0f,
            sinf(f * 0.7f) * 70.0f,
        };

        glUseProgram(program);

        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, proj_matrix * mv_matrix);
        glUniformMatrix4fv(mv_location, 1, GL_FALSE, mv_matrix);
        glUniform3fv(viewport_location, 1, vViewport);

        glPolygonMode(GL_FRONT_AND_BACK, wireframeMode ? GL_LINE : GL_FILL);

        object.render();
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
            case 'W':
            {
                wireframeMode = !wireframeMode;
            }
            break;
            }
        }
    }
};

//DECLARE_MAIN(Chapter8_GSCulling);
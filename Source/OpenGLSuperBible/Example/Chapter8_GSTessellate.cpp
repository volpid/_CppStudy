
#include "../Book/object.h"
#include "../Book/sb6.h"
#include "../Book/sb6ktx.h"
#include "../Book/shader.h"
#include "../Book/vmath.h"

#include <math.h>

class Chapter8_GSTessellate : public sb6::application
{
public:
    GLuint program;
    GLuint vao;

    GLuint mv_location;
    GLuint mvp_location;
    GLuint stretch_location;
    GLuint buffer;

    bool wireframeMode;

public:
    Chapter8_GSTessellate()
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

            void main(void)
            {
                gl_Position = vVertex;
            }

            )vs"
        };

        static const GLchar* fragShader[] =
        {
            R"fs(

            #version 420 core
            
            flat in vec4 color;
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
                layout (triangle_strip, max_vertices = 12) out;

                uniform float stretch = 0.7;
                uniform mat4 mvpMatrix;
                uniform mat4 mvMatrix;

                flat out vec4 color;

                void make_face(vec3 a, vec3 b, vec3 c)
                {
                    vec3 face_normal = normalize(cross(c - a, c - b));
                    vec4 face_color = vec4(1.0, 0.2, 0.4, 1.0) * (mat3(mvMatrix) * face_normal).z;
                    gl_Position = mvpMatrix * vec4(a, 1.0);
                    color = face_color;
                    EmitVertex();

                    gl_Position = mvpMatrix * vec4(b, 1.0);
                    color = face_color;
                    EmitVertex();

                    gl_Position = mvpMatrix * vec4(c, 1.0);
                    color = face_color;
                    EmitVertex();
                }

                void main(void)
                {
                    int n;
                    vec3 a = gl_in[0].gl_Position.xzy;
                    vec3 b = gl_in[1].gl_Position.xzy;
                    vec3 c = gl_in[2].gl_Position.xzy;

                    vec3 d = (a + b) * stretch;
                    vec3 e = (b + c) * stretch;
                    vec3 f = (c + a) * stretch;

                    a *= (2.0 - stretch);
                    b *= (2.0 - stretch);
                    c *= (2.0 - stretch);

                    make_face(a, d, f);
                    make_face(d, b, e);
                    make_face(e, c, f);
                    make_face(d, e, f);

                    EndPrimitive();
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

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        mv_location = glGetUniformLocation(program, "mvMatrix");
        mvp_location = glGetUniformLocation(program, "mvpMatrix");
        stretch_location = glGetUniformLocation(program, "stretch");

        static const GLfloat tetrahedron_verts[] =
        {
             0.000f,  0.000f,  1.000f,
             0.943f,  0.000f, -0.333f,
            -0.471f,  0.816f, -0.333f,
            -0.471f, -0.816f, -0.333f
        };

        static const GLushort tetrahedron_indices[] =
        {
            0, 1, 2,
            0, 2, 3,
            0, 3, 1,
            3, 2, 1
        };

        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(tetrahedron_verts) + sizeof(tetrahedron_indices), nullptr, GL_STATIC_DRAW);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(tetrahedron_indices), tetrahedron_indices);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, sizeof(tetrahedron_indices), sizeof(tetrahedron_verts), tetrahedron_verts);

        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*) sizeof(tetrahedron_indices));
        glEnableVertexAttribArray(0);

    }

    virtual void shutdown() override
    {
        glDeleteProgram(program);
        glDeleteVertexArrays(1, &vao);
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
        vmath::mat4 mv_matrix = vmath::translate(0.0f, 0.0f, -10.0f)
            * vmath::rotate((float) currentTime * 71.0f, 0.0f, 1.0f, 0.0f)
            * vmath::rotate((float) currentTime * 10.0f, 1.0f, 0.0f, 0.0f);
        vmath::mat4 mvp = proj_matrix * mv_matrix;

        glUseProgram(program);

        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, proj_matrix * mv_matrix);
        glUniformMatrix4fv(mv_location, 1, GL_FALSE, mv_matrix);
        glUniform1f(stretch_location, sinf(f * 4.0f) * 0.75 + 1.0f);

        glPolygonMode(GL_FRONT_AND_BACK, wireframeMode ? GL_LINE : GL_FILL);

        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_SHORT, nullptr);
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

//DECLARE_MAIN(Chapter8_GSTessellate);
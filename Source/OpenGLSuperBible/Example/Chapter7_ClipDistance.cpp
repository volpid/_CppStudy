
#include "../Book/object.h"
#include "../Book/sb6.h"
#include "../Book/sb6ktx.h"
#include "../Book/shader.h"
#include "../Book/vmath.h"

#include <math.h>

class Chapter7_ClipDistance : public sb6::application
{
public:
    bool paused;
    
    GLuint renderProgram;
    sb6::object object;

    struct
    {
        GLint projMatrix;
        GLint mvMatrix;
        GLint clipPlane;
        GLint clipSphere;
    } uniforms;

public:
    Chapter7_ClipDistance()
    {
        renderProgram = 0;
        paused = false;
    }

    virtual void startup() override
    {
        object.load("../Media/objects/dragon.sbm");
        loadShader();
    }

    void loadShader()
    {
        if (renderProgram != 0)
        {
            glDeleteProgram(renderProgram);
        }
        GLuint shaders[] =
        {
            sb6::shader::load("../Media/shaders/clipdistance/render.vs.glsl", GL_VERTEX_SHADER),
            sb6::shader::load("../Media/shaders/clipdistance/render.fs.glsl", GL_FRAGMENT_SHADER),
        };

        renderProgram = sb6::program::link_from_shaders(shaders, 2, true);

        uniforms.projMatrix = glGetUniformLocation(renderProgram, "proj_matrix");
        uniforms.mvMatrix = glGetUniformLocation(renderProgram, "mv_matrix");
        uniforms.clipPlane = glGetUniformLocation(renderProgram, "clip_plane");
        uniforms.clipSphere = glGetUniformLocation(renderProgram, "clip_sphere");
    }

    virtual void render(double currentTime) override
    {
        static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        static const GLfloat One = 1.0f;

        static double lastTime = 0.0;
        static double totalTime = 0.0; 

        if (paused == false)
        {
            totalTime += (currentTime - lastTime);
        }
        lastTime = currentTime;

        float f = (float) totalTime;

        glClearBufferfv(GL_COLOR, 0, black);
        glClearBufferfv(GL_DEPTH, 0, &One);

        glUseProgram(renderProgram);

        vmath::mat4 projMatrix = vmath::perspective(50.0f, 
            (float) info.windowWidth / (float) info.windowHeight,
            0.1f,
            1000.0f);

        vmath::mat4 mvMatrix = vmath::translate(0.0f, 0.0f, -15.0f) 
            * vmath::rotate(f * 0.34f, 0.0f, 1.0f, 0.0f)
            * vmath::translate(0.0f, -4.0f, 0.0f);

        vmath::mat4 planeMatrix = vmath::rotate(f * 6.0f, 1.0f, 0.0f, 0.0f) * vmath::rotate(f * 7.3f, 0.0f, 1.0f, 0.0f);

        vmath::vec4 plane = planeMatrix[0];
        plane[3] = 0.0f;
        plane = vmath::normalize(plane);

        vmath::vec4 clipSphere = vmath::vec4(sinf(f * 0.7f) * 3.0f, cosf(f * 1.9f) * 3.0f, sinf(f * 0.1) * 3.0f, cosf(f * 1.7f) + 2.5f);

        glUniformMatrix4fv(uniforms.projMatrix, 1, GL_FALSE, projMatrix);
        glUniformMatrix4fv(uniforms.mvMatrix, 1, GL_FALSE, mvMatrix);
        glUniform4fv(uniforms.clipPlane, 1, plane);
        glUniform4fv(uniforms.clipSphere, 1, clipSphere);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CLIP_DISTANCE0);
        glEnable(GL_CLIP_DISTANCE1);

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
                case 'R': 
                {
                    loadShader();
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

//DECLARE_MAIN(Chapter7_ClipDistance);
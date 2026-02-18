
#include "../Book/object.h"
#include "../Book/sb6.h"
#include "../Book/sb6ktx.h"
#include "../Book/shader.h"
#include "../Book/vmath.h"

#include <cstdio>
#include <math.h>

class Chapter9_Bumpmapping : public sb6::application
{
public:
    GLuint program;

    struct 
    {
        GLuint color;
        GLuint normals;
    } textures;
    
    struct
    {
        GLint mv_matrix;
        GLint proj_matrix;
        GLint light_pos;

    } uniforms;

    sb6::object object;
    
    bool paused;

public:
    Chapter9_Bumpmapping()
    {
        program = 0;
        paused = false;
    }

    virtual void init() override
    {
        sb6::application::init();

        info.samples = 1;
    }

    void CompileShader()
    {   
        if (program != 0)
        {
            glDeleteProgram(program);
        }

        GLuint vs;
        GLuint fs;
        vs = sb6::shader::load("../Media/shaders/bumpmapping/bumpmapping.vs.glsl", GL_VERTEX_SHADER);
        fs = sb6::shader::load("../Media/shaders/bumpmapping/bumpmapping.fs.glsl", GL_FRAGMENT_SHADER);

        program = glCreateProgram();
        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glLinkProgram(program);

        uniforms.mv_matrix = glGetUniformLocation(program, "mv_matrix");
        uniforms.proj_matrix = glGetUniformLocation(program, "proj_matrix");
        uniforms.light_pos = glGetUniformLocation(program, "light_pos");

        glDeleteShader(vs);
        glDeleteShader(fs);
    }

    void MakeScreeshot()
    {
        int row_size = ((info.windowWidth * 3 + 3) & ~3);
        int data_size = row_size * info.windowHeight;
        unsigned char* data = new unsigned char[data_size];

#pragma pack(push, 1)
        struct
        {
            unsigned char identsize;
            unsigned char cmaptype;
            unsigned char imagetype;
            short cmapstart;
            short cmapsize;
            unsigned char cmapbpp;
            short xorgin;
            short yorgin;
            short width;
            short height;
            unsigned char bpp;
            unsigned char descriptor;
        } tga_header;
#pragma pack(pop)

        glReadPixels(0, 0, info.windowWidth, info.windowHeight,
            GL_BGR, GL_UNSIGNED_BYTE,
            data);

        memset(&tga_header, 0, sizeof(tga_header));
        tga_header.imagetype = 2;
        tga_header.width = (short) info.windowWidth;
        tga_header.height = (short) info.windowHeight;
        tga_header.bpp = 24;

        FILE* f_out = nullptr;
        fopen_s(&f_out, "screenshot.tga", "wb");

        fwrite(&tga_header, sizeof(tga_header), 1, f_out);
        fwrite(data, data_size, 1, f_out);
        fclose(f_out);

        delete[] data;
    }

    virtual void startup() override
    {   
        CompileShader();

        glUseProgram(program);

        glActiveTexture(GL_TEXTURE0);
        textures.color = sb6::ktx::file::load("../Media/textures/ladybug_co.ktx");

        glActiveTexture(GL_TEXTURE1);
        textures.normals = sb6::ktx::file::load("../Media/textures/ladybug_nm.ktx");

        object.load("../Media/objects/ladybug.sbm");
    }

    virtual void shutdown() override
    {
        glDeleteProgram(program);
    }

    virtual void render(double currentTime) override
    {
        static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        static const GLfloat gray[] = { 0.1f, 0.1f, 0.1f, 1.0f };
        static const GLfloat one = 1.0f;

        static double last_time = 0.0;
        static double total_time = 0.0;

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, gray);
        glClearBufferfv(GL_DEPTH, 0, &one);

        glEnable(GL_DEPTH_TEST);

        glUseProgram(program);
        
        if (paused == false)
        {
            total_time += (currentTime - last_time);
        }
        last_time = currentTime;
        
        float t = (float) currentTime;
        vmath::mat4 proj_matrix = vmath::perspective(50.0f, (float)info.windowWidth / (float)info.windowHeight, 0.1f, 1000.0f);

        glUniformMatrix4fv(uniforms.proj_matrix, 1, GL_FALSE, proj_matrix);

        vmath::mat4 mv_matrix = vmath::translate(0.0f, -0.2f, -15.5f)
            * vmath::rotate(14.0f, 1.0f, 0.0f, 0.0f)
            * vmath::rotate(-20.0f, 0.0f, 1.0f, 0.0f);

        glUniformMatrix4fv(uniforms.mv_matrix, 1, GL_FALSE, mv_matrix);

        glUniform3fv(uniforms.light_pos, 1, vmath::vec3(40.0f * sinf(t), 30.0f + 20.0f * cosf(t), 40.0f));

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
                case 'P':
                {
                    paused = !paused;
                }
                break;
                case 'S':
                {
                    MakeScreeshot();
                }
                break;
            }
        }
    }
};

//DECLARE_MAIN(Chapter9_Bumpmapping);
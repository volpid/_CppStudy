
#include "../Book/object.h"
#include "../Book/sb6.h"
#include "../Book/sb6ktx.h"
#include "../Book/shader.h"
#include "../Book/vmath.h"

#include <math.h>

enum BufferType
{
    POSITION_A,
    POSITION_B,
    VELOCITY_A,
    VELOCITY_B,
    CONNECTION,
};

enum
{
    POINT_X = 50,
    POINT_Y = 50,
    POINT_TOTAL = POINT_X * POINT_Y,
    CONNECTION_TOTAL = (POINT_X - 1) * POINT_Y + (POINT_Y - 1) * POINT_X, //nanpid?
};

class Chapter7_ClipDistance : public sb6::application
{
public:
    GLuint _vao[2];
    GLuint _vbo[5];
    GLuint _indexBuffer;
    GLuint _pos_tbo[2];
    GLuint _updateProgram;
    GLuint _renderProgram;
    GLuint _c_loc;
    GLuint _iterationIndex;

    bool _darwPoint;
    bool _drawLine;
    int _iterationPerFrame;

public:
    Chapter7_ClipDistance()
    {
        _iterationIndex = 0;
        _updateProgram = 0;
        _renderProgram = 0;

        _darwPoint = true;
        _drawLine = true;
        _iterationPerFrame = 16;
    }

    virtual void startup() override
    {
        loadShader();

        vmath::vec4* initialPosition = new vmath::vec4[POINT_TOTAL];
        vmath::vec3* initialVelocity = new vmath::vec3[POINT_TOTAL];
        vmath::ivec4* connectionVectors = new vmath::ivec4[POINT_TOTAL];

        int32_t index = 0;
        for (int32_t posY = 0; posY < POINT_Y; ++posY)
        {
            float fy = static_cast<float>(posY) / static_cast<float>(POINT_Y);
            for (int32_t posX = 0; posX < POINT_X; ++posX)
            {
                float fx = static_cast<float>(posX) / static_cast<float>(POINT_X);

                initialPosition[index] = vmath::vec4((fx - 0.5f) * static_cast<float>(POINT_X),
                    (fy - 0.5f) * static_cast<float>(POINT_Y),
                    0.6f * sinf(fx) * cosf(fy),
                    1.0f);
                    
                initialVelocity[index] = vmath::vec3(0.0f);
                
                connectionVectors[index] = vmath::ivec4(-1);

                if (posY != (POINT_Y - 1))
                {
                    if (posX != 0)
                    {
                        connectionVectors[index][0] = index - 1;
                    }
                    if (posY != 0)
                    {
                        connectionVectors[index][1] = index - POINT_X;
                    }
                    if (posX != (POINT_X - 1))
                    {
                        connectionVectors[index][2] = index + 1;
                    }
                    if (posY != (POINT_Y - 1))
                    {
                        connectionVectors[index][3] = index + POINT_X;
                    }
                }

                ++index;
            }
        }

        glGenVertexArrays(2, _vao);
        glGenBuffers(5, _vbo);

        for (int32_t idx = 0; idx < 2; ++idx)
        {
            glBindVertexArray(_vao[idx]);

            glBindBuffer(GL_ARRAY_BUFFER, _vbo[POSITION_A + idx]);
            glBufferData(GL_ARRAY_BUFFER, POINT_TOTAL * sizeof(vmath::vec4), initialPosition, GL_DYNAMIC_COPY);
            glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
            glEnableVertexAttribArray(0);

            glBindBuffer(GL_ARRAY_BUFFER, _vbo[VELOCITY_A + idx]);
            glBufferData(GL_ARRAY_BUFFER, POINT_TOTAL * sizeof(vmath::vec3), initialVelocity, GL_DYNAMIC_COPY);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
            glEnableVertexAttribArray(1);

            glBindBuffer(GL_ARRAY_BUFFER, _vbo[CONNECTION]);
            glBufferData(GL_ARRAY_BUFFER, POINT_TOTAL * sizeof(vmath::ivec4), connectionVectors, GL_STATIC_DRAW);
            glVertexAttribIPointer(2, 4, GL_INT, 0, nullptr);
            glEnableVertexAttribArray(2);
        }
        
        delete initialPosition;
        delete initialVelocity;
        delete connectionVectors;

        glGenTextures(2, _pos_tbo);
        glBindTexture(GL_TEXTURE_BUFFER, _pos_tbo[0]);
        glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, _vbo[POSITION_A]);

        glBindTexture(GL_TEXTURE_BUFFER, _pos_tbo[1]);
        glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, _vbo[POSITION_B]);

        int32_t lines = (POINT_X - 1) * POINT_Y + (POINT_Y - 1) * POINT_X;

        glGenBuffers(1, &_indexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, lines * 2 * sizeof(int32_t), nullptr, GL_STATIC_DRAW);

        int32_t* mapping = (int32_t*) glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, lines * 2 * sizeof(int32_t), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

        for (int32_t posY = 0; posY < POINT_Y; ++posY)
        {
            for (int32_t posX = 0; posX < POINT_X - 1; ++posX)
            {
                (*mapping++) = posY * POINT_X + posX;
                (*mapping++) = posY * POINT_X + posX + 1;
            }
        }

        for (int32_t posX = 0; posX < POINT_X; ++posX)
        {
            for (int32_t posY = 0; posY < POINT_Y - 1; ++posY)
            {
                (*mapping++) = posY * POINT_X + posX;
                (*mapping++) = (posY + 1) * POINT_X + posX;
            }
        }

        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    }

    virtual void shutdown() override
    {
        glDeleteProgram(_updateProgram);
        glDeleteProgram(_renderProgram);
        glDeleteVertexArrays(2, _vao);
        glDeleteBuffers(5, _vbo);
    }

    void loadShader()
    {
        GLuint vs;
        GLuint fs;
        char buffer[1024] = { 0, };

        vs = sb6::shader::load("../Media/shaders/springmass/update.vs.glsl", GL_VERTEX_SHADER);

        if (_updateProgram != 0)
        {
            glDeleteProgram(_updateProgram);
        }
        _updateProgram = glCreateProgram();
        glAttachShader(_updateProgram, vs);

        static const char* tf_varyings[] = 
        {
            "tf_position_mass",
            "tf_velocity",
        };

        glTransformFeedbackVaryings(_updateProgram, 2, tf_varyings, GL_SEPARATE_ATTRIBS);
        glLinkProgram(_updateProgram);

        glGetShaderInfoLog(vs, 1024, nullptr, buffer);
        glGetProgramInfoLog(_updateProgram, 1024, nullptr, buffer);
        glDeleteShader(vs);

        vs = sb6::shader::load("../Media/shaders/springmass/render.vs.glsl", GL_VERTEX_SHADER);
        fs = sb6::shader::load("../Media/shaders/springmass/render.fs.glsl", GL_FRAGMENT_SHADER);

        if (_renderProgram != 0)
        {
            glDeleteProgram(_renderProgram);
        }

        _renderProgram = glCreateProgram();
        glAttachShader(_renderProgram, vs);
        glAttachShader(_renderProgram, fs);
        glLinkProgram(_renderProgram);
    }

    virtual void render(double currentTime) override
    {
        glUseProgram(_updateProgram);
        glEnable(GL_RASTERIZER_DISCARD);

        for (int32_t idx = _iterationPerFrame; idx != 0 ; --idx)
        {
            glBindVertexArray(_vao[(_iterationIndex & 1)]);
            glBindTexture(GL_TEXTURE_BUFFER, _pos_tbo[(_iterationIndex & 1)]);

            ++_iterationIndex;

            glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, _vbo[POSITION_A + (_iterationIndex & 1)]);
            glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, _vbo[VELOCITY_A + (_iterationIndex & 1)]);
            glBeginTransformFeedback(GL_POINTS);
            glDrawArrays(GL_POINTS, 0, POINT_TOTAL);
            glEndTransformFeedback();
        }

        glDisable(GL_RASTERIZER_DISCARD);
        
        static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, black);

        glUseProgram(_renderProgram);

        if (_darwPoint == true)
        {
            glPointSize(4.0f);
            glDrawArrays(GL_POINTS, 0, POINT_TOTAL);
        }

        if (_drawLine == true)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
            glDrawElements(GL_LINES, CONNECTION_TOTAL * 2, GL_UNSIGNED_INT, nullptr);
        }
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
                case 'L':
                {
                    _drawLine = !_drawLine;
                }
                break;
                case 'P':
                {
                    _darwPoint = !_darwPoint;
                }
                break;
                case GLFW_KEY_KP_ADD:
                {
                    ++_iterationPerFrame;
                }
                case GLFW_KEY_KP_SUBTRACT:
                {
                    --_iterationPerFrame;
                }
                break;
            }
        }
    }
};

//DECLARE_MAIN(Chapter7_ClipDistance);
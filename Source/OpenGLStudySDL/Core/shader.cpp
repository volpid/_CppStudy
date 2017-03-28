
#include "shader.h"

#include <fstream>
#include <iostream>

//---------------------------------------------------------------------------
// define - Local
//---------------------------------------------------------------------------
namespace
{
    void CheckShaderError(GLuint shader, GLuint flag, bool isProgram, const std::string& errorMessage)
    {
        GLint success = GL_FALSE;
        GLchar error[_MAX_PATH] = {0,};

        if (isProgram == true)
        {
            glGetProgramiv(shader, flag, &success);
        }
        else
        {
            glGetShaderiv(shader, flag, &success);
        }

        if (success == GL_FALSE)
        {
            if (isProgram == true)
            {
                glGetProgramInfoLog(shader, sizeof(error), NULL, error);
            }
            else
            {
                glGetShaderInfoLog(shader, sizeof(error), NULL, error);
            }

            std::cerr << errorMessage << " " << error << std::endl;
        }
    }

    std::string LoadShader(const std::string& filename)
    {
        std::ifstream file;
        file.open(filename.c_str());

        std::string output;
        std::string line;

        if (file.is_open())
        {
            while(file.good() == true)
            {
                getline(file, line);
                output.append(line + "\n");
            }
        }
        else 
        {
            std::cerr << "Unable to load shader" << std::endl;
        }

        return output;
    }

    GLuint CreateShader(const std::string& text, GLenum shaderType)
    {
        GLuint shader = glCreateShader(shaderType);
        if (shader == 0)
        {
            std::cerr << "Error: shader creation fialed!" << std::endl;
        }

        const GLchar* shaderSourceStrings[1];
        GLint shaderSourceStringLengths[1];
        shaderSourceStrings[0] = text.c_str();
        shaderSourceStringLengths[0] = text.length();

        glShaderSource(shader, 1, shaderSourceStrings, shaderSourceStringLengths);
        glCompileShader(shader);
        CheckShaderError(shader, GL_COMPILE_STATUS, false, "Error: shader complation failed.");

        return shader;
    }
}

//---------------------------------------------------------------------------
// define - Shader
//---------------------------------------------------------------------------
Shader::Shader(const std::string& filename)
{
    program_ = glCreateProgram();
    shaders_[0] = CreateShader(LoadShader(filename + ".vs"), GL_VERTEX_SHADER);
    shaders_[1] = CreateShader(LoadShader(filename + ".fs"), GL_FRAGMENT_SHADER);

    for (unsigned int i = 0; i < NumShader; ++i)
    {
        glAttachShader(program_, shaders_[i]);
    }

    glBindAttribLocation(program_, 0, "position");
    glBindAttribLocation(program_, 1, "coords");
    glBindAttribLocation(program_, 2, "normal");

    glLinkProgram(program_);
    CheckShaderError(program_, GL_LINK_STATUS, true, "Error : program link failed!");

    glValidateProgram(program_);
    CheckShaderError(program_, GL_VALIDATE_STATUS, true, "Error : program validate failed!");

    uniforms_[Uniform::TransformMatrix] = glGetUniformLocation(program_, "transform");
}

Shader::~Shader(void)
{
    for (unsigned int i = 0; i < NumShader; ++i)
    {
        glDetachShader(program_, shaders_[i]);
        glDeleteShader(shaders_[i]);
    }

    glDeleteProgram(program_);
}

void Shader::Bind(void)
{   
    glUseProgram(program_);
}

void Shader::Update(const Transform& transform, const Camera& camera)
{
    glm::mat4 model = camera.GetViewPorjection() * transform.GetModel();
    glUniformMatrix4fv(uniforms_[Uniform::TransformMatrix],
        1,
        GL_FALSE,
        &model[0][0]);
}

#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <sstream>

class Shader
{
public:
    Shader() : compiled(false) {}
    ~Shader()
    {
        for(auto& shader : _shaders) glDeleteShader(shader);
        if(compiled) glDeleteProgram(_program);
    }

    void add(const std::string& source, GLenum type)
    {
        if(compiled) return;

        std::stringstream buffer;
        std::ifstream inFile(source.c_str());
        if(!inFile.is_open())
        {
            std::string errorContent = "Failed to open file: " + source;
            throw std::runtime_error(errorContent);
        }
        buffer << inFile.rdbuf();

        std::string contentStr = buffer.str();
        const char* content = contentStr.c_str();
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &content, nullptr);
        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if(!success)
        {
            GLint infoLen;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            std::vector<GLchar> infoLog(infoLen+1);
            glGetShaderInfoLog(shader, infoLen, nullptr, &infoLog[0]);
            std::string content = "Shader " + source + " failed to compile: " +
                std::string(&infoLog[0]);
            throw std::runtime_error(content);
        }

        _shaders.push_back(shader);
    }

    void compile()
    {
        if(compiled) return;

        _program = glCreateProgram();
        for(auto& shader : _shaders) glAttachShader(_program, shader);
        glLinkProgram(_program);

        GLint success;
        glGetProgramiv(_program, GL_LINK_STATUS, &success);
        if(!success)
        {
            GLint infoLen;
            glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &infoLen);
            std::vector<GLchar> infoLog(infoLen+1);
            glGetProgramInfoLog(_program, infoLen, nullptr, &infoLog[0]);
            std::string content = std::string("Shader failed to link: ") + std::string(&infoLog[0]);
            throw std::runtime_error(content);
        }

        for(auto& shader : _shaders) glDeleteShader(shader);
        _shaders.clear();

        compiled = true;
    }

    void uniformBool(const char* name, bool val) const
    {
        if(!compiled) return;
        glUniform1i(glGetUniformLocation(_program, name), static_cast<int>(val));
    }

    void uniformInt(const char* name, int val) const
    {
        if(!compiled) return;
        glUniform1i(glGetUniformLocation(_program, name), val);
    }

    void uniformFloat(const char* name, float val) const
    {
        if(!compiled) return;
        glUniform1f(glGetUniformLocation(_program, name), val);
    }

    void uniformVec2(const char* name, const glm::vec2& val) const
    {
        if(!compiled) return;
        glUniform2fv(glGetUniformLocation(_program, name), 1, glm::value_ptr(val));
    }

    void uniformVec3(const char* name, const glm::vec3& val) const
    {
        if(!compiled) return;
        glUniform3fv(glGetUniformLocation(_program, name), 1, glm::value_ptr(val));
    }

    void uniformMat3x3(const char* name, const glm::mat3& matrix) const
    {
        if(!compiled) return;
        glUniformMatrix3fv(glGetUniformLocation(_program, name), 1, GL_FALSE, glm::value_ptr(matrix));
    }

    void uniformMat4x3(const char* name, const glm::mat4x3& matrix) const
    {
        if(!compiled) return;
        glUniformMatrix4x3fv(glGetUniformLocation(_program, name), 1, GL_FALSE, glm::value_ptr(matrix));
    }

    void uniformMat4x4(const char* name, const glm::mat4& matrix) const
    {
        if(!compiled) return;
        glUniformMatrix4fv(glGetUniformLocation(_program, name), 1, GL_FALSE, glm::value_ptr(matrix));
    }

    GLuint program() {return _program;}

    bool compiled;

private:
    GLuint _program;
    std::vector<GLuint> _shaders;
};
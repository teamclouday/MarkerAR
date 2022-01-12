#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <string>
#include <tuple>
#include "shader.hpp"

class Model
{
public:
    virtual void UI() = 0;
    virtual void render(
        const glm::mat3& cameraK, const glm::mat4x3& poseM,
        const glm::mat4& cameraProj,
        float cameraRatio, float windowRatio
    ) = 0;

protected:
    std::shared_ptr<Shader> _shader;
    GLuint _vao, _ebo;
    int _count;
    bool _lineMode;

    int _width, _height;

    bool loadObj(
        const std::string& filename,
        std::vector<glm::vec3>& vertices,
        std::vector<unsigned>& indices
    );

    bool loadObj(
        const std::string& filename,
        // vertex position, normal, and uv
        std::vector<float>& buffer
        // std::vector<std::tuple<glm::vec3,glm::vec3,glm::vec2>>& buffer
    );

    bool loadTexture(const std::string& filename, GLuint& texIdx);
};

class ModelCube : public Model
{
public:
    ModelCube(int camWidth, int camHeight);
    ~ModelCube();
    void UI();
    void render(
        const glm::mat3& cameraK, const glm::mat4x3& poseM,
        const glm::mat4& cameraProj,
        float cameraRatio, float windowRatio
    );

private:
    float _scale = 1.0f;
    glm::vec3 _translation = glm::vec3(0.0f, 0.5f, 0.0f);
};

class ModelTeapot : public Model
{
public:
    ModelTeapot(int camWidth, int camHeight);
    ~ModelTeapot();
    void UI();
    void render(
        const glm::mat3& cameraK, const glm::mat4x3& poseM,
        const glm::mat4& cameraProj,
        float cameraRatio, float windowRatio
    );

private:
    float _scale = 0.5f;
    glm::vec3 _translation = glm::vec3(0.0f, 0.5f, 0.0f);
    glm::vec3 _lightPos = glm::vec3(1.0f);
    glm::vec3 _model_diffuse = glm::vec3(0.6f, 1.0f, 0.87f);
};

class ModelBunny : public Model
{
public:
    ModelBunny(int camWidth, int camHeight);
    ~ModelBunny();
    void UI();
    void render(
        const glm::mat3& cameraK, const glm::mat4x3& poseM,
        const glm::mat4& cameraProj,
        float cameraRatio, float windowRatio
    );

private:
    float _scale = 20.0f;
    glm::vec3 _translation = glm::vec3(0.0f);
    glm::vec3 _lightPos = glm::vec3(1.0f);
    glm::vec3 _model_diffuse = glm::vec3(0.87f, 1.0f, 0.848f);
};

class ModelTyra : public Model
{
public:
    ModelTyra(int camWidth, int camHeight);
    ~ModelTyra();
    void UI();
    void render(
        const glm::mat3& cameraK, const glm::mat4x3& poseM,
        const glm::mat4& cameraProj,
        float cameraRatio, float windowRatio
    );

private:
    float _scale = 0.9f;
    glm::vec3 _translation = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 _lightPos = glm::vec3(1.0f);
    glm::vec3 _model_diffuse = glm::vec3(0.485f, 0.413f, 0.297f);
};

class ModelArmadillo : public Model
{
public:
    ModelArmadillo(int camWidth, int camHeight);
    ~ModelArmadillo();
    void UI();
    void render(
        const glm::mat3& cameraK, const glm::mat4x3& poseM,
        const glm::mat4& cameraProj,
        float cameraRatio, float windowRatio
    );

private:
    float _scale = 1.0f;
    glm::vec3 _translation = glm::vec3(0.0f, 0.5f, 0.0f);
    glm::vec3 _lightPos = glm::vec3(1.0f, 1.0f, -1.0f);
    glm::vec3 _model_diffuse = glm::vec3(1.0f);
};

class ModelSpiderMan : public Model
{
public:
    ModelSpiderMan(int camWidth, int camHeight);
    ~ModelSpiderMan();
    void UI();
    void render(
        const glm::mat3& cameraK, const glm::mat4x3& poseM,
        const glm::mat4& cameraProj,
        float cameraRatio, float windowRatio
    );

private:
    float _scale = 0.1f;
    glm::vec3 _translation = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 _lightPos = glm::vec3(1.0f, 1.0f, -1.0f);
    GLuint _texNormal, _texSpec, _texColor;
};
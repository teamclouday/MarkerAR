#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <string>
#include "shader.hpp"

class Model
{
public:
    virtual void UI() = 0;
    virtual void render(
        const glm::mat3& cameraK, const glm::mat4x3& poseM,
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
};

class ModelCube : public Model
{
public:
    ModelCube(int camWidth, int camHeight);
    ~ModelCube();
    void UI();
    void render(
        const glm::mat3& cameraK, const glm::mat4x3& poseM,
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
        float cameraRatio, float windowRatio
    );

private:
    float _scale = 0.5f;
    glm::vec3 _translation = glm::vec3(0.0f, 0.5f, 0.0f);
    glm::vec3 _lightPos = glm::vec3(1.0f);
};
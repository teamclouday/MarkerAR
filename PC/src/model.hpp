#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include "shader.hpp"

class Model
{
public:
    virtual void UI() = 0;
    virtual void render(const glm::mat4x3& poseM, float cameraRatio, float windowRatio) = 0;

protected:
    std::shared_ptr<Shader> _shader;
    GLuint _vao;
    int _vertex_count;
    bool _lineMode;
};

class ModelCube : public Model
{
public:
    ModelCube();
    ~ModelCube();
    void UI();
    void render(const glm::mat4x3& poseM, float cameraRatio, float windowRatio);
};
#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include "shader.hpp"

struct BoxData
{
    glm::ivec2 p1 = glm::ivec2(-1);
    glm::ivec2 p2 = glm::ivec2(-1);
    glm::ivec2 p3 = glm::ivec2(-1);
    glm::ivec2 p4 = glm::ivec2(-1);
};

class Marker
{
public:
    Marker(int width, int height);
    ~Marker();

    void process(GLuint sourceImg, int groupX, int groupY);
    void drawCorners(float ratioCon, float ratioCam);

    // fetch texture and update index
    GLuint fetchTex()
    {
        GLuint tex = _tex[_currentTex];
        _currentTex = !_currentTex;
        return tex;
    }
    // only get current texture
    GLuint lastTex() {return _tex[!_currentTex];}
    bool debug() {return _debug_mode && _debug_level < 3;}

    void UI();


private:
    int _width, _height;
    GLuint _tex[2];
    int _currentTex = 0;
    std::shared_ptr<Shader> _shader1, _shader2,
        _shader3, _shader4, _shaderDraw;

    int _gray_shades = 1;
    bool _blur = false;
    float _blur_radius = 5.0f, _blur_quality = 6.0f, _blur_directions = 12.0f;
    float _threshold = 0.5f;
    bool _auto_threshold = false;
    int _auto_threshold_level = 0;
    int _boxesX, _boxesY;
    std::vector<BoxData> _boxes;
    GLuint _boxesSSBO, _boxesVAO;

    int _debug_level = 0;
    bool _debug_mode = false;
};
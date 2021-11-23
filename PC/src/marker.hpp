#pragma once

#include <GL/glew.h>
#include <memory>
#include "shader.hpp"

class Marker
{
public:
    Marker(int width, int height);
    ~Marker();

    void process(GLuint sourceImg, int groupX, int groupY);

    // fetch texture and update index
    GLuint fetchTex()
    {
        GLuint tex = _tex[_currentTex];
        _currentTex = !_currentTex;
        return tex;
    }
    // only get current texture
    GLuint lastTex() {return _tex[!_currentTex];}
    bool debug() {return _debug_mode;}

    void UI();


private:
    int _width, _height;
    GLuint _tex[2];
    int _currentTex = 0;
    std::shared_ptr<Shader> _shader1, _shader2;

    int _gray_shades = 1;
    float _threshold = 0.5f;
    bool _auto_threshold = true;

    int _debug_level = 0;
    bool _debug_mode = false;
};
#pragma once

#include <GL/glew.h>
#include <memory>
#include "shader.hpp"

class Marker
{
public:
    Marker(int width, int height);
    ~Marker();

    void process(GLuint sourceImg);

    // fetch texture and update index
    GLuint fetchTex()
    {
        GLuint tex = _tex[_currentTex];
        _currentTex = !_currentTex;
        return tex;
    }
    // only get current texture
    GLuint currentTex() {return _tex[_currentTex];}

private:
    int _width, _height, _groupX, _groupY;
    GLuint _tex[2];
    int _currentTex = 0;
    std::shared_ptr<Shader> _shader1, _shader2;

    float _threshold = 0.5f;
    bool _auto_threshold = true;
};
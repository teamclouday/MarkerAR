#include "marker.hpp"
#include <cmath>

Marker::Marker(int width, int height) : _width(width), _height(height)
{
    _groupX = static_cast<int>(std::ceil(width / 32.0f));
    _groupY = static_cast<int>(std::ceil(height / 32.0f));
    // initialize texture buffer
    glGenTextures(2, _tex);
    for(int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, _tex[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8, width, height);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    // prepare threshold shader
    _shader1 = std::make_shared<Shader>();
    _shader1->add("shaders/grayscale.comp.glsl", GL_COMPUTE_SHADER);
    _shader1->compile();
    _shader2 = std::make_shared<Shader>();
    _shader2->add("shaders/threshold.comp.glsl", GL_COMPUTE_SHADER);
    _shader2->compile();
}

Marker::~Marker()
{
    glDeleteTextures(1, _tex);
}

void Marker::process(GLuint sourceImg)
{
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    // step 1: convert rgb image to grayscale
    glUseProgram(_shader1->program());
    glBindImageTexture(0, sourceImg,    0, GL_FALSE, 0, GL_READ_ONLY,  GL_RGBA32F);
    glBindImageTexture(1, currentTex(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8);
    glDispatchCompute(
        static_cast<GLuint>(_groupX),
        static_cast<GLuint>(_groupY), 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    return;
    // step 2: convert grayscale to black-white
    if(_auto_threshold)
    {
        // choose a threshold

    }
    glUseProgram(_shader2->program());
    glBindImageTexture(0, currentTex(), 0, GL_FALSE, 0, GL_READ_ONLY,  GL_R8);
    glBindImageTexture(1, fetchTex(),   0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8);
    _shader2->uniformFloat("threshold", _threshold);
    glDispatchCompute(
        static_cast<GLuint>(_groupX),
        static_cast<GLuint>(_groupY), 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}
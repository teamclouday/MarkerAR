#include "marker.hpp"
#include <cmath>

Marker::Marker(int width, int height) : _width(width), _height(height)
{
    _auto_threshold_level = 1 + static_cast<int>(
        std::floor(std::log2(static_cast<double>(std::max(width, height))))
    );
    // initialize texture buffer
    glGenTextures(2, _tex);
    for(int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, _tex[i]);
        glTexStorage2D(GL_TEXTURE_2D, _auto_threshold_level, GL_R32F, width, height);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
    glDeleteTextures(2, _tex);
}

void Marker::process(GLuint sourceImg, int groupX, int groupY)
{
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    // step 1: convert rgb image to grayscale
    glUseProgram(_shader1->program());
    glBindImageTexture(0, sourceImg,    0, GL_FALSE, 0, GL_READ_ONLY,  GL_RGBA32F);
    glBindImageTexture(1, fetchTex(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
    _shader1->uniformInt("shades", _gray_shades);
    glDispatchCompute(
        static_cast<GLuint>(groupX),
        static_cast<GLuint>(groupY), 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    if(_debug_mode && _debug_level == 0) return;
    // step 2: convert grayscale to black-white
    if(_auto_threshold)
    {
        // set threshold as the average (top level of mipmap) value
        glGenerateTextureMipmap(lastTex());
        glGetTextureImage(lastTex(), _auto_threshold_level-1, GL_RED, GL_FLOAT, 4, &_threshold);
    }
    glUseProgram(_shader2->program());
    glBindImageTexture(0, lastTex(),  0, GL_FALSE, 0, GL_READ_ONLY,  GL_R32F);
    glBindImageTexture(1, fetchTex(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
    _shader2->uniformFloat("threshold", _threshold);
    glDispatchCompute(
        static_cast<GLuint>(groupX),
        static_cast<GLuint>(groupY), 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    if(_debug_mode && _debug_level == 1) return;
}
#include "marker.hpp"
#include <cmath>
#include <cstring>
#include <iostream>

Marker::Marker(int width, int height) : _width(width), _height(height)
{
    // config
    _auto_threshold_level = 1 + static_cast<int>(
        std::floor(std::log2(static_cast<double>(std::max(width, height))))
    );
    _boxesX = static_cast<int>(std::ceil(width / 160.0)); // 16 subgroup, each index manage 10x10 space
    _boxesY = static_cast<int>(std::ceil(height / 160.0));
    _boxes.resize(_boxesX * _boxesY);
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
    // initialize shader storage buffer
    glGenBuffers(1, &_boxesSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _boxesSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(BoxData) * _boxes.size(), _boxes.data(), GL_DYNAMIC_READ);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _boxesSSBO);
    glGenVertexArrays(1, &_boxesVAO);
    glBindVertexArray(_boxesVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_INT, GL_FALSE, 0, nullptr);
    glBindVertexArray(0);
    // prepare threshold shader
    _shader1 = std::make_shared<Shader>();
    _shader1->add("shaders/grayscale.comp.glsl", GL_COMPUTE_SHADER);
    _shader1->compile();
    _shader2 = std::make_shared<Shader>();
    _shader2->add("shaders/threshold.comp.glsl", GL_COMPUTE_SHADER);
    _shader2->compile();
    _shader3 = std::make_shared<Shader>();
    _shader3->add("shaders/contours.comp.glsl", GL_COMPUTE_SHADER);
    _shader3->compile();
    _shader4 = std::make_shared<Shader>();
    _shader4->add("shaders/boxes.comp.glsl", GL_COMPUTE_SHADER);
    _shader4->compile();
    _shaderDraw = std::make_shared<Shader>();
    _shaderDraw->add("shaders/corners.vert.glsl", GL_VERTEX_SHADER);
    _shaderDraw->add("shaders/corners.frag.glsl", GL_FRAGMENT_SHADER);
    _shaderDraw->compile();
}

Marker::~Marker()
{
    glDeleteTextures(2, _tex);
    glDeleteBuffers(1, &_boxesSSBO);
    glDeleteVertexArrays(1, &_boxesVAO);
}

void Marker::process(GLuint sourceImg, int groupX, int groupY)
{
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    // step 1: convert rgb image to grayscale
    glUseProgram(_shader1->program());
    glBindImageTexture(0, sourceImg,  0, GL_FALSE, 0, GL_READ_ONLY,  GL_RGBA32F);
    glBindImageTexture(1, fetchTex(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
    _shader1->uniformInt("shades", _gray_shades);
    _shader1->uniformBool("blurFilter", _blur);
    _shader1->uniformFloat("blurRadius", _blur_radius);
    _shader1->uniformFloat("blurQuality", _blur_quality);
    _shader1->uniformFloat("blurDirections", _blur_directions);
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
        glGetTextureImage(lastTex(), _auto_threshold_level-1, GL_RED, GL_FLOAT, sizeof(float), &_threshold);
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
    // step 3: detect contour edges
    glUseProgram(_shader3->program());
    glBindImageTexture(0, lastTex(),  0, GL_FALSE, 0, GL_READ_ONLY,  GL_R32F);
    glBindImageTexture(1, fetchTex(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
    glDispatchCompute(
        static_cast<GLuint>(groupX),
        static_cast<GLuint>(groupY), 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    if(_debug_mode && _debug_level == 2) return;
    // step 4: locate the square shape
    glUseProgram(_shader4->program());
    glBindImageTexture(0, lastTex(),  0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _boxesSSBO);
    glDispatchCompute(
        static_cast<GLuint>(_boxesX),
        static_cast<GLuint>(_boxesY), 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
    // update corners
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _boxesSSBO);
    void* bufferPtr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
    std::memcpy(_boxes.data(), bufferPtr, sizeof(BoxData) * _boxes.size());
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    for(size_t i = 0; i < _boxes.size(); i++)
    {
        auto& box = _boxes[i];
        if(box.p1.x >= 0)
        {
            std::cout << "Box " << i <<  " Found:\n" << box.p1.x << "," << box.p1.y << "\n" <<
                box.p2.x << "," << box.p2.y << "\n" <<
                box.p3.x << "," << box.p3.y << "\n" <<
                box.p4.x << "," << box.p4.y << "\n";
        }
    }
}

void Marker::drawCorners(float ratioCon, float ratioCam)
{
    if(_debug_mode && _debug_level == 3)
    {
        glLineWidth(2.0f);
        glUseProgram(_shaderDraw->program());
        _shaderDraw->uniformFloat("ratio_img", ratioCam);
        _shaderDraw->uniformFloat("ratio_win", ratioCon);
        _shaderDraw->uniformFloat("halfImgX", _width * 0.5f);
        _shaderDraw->uniformFloat("halfImgY", _height * 0.5f);
        glBindVertexArray(_boxesVAO);
        glBindBuffer(GL_ARRAY_BUFFER, _boxesSSBO);
        glDrawArrays(GL_LINE_LOOP, 0, _boxes.size() * 4);
        glBindVertexArray(0);
        glUseProgram(0);
    }
}
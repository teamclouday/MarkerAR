#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <random>
#include "shader.hpp"

struct BoxData
{
    glm::ivec2 p1 = glm::ivec2(-1);
    glm::ivec2 p2 = glm::ivec2(-1);
    glm::ivec2 p3 = glm::ivec2(-1);
    glm::ivec2 p4 = glm::ivec2(-1);
};

class RandomIntGenerator
{
public:
    RandomIntGenerator(int min, int max) : rng(rd()), dist(min, max) {}
    int next() {return dist(rng);}

private:
    std::random_device rd;
    std::mt19937 rng;
    std::uniform_int_distribution<int> dist;
};

class Marker
{
public:
    Marker(int width, int height);
    ~Marker();

    void process(GLuint sourceImg, int groupX, int groupY);
    void drawCorners(float ratioCon, float ratioCam);
    void estimatePoseSVD(
        const glm::mat3& cameraK, const glm::mat3& cameraInvK,
        const glm::vec3& cameraDistK, const glm::vec2& cameraDistP
    );
    void estimatePoseLinear(
        const glm::mat3& cameraK, const glm::mat3& cameraInvK,
        const glm::vec3& cameraDistK, const glm::vec2& cameraDistP
    );
    void estimatePoseOpenCV(
        const glm::mat3& cameraK, const glm::mat3& cameraInvK,
        const glm::vec3& cameraDistK, const glm::vec2& cameraDistP
    );

    // fetch texture and update index
    GLuint fetchTex()
    {
        GLuint tex = _tex[_currentTex];
        _currentTex = !_currentTex;
        return tex;
    }
    // only get current texture
    GLuint lastTex() {return _tex[!_currentTex];}
    bool debug() {return _debug_mode && _debug_level < 2;}
    glm::mat4x3 poseM() {return _poseMRefined;}

    void UI();
    void UIpose();

private:
    int _width, _height;
    GLuint _tex[2];
    int _currentTex = 0;
    std::shared_ptr<Shader> _shader1, _shader2, _shaderDraw;

    // variables for preprocessing image
    int _gray_shades = 1;
    bool _blur = false;
    float _blur_radius = 5.0f, _blur_quality = 6.0f, _blur_directions = 12.0f;
    float _threshold = 0.5f;
    bool _auto_threshold = false;
    int _auto_threshold_level = 0;
    // variables for closed contour detection
    std::vector<int8_t> _image_data;
    int _image_scan_step;
    glm::vec4 _marker_borderp1p2;
    glm::vec4 _marker_borderp3p4;
    bool _new_marker = false;
    int _tracing_max_iter = 5000;
    int _tracing_thres_contour = 200;
    float _tracing_thres_quadra = 6.0f;
    // variables for marker render
    GLuint _drawVAO, _drawVBO;
    int _marker_not_found = 0;
    // variable for pose estimation
    glm::mat4x3 _poseM;
    glm::mat4x3 _poseMRefined;
    float _err_reproj = 0.0f, _err_LM = 0.0f;

    int _debug_level = 0;
    bool _debug_mode = false;

    bool follow_contour(int x, int y);
    bool fit_quadrilateral(std::vector<glm::vec2>& track);
    void update_corners();
    void refinePoseM(
        const glm::mat3& cameraInvK,
        const std::vector<glm::vec2>& objPoints,
        const std::vector<glm::vec2>& imgPoints
    );
};
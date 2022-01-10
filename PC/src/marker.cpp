#include "marker.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>

Marker::Marker(int width, int height) : _width(width), _height(height),
    _marker_borderp1p2(0.0f), _marker_borderp3p4(0.0f)
{
    // config
    _auto_threshold_level = 1 + static_cast<int>(
        std::floor(std::log2(static_cast<double>(std::max(width, height))))
    );
    _image_data.resize(width * height);
    _image_scan_step = static_cast<int>(std::floor(height / 20.0f)); // assume that the marker is near camera, covering at least 1/20 screen height
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
    // prepare buffers
    glGenBuffers(1, &_drawVBO);
    glBindBuffer(GL_ARRAY_BUFFER, _drawVBO);
    glBufferStorage(GL_ARRAY_BUFFER, 8 * sizeof(float), nullptr, GL_MAP_WRITE_BIT);
    // glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glGenVertexArrays(1, &_drawVAO);
    glBindVertexArray(_drawVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glBindVertexArray(0);
    // prepare threshold shader
    _shader1 = std::make_shared<Shader>();
    _shader1->add("shaders/grayscale.comp.glsl", GL_COMPUTE_SHADER);
    _shader1->compile();
    _shader2 = std::make_shared<Shader>();
    _shader2->add("shaders/threshold.comp.glsl", GL_COMPUTE_SHADER);
    _shader2->compile();
    _shaderDraw = std::make_shared<Shader>();
    _shaderDraw->add("shaders/corners.vert.glsl", GL_VERTEX_SHADER);
    _shaderDraw->add("shaders/corners.frag.glsl", GL_FRAGMENT_SHADER);
    _shaderDraw->compile();
}

Marker::~Marker()
{
    glDeleteTextures(2, _tex);
    glDeleteBuffers(1, &_drawVBO);
    glDeleteVertexArrays(1, &_drawVAO);
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
    if(_debug_mode && _debug_level == 0) 
    {
        glGenerateTextureMipmap(lastTex());
        return;
    }
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
    if(_debug_mode && _debug_level == 1) 
    {
        glGenerateTextureMipmap(lastTex());
        return;
    }
    // step 3: contour tracking on CPU
    glGetTextureImage(lastTex(), 0, GL_RED, GL_BYTE, _image_data.size() * sizeof(int8_t), _image_data.data());
    bool markerFound = false;
    for(int y = 1; (y < _height - 1) && !markerFound; y+=_image_scan_step)
    {
        // image memory starts from bottom left
        for(int x = 2; (x < _width - 1) && !markerFound; x++)
        {
            // initial values: white -> 127, black -> -127, visited -> 0
            int8_t cNow = _image_data[x + y * _width];
            if(cNow >= 0) continue;
            int8_t cPrev = _image_data[x + y * _width - 1];
            if(cPrev > 0 && cNow < 0)
                markerFound = follow_contour(x, y);
        }
    }
    // step4: update VBO
    if(markerFound) 
    {
        update_corners();
        _marker_not_found = 0;
    }
    else if(_marker_borderp1p2.x >= 0.0f && _marker_not_found > 20)
    {
        _marker_borderp1p2 = glm::vec4(0.0f);
        _marker_borderp3p4 = glm::vec4(0.0f);
        _poseMRefined = glm::mat4x3(0.0f);
        update_corners();
    }
    else if(_marker_not_found < 30)
    {
        _marker_not_found++;
    }
}

void rotate_90(glm::ivec2& dir)
{
    // rotate 90 degrees clock wise
    if(dir.x == 0)
    {
        dir.x = dir.y;
        dir.y = 0;
    }
    else
    {
        dir.y = -dir.x;
        dir.x = 0;
    }
}

void rotate_neg90(glm::ivec2& dir)
{
    // rotate -90 degrees clock wise
    if(dir.x == 0)
    {
        dir.x = -dir.y;
        dir.y = 0;
    }
    else
    {
        dir.y = dir.x;
        dir.x = 0;
    }
}

// I implemented Theo Pavlidis' Algorithm
// http://www.imageprocessingplace.com/downloads_V3/root_downloads/tutorials/contour_tracing_Abeer_George_Ghuneim/theo.html
bool Marker::follow_contour(int x, int y)
{
    std::vector<glm::vec2> track;
    track.reserve(2000);
    glm::ivec2 pCurr = glm::ivec2(x, y);
    glm::ivec2 pStart = pCurr;
    // set initial forward to up
    glm::ivec2 dirForward = glm::ivec2(0, 1);
    glm::ivec2 dirRight = glm::ivec2(1, 0);
    glm::ivec2 p1, p2, p3;
    int rotationCounter = 0, iterCounter = 0;
    track.push_back(glm::vec2(pCurr));
    do
    {
        // if on border, return false
        if(pCurr.x <= 0 || pCurr.y <= 0 ||
            (pCurr.x >= _width - 1) || (pCurr.y >= _height - 1))
            return false;
        // prepare p1, p2, p3
        p2 = pCurr + dirForward;
        p1 = pCurr + dirForward - dirRight;
        p3 = pCurr + dirForward + dirRight;
        int8_t& p1Val = _image_data[p1.x + p1.y * _width];
        int8_t& p2Val = _image_data[p2.x + p2.y * _width];
        int8_t& p3Val = _image_data[p3.x + p3.y * _width];
        if(p1Val <= 0)
        {
            // if p1 is black
            track.push_back(glm::vec2(p1));
            pCurr = p1;
            rotate_neg90(dirForward);
            rotate_neg90(dirRight);
            p1Val = 0;
            rotationCounter = 0;
        }
        else if(p2Val <= 0)
        {
            // if p2 is black
            track.push_back(glm::vec2(p2));
            pCurr = p2;
            p2Val = 0;
            rotationCounter = 0;
        }
        else if(p3Val <= 0)
        {
            // if p3 is black
            track.push_back(glm::vec2(p3));
            pCurr = p3;
            p3Val = 0;
            rotationCounter = 0;
        }
        else if(rotationCounter >= 3)
        {
            // if already rotated 3 times
            return false;
        }
        else
        {
            // rotate 90 degrees
            rotate_90(dirForward);
            rotate_90(dirRight);
            rotationCounter++;
        }
        iterCounter++;
    }
    while (pCurr != pStart && iterCounter < _tracing_max_iter);
    if(iterCounter >= _tracing_max_iter) return false;
    // check contour border size
    if(static_cast<int>(track.size()) < _tracing_thres_contour) return false;
    // try to fit a quadrilateral
    return fit_quadrilateral(track);
}

// L2 distance from point to point
float distance_p2p(glm::vec2& p1, glm::vec2& p2)
{
    return static_cast<float>(std::sqrt(
        static_cast<double>((p1.x - p2.x) * (p1.x - p2.x)) +
        static_cast<double>((p1.y - p2.y) * (p1.y - p2.y))
    ));
}

// L2 distance from point to line
// https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line
float distance_p2l(glm::vec2& p, glm::vec2& lp, const glm::vec3& ldata)
{
    // lp.x = x1    lp.y = y1
    // ldata.x = (x2-x1)
    // ldata.y = (y2-y1)
    // ldata.z = 1 / (dist(p1, p2))
    return static_cast<float>(
        (ldata.x * (lp.y - p.y) - (lp.x - p.x) * ldata.y) *
        ldata.z
    );
}

// validate cosine angle between two vectors, should not be too small
bool validate_angle(const glm::vec2& v1, const glm::vec2& v2)
{
    // angle in cosine
    float angleCos = glm::dot(glm::normalize(v1), glm::normalize(v2));
    return angleCos <= 0.94f && angleCos >= -0.94f;
}

// a simple algorithm from book: "Augmented Reality: Principles and Practice"
// Chapter 4 Marker Detection
// runtime: 3.5 loops, O(N)
bool Marker::fit_quadrilateral(std::vector<glm::vec2>& track)
{
    // step 1: find farthest point as first corner p1
    // and get the centroid at the same time
    int trackSize = static_cast<int>(track.size());
    int p1 = 0, p2 = 0, p3 = 0, p4 = 0;
    float dist1 = 0.0f, dist2 = 0.0f;
    glm::vec2 centeroid;
    for(int i = 1; i < static_cast<int>(track.size()); i++)
    {
        float newDist = distance_p2p(track[0], track[i]);
        if(newDist > dist1)
        {
            dist1 = newDist;
            p1 = i;
        }
        centeroid += track[i];
    }
    centeroid /= static_cast<float>(track.size() - 1);
    // step 2: start from p1, find p2 (max pos dist), p3 (max neg dist)
    glm::vec3 lineData = glm::vec3(
        track[p1].x - centeroid.x,
        track[p1].y - centeroid.y,
        1.0f / distance_p2p(centeroid, track[p1])
    );
    dist1 = 0.0f;
    int pCurr = p1;
    do
    {
        float newDist = distance_p2l(track[pCurr], centeroid, lineData);
        if(newDist > 0.0f)
        {
            // on right side of vector, p2
            if(newDist > dist1)
            {
                dist1 = newDist;
                p2 = pCurr;
            }
        }
        else
        {
            // on left side of vector, p3
            if(newDist < dist2)
            {
                dist2 = newDist;
                p3 = pCurr;
            }
        }
        pCurr++;
        if(pCurr >= trackSize) pCurr = 0;
    }
    while (pCurr != p1);
    // step 3: given p2 and p3, find p4
    lineData = glm::vec3(
        track[p2].x - track[p3].x,
        track[p2].y - track[p3].y,
        1.0f / distance_p2p(track[p2], track[p3])
    );
    dist1 = 0.0f;
    pCurr = p2;
    do
    {
        float newDist = distance_p2l(track[pCurr], track[p3], lineData);
        if(newDist > dist1)
        {
            dist1 = newDist;
            p4 = pCurr;
        }
        pCurr++;
        if(pCurr >= trackSize) pCurr = 0;
    }
    while (pCurr != p3);
    // validate angle p1p2, p1p3, p4p2, p4p3
    if(!validate_angle(track[p2]-track[p1], track[p3]-track[p1])) return false;
    if(!validate_angle(track[p2]-track[p4], track[p3]-track[p4])) return false;
    // step 4: check p1-p2, p2-p4, p4-p3, p1-p3
    // this is an important step to make sure that a quadrilateral fit exists
    lineData = glm::vec3(
        track[p2].x - track[p1].x,
        track[p2].y - track[p1].y,
        1.0f / distance_p2p(track[p1], track[p2])
    );
    pCurr = p1;
    do
    {
        if(
            std::abs(distance_p2l(track[pCurr], track[p1], lineData))
            >= _tracing_thres_quadra
        ) return false;
        pCurr++;
        if(pCurr >= trackSize) pCurr = 0;
    }
    while (pCurr != p2);

    lineData = glm::vec3(
        track[p4].x - track[p2].x,
        track[p4].y - track[p2].y,
        1.0f / distance_p2p(track[p2], track[p4])
    );
    pCurr = p2;
    do
    {
        if(
            std::abs(distance_p2l(track[pCurr], track[p2], lineData))
            >= _tracing_thres_quadra
        ) return false;
        pCurr++;
        if(pCurr >= trackSize) pCurr = 0;
    }
    while (pCurr != p4);

    lineData = glm::vec3(
        track[p3].x - track[p4].x,
        track[p3].y - track[p4].y,
        1.0f / distance_p2p(track[p3], track[p4])
    );
    pCurr = p4;
    do
    {
        if(
            std::abs(distance_p2l(track[pCurr], track[p4], lineData))
            >= _tracing_thres_quadra
        ) return false;
        pCurr++;
        if(pCurr >= trackSize) pCurr = 0;
    }
    while (pCurr != p3);

    lineData = glm::vec3(
        track[p1].x - track[p3].x,
        track[p1].y - track[p3].y,
        1.0f / distance_p2p(track[p1], track[p3])
    );
    pCurr = p3;
    do
    {
        if(
            std::abs(distance_p2l(track[pCurr], track[p3], lineData))
            >= _tracing_thres_quadra
        ) return false;
        pCurr++;
        if(pCurr >= trackSize) pCurr = 0;
    }
    while (pCurr != p1);
    // step 5: determine orientation of the marker
    // p1 near the black area
    glm::ivec2 sampleP2 = glm::ivec2(glm::round((track[p2] + centeroid) * 0.5f));
    glm::ivec2 sampleP3 = glm::ivec2(glm::round((track[p3] + centeroid) * 0.5f));
    glm::ivec2 sampleP4 = glm::ivec2(glm::round((track[p4] + centeroid) * 0.5f));
    if(_image_data[sampleP2.x + sampleP2.y * _width] <= 0)
    {
        // if P2 is near black area
        int tmp = p1;
        p1 = p2;
        p2 = p4;
        p4 = p3;
        p3 = tmp;
    }
    else if(_image_data[sampleP3.x + sampleP3.y * _width] <= 0)
    {
        // if P3 is near black area
        int tmp = p1;
        p1 = p3;
        p3 = p4;
        p4 = p2;
        p2 = tmp;
    }
    else if(_image_data[sampleP4.x + sampleP4.y * _width] <= 0)
    {
        // if P4 is near black area
        int tmp = p1;
        p1 = p4;
        p4 = tmp;
        tmp = p3;
        p3 = p2;
        p2 = tmp;
    }
    // step 6: update and store data
    _marker_borderp1p2.x = track[p1].x;
    _marker_borderp1p2.y = track[p1].y;
    _marker_borderp1p2.z = track[p2].x;
    _marker_borderp1p2.w = track[p2].y;

    _marker_borderp3p4.x = track[p3].x;
    _marker_borderp3p4.y = track[p3].y;
    _marker_borderp3p4.z = track[p4].x;
    _marker_borderp3p4.w = track[p4].y;

    // _marker_borderp1p2.x = track[p1].x * 2.0f - _width;
    // _marker_borderp1p2.y = track[p1].y * 2.0f - _height;
    // _marker_borderp1p2.z = track[p2].x * 2.0f - _width;
    // _marker_borderp1p2.w = track[p2].y * 2.0f - _height;

    // _marker_borderp3p4.x = track[p3].x * 2.0f - _width;
    // _marker_borderp3p4.y = track[p3].y * 2.0f - _height;
    // _marker_borderp3p4.z = track[p4].x * 2.0f - _width;
    // _marker_borderp3p4.w = track[p4].y * 2.0f - _height;
    return true;
}

void Marker::update_corners()
{
    glBindBuffer(GL_ARRAY_BUFFER, _drawVBO);
    float* ptr = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    ptr[0] = _marker_borderp1p2.x;
    ptr[1] = _marker_borderp1p2.y;
    ptr[4] = _marker_borderp1p2.z;
    ptr[5] = _marker_borderp1p2.w;
    
    ptr[2] = _marker_borderp3p4.x;
    ptr[3] = _marker_borderp3p4.y;
    ptr[6] = _marker_borderp3p4.z;
    ptr[7] = _marker_borderp3p4.w;
    glUnmapBuffer(GL_ARRAY_BUFFER);
}

void Marker::drawCorners(float ratioCon, float ratioCam)
{
    if(_debug_mode && _debug_level == 2)
    {
        glUseProgram(_shaderDraw->program());
        _shaderDraw->uniformFloat("ratio_img", ratioCam);
        _shaderDraw->uniformFloat("ratio_win", ratioCon);
        _shaderDraw->uniformFloat("cam_width", _width);
        _shaderDraw->uniformFloat("cam_height", _height);
        glBindVertexArray(_drawVAO);
        glBindBuffer(GL_ARRAY_BUFFER, _drawVBO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
        glUseProgram(0);
    }
}
#pragma once

#include <opencv2/opencv.hpp>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <stdexcept>
#include <memory>
#include <cmath>
#include <string>
#include "shader.hpp"

class Camera
{
public:
    Camera()
    {
        // try to open camera
        // first try directshow for Windows
        // next try V4L2 for Linux
        // finally autodetect
        if(!_cam.open(0, cv::CAP_DSHOW) &&
           !_cam.open(0, cv::CAP_V4L2) &&
           !_cam.open(0, cv::CAP_ANY))
            throw std::runtime_error("Failed to open camera!");
        _backend = _cam.getBackendName();
        // set properties
        _cam.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
        _cam.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
        _width = static_cast<int>(_cam.get(cv::CAP_PROP_FRAME_WIDTH));
        _height = static_cast<int>(_cam.get(cv::CAP_PROP_FRAME_HEIGHT));
        _ratio = static_cast<float>(_width) / _height;
        _groupX = static_cast<int>(std::ceil(_width / 32.0f));
        _groupY = static_cast<int>(std::ceil(_height / 32.0f));
        // init opengl texture
        glGenTextures(2, _tex);
        for(int i = 0; i < 2; i++)
        {
            glBindTexture(GL_TEXTURE_2D, _tex[i]);
            glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, _width, _height);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        // try to get first frame
        if(!update())
            throw std::runtime_error("Failed to get frame from camera!");
        // init VAO
        const float vertices[] = {
            -1.0f, -1.0f,
             1.0f, -1.0f,
             1.0f,  1.0f,
            -1.0f,  1.0f,
        };
        GLuint vbo;
        glGenVertexArrays(1, &_vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(_vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(0));
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
        // prepare denoise shader
        _denoise_shader = std::make_shared<Shader>();
        _denoise_shader->add("shaders/denoise.comp.glsl", GL_COMPUTE_SHADER);
        _denoise_shader->compile();
        // set camera calibration matrix
        // TODO: support loading camera data from a local file (json)
        _camK = glm::mat3(
            glm::vec3(776.39107688f, 0.0f,          0.0f),
            glm::vec3(0.0f,          770.19292802f, 0.0f),
            glm::vec3(346.33206594f, 315.46822203f, 1.0f)
        );
        // TODO: make this a parameter
        const float cameraCalibWidth = 800.0f, cameraCalibHeight = 600.0f;
        _camK[0][0] *= _width / cameraCalibWidth;
        _camK[2][0] *= _width / cameraCalibWidth;
        _camK[1][1] *= _height / cameraCalibHeight;
        _camK[2][1] *= _height / cameraCalibHeight;
        _camInvK = glm::inverse(_camK);
        // set camera distortion data (k1, k2, p1, p2, k3)
        // https://docs.opencv.org/3.4/d4/d94/tutorial_camera_calibration.html
        _camDistCoeffK = glm::vec3(-3.58177671e-1,  5.58704262e-1, -8.93211088e-1);
        _camDistCoeffP = glm::vec2(8.46171348e-4, -4.12405134e-3);
    }

    ~Camera()
    {
        glDeleteTextures(2, _tex);
        glDeleteVertexArrays(1, &_vao);
    }

    bool update()
    {
        bool updated = _cam.read(_frame);
        // if updated, update texture pixels
        if(updated)
        {
            cv::flip(_frame, _frame, 0);
            // cv::fastNlMeansDenoisingColored(_frame, _frame);
            glBindTexture(GL_TEXTURE_2D, fetchTex());
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _width, _height, GL_BGR, GL_UNSIGNED_BYTE, _frame.data);
            glBindTexture(GL_TEXTURE_2D, 0);
            if(_denoise) denoise();
        }
        return updated;
    }

    void draw()
    {
        glBindVertexArray(_vao);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindVertexArray(0);
    }

    void denoise()
    {
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glUseProgram(_denoise_shader->program());
        glBindImageTexture(0, lastTex(),  0, GL_FALSE, 0, GL_READ_ONLY,  GL_RGBA32F);
        glBindImageTexture(1, fetchTex(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        _denoise_shader->uniformFloat("coeff_sigma", _sigma);
        _denoise_shader->uniformFloat("coeff_kSigma", _kSigma);
        _denoise_shader->uniformFloat("coeff_threshold", _threshold);
        glDispatchCompute(
            static_cast<GLuint>(_groupX),
            static_cast<GLuint>(_groupY), 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glUseProgram(0);
    }

    // fetch texture and update index
    GLuint fetchTex()
    {
        GLuint tex = _tex[_currentTex];
        _currentTex = !_currentTex;
        return tex;
    }
    // only get current texture
    GLuint lastTex() {return _tex[!_currentTex];}
    GLuint vao() const {return _vao;}
    int width() const {return _width;}
    int height() const {return _height;}
    float ratio() const {return _ratio;}
    int groupX() const {return _groupX;}
    int groupY() const {return _groupY;}
    glm::mat3 cameraK() const {return _camK;}
    glm::mat3 cameraInvK() const {return _camInvK;}
    glm::vec3 cameraDistK() const {return _camDistCoeffK;}
    glm::vec2 cameraDistP() const {return _camDistCoeffP;}

    void UI();

private:
    int _width, _height, _groupX, _groupY;
    float _ratio;
    cv::VideoCapture _cam;
    cv::Mat _frame;
    GLuint _tex[2], _vao;
    int _currentTex = 0;
    std::shared_ptr<Shader> _denoise_shader;
    bool _denoise = false;
    float _sigma = 3.0f, _kSigma = 6.0f, _threshold = 0.1f;
    std::string _backend;
    glm::mat3 _camK;
    glm::mat3 _camInvK;
    glm::vec3 _camDistCoeffK;
    glm::vec2 _camDistCoeffP;
};
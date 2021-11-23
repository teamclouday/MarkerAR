#pragma once

#include <opencv2/opencv.hpp>
#include <GL/glew.h>
#include <stdexcept>

class Camera
{
public:
    Camera()
    {
        // open default camera
        if(!_cam.open(0, cv::CAP_ANY) || !_cam.isOpened())
            throw std::runtime_error("Failed to open camera!");
        // set properties
        _cam.set(cv::CAP_PROP_BUFFERSIZE, 1);
        _cam.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
        _width = static_cast<int>(_cam.get(cv::CAP_PROP_FRAME_WIDTH));
        _height = static_cast<int>(_cam.get(cv::CAP_PROP_FRAME_HEIGHT));
        _ratio = static_cast<float>(_width) / _height;
        // init opengl texture
        glGenTextures(1, &_tex);
        glBindTexture(GL_TEXTURE_2D, _tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _width, _height, 0, GL_BGR, GL_UNSIGNED_BYTE, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);
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
    }

    ~Camera()
    {
        glDeleteTextures(1, &_tex);
        glDeleteVertexArrays(1, &_vao);
    }

    bool update()
    {
        bool updated = _cam.read(_frame);
        // if updated, update texture pixels
        if(updated)
        {
            cv::flip(_frame, _frame, 0);
            glBindTexture(GL_TEXTURE_2D, _tex);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _width, _height, GL_BGR, GL_UNSIGNED_BYTE, _frame.data);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        return updated;
    }

    void draw()
    {
        glBindVertexArray(_vao);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindVertexArray(0);
    }

    GLuint texture() const {return _tex;}
    GLuint vao() const {return _vao;}
    int width() const {return _width;}
    int height() const {return _height;}
    float ratio() const {return _ratio;}

    void UI();

private:
    int _width, _height;
    float _ratio;
    cv::VideoCapture _cam;
    cv::Mat _frame;
    GLuint _tex, _vao;
};
#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <functional>

#define CONTEXT_FPS 60
#define CONTEXT_SPF 0.01666666

class Context
{
public:
    Context(const std::string& title);
    ~Context();

    bool loop();
    void beginFrame();
    void endFrame(std::function<void()> customUI = nullptr);
    float ratio() const {return _ratio;}
    int width() const {return _winWidth;}
    int height() const {return _winHeight;}

    void UI();

private:
    int _winWidth, _winHeight;
    float _ratio;
    std::string _title;
    GLFWwindow* _window;
    bool _displayUI = true;
    double _timer = 0.0;

    static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        Context* user = reinterpret_cast<Context*>(glfwGetWindowUserPointer(window));
        if(action == GLFW_PRESS)
        {
            switch(key)
            {
                case GLFW_KEY_ESCAPE:
                    glfwSetWindowShouldClose(window, GLFW_TRUE);
                    break;
                case GLFW_KEY_F12:
                    user->_displayUI = !user->_displayUI;
                    break;
            }
        }
    }
};
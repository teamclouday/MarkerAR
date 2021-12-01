#include "context.hpp"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <iostream>

void GLAPIENTRY GLDebugCallback(
    GLenum source, GLenum type, GLuint id, GLenum severity,
    GLsizei length, const GLchar* message, const void* userParam
)
{
    std::cout << "**** GL Callback ****" << std::endl;
    std::cout << "(";
    switch(severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:            std::cout << "High"; break;
        case GL_DEBUG_SEVERITY_LOW:             std::cout << "Low"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:          std::cout << "Med"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:    std::cout << "Noti"; break;
    }
    std::cout << ") ";
    std::cout << "<";
    switch(source)
    {
        case GL_DEBUG_SOURCE_API:               std::cout << "API"; break;
        case GL_DEBUG_SOURCE_APPLICATION:       std::cout << "Application"; break;
        case GL_DEBUG_SOURCE_OTHER:             std::cout << "Other"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:   std::cout << "Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:       std::cout << "Third Party"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:     std::cout << "Window System"; break;
    }
    std::cout << "> ";
    std::cout << "[";
    switch(type)
    {
        case GL_DEBUG_TYPE_ERROR:               std::cout << "Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Undefined Behaviour"; break; 
        case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              std::cout << "Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               std::cout << "Other"; break;
    }
    std::cout << "] " << std::endl;
    std::cout << message << std::endl;
}



Context::Context(const std::string& title)
{
    // init window
    _winWidth = 800;
    _winHeight = 600;
    _ratio = 800.0f / 600.0f;
    _title = title;
    if(!glfwInit())
        throw std::runtime_error("Failed to init GLFW!");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    _window = glfwCreateWindow(_winWidth, _winHeight, _title.c_str(), nullptr, nullptr);
    if(!_window)
        throw std::runtime_error("Failed to create GLFW window!");
    glfwSetWindowUserPointer(_window, this);
    glfwMakeContextCurrent(_window);
    glfwSetKeyCallback(_window, glfw_key_callback);
    glfwSwapInterval(1);
    // init opengl context
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK)
        throw std::runtime_error("Failed to init GLEW!");
    glEnable(GL_TEXTURE_2D);
    // glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glDebugMessageCallback(GLDebugCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
    // init imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(_window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
}

Context::~Context()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(_window);
    glfwTerminate();
}

bool Context::loop()
{
    return !glfwWindowShouldClose(_window);
}

void Context::beginFrame()
{
    glfwPollEvents();
    glfwGetFramebufferSize(_window, &_winWidth, &_winHeight);
    _ratio = static_cast<float>(_winWidth) / _winHeight;
    glViewport(0, 0, _winWidth, _winHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    _timer = glfwGetTime();
}

void Context::endFrame(std::function<void()> customUI)
{
    if(_displayUI && customUI)
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        customUI();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
    glfwSwapBuffers(_window);
    // fps control
    double elapsed = glfwGetTime() - _timer;
    if(elapsed < CONTEXT_SPF)
        std::this_thread::sleep_for(std::chrono::duration<double>(CONTEXT_SPF - elapsed));
}

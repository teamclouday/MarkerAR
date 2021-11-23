#include "context.hpp"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <stdexcept>
#include <thread>
#include <chrono>

Context::Context(const std::string& title)
{
    // init window
    _winWidth = 800;
    _winHeight = 600;
    _ratio = 800.0f / 600.0f;
    _title = title;
    if(!glfwInit())
        throw std::exception("Failed to init GLFW!");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    _window = glfwCreateWindow(_winWidth, _winHeight, _title.c_str(), nullptr, nullptr);
    if(!_window)
        throw std::exception("Failed to create GLFW window!");
    glfwSetWindowUserPointer(_window, this);
    glfwMakeContextCurrent(_window);
    glfwSetKeyCallback(_window, glfw_key_callback);
    glfwSwapInterval(1);
    // init opengl context
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK)
        throw std::exception("Failed to init GLEW!");
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
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
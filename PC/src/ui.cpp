#include "camera.hpp"
#include "context.hpp"
#include <imgui.h>

void Camera::UI()
{
    ImGui::Text("Capture Size: %dx%d", _width, _height);
    ImGui::Text("Capture FPS: %.2f", _fps);
}

void Context::UI()
{
    ImGui::Text("Window Size: %dx%d", _winWidth, _winHeight);
    ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);
    ImGui::Text("Author: Teamclouday");
}
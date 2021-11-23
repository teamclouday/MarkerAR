#include "camera.hpp"
#include "context.hpp"
#include "marker.hpp"
#include <imgui.h>

void Camera::UI()
{
    ImGui::Text("Capture Size: %dx%d", _width, _height);
}

void Context::UI()
{
    ImGui::Text("Window Size: %dx%d", _winWidth, _winHeight);
    ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);
    ImGui::Text("Author: Teamclouday");
}

void Marker::UI()
{
    ImGui::Text("Grayscale");
    ImGui::DragInt("Shades", &_gray_shades, 1.0f, 1, 50);
    ImGui::Separator();
    ImGui::Text("Thresholding");
    ImGui::Checkbox("Auto", &_auto_threshold);
    if(!_auto_threshold)
        ImGui::DragFloat("Manual", &_threshold, 0.001f, 0.0f, 1.0f, "%.3f");
    ImGui::Separator();
}
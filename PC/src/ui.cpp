#include "camera.hpp"
#include "context.hpp"
#include "marker.hpp"
#include <imgui.h>

void Camera::UI()
{
    ImGui::Text("Capture Size: %dx%d", _width, _height);
    ImGui::Checkbox("Denoising", &_denoise);
    if(_denoise)
    {
        ImGui::DragFloat("Denoise Threshold", &_threshold, 0.001f, 0.001f, 0.5f, "%.3f");
        ImGui::DragFloat("Denoise Sigma", &_sigma, 0.01f, 0.001f, 5.0f, "%.2f");
        ImGui::DragFloat("Denoise kSigma", &_kSigma, 0.01f, 0.001f, 10.0f, "%.2f");
    }
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
    ImGui::Checkbox("Blur Filter", &_blur);
    if(_blur)
    {
        ImGui::DragFloat("Blur Radius", &_blur_radius, 0.01f, 0.01f, 10.0f, "%.2f");
        ImGui::DragFloat("Blur Quality", &_blur_quality, 0.01f, 0.01f, 10.0f, "%.2f");
        ImGui::DragFloat("Blur Directions", &_blur_directions, 1.0f, 1.0f, 12.0f, "%.0f");
    }
    ImGui::Separator();
    ImGui::Text("Thresholding");
    ImGui::Checkbox("Auto", &_auto_threshold);
    if(!_auto_threshold)
        ImGui::DragFloat("Manual", &_threshold, 0.001f, 0.0f, 1.0f, "%.3f");
    ImGui::Separator();
    ImGui::Text("Contour Tracing");
    ImGui::DragInt("Max Iteration", &_tracing_max_iter, 5.0f, 200, 10000);
    ImGui::DragInt("Min Contour Length", &_tracing_thres_contour, 5.0f, 10, 5000);
    ImGui::DragFloat("Min Quadra Distance", &_tracing_thres_quadra, 0.01f, 0.01f, 20.0f, "%.2f");
    ImGui::Text("p1 = (%.3f, %.3f)", _marker_borderp1p2.x, _marker_borderp1p2.y);
    ImGui::Text("p2 = (%.3f, %.3f)", _marker_borderp1p2.z, _marker_borderp1p2.w);
    ImGui::Text("p3 = (%.3f, %.3f)", _marker_borderp3p4.x, _marker_borderp3p4.y);
    ImGui::Text("p4 = (%.3f, %.3f)", _marker_borderp3p4.z, _marker_borderp3p4.w);
    ImGui::Separator();
    ImGui::Checkbox("Debug Mode", &_debug_mode);
    if(_debug_mode)
    {
        ImGui::RadioButton("Grayscale", &_debug_level, 0);
        ImGui::RadioButton("Thresholding", &_debug_level, 1);
        ImGui::RadioButton("Contour Edge", &_debug_level, 2);
    }
}
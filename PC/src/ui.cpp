#include "camera.hpp"
#include "context.hpp"
#include "marker.hpp"
#include "model.hpp"
#include <imgui.h>

void Camera::UI()
{
    ImGui::Text("OpenCV Backend: %s", _backend.c_str());
    ImGui::Text("Capture Size: %dx%d", _width, _height);
    ImGui::Separator();
    ImGui::Checkbox("Denoising", &_denoise);
    if(_denoise)
    {
        ImGui::DragFloat("Denoise Threshold", &_threshold, 0.001f, 0.001f, 0.5f, "%.3f");
        ImGui::DragFloat("Denoise Sigma", &_sigma, 0.01f, 0.001f, 5.0f, "%.2f");
        ImGui::DragFloat("Denoise kSigma", &_kSigma, 0.01f, 0.001f, 10.0f, "%.2f");
    }
    ImGui::Separator();
    ImGui::Text("Camera Calibration");
    if(ImGui::DragFloat("focal fx", &_camK[0][0], 0.01f, 0.01f, 10000.0f, "%.6f")) _camInvK = glm::inverse(_camK);
    if(ImGui::DragFloat("focal fy", &_camK[1][1], 0.01f, 0.01f, 10000.0f, "%.6f")) _camInvK = glm::inverse(_camK);
    if(ImGui::DragFloat("principle cx", &_camK[2][0], 0.01f, -10000.0f, 10000.0f, "%.6f")) _camInvK = glm::inverse(_camK);
    if(ImGui::DragFloat("principle cy", &_camK[2][1], 0.01f, -10000.0f, 10000.0f, "%.6f")) _camInvK = glm::inverse(_camK);
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
    ImGui::Separator();
    ImGui::Text("p1 = (%.0f, %.0f)", _marker_borderp1p2.x, _marker_borderp1p2.y);
    ImGui::Text("p2 = (%.0f, %.0f)", _marker_borderp1p2.z, _marker_borderp1p2.w);
    ImGui::Text("p3 = (%.0f, %.0f)", _marker_borderp3p4.x, _marker_borderp3p4.y);
    ImGui::Text("p4 = (%.0f, %.0f)", _marker_borderp3p4.z, _marker_borderp3p4.w);
    ImGui::Separator();
    ImGui::Checkbox("Debug Mode", &_debug_mode);
    if(_debug_mode)
    {
        ImGui::RadioButton("Grayscale", &_debug_level, 0);
        ImGui::RadioButton("Thresholding", &_debug_level, 1);
        ImGui::RadioButton("Contour Edge", &_debug_level, 2);
    }
}

void Marker::UIpose()
{
    ImGui::Text("Estimated M");
    ImGui::Text("%.2f, %.2f, %.2f, %.2f\n%.2f, %.2f, %.2f, %.2f\n%.2f, %.2f, %.2f, %.2f",
        _poseM[0][0], _poseM[1][0], _poseM[2][0], _poseM[3][0],
        _poseM[0][1], _poseM[1][1], _poseM[2][1], _poseM[3][1],
        _poseM[0][2], _poseM[1][2], _poseM[2][2], _poseM[3][2]);
    ImGui::Separator();
    ImGui::Text("Reprojection Error: %.3f", _err_reproj);
    ImGui::Text("Levenberg Error: %.3f", _err_LM);
}

void ModelCube::UI()
{
    ImGui::Text("Cube");
    ImGui::Separator();
    ImGui::Checkbox("Linestyle", &_lineMode);
    ImGui::Separator();
    ImGui::DragFloat3("Translation", &_translation[0], 0.01f, -10.0f, 10.0f, "%.2f");
    ImGui::DragFloat("Scale", &_scale, 0.01f, 0.01f, 10.0f, "%.2f");
}

void ModelTeapot::UI()
{
    ImGui::Text("Teapot");
    ImGui::Separator();
    ImGui::Checkbox("Linestyle", &_lineMode);
    ImGui::Separator();
    ImGui::DragFloat3("Translation", &_translation[0], 0.01f, -10.0f, 10.0f, "%.2f");
    ImGui::DragFloat("Scale", &_scale, 0.001f, 0.001f, 10.0f, "%.3f");
    ImGui::Separator();
    ImGui::DragFloat3("Light Pos", &_lightPos[0], 0.01f);
    ImGui::ColorEdit3("Diffuse", &_model_diffuse[0]);
}

void ModelBunny::UI()
{
    ImGui::Text("Bunny");
    ImGui::Separator();
    ImGui::Checkbox("Linestyle", &_lineMode);
    ImGui::Separator();
    ImGui::DragFloat3("Translation", &_translation[0], 0.01f, -10.0f, 10.0f, "%.2f");
    ImGui::DragFloat("Scale", &_scale, 0.001f, 0.001f, 100.0f, "%.3f");
    ImGui::Separator();
    ImGui::DragFloat3("Light Pos", &_lightPos[0], 0.01f);
    ImGui::ColorEdit3("Diffuse", &_model_diffuse[0]);
}

void ModelTyra::UI()
{
    ImGui::Text("Tyra");
    ImGui::Separator();
    ImGui::Checkbox("Linestyle", &_lineMode);
    ImGui::Separator();
    ImGui::DragFloat3("Translation", &_translation[0], 0.01f, -10.0f, 10.0f, "%.2f");
    ImGui::DragFloat("Scale", &_scale, 0.001f, 0.001f, 100.0f, "%.3f");
    ImGui::Separator();
    ImGui::DragFloat3("Light Pos", &_lightPos[0], 0.01f);
    ImGui::ColorEdit3("Diffuse", &_model_diffuse[0]);
}

void ModelArmadillo::UI()
{
    ImGui::Text("Armadillo");
    ImGui::Separator();
    ImGui::Checkbox("Linestyle", &_lineMode);
    ImGui::Separator();
    ImGui::DragFloat3("Translation", &_translation[0], 0.01f, -10.0f, 10.0f, "%.2f");
    ImGui::DragFloat("Scale", &_scale, 0.001f, 0.001f, 100.0f, "%.3f");
    ImGui::Separator();
    ImGui::DragFloat3("Light Pos", &_lightPos[0], 0.01f);
    ImGui::ColorEdit3("Diffuse", &_model_diffuse[0]);
}
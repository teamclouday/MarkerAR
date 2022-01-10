#version 450 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inColor;

layout (location = 0) out vec3 vertColor;

uniform float ratio_img;
uniform float ratio_win;
uniform float cam_width;
uniform float cam_height;
uniform mat4x3 poseM;
uniform mat4 modelMat;
uniform mat3 cameraK;

void main()
{
    vertColor = inColor;
    // change Z up coordinate to Y up
    vec3 modelPos = vec3(inPos.x, inPos.z, -inPos.y);
    vec3 screenPos = cameraK * poseM * modelMat * vec4(modelPos, 1.0);
    // vec2 pos = screenPos.xy;
    vec2 pos = screenPos.xy / vec2(cam_width, cam_height) * 2.0 - 1.0;
    if(ratio_img > ratio_win)
    {
        pos.y = pos.y * ratio_win / ratio_img;
    }
    else
    {
        pos.x = pos.x * ratio_img / ratio_win;
    }
    pos.y *= ratio_img;
    gl_Position = vec4(pos, screenPos.z * 0.5, 1.0);
}
#version 450 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inColor;

layout (location = 0) out vec3 vertColor;

uniform float ratio_img;
uniform float ratio_win;
uniform mat4x3 poseM;
uniform mat4 modelMat;
uniform mat3 cameraK;
uniform mat4 cameraProj;

void main()
{
    vertColor = inColor;
    vec4 pos = modelMat * vec4(inPos, 1.0);
    pos.yz = vec2(pos.z, -pos.y);
    vec3 screenPos = cameraK * poseM * pos;
    // refer to https://nghiaho.com/?p=2559
    pos = cameraProj * vec4(screenPos, 1.0);
    if(ratio_img > ratio_win)
    {
        pos.y = pos.y * ratio_win / ratio_img;
    }
    else
    {
        pos.x = pos.x * ratio_img / ratio_win;
    }
    gl_Position = vec4(pos.xy, screenPos.z, 1.0);
}
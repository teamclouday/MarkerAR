#version 450 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inColor;

layout (location = 0) out vec3 vertColor;

uniform float ratio_img;
uniform float ratio_win;
uniform mat4x3 poseM;
uniform mat4 modelMat;

void main()
{
    vec3 modelPos = vec3(inPos.x, inPos.z, -inPos.y);
    vertColor = inColor;
    vec3 screenPos = poseM * modelMat * vec4(modelPos, 1.0);

    // vec3 screenPos = poseM[3] + inPos;
    vec2 pos = screenPos.xy;
    if(ratio_img > ratio_win)
    {
        pos.y = pos.y * ratio_win / ratio_img;
    }
    else
    {
        pos.x = pos.x * ratio_img / ratio_win;
    }
    pos.y *= ratio_img;
    gl_Position = vec4(pos, screenPos.z, 1.0);
}
#version 450 core

layout (location = 0) in vec3 inPos;
layout (location = 0) out vec3 worldPos;

uniform float ratio_img;
uniform float ratio_win;
uniform float cam_width;
uniform float cam_height;
uniform mat4x3 poseM;
uniform mat4 modelMat;
uniform mat3 cameraK;

void main()
{
    // change Z up coordinate to Y up
    vec4 modelPos = modelMat * vec4(inPos, 1.0);
    worldPos = modelPos.xyz;
    modelPos.yz = vec2(modelPos.z, -modelPos.y);
    vec3 screenPos = cameraK * poseM * modelPos;
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
    gl_Position = vec4(pos, screenPos.z, 1.0);
}
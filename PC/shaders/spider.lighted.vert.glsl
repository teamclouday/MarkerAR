#version 450 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

uniform float ratio_img;
uniform float ratio_win;
uniform mat4x3 poseM;
uniform mat4 modelMat;
uniform mat3 cameraK;
uniform mat4 cameraProj;

layout (location = 0) out Data
{
    vec3 pos;
    vec3 normal;
    vec2 uv;
} VertexOut;

void main()
{
    VertexOut.uv = inUV;
    VertexOut.normal = normalize(inNormal);
    // change Z up coordinate to Y up
    vec4 pos = modelMat * vec4(inPos, 1.0);
    VertexOut.pos = pos.xyz;
    pos.yz = pos.zy * vec2(1.0, -1.0);
    vec3 screenPos = poseM * pos;
    screenPos.xy -= 0.5 * screenPos.z;
    screenPos = cameraK * screenPos;
    pos = cameraProj * vec4(screenPos, 1.0);
    if(ratio_img > ratio_win)
    {
        pos.y = pos.y * ratio_win / ratio_img;
    }
    else
    {
        pos.x = pos.x * ratio_img / ratio_win;
    }
    gl_Position = vec4(pos.xy, screenPos.z+0.5, 1.0);
}
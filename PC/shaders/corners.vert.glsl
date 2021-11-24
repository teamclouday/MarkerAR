#version 450 core

layout (location = 0) in vec2 inPos;

uniform float ratio_img;
uniform float ratio_win;
uniform float halfImgX;
uniform float halfImgY;

void main()
{
    vec2 halfImg = vec2(halfImgX, halfImgY);
    vec2 pos = clamp((vec2(inPos) - halfImg) / halfImg, -1.0, 1.0);
    if(ratio_img > ratio_win)
    {
        pos.y = pos.y * ratio_win / ratio_img;
    }
    else
    {
        pos.x = pos.x * ratio_img / ratio_win;
    }
    gl_Position = vec4(pos, -0.1, 1.0);
}
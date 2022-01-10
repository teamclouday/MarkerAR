#version 450 core

layout (location = 0) in vec2 inPos;

uniform float ratio_img;
uniform float ratio_win;
uniform float cam_width;
uniform float cam_height;

void main()
{
    vec2 pos = inPos / vec2(cam_width, cam_height) * 2.0 - 1.0;
    if(ratio_img > ratio_win)
    {
        pos.y = pos.y * ratio_win / ratio_img;
    }
    else
    {
        pos.x = pos.x * ratio_img / ratio_win;
    }
    gl_Position = vec4(pos, 0.0, 1.0);
}
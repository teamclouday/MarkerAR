#version 450 core

layout (location = 0) in vec2 inPos;
layout (location = 0) out vec2 imgUV;

uniform float ratio_img;
uniform float ratio_win;

void main()
{
    imgUV = (inPos + 1.0) * 0.5;
    vec2 pos = inPos;
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
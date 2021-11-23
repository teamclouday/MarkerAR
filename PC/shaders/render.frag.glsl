#version 450 core

layout (location = 0) in vec2 imgUV;
layout (location = 0) out vec4 color;

uniform sampler2D image;

void main()
{
    color = vec4(texture(image, imgUV).rrr, 1.0);
}
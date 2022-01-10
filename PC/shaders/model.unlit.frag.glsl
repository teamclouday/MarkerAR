#version 450 core

layout (location = 0) in vec3 vertColor;
layout (location = 0) out vec4 color;

void main()
{
    color = vec4(vertColor, 1.0);
}
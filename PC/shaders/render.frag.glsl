#version 450 core

layout (location = 0) in vec2 imgUV;
layout (location = 0) out vec4 color;

uniform sampler2D image;

void main()
{
    vec3 imgColor = texture(image, imgUV).rgb;
    color = vec4(imgColor, 1.0);
}
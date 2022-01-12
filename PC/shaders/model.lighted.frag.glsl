#version 450 core

layout (location = 0) out vec4 color;
layout (location = 0) in vec3 worldPos;

uniform vec3 lightPos;
uniform vec3 diffuse;

void main()
{
    vec3 dFdxPos = dFdx(worldPos);
    vec3 dFdyPos = dFdy(worldPos);
    vec3 norm = normalize(cross(dFdxPos,dFdyPos));

    vec3 dir = -normalize(lightPos);
    float coeff = clamp(dot(norm, dir), 0.4, 1.0);

    color = vec4(diffuse * coeff, 1.0);
}
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
    float coeff = max(0.0, dot(norm, dir));
    
    vec3 ambient = diffuse * 0.1;

    color = vec4(ambient + diffuse * coeff, 1.0);
}
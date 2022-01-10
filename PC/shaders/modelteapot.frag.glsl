#version 450 core

layout (location = 0) out vec4 color;
layout (location = 0) in vec3 worldPos;

uniform vec3 lightPos;

void main()
{
    vec3 dFdxPos = dFdx(worldPos);
    vec3 dFdyPos = dFdy(worldPos);
    vec3 norm = normalize(cross(dFdxPos,dFdyPos));
    
    vec3 diffuse = vec3(0.7,0.4,0.6);
    vec3 specular = vec3(0.8,0.8,0.8);

    // https://learnopengl.com/Lighting/Multiple-lights
    vec3 dir = -normalize(lightPos);
    float diff = max(0.0, dot(norm, dir));
    
    color = vec4(diffuse * diff, 1.0);
}
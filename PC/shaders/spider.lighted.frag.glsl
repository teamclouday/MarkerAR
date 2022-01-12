#version 450 core

layout (location = 0) out vec4 outColor;

uniform vec3 lightPos;

uniform sampler2D colorMap;
uniform sampler2D normalMap;
uniform sampler2D specMap;

layout (location = 0) in Data
{
    vec3 pos;
    vec3 normal;
    vec2 uv;
} FragmentIn;

vec3 computeNormal()
{
    vec3 normal = FragmentIn.normal;
    vec3 pos_dx = dFdx(FragmentIn.pos);
    vec3 pos_dy = dFdy(FragmentIn.pos);
    vec3 tex_dx = dFdx(vec3(FragmentIn.uv, 0.0));
    vec3 tex_dy = dFdy(vec3(FragmentIn.uv, 0.0));
    vec3 T = (tex_dy.t * pos_dx - tex_dx.t * pos_dy) / (tex_dx.s * tex_dy.t - tex_dy.s * tex_dx.t);
    vec3 tangent = normalize(T - normal * dot(normal, T));
    vec3 binorm = normalize(cross(normal, tangent));
    mat3 TBN = mat3(tangent, binorm, normal);
    vec3 normalTS = texture(normalMap, FragmentIn.uv).xyz * 2.0 - 1.0;
    return normalize(TBN * normalTS);
}

void main()
{
    vec3 normal = computeNormal();

    vec3 diffuse = texture(colorMap, FragmentIn.uv).rgb;
    vec3 ambient = diffuse * 0.4;
    vec3 specular = texture(specMap, FragmentIn.uv).rgb;

    vec3 color = vec3(0.0);

    vec3 dir = -normalize(lightPos);
    float coeff = max(dot(normal, dir), 0.0);
    if(coeff > 0.0)
    {
        // here we don't have access to camera position
        color += specular * pow(coeff * 0.4, 10.0);
    }

    color += diffuse * coeff;
    color = max(color, ambient);

    outColor = vec4(color, 1.0);
}
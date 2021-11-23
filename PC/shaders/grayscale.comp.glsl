#version 450 core

layout (local_size_x=32, local_size_y=32, local_size_z=1) in;

layout (rgba32f, binding=0) readonly  uniform image2D imageIn;
layout (r32f,    binding=1) writeonly uniform image2D imageOut;

uniform int shades;

void main()
{
    ivec2 baseUV = ivec2(gl_GlobalInvocationID.xy);
    baseUV = clamp(baseUV, ivec2(0), imageSize(imageIn));
    vec3 imgColor = imageLoad(imageIn, baseUV).rgb;
    float grayscale = dot(imgColor, vec3(0.33333333));
    if(shades > 1)
    {
        float convert = 1.0 / (shades - 1.0);
        grayscale = floor(grayscale / convert + 0.5) * convert;
    }
    imageStore(imageOut, baseUV, vec4(grayscale));
}
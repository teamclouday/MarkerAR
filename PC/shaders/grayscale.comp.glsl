// this shader converts rgb image to grayscale
#version 450 core

#define PI_2 6.283185307179586

layout (local_size_x=32, local_size_y=32, local_size_z=1) in;

layout (rgba32f, binding=0) readonly  uniform image2D imageIn;
layout (r32f,    binding=1) writeonly uniform image2D imageOut;

uniform int shades;
uniform bool blurFilter;
uniform float blurRadius;
uniform float blurQuality;
uniform float blurDirections;

void blur(inout vec3 imgColor, ivec2 baseUV)
{
    for(float d = 0.0; d < PI_2; d+=PI_2/blurDirections)
    {
        for(float i = 1.0/blurQuality; i <= 1.0; i+=1.0/blurQuality)
        {
            ivec2 uv = ivec2(vec2(cos(d),sin(d))*blurRadius*i);
            imgColor += imageLoad(imageIn, baseUV + uv).rgb;
        }
    }
    imgColor /= blurQuality * blurDirections + 1.0;
}

float grayscale(vec3 imgColor, int n)
{
    // float grayscale = dot(imgColor, vec3(0.33333333));
    // https://en.wikipedia.org/wiki/Luma_(video)
    float color = dot(imgColor, vec3(0.299, 0.587, 0.114));
    if(n > 1)
    {
        float convert = 1.0 / (n - 1.0);
        color = floor(color / convert + 0.5) * convert;
    }
    return color;
}

void main()
{
    ivec2 baseUV = ivec2(gl_GlobalInvocationID.xy);
    baseUV = clamp(baseUV, ivec2(0), imageSize(imageIn));
    vec3 imgColor = imageLoad(imageIn, baseUV).rgb;
    if(blurFilter)
    {
        blur(imgColor, baseUV);
    }
    imageStore(imageOut, baseUV, vec4(grayscale(imgColor, shades)));
}
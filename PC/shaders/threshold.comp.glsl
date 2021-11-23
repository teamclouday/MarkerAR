#version 450 core

layout (local_size_x=32, local_size_y=32, local_size_z=1) in;

layout (r32f, binding=0) readonly  uniform image2D imageIn;
layout (r32f, binding=1) writeonly uniform image2D imageOut;

uniform float threshold;

void main()
{
    ivec2 baseUV = ivec2(gl_GlobalInvocationID.xy);
    baseUV = clamp(baseUV, ivec2(0), imageSize(imageIn));
    float imgColor = imageLoad(imageIn, baseUV).r;
    imgColor = sign(imgColor - threshold);
    imageStore(imageOut, baseUV, vec4(imgColor));
}
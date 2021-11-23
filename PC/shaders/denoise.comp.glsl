// this shader denoise an image by applying spatial filter with guassian kernel
#version 450 core

#define INV_PI          0.31830988618379067153776752674503
#define INV_SQRT_OF_2PI 0.39894228040143267793994605993439 // 1.0/SQRT_OF_2PI

layout (local_size_x=32, local_size_y=32, local_size_z=1) in;

layout (rgba32f, binding=0) readonly  uniform image2D imageIn;
layout (rgba32f, binding=1) writeonly uniform image2D imageOut;

uniform float coeff_sigma;
uniform float coeff_kSigma;
uniform float coeff_threshold;

// reference: https://github.com/BrutPitt/glslSmartDeNoise
vec4 smartDeNoise(ivec2 uv, float sigma, float kSigma, float threshold)
{
    float radius = round(kSigma * sigma);
    float radQ = radius * radius;

    float invSigmaQx2 = 0.5 / (sigma * sigma);      // 1.0 / (sigma^2 * 2.0)
    float invSigmaQx2PI = INV_PI * invSigmaQx2;    // 1/(2 * PI * sigma^2)

    float invThresholdSqx2 = 0.5 / (threshold * threshold);     // 1.0 / (sigma^2 * 2.0)
    float invThresholdSqrt2PI = INV_SQRT_OF_2PI / threshold;   // 1.0 / (sqrt(2*PI) * sigma^2)

    vec4 centrPx = imageLoad(imageIn, uv); 

    float zBuff = 0.0;
    vec4 aBuff = vec4(0.0);

    vec2 d;
    for (d.x = -radius; d.x <= radius; d.x++) {
        float pt = sqrt(radQ - d.x * d.x);       // pt = yRadius: have circular trend
        for (d.y = -pt; d.y <= pt; d.y++) {
            float blurFactor = exp(-dot(d, d) * invSigmaQx2) * invSigmaQx2PI;

            vec4 walkPx = imageLoad(imageIn, uv + ivec2(d));
            vec4 dC = walkPx - centrPx;
            float deltaFactor = exp(-dot(dC, dC) * invThresholdSqx2) * invThresholdSqrt2PI * blurFactor;

            zBuff += deltaFactor;
            aBuff += deltaFactor * walkPx;
        }
    }
    return aBuff / zBuff;
}

vec4 smartDeNoise_lum(ivec2 uv, float sigma, float kSigma, float threshold)
{
    float radius = round(kSigma * sigma);
    float radQ = radius * radius;

    float invSigmaQx2 = 0.5 / (sigma * sigma);      // 1.0 / (sigma^2 * 2.0)
    float invSigmaQx2PI = INV_PI * invSigmaQx2;    // // 1/(2 * PI * sigma^2)

    float invThresholdSqx2 = 0.5 / (threshold * threshold);     // 1.0 / (sigma^2 * 2.0)
    float invThresholdSqrt2PI = INV_SQRT_OF_2PI / threshold;   // 1.0 / (sqrt(2*PI) * sigma)


    vec3 c = imageLoad(imageIn, uv).rgb;
    float centrLum = dot(c, vec3(0.299, 0.587, 0.114));

    float zBuff = 0.0;
    vec4 aBuff = vec4(0.0);

    vec2 d;
    for (d.x = -radius; d.x <= radius; d.x++) {
        float pt = sqrt(radQ - d.x * d.x);       // pt = yRadius: have circular trend
        for (d.y=-pt; d.y <= pt; d.y++) {
            float blurFactor = exp(-dot(d, d) * invSigmaQx2) * invSigmaQx2PI;

            vec4 walkPx = imageLoad(imageIn, uv + ivec2(d));

            float dC = dot(walkPx.rgb, vec3(0.299, 0.587, 0.114)) - centrLum;
            float deltaFactor = exp(-(dC * dC) * invThresholdSqx2) * invThresholdSqrt2PI * blurFactor;

            zBuff += deltaFactor;
            aBuff += deltaFactor * walkPx;
        }
    }
    return aBuff / zBuff;
}

void main()
{
    ivec2 baseUV = ivec2(gl_GlobalInvocationID.xy);
    baseUV = clamp(baseUV, ivec2(0), imageSize(imageIn));
    vec3 imgColor = imageLoad(imageIn, baseUV).rgb;
    
    vec4 denoised = smartDeNoise(baseUV, coeff_sigma, coeff_kSigma, coeff_threshold);

    imageStore(imageOut, baseUV, denoised);
}
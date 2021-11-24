// this shader creates contour edges in a binary image
#version 450 core

layout (local_size_x=32, local_size_y=32, local_size_z=1) in;

layout (r32f, binding=0) readonly  uniform image2D imageIn;
layout (r32f, binding=1) writeonly uniform image2D imageOut;

void main()
{
    ivec2 baseUV = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(imageIn) - ivec2(1);
    baseUV = clamp(baseUV, ivec2(0), size);
    bool onBorder = baseUV.x == 0 || baseUV.y == 0 || baseUV.x == size.x || baseUV.y == size.y;
    float imgColor = imageLoad(imageIn, baseUV).r;
    
    // 8 neighbors
    int count = 0;
    for(int x = -1; x <= 1; x++)
    {
        for(int y = -1; y <= 1; y++)
        {
            if(x == 0 && y == 0)
            {
                continue;
            }
            float val = imageLoad(imageIn, baseUV + ivec2(x, y)).r;
            count += int(imgColor == val);
        }
    }

    imgColor = (count <= 6 && !onBorder) ? 1.0 : 0.0;
    imageStore(imageOut, baseUV, vec4(imgColor));
}
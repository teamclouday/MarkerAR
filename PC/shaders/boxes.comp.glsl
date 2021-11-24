// this shader detects boxes from its area and store the four corners of box
#version 450 core

#define MAX_CORNERS_IN_AREA     5
#define MAX_STEPS_ALONG_LINE    200

layout (local_size_x=16, local_size_y=16, local_size_z=1) in;

layout (r32f, binding=0) readonly uniform image2D imageIn;

struct Box
{
    ivec2 p1;
    ivec2 p2;
    ivec2 p3;
    ivec2 p4;
};

layout (std430, binding=0) buffer boxesSSBO
{
    Box boxes[];
};

float angleCosine(vec2 p1, vec2 p2, vec2 p3)
{
    vec2 dir1 = normalize(p2 - p1);
    vec2 dir2 = normalize(p3 - p1);
    return dot(dir1, dir2);
}

bool onBorder(ivec2 pos, ivec2 size)
{
    return pos.x <= 1 || pos.y <= 1 || pos.x >= size.x || pos.y >= size.y;
}

bool isCorner(ivec2 center)
{
    // if center not white, not corner
    if(imageLoad(imageIn, center).r == 0.0) 
    {
        return false;
    }
    //
    //        N4      N3      N2
    //
    //                 
    //        N5    center    N1
    //
    //
    //        N6      N7      N8
    //
    float sumcolN = 0.0;
    vec2 sumpN = vec2(0.0);
    for(int i = -1; i <= 1; i++)
    {
        for(int j = -1; j <= 1; j++)
        {
            if(i == 0 && j == 0)
            {
                continue;
            }
            float col = imageLoad(imageIn, center + ivec2(i, j)).r;
            sumcolN += col;
            sumpN += col * vec2(i, j);
        }
    }
    // if too many or too few neighbors
    if(sumcolN < 2 || sumcolN > 3)
    {
        return false;
    }
    // if not a corner
    return sumpN.x >= 2 || sumpN.x <= -2 || sumpN.y >= 2 || sumpN.y <= -2 && sumcolN >= 2.0 && sumcolN <= 4.0;
}

ivec2 nextNeighbor(ivec2 center, ivec2 prev)
{
    for(int i = -2; i <= 2; i++)
    {
        for(int j = -2; j <= 2; j++)
        {
            if(i == 0 && j == 0)
            {
                continue;
            }
            ivec2 newPos = center + ivec2(i, j);
            if(newPos.x == prev.x && newPos.y == prev.y)
            {
                continue;
            }
            if(imageLoad(imageIn, newPos).r > 0.0)
            {
                return newPos;
            }
        }
    }
    return ivec2(-1);
}

ivec2 nextNeighborOnLine(ivec2 center, ivec2 prev)
{

    for(int i = -2; i <= 2; i++)
    {
        for(int j = -2; j <= 2; j++)
        {
            if(i == 0 && j == 0)
            {
                continue;
            }
            ivec2 newPos = center + ivec2(i, j);
            if(newPos.x == prev.x && newPos.y == prev.y)
            {
                continue;
            }
            if(imageLoad(imageIn, newPos).r > 0.0 &&
                angleCosine(vec2(prev), vec2(center), vec2(newPos)) > 0.8)
            {
                return newPos;
            }
        }
    }
    return ivec2(-1);
}

void main()
{
    // prepare basic information
    ivec2 size = imageSize(imageIn) - ivec2(1);
    ivec2 areaStart = ivec2(gl_GlobalInvocationID.xy) * 10;
    // prepare box ssbo index
    uvec2 boxesGroupID = gl_WorkGroupID.xy;
    uint boxesID = boxesGroupID.y * gl_NumWorkGroups.x + boxesGroupID.x;
    // step 1: scan in local area and find all box corners
    ivec2 cornerStack[MAX_CORNERS_IN_AREA];
    int cornerStackIdx = 0;
    for(int x = areaStart.x + 2; x <= min(areaStart.x + 7, size.x); x++)
    {
        for(int y = areaStart.y + 2; y <= min(areaStart.y + 7, size.y); y++)
        {
            ivec2 center = ivec2(x, y);
            if(isCorner(center) && cornerStackIdx < MAX_CORNERS_IN_AREA)
            {
                cornerStack[cornerStackIdx++] = center;
            }
        }
    }
    // step 2: go along a corner and find the next corner
    // until fifth corner is found and it is near our first corner
    ivec2 cornersFound[4] = ivec2[](
        ivec2(0), ivec2(0),
        ivec2(0), ivec2(0)
    );
    int corners = 0;
    int stepCounter = 0;
    ivec2 prevPos = ivec2(-1);
    ivec2 currentPos = ivec2(0);
    while(cornerStackIdx > 0)
    {
        // retrieve first corner as start point
        currentPos = cornerStack[--cornerStackIdx];
        cornersFound[corners++] = currentPos;
        // find next neighbor
        ivec2 newPos = nextNeighbor(currentPos, prevPos);
        prevPos = currentPos;
        currentPos = newPos;
        // if no neighbor, this is invalid corner, reset
        if(currentPos.x < 0)
        {
            corners = 0;
            prevPos = ivec2(-1);
            continue;
        }
        // continue to go along the line if current pos is valid
        for(stepCounter = 0;
            currentPos.x >= 0 && !onBorder(currentPos, size) && stepCounter < MAX_STEPS_ALONG_LINE;
            stepCounter++)
        {
            prevPos = currentPos;
            currentPos = nextNeighborOnLine(currentPos, prevPos);
            stepCounter++;
        }
        // now check if the new position is a corner
        if(isCorner(prevPos))
        {
            // if already 5 corners, check position
            if(corners >= 5)
            {
                if(distance(vec2(prevPos), vec2(cornersFound[0])) < 10)
                {
                    break;
                }
                else
                {
                    corners = 0;
                    prevPos = ivec2(-1);
                }
            }
            else
            {
                cornerStack[cornerStackIdx++] = prevPos;
            }
        }
        else
        {
            corners = 0;
            prevPos = ivec2(-1);
        }
    }
    // now collect detected corners
    if(corners >= 5)
    {
        boxes[boxesID].p1 = cornersFound[0];
        boxes[boxesID].p2 = cornersFound[1];
        boxes[boxesID].p3 = cornersFound[2];
        boxes[boxesID].p4 = cornersFound[3];
    }
    else
    {
        boxes[boxesID].p1 = ivec2(-1);
        boxes[boxesID].p2 = ivec2(-1);
        boxes[boxesID].p3 = ivec2(-1);
        boxes[boxesID].p4 = ivec2(-1);
    }
}
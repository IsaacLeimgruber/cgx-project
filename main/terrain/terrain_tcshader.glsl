#version 410 core

// define the number of CPs in the output patch
layout (vertices = 4) out;

uniform mat4 MVP;
uniform mat4 MV;

// attributes of the input CPs
in vec3 vpoint_TC[];
in vec2 uv_TC[];

// attributes of the output CPs
out vec3 vpoint_TE[];
out vec2 uv_TE[];

const float CLOSEST_TESS_DISTANCE = 0.2f;
const float FURTHEST_TESS_DISTANCE = 2.5f;
const float MIN_TESSELATION = 1.0f;
const float MAX_TESSELATION = 1.0f;

float GetTessLevel(in float Distance0, in float Distance1)
{
    float avgDistance = (Distance0 + Distance1) / 2.0f;

    //Clamp average between closest and furthest tesselation distance
    avgDistance = clamp(avgDistance, CLOSEST_TESS_DISTANCE, FURTHEST_TESS_DISTANCE);

    //More tesselation the closer we are from the point
    return mix(MAX_TESSELATION,
               MIN_TESSELATION,
               (avgDistance - CLOSEST_TESS_DISTANCE) / (FURTHEST_TESS_DISTANCE - CLOSEST_TESS_DISTANCE));
}

bool offscreen(in vec3 v){
    vec4 vProj = MVP * vec4(v, 1.0f);
    vProj /= vProj.w;

    //Rough estimate
    return  any(bvec2(vProj.z < -1.1f, vProj.z > 1.1f)) ||
            any(lessThan(vProj.xy, vec2(-2.0f))) ||
            any(greaterThan(vProj.xy, vec2(2.0f)));
}

void main()
{
    // Set the control points of the output patch
    uv_TE[gl_InvocationID] = uv_TC[gl_InvocationID];
    vpoint_TE[gl_InvocationID] = vpoint_TC[gl_InvocationID];

    // Calculate the distance from the camera to the three control points
    vec4 v0 = MV * vec4(vpoint_TC[0], 1.0f);
    vec4 v1 = MV * vec4(vpoint_TC[1], 1.0f);
    vec4 v2 = MV * vec4(vpoint_TC[2], 1.0f);
    vec4 v3 = MV * vec4(vpoint_TC[3], 1.0f);

    if(all(bvec4(offscreen(vpoint_TC[0]), offscreen(vpoint_TC[1]), offscreen(vpoint_TC[2]), offscreen(vpoint_TC[3])))){
        // No tesselation means patch is dropped -> save computation time !
        gl_TessLevelOuter[0] = gl_TessLevelOuter[1] = gl_TessLevelOuter[2] = gl_TessLevelOuter[3] = 0;
        gl_TessLevelInner[0] = gl_TessLevelInner[1] = 0;
    } else {

       float EyeToVertexDistance0 = length(v0);
       float EyeToVertexDistance1 = length(v1);
       float EyeToVertexDistance2 = length(v2);
       float EyeToVertexDistance3 = length(v3);

       /*   Calculate the tessellation levels
       *
       *    Points are given to vertex shader in counter-clockwise order
       *    OL = outer tesselation level
       *    IL = inner tesselation level IL0 = horizontal IL1 = vertical
       *
       *    3-------2
       *    |       |
       *    |       |
       *    0-------1
       *
       *  OL 0 = 0-3
       *  OL 1 = 0-1
       *  OL 2 = 2-1
       *  OL 3 = 2-3
       */
       gl_TessLevelOuter[0] = GetTessLevel(EyeToVertexDistance0, EyeToVertexDistance3);
       gl_TessLevelOuter[1] = GetTessLevel(EyeToVertexDistance1, EyeToVertexDistance0);
       gl_TessLevelOuter[2] = GetTessLevel(EyeToVertexDistance2, EyeToVertexDistance1);
       gl_TessLevelOuter[3] = GetTessLevel(EyeToVertexDistance3, EyeToVertexDistance2);
       gl_TessLevelInner[0] = (gl_TessLevelOuter[1] + gl_TessLevelOuter[3]) / 2.0f;
       gl_TessLevelInner[1] = (gl_TessLevelOuter[0] + gl_TessLevelOuter[2]) / 2.0f;
    }
}

#version 410 core

// define the number of CPs in the output patch
layout (vertices = 4) out;

uniform mat4 view;
uniform mat4 model;

// attributes of the input CPs
in vec3 vpoint_TC[];
in vec2 uv_TC[];

// attributes of the output CPs
out vec3 vpoint_TE[];
out vec2 uv_TE[];

const float CLOSEST_TESS_DISTANCE = 0.2f;
const float FURTHEST_TESS_DISTANCE = 5.0f;
const float MIN_TESSELATION = 1.0f;
const float MAX_TESSELATION = 32.0f;

float GetTessLevel(float Distance0, float Distance1)
{
    float avgDistance = (Distance0 + Distance1) / 2.0;

    //Clamp average between closest and furthest tesselation distance
    avgDistance = min(max(CLOSEST_TESS_DISTANCE, avgDistance), FURTHEST_TESS_DISTANCE);

    //More tesselation the closer we are from the point
    return mix(MAX_TESSELATION,
               MIN_TESSELATION,
               (avgDistance - CLOSEST_TESS_DISTANCE) / (FURTHEST_TESS_DISTANCE - CLOSEST_TESS_DISTANCE));
}

void main()
{
    // Set the control points of the output patch
    uv_TE[gl_InvocationID] = uv_TC[gl_InvocationID];
    vpoint_TE[gl_InvocationID] = vpoint_TC[gl_InvocationID];

    // Calculate the distance from the camera to the three control points
    //TODO pass camera world pos instead of computing point pos in view space
   float EyeToVertexDistance0 = length(view * model * vec4(vpoint_TC[0], 1.0));
   float EyeToVertexDistance1 = length(view * model * vec4(vpoint_TC[1], 1.0));
   float EyeToVertexDistance2 = length(view * model * vec4(vpoint_TC[2], 1.0));
   float EyeToVertexDistance3 = length(view * model * vec4(vpoint_TC[3], 1.0));

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
   gl_TessLevelInner[0] = (gl_TessLevelOuter[1] + gl_TessLevelOuter[3]) / 2.0;
   gl_TessLevelInner[1] = (gl_TessLevelOuter[0] + gl_TessLevelOuter[2]) / 2.0;
}

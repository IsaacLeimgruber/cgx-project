#version 410 core

// define the number of CPs in the output patch
layout (vertices = 3) out;

uniform mat4 view;
uniform mat4 model;

// attributes of the input CPs
in vec3 vpoint_TC[];
in vec2 uv_TC[];

// attributes of the output CPs
out vec3 vpoint_TE[];
out vec2 uv_TE[];

float GetTessLevel(float Distance0, float Distance1)
{
    float AvgDistance = (Distance0 + Distance1) / 2.0;

    if (AvgDistance <= 1.5) {
        return 10.0;
    }
    else if (AvgDistance <= 2) {
        return 7.0;
    }
    else {
        return 3.0;
    }
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

   // Calculate the tessellation levels
   gl_TessLevelOuter[0] = GetTessLevel(EyeToVertexDistance1, EyeToVertexDistance2);
   gl_TessLevelOuter[1] = GetTessLevel(EyeToVertexDistance2, EyeToVertexDistance0);
   gl_TessLevelOuter[2] = GetTessLevel(EyeToVertexDistance0, EyeToVertexDistance1);
   gl_TessLevelInner[0] = gl_TessLevelOuter[2];
}

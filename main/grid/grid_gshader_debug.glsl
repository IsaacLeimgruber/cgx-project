#version 410
layout(triangles) in;

// Three lines will be generated: 6 vertices
layout(line_strip, max_vertices=6) out;

uniform sampler2D normalMap;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 normalMatrix;

uniform vec2 zoomOffset;
uniform float zoom;

in vec4 vpoint_MV_G[];
in vec4 vpoint_M_G[];
in vec4 normal_MV_G[];
in vec2 uv_G[];
in vec3 lightDir_G[];
in vec3 viewDir_G[];
in float vheight_G[];

out vec2 uv_F;

void main()
{
      for(int i=0; i<gl_in.length(); i++)
      {
        uv_F = uv_G[i];

        vec3 P = vpoint_MV_G[i].xyz;
        vec3 N = normal_MV_G[i].xyz;

        gl_Position = projection * vec4(P, 1.0);
        //vertex_color = vertex[i].color;
        EmitVertex();

        gl_Position = projection * vec4(P + N, 1.0);
        uv_F = uv_G[i];
        //vertex_color = vertex[i].color;
        EmitVertex();

        EndPrimitive();
      }
}

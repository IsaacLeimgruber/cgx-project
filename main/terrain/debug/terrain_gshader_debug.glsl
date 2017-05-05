#version 410 core
layout(triangles) in;

// Three lines will be generated: 6 vertices
layout(line_strip, max_vertices=12) out;

uniform sampler2D normalMap;

uniform mat4 MVP;
uniform mat4 MV;
uniform mat4 NORMALM;

in vec4 vpoint_MV_G[];
in vec3 normal_G[];
in vec2 uv_G[];
in vec3 lightDir_G[];
in vec3 viewDir_G[];
in float vheight_G[];

out vec2 uv_F;
out vec3 vcolor;

const vec3 RED = vec3(1.0, 0.0, 0.0);
const vec3 YELLOW = vec3(1.0, 1.0, 0.0);
void main()
{
    vec3 P = vec3(0);
    vec3 N = vec3(0);

      for(int i=0; i<gl_in.length(); i++)
      {

        //Display light direction
        P = gl_in[i].gl_Position.xyz;
        N = lightDir_G[i].xyz;
        gl_Position = MVP * vec4(P, 1.0);
        uv_F = uv_G[i];
        vcolor = RED;
        EmitVertex();

        gl_Position = MVP * vec4(P + N * 0.1, 1.0);
        uv_F = uv_G[i];
        vcolor = RED;
        EmitVertex();
        EndPrimitive();

        //Display normal
        N = normal_G[i];

        gl_Position = MVP * vec4(P, 1.0);
        uv_F = uv_G[i];
        vcolor = YELLOW;
        EmitVertex();

        gl_Position = MVP * vec4(P + N * 0.1, 1.0);
        uv_F = uv_G[i];
        vcolor = YELLOW;
        EmitVertex();

        EndPrimitive();
      }
}

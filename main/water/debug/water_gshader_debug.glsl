#version 410 core
layout(triangles) in;

// Three lines will be generated: 6 vertices
layout(line_strip, max_vertices=6) out;

uniform mat4 MVP;
uniform mat4 MV;
uniform mat4 NORMALM;

in vec2 uv_G[];
in vec2 reflectOffset_G[];
in vec3 waveNormal_G[];

out vec2 uv_F;
out vec2 reflectOffset_F;

void main()
{

    int i;
      for(i=0; i<gl_in.length(); i++)
      {
        uv_F = uv_G[i];
        reflectOffset_F = reflectOffset_G[i];

        vec3 P = gl_in[i].gl_Position.xyz;
        vec3 N = normalize(waveNormal_G[i]);

        gl_Position = MVP * vec4(P, 1.0);
        //vertex_color = vertex[i].color;
        EmitVertex();

        gl_Position = MVP * vec4(P + N * 0.1, 1.0);
        uv_F = uv_G[i];
        reflectOffset_F = reflectOffset_G[i];
        //vertex_color = vertex[i].color;
        EmitVertex();

        EndPrimitive();
      }

}

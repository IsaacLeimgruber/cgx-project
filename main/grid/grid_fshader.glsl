#version 330

uniform vec3 La, Ld;
uniform vec3 ka, kd;

in vec4 vpoint_mv;
in vec3 light_dir, view_dir;

in vec2 uv;
in vec3 mv_normal;

out vec3 color;

void main() {

    vec3 triangleNormal = normalize(cross(dFdx(vpoint_mv.xyz), dFdy(vpoint_mv.xyz)));

    float cosNL = dot(triangleNormal, light_dir);

    //We dont want contributions from reflections coming from "under" the mesh
    if(cosNL > 0){
        vec3 reflection_dir = normalize( 2 * triangleNormal * cosNL - light_dir);
        color = (ka * La) + (kd * cosNL * Ld);
    } else {
        color = vec3(0.0,0.0,0.0);
    }
}

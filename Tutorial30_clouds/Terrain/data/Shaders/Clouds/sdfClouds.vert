#version 440

/*
==========================================
attributes
==========================================
*/
layout( location = 0 ) in vec3 position;
layout( location = 1 ) in vec2 st;
layout( location = 2 ) in vec4 normal;
layout( location = 3 ) in vec4 tangent;
layout( location = 4 ) in vec4 color;

/*
==========================================
uniforms
==========================================
*/
uniform mat4 projInverse;
uniform mat4 viewInverse;

/*
==========================================
output
==========================================
*/
out vec2 v_coords;
out vec3 v_ray;

/*
==========================================
main
==========================================
*/
void main() {
    v_coords = position.xy * 0.5 + 0.5;

    vec4 vert = vec4( position.xyz, 1.0 );
    v_ray = ( viewInverse * vec4( ( projInverse * vert ).xyz, 0.0 ) ).xyz;

    gl_Position = vert;
}
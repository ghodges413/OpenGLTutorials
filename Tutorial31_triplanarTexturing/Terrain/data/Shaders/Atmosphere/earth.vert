// This lines tells OpenGL which version of GLSL we're using
#version 440

// This is the incoming vertex data
layout( location = 0 ) in vec3 position;
//layout( location = 1 ) in vec2 st;
//layout( location = 2 ) in vec4 normal;
//layout( location = 3 ) in vec4 tangent;
//layout( location = 4 ) in vec4 color;

uniform mat4 projInverse;
uniform mat4 viewInverse;

out vec2 v_coords;
out vec3 v_ray;

void main() {
    v_coords = position.xy * 0.5 + 0.5;
    vec4 vert = vec4( position.xyz, 1.0 );
    v_ray = ( viewInverse * vec4( ( projInverse * vert ).xyz, 0.0 ) ).xyz;
    gl_Position = vert;
}

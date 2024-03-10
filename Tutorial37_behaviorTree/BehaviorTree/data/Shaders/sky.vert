#version 430

/*
==========================================
uniforms
==========================================
*/

uniform mat4 matView;
uniform mat4 matProj;

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
output
==========================================
*/

layout( location = 0 ) out vec4 modelPos;

/*
==========================================
main
==========================================
*/
void main() {
    modelPos = vec4( position, 1.0 );

    vec4 viewPos = matView * vec4( position, 0.0 );

    // Project coordinate to screen
    gl_Position = matProj * vec4( viewPos.xyz, 1.0 );
}
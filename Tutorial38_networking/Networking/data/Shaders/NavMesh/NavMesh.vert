#version 430

/*
==========================================
uniforms
==========================================
*/

uniform mat4 matModelWorld;	// This isn't necessary, the terrain will be in world space
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

/*
==========================================
main
==========================================
*/
void main() {
    gl_Position	= matProj * matView * matModelWorld * vec4( position.xyz, 1.0 );
}
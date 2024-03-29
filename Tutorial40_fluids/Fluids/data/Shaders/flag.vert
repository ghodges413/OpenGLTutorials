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

//layout( location = 0 ) out vec2 v_st;

// The texture coordinate value will be sent to the fragment shader
out vec3 v_pos;
out vec2 v_st;
out vec3 v_normal;
out vec3 v_tangent;
out vec3 v_bitangent;
out vec3 v_color;

/*
==========================================
main
==========================================
*/
void main() {
    gl_Position	= matProj * matView * matModelWorld * vec4( position.xyz, 1.0 );
	v_st = st;
}
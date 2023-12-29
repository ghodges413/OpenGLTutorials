// This lines tells OpenGL which version of GLSL we're using
#version 430

// This is the incoming vertex data
layout( location = 0 ) in vec3 position;
layout( location = 1 ) in vec3 normal;
layout( location = 2 ) in vec3 tangent;
//layout( location = 3 ) in vec2 st;

uniform mat4 matModelWorld;	// This isn't necessary, the terrain will be in world space
uniform mat4 matView;
uniform mat4 matProj;
uniform int lod;

// The texture coordinate value will be sent to the fragment shader
out vec3 v_color;
out vec3 v_normal;

/*
==========================
main
==========================
*/
void main() {
	gl_Position	= matProj * matView * matModelWorld * vec4( position.xyz, 1.0 );
	v_color	= tangent;
	v_normal = normal;

	if ( 0 == lod ) {
		v_color = vec3( 1, 0, 0 );
	}
	if ( 1 == lod ) {
		v_color = vec3( 0, 1, 0 );
	}
	if ( 2 == lod ) {
		v_color = vec3( 0, 0, 1 );
	}
	if ( 3 == lod ) {
		v_color = vec3( 1, 1, 0 );
	}
	if ( 4 == lod ) {
		v_color = vec3( 1, 0, 1 );		
	}
	if ( 5 == lod ) {
		v_color = vec3( 1, 1, 1 );
	}
}

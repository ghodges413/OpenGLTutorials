// This lines tells OpenGL which version of GLSL we're using
#version 330

// This is the incoming vertex data
layout( location = 0 ) in vec3 position;

uniform mat4 matProj;
uniform mat4 matView;

/*
==========================
main
==========================
*/
void main() {
	// gl_Position is a special value that tells the GPU the transformed location of the vertex
	gl_Position	= matProj * matView * vec4( position.xyz, 1.0 );
}

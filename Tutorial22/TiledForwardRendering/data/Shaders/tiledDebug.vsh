// This lines tells OpenGL which version of GLSL we're using
#version 430

// This is the incoming vertex data
layout( location = 0 ) in vec3 position;

/*
==========================
main
==========================
*/
void main() {
	// gl_Position is a special value that tells the GPU the transformed location of the vertex
	gl_Position	= vec4( position.xyz, 1.0 );
}

// This lines tells OpenGL which version of GLSL we're using
#version 330

// This is the incoming vertex data
layout( location = 0 ) in vec3 position;
layout( location = 1 ) in vec2 st;

uniform mat4 matTransform;

// The texture coordinate value will be sent to the fragment shader
out vec2 v_texCoord;

/*
==========================
main
==========================
*/
void main() {
	// gl_Position is a special value that tells the GPU the transformed location of the vertex
	gl_Position	= matTransform * vec4( position.xyz, 1.0 );
	
	// Simply copy the texturing coordinates out to the fragment shader
	v_texCoord	= st;
}

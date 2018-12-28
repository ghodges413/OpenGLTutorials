// This lines tells OpenGL which version of GLSL we're using
#version 330

// This is the incoming vertex data
layout( location = 0 ) in vec3 position;
layout( location = 1 ) in vec3 color;

// This color value will be sent to the fragment shader
out vec3 v_color;

/*
==========================
main
==========================
*/
void main() {
	// gl_Position is a special value that tells the GPU the transformed location of the vertex
	gl_Position	= vec4( position.xyz, 1.0 );
	
	// Simply copy the color value out to the fragment shader
	v_color			= color;
}

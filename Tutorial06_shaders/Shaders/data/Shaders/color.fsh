// This lines tells OpenGL which version of GLSL we're using
#version 330

// This is the incoming color that came from the vertex shader
in vec3 v_color;

// This is the outgoing color that'll be the final color for output
out vec4 FragColor;

/*
==========================
main
==========================
*/
void main() {
	// Simply copy over the color value
	FragColor.rgb = v_color.rgb;
	
	// Set the alpha channel to 1
	FragColor.a = 1.0;
}

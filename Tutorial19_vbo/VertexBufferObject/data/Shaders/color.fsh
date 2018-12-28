// This lines tells OpenGL which version of GLSL we're using
#version 330

uniform vec3 color;

// This is the outgoing color that'll be the final color for output
out vec4 FragColor;

/*
==========================
main
==========================
*/
void main() {
	// Use the texture coordinates to look up the texel value in the texture and copy out to the fragment
	FragColor.rgb = color.rgb;
	FragColor.a = 1.0;
}

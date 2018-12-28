// This lines tells OpenGL which version of GLSL we're using
#version 330

// This defines the 2D texture
uniform sampler2D s_texture;

// This is the incoming color that came from the vertex shader
in vec2 v_texCoord;

// This is the outgoing color that'll be the final color for output
out vec4 FragColor;

/*
==========================
main
==========================
*/
void main() {
	// Use the texture coordinates to look up the texel value in the texture and copy out to the fragment
	FragColor = texture( s_texture, v_texCoord );
	FragColor.a = 1.0;
}

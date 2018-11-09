// This lines tells OpenGL which version of GLSL we're using
#version 330

// This defines the 2D texture
uniform sampler2D s_texture;
uniform vec3 rayToLight;
uniform vec3 lightColor;

// This is the incoming color that came from the vertex shader
in vec2 v_texCoord;
in vec3 v_normal;

// This is the outgoing color that'll be the final color for output
out vec4 FragColor;

/*
==========================
main
==========================
*/
void main() {
	// Calculate the shading from the light
	float shading = dot( normalize( v_normal.xyz ), rayToLight.xyz );
	
	// Use the texture coordinates to look up the texel value in the texture and copy out to the fragment
	FragColor.rgb = texture( s_texture, v_texCoord ).rgb * shading * lightColor;
	FragColor.a = 1.0;
}

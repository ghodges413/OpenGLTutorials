// This lines tells OpenGL which version of GLSL we're using
#version 330

// This is the incoming vertex data
layout( location = 0 ) in vec3 position;
layout( location = 1 ) in vec3 normal;
layout( location = 2 ) in vec2 st;

uniform mat4 matModelWorld;
uniform mat4 matProj;
uniform vec3 rayToLight;

// The texture coordinate value will be sent to the fragment shader
out vec2 v_texCoord;
out float v_shading;

/*
==========================
main
==========================
*/
void main() {
	// gl_Position is a special value that tells the GPU the transformed location of the vertex
	gl_Position	= matProj * matModelWorld * vec4( position.xyz, 1.0 );
	
	// Transform the normal to world space
	vec4 normalWorldSpace = matModelWorld * vec4( normal.xyz, 0.0 );

	// Calculate how shading on this vertex
	v_shading = dot( normalWorldSpace.xyz, rayToLight.xyz );
	
	// Simply copy the texturing coordinates out to the fragment shader
	v_texCoord	= st;
}

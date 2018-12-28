// This lines tells OpenGL which version of GLSL we're using
#version 330

// This is the incoming vertex data
layout( location = 0 ) in vec3 position;
layout( location = 1 ) in vec3 normal;
layout( location = 2 ) in vec3 tangent;
layout( location = 3 ) in vec2 st;

uniform mat4 matModelWorld;
uniform mat4 matView;
uniform mat4 matProj;

// The texture coordinate value will be sent to the fragment shader
out vec2 v_texCoord;
out vec3 v_normal;
out vec3 v_tangent;

/*
==========================
main
==========================
*/
void main() {
	// gl_Position is a special value that tells the GPU the transformed location of the vertex
	gl_Position	= matProj * matView * matModelWorld * vec4( position.xyz, 1.0 );
	
	// Transform the normal to world space (notice we only want to rotate our normal, so the 'w' value is 0 instead of 1)
	vec4 normalWorldSpace = matModelWorld * vec4( normal.xyz, 0.0 );
	vec4 tangentWorldSpace = matModelWorld * vec4( tangent.xyz, 0.0 );
	
	// Pass the transformed normal to the fragment shader
	v_normal.xyz = normalWorldSpace.xyz;
	v_tangent.xyz = tangentWorldSpace.xyz;
	
	// Simply copy the texturing coordinates out to the fragment shader
	v_texCoord	= st;
}

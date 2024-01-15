// This lines tells OpenGL which version of GLSL we're using
#version 440

// This is the incoming vertex data
layout( location = 0 ) in vec3 position;
layout( location = 1 ) in vec2 st;
layout( location = 2 ) in vec4 normal;
layout( location = 3 ) in vec4 tangent;
layout( location = 4 ) in vec4 color;

uniform mat4 matModelWorld;	// This isn't necessary, the terrain will be in world space
uniform mat4 matView;
uniform mat4 matProj;

//uniform mat4 projInverse;
//uniform mat4 viewInverse;

// The texture coordinate value will be sent to the fragment shader
out vec3 v_color;
out vec3 v_normal;
out vec3 v_position;
out vec3 v_ray;

/*
==========================
main
==========================
*/
void main() {
	gl_Position	= matProj * matView * matModelWorld * vec4( position.xyz, 1.0 );

	v_color		= color.xyz;
	v_normal	= 2.0 * ( normal.xyz - vec3( 0.5 ) ); // convert from [0,1] to [-1,1]
	v_position	= position;

	//vec4 vertex = vec4( position.xyz, 1.0f );
	
//    v_ray = ( viewInverse * vec4( ( projInverse * vertex ).xyz, 0.0 ) ).xyz;
}

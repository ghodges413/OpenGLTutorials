// This lines tells OpenGL which version of GLSL we're using
#version 430

// This is the incoming vertex data
layout( location = 0 ) in vec3 position;
layout( location = 1 ) in vec2 st;
layout( location = 2 ) in vec4 normal;
layout( location = 3 ) in vec4 tangent;
layout( location = 4 ) in vec4 color;

uniform mat4 matModelWorld;	// This isn't necessary, the terrain will be in world space
uniform mat4 matView;
uniform mat4 matProj;

// The texture coordinate value will be sent to the fragment shader
out vec3 v_color;
out vec3 v_normal;

/*
==========================
main
==========================
*/
void main() {
	gl_Position	= matProj * matView * matModelWorld * vec4( position.xyz, 1.0 );
	v_color	= color.xyz;
	v_normal = normal.xyz;
	v_normal = 2.0 * ( normal.xyz - vec3( 0.5 ) ); // convert from [0,1] to [-1,1]
}

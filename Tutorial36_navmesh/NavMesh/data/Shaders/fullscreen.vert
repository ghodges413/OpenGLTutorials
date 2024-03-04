// This lines tells OpenGL which version of GLSL we're using
#version 430

// This is the incoming vertex data
layout( location = 0 ) in vec3 position;
layout( location = 1 ) in vec2 st;
layout( location = 2 ) in vec4 normal;
layout( location = 3 ) in vec4 tangent;
layout( location = 4 ) in vec4 color;

//uniform mat4 matModelWorld;	// This isn't necessary, the terrain will be in world space
//uniform mat4 matView;
//uniform mat4 matProj;

// The texture coordinate value will be sent to the fragment shader
out vec3 v_pos;
out vec2 v_st;
out vec3 v_normal;
out vec3 v_tangent;
out vec3 v_bitangent;
out vec3 v_color;

/*
==========================
main
==========================
*/
void main() {
	//gl_Position	= matProj * matView * matModelWorld * vec4( position.xyz, 1.0 );
	gl_Position	= vec4( position.xyz, 1.0 );
	v_pos = position;
	v_st = st;
	v_st.y = 1.0 - st.y;
	v_color	= color.rgb;
	v_normal = normal.rgb;
	v_normal = 2.0 * ( normal.xyz - vec3( 0.5 ) ); // convert from [0,1] to [-1,1]
	//v_normal = ( matModelWorld * vec4( v_normal.xyz, 1.0 ) ).xyz;
	v_tangent = tangent.rgb;
	v_tangent = 2.0 * ( tangent.xyz - vec3( 0.5 ) ); // convert from [0,1] to [-1,1]
	//v_tangent = ( matModelWorld * vec4( v_tangent.xyz, 1.0 ) ).xyz;
	v_bitangent = cross( v_normal, v_tangent );
}

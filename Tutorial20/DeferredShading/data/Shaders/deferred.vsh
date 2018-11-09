#version 330

layout( location = 0 ) in vec3 position;
layout( location = 1 ) in vec3 normal;
layout( location = 2 ) in vec3 tangent;
layout( location = 3 ) in vec2 st;

uniform mat4 matModelWorld;
uniform mat4 matView;
uniform mat4 matProj;

out vec4 v_position;
out vec4 v_normal;
out vec4 v_tangent;
out vec2 v_texCoord;

/*
 ==========================
 main
 ==========================
 */
void main() {
	vec4 vert	= vec4( position.xyz, 1.0 );
    
	//  Normal vertex transformation
	gl_Position     = matProj * matView * matModelWorld * vert;
    
	//  Copy the texture coordinates over
	v_texCoord      = st;

	//  Transform the position to view space
	v_position      = matModelWorld * vert;
    
	//  Transform the normal and tangent to view space
	vec4 norm	= vec4( normal,     0.0 );
	vec4 tang	= vec4( tangent,    0.0 );
	v_normal	= matModelWorld * norm;
	v_tangent	= matModelWorld * tang;
}


#version 330

layout( location = 0 ) in vec3	position;

uniform mat4 matModelWorld;

uniform mat4 matView;
uniform mat4 matProj;

/*
==========================
main
==========================
*/
void main() {
	// transform vertex
	gl_Position = matProj * matView * matModelWorld * vec4( position.xyz, 1.0 );
}

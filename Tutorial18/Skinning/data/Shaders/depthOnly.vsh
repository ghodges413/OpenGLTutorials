#version 330

layout( location = 0 ) in vec3	position;

uniform mat4 mvp;

/*
==========================
main
==========================
*/
void main() {
	// transform vertex
	gl_Position = mvp * vec4( position.xyz, 1.0 );
}

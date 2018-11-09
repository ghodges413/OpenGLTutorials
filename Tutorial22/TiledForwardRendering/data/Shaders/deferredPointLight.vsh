#version 330

layout( location = 0 ) in vec3 position;

uniform mat4 matView;
uniform mat4 matProj;

uniform vec3 u_lightPosition;
uniform float u_scale;

out vec4 v_transformedVert;
out vec3 v_lightPosition;

/*
==========================
main
==========================
*/
void main() {    
	// grab vertex position
	vec4 vert   = vec4( position.xyz, 1.0 );
	
	// Put the vert in world space
	vert.xyz *= u_scale;
	vert.xyz += u_lightPosition.xyz;
	
	// Output the world space position
	v_lightPosition = u_lightPosition;
	
	// transform vertex
	gl_Position = matProj * matView * vert;
	
	// Send this to the fragment shader, we'll need it for looking up the g-buffer
	v_transformedVert = matProj * matView * vert;
}

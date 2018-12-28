#version 330

layout( location = 0 ) in vec3 position;

uniform mat4 matView;
uniform mat4 matProj;

uniform vec3 u_lightPosition;
uniform float u_scale;

out vec4 v_transformedVert;
//out vec2 v_texCoord;
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
	
	v_transformedVert = matProj * matView * vert;

	// calculate the texture coordinate lookup into the geometry buffer
//	v_texCoord = vec2( gl_Position.x, gl_Position.y ) / gl_Position.w;
//	v_texCoord = v_texCoord * 0.5 + 0.5;
}

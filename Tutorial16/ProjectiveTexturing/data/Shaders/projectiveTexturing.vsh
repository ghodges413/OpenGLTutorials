#version 330

// This is the incoming vertex data
layout( location = 0 ) in vec3 position;

uniform mat4 matModelWorld;
uniform mat4 matView;
uniform mat4 matProj;

uniform mat4 matLightProj;
uniform mat4 matLightView;
uniform mat4 matLightBias;

out vec4 v_posLightSpace;

/*
==========================
main
==========================
*/
void main() {
	// Calculate the world space position of the vertex
	vec4 posWorld = matModelWorld * vec4( position.xyz, 1.0 );
	
	// gl_Position is a special value that tells the GPU the transformed location of the vertex
	gl_Position	= matProj * matView * vec4( posWorld.xyz, 1.0 );
    
	// Transform the position into the light's space
    v_posLightSpace  = matLightBias * matLightProj * matLightView * vec4( posWorld.xyz, 1.0 );
}

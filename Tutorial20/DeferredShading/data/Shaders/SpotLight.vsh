#version 330

// This is the incoming vertex data
layout( location = 0 ) in vec3 position;
layout( location = 1 ) in vec3 normal;
layout( location = 2 ) in vec3 tangent;
layout( location = 3 ) in vec2 st;

uniform mat4 matModelWorld;
uniform mat4 matView;
uniform mat4 matProj;

uniform mat4 matLightProj;
uniform mat4 matLightView;
uniform mat4 matLightBias;

out vec2 v_texCoord;
out vec3 v_normal;
out vec3 v_tangent;
out vec3 v_posWorldSpace;
out vec4 v_posLightSpace;

/*
==========================
main
==========================
*/
void main() {
	// Calculate the world space position of the vertex
	vec4 posWorld = matModelWorld * vec4( position.xyz, 1.0 );
	v_posWorldSpace = posWorld.xyz;
	
	// gl_Position is a special value that tells the GPU the transformed location of the vertex
	gl_Position	= matProj * matView * vec4( posWorld.xyz, 1.0 );
	
	// Transform the normal to world space (notice we only want to rotate our normal, so the 'w' value is 0 instead of 1)
	vec4 normalWorldSpace = matModelWorld * vec4( normal.xyz, 0.0 );
	vec4 tangentWorldSpace = matModelWorld * vec4( tangent.xyz, 0.0 );
	
	// Pass the transformed normal to the fragment shader
	v_normal.xyz = normalWorldSpace.xyz;
	v_tangent.xyz = tangentWorldSpace.xyz;
    
	// copy texture coordinate
	v_texCoord = st;
	
	// Transform the position into the light's space
    v_posLightSpace  = matLightBias * matLightProj * matLightView * vec4( posWorld.xyz, 1.0 );
}

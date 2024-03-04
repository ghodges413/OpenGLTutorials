#version 430

/*
==========================================
uniforms
==========================================
*/

uniform mat4 matModelWorld;	// This isn't necessary, the terrain will be in world space
uniform mat4 matView;
uniform mat4 matProj;
/*
layout( binding = 0 ) uniform uboCamera {
    mat4 view;
    mat4 proj;
} camera;
layout( binding = 1 ) uniform uboModel {
    mat4 model;
} model;

/*
==========================================
attributes
==========================================
*/

layout( location = 0 ) in vec3 position;
layout( location = 1 ) in vec2 st;
layout( location = 2 ) in vec4 normal;
layout( location = 3 ) in vec4 tangent;
layout( location = 4 ) in vec4 color;

/*
==========================================
output
==========================================
*/

layout( location = 0 ) out vec3 v_worldNormal;
layout( location = 1 ) out vec4 v_modelPos;
layout( location = 2 ) out vec3 v_modelNormal;

/*
==========================================
main
==========================================
*/
void main() {
	//vec3 normal = 2.0 * ( inNormal.xyz - vec3( 0.5 ) );
	//modelNormal = normal;
	//modelPos = vec4( inPosition, 1.0 );
   
    // Get the tangent space in world coordinates
    //worldNormal = matModelWorld * vec4( normal.xyz, 0.0 );
   
    // Project coordinate to screen
    //gl_Position = camera.proj * camera.view * model.model * vec4( inPosition, 1.0 );


    gl_Position	= matProj * matView * matModelWorld * vec4( position.xyz, 1.0 );
	v_modelPos = vec4( position, 1.0 );

	v_modelNormal = normal.rgb;
	v_modelNormal = 2.0 * ( normal.xyz - vec3( 0.5 ) ); // convert from [0,1] to [-1,1]
	v_worldNormal = ( matModelWorld * vec4( v_modelNormal.xyz, 0.0 ) ).xyz;
}
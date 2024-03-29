#version 430

/*
==========================================
uniforms
==========================================
*/

uniform mat4 matModelWorld;	// This isn't necessary, the terrain will be in world space
uniform mat4 matView;
uniform mat4 matProj;

uniform float speed;
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
layout( location = 3 ) out vec3 v_color;

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

    float t = speed;// * 2.0;
    if ( t > 1.0 ) {
        t = 1.0;
    }
#if 0
    vec3 colorSlow = vec3( 0, 0, 1 );
    vec3 colorMid0 = vec3( 0, 1, 1 );
    vec3 colorMid1 = vec3( 1, 1, 0 );
    vec3 colorFast = vec3( 1, 0, 0 );

    if ( t < 0.5 ) {
        float s = t / 0.5;  // remap to [0,1]
        v_color = mix( colorSlow, colorMid0, s );
    } else if ( t < 0.75 ) {
        float s = ( t - 0.5 ) / 0.25;  // remap to [0,1]
        v_color = mix( colorMid0, colorMid1, s );
    } else {
        float s = ( t - 0.75 ) / 0.25;  // remap to [0,1]
        v_color = mix( colorMid1, colorFast, s );
    }
#elif 1
    vec3 colorSlow = vec3( 0, 0.2, 1 );
    vec3 colorMid = vec3( 0.2, 1, 0.2 );
    vec3 colorFast = vec3( 1, 0.2, 0 );

    t = speed * speed;
    t = sqrt( speed * 2.0 );
    if ( t > 1 ) {
        t = 1;
    }

    if ( t < 0.5 ) {
        float s = t / 0.5;  // remap to [0,1]
        v_color = mix( colorSlow, colorMid, s );
    } else {
        float s = ( t - 0.5 ) / 0.5;  // remap to [0,1]
        v_color = mix( colorMid, colorFast, s );
    }
#else
    vec3 colorSlow = vec3( 0, 1, 1 );
    vec3 colorFast = vec3( 1, 0, 0 );

    v_color = mix( colorSlow, colorFast, t );
#endif
}
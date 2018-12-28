#version 330

uniform sampler2D s_diffuse;
uniform sampler2D s_position;
uniform sampler2D s_normal;

uniform float	u_lightIntensity;
uniform vec3	u_lightColor;
uniform float	u_maxDist;

in vec4 v_transformedVert;
//in vec2 v_texCoord;
in vec3 v_lightPosition;

out vec4 FragColor;

/*
==========================
main
 It's important to note that these lighting calculations are taking place in world space
==========================
*/
void main() {
	// calculate the texture coordinate lookup into the geometry buffer
	vec2 v_texCoord = vec2( v_transformedVert.x, v_transformedVert.y ) / v_transformedVert.w;
	v_texCoord = v_texCoord * 0.5 + 0.5;
	
    //
    // Get the g-buffer data
    //
    vec3 diffuse    = texture( s_diffuse, v_texCoord ).rgb;
    vec3 position   = texture( s_position, v_texCoord ).rgb;
    vec3 normal     = texture( s_normal, v_texCoord ).rgb;

    //
    //  Perform the normal mapping lighting
    //
     
    // Calculate the direction of the light from the fragment to the light
    vec3 rayToLight = v_lightPosition.xyz - position.xyz;
	
	float dist = length( rayToLight );
	float t = dist / u_maxDist;
    
    // Calculate the inverse distance squared
    float invDistanceSquare = 1.0 / dot( rayToLight, rayToLight );
    rayToLight = normalize( rayToLight );

	// Include a linear fall off so that we don't see that hard edge of the light
	invDistanceSquare *= mix( 1.0, 0.0, t );
    
    // calculate the brightness
    float dot3          = max( dot( normal, rayToLight ), 0.0 );
    float brightness    = u_lightIntensity * dot3 * invDistanceSquare;
    
    // light the fragment
    FragColor.rgb = diffuse * brightness * u_lightColor;
	FragColor.a = 1.0;
	
	//FragColor.r = 0.1;
}

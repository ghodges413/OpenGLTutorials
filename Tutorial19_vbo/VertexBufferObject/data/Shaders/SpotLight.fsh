#version 330

in vec2 v_texCoord;
in vec3 v_normal;
in vec3 v_tangent;
in vec3 v_posWorldSpace;
in vec4 v_posLightSpace;

uniform sampler2D s_textureDiffuse;
uniform sampler2D s_textureNormals;

uniform float   u_lightIntensity;
uniform float   u_lightCosAngle;
uniform vec3    u_lightPosition;
uniform vec3    u_lightDir;
uniform vec3    u_lightColor;

// the light's projection matrix
uniform sampler2D s_light_depth_buffer;
uniform sampler2D s_light_proj_texture;

/*
 ==========================
 main
 It's important to note that these lighting calculations are taking place in world space
 ==========================
 */
void main() {
    vec3 colorDiffuse = texture( s_textureDiffuse, v_texCoord ).rgb;
	vec3 colorNormal = texture( s_textureNormals, v_texCoord ).rgb;
    
    // Copy the diffuse color out
    vec4 color = vec4( colorDiffuse.rgb, 1.0 );
	
	// Get the normal from the normal map
	vec3 normal = normalize( ( colorNormal.xyz - 0.5 ) * 2.0 );
	
	vec3 norm = normalize( v_normal );
	vec3 tang = normalize( v_tangent );
	vec3 bitang = normalize( cross( tang, norm ) );
	
	// Transform the normal map's normal from tangent space to world space
	normal = normal.x * tang + normal.y * bitang + normal.z * norm;
    
    
    //
    //  Perform the normal mapping lighting
    //
    
    // Calculate the ray of the light from the fragment to the light
    vec3 rayToLight = u_lightPosition.xyz - v_posWorldSpace.xyz;
    
    // Calculate the inverse distance squared
    float invDistanceSquare = 1.0 / dot( rayToLight, rayToLight );
    rayToLight = normalize( rayToLight );
    
    // calculate the brightness
    float dot3 = max( dot( normal, rayToLight ), 0.0 );
    float brightness    = u_lightIntensity * dot3 * invDistanceSquare;
	
	// Assuming that the light ray and light direction is normalized
    if ( dot( rayToLight, -u_lightDir ) < u_lightCosAngle ) {
        brightness = 0.0;
    }
    
    // light the fragment
    color.rgb *= brightness * u_lightColor;
   
    
    //
    //  Handle shadow mapping
    //
    
    // Get the color of the projected texture (the spotlight texture)
    vec4 projectedFrag = texture2DProj( s_light_proj_texture, v_posLightSpace );
    color *= projectedFrag;
    
    // Check depth value for shadowing  (this should be done first)
    float fragDepth = v_posLightSpace.z / v_posLightSpace.w;
    float lightDepth = texture2DProj( s_light_depth_buffer, v_posLightSpace ).z;
	float shadow = 1.0;
    if ( lightDepth < fragDepth + 0.0001 ) {
        shadow = 0.0;
    }
    
    gl_FragColor = color * shadow;
}

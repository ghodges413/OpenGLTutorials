// This lines tells OpenGL which version of GLSL we're using
#version 430

struct pointLight_t {
	vec4 mSphere;	// xyz = position, w = radius, these are in world space coordinates
	vec4 mColor;		// rgb = color, a = intensity
};

layout( std430, binding = 1 ) buffer bufferLights {
	pointLight_t pointLights[];
};

const int gMaxLightsPerTile = 32;
struct lightList_t {
	int mNumLights;
	int mLightIds[ gMaxLightsPerTile ];
};

layout( std430, binding = 2 ) buffer bufferLightList {
	lightList_t lightLists[];
};

// This is the outgoing color that'll be the final color for output
out vec4 FragColor;

uniform int workGroupSize;// = 16;
uniform int screenWidth;// = 1200;

in vec2 v_texCoord;
in vec3 v_normal;
in vec3 v_tangent;
in vec3 v_posWorldSpace;

uniform sampler2D s_textureDiffuse;
uniform sampler2D s_textureNormals;

/*
==========================
main
==========================
*/
void main() {
	ivec2 tiledCoord = ivec2( gl_FragCoord.xy ) / workGroupSize;
	int workGroupID = tiledCoord.x + tiledCoord.y * screenWidth / workGroupSize;
	
	vec3 colorDiffuse = texture( s_textureDiffuse, v_texCoord ).rgb;
	vec3 colorNormal = texture( s_textureNormals, v_texCoord ).rgb;
	
	// Get the normal from the normal map
	vec3 normal = normalize( ( colorNormal.xyz - 0.5 ) * 2.0 );
	
	vec3 norm = normalize( v_normal );
	vec3 tang = normalize( v_tangent );
	vec3 bitang = normalize( cross( norm, tang ) );
	
	// Transform the normal map's normal from tangent space to world space
	normal = normal.x * tang + normal.y * bitang + normal.z * norm;
	
	FragColor.rgb = vec3( 0.0 );
	FragColor.a = 1.0;
	
	int numLights = lightLists[ workGroupID ].mNumLights;
	for ( int i = 0; i < numLights; ++i ) {
		int lightID				= lightLists[ workGroupID ].mLightIds[ i ];
		
		vec4 lightSphere	= pointLights[ lightID ].mSphere;
		vec4 lightColor		= pointLights[ lightID ].mColor;
		
		vec3 rayToLight = lightSphere.xyz - v_posWorldSpace.xyz;
		
		float dist = length( rayToLight );
		float t = dist / lightSphere.w;
		
		if ( dist > lightSphere.w ) {
			continue;
		}
		
		// Calculate the inverse distance squared
		float invDistanceSquare = 1.0 / dot( rayToLight, rayToLight );
		rayToLight = normalize( rayToLight );
		
		// Include a linear fall off so that we don't see that hard edge of the light
		invDistanceSquare *= mix( 1.0, 0.0, t );
		
		// calculate the brightness
		float dot3          = max( dot( normal, rayToLight ), 0.0 );
		float brightness    = lightColor.a * dot3 * invDistanceSquare;
		
		// light the fragment
		FragColor.rgb += colorDiffuse * brightness * lightColor.rgb;
	}
}

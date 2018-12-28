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

/*
==========================
main
==========================
*/
void main() {
	ivec2 tiledCoord = ivec2( gl_FragCoord.xy ) / workGroupSize;
	int workGroupID = tiledCoord.x + tiledCoord.y * screenWidth / workGroupSize;
	
	// Use the texture coordinates to look up the texel value in the texture and copy out to the fragment
	FragColor.rgb = vec3( 0.0 );
	FragColor.r = float( lightLists[ workGroupID ].mNumLights ) / float( gMaxLightsPerTile );
	FragColor.a = 1.0;
}

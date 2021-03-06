 #version 430
 
// gimage* requires binding via glBindImageTexture
// sampler* requires binding via glBindTexture

// ImageStore/Load only works for pure textures.  It does not work for textures attached to an FBO.
// Textures that are attached to an FBO must be passed in as a sampler instead of as an image.
//layout( binding = 1, rgba32f ) uniform readonly image2D textureDiffuse;
//layout( rgba16f ) uniform readonly image2D textureNormal;	// should be in view space
//layout( rgba8ui ) uniform readonly uimage2D textureSpecular;
//layout( rgba32f ) uniform readonly image2D texturePosition;	// should be in view space

// The incoming G-Buffer
layout( binding = 0 ) uniform sampler2D textureDepth;

struct pointLight_t {
	vec4 mSphere;	// xyz = position, w = radius, these are in world space coordinates
	vec4 mColor;	// rgb = color, a = intensity
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

uniform mat4 matView;
uniform mat4 matProjInv;

uniform int screenWidth;
uniform int screenHeight;

uniform int maxLights;

#define WORK_GROUP_SIZE 16

layout ( local_size_x = WORK_GROUP_SIZE, local_size_y = WORK_GROUP_SIZE, local_size_z = 1 ) in;

#define MAX_UNSIGNED_INT 0xFFFFFFFF
#define MAX_INT 0x7FFFFFFF

// This shares memory between the threads in the current workgroup
// Initialization of shared variables is not allowed on declaration
shared uint minDepth;//		= MAX_INT;
shared uint maxDepth;//		= 0;

/*
 =======================
 IsPointInBox
 =======================
 */
bool IsPointInBox( in vec3 point, in vec3 boundsMin, in vec3 boundsMax ) {
	if ( point.x > boundsMax.x || point.x < boundsMin.x ) {
		return false;
	}
	if ( point.y > boundsMax.y || point.y < boundsMin.y ) {
		return false;
	}
	if ( point.z > boundsMax.z || point.z < boundsMin.z ) {
		return false;
	}
	
	return true;
}

/*
 ==========================
 main
 ==========================
 */
void main() {
	//
	// Initialize the shared variables
	//
	
	minDepth		= MAX_INT;
	maxDepth		= 0;
	barrier();
	
	//
	//	Determine the min/max depth values of this tile
	//
	
	//gl_LocalInvocationID
	//gl_WorkGroupID
	ivec2 storePos	= ivec2( gl_GlobalInvocationID.xy );
	vec2 st			= ( vec2( gl_GlobalInvocationID.xy ) + vec2( 0.5 ) ) / vec2( screenWidth, screenHeight );
	
	float depth	= texture( textureDepth, st ).r;
	
	uint uDepth = uint( depth * MAX_INT );

	atomicMin( minDepth, uDepth );
	atomicMax( maxDepth, uDepth );
	
	// Block until all threads finish the above code
	barrier();
	
	//
	//	Build the bounds of this tile
	//

	// Map the depths to -1,1 normalized device coordinates
	float minDepthZ			= 2.0 * float( minDepth ) / float( MAX_INT ) - 1.0;
	float maxDepthZ			= 2.0 * float( maxDepth ) / float( MAX_INT ) - 1.0;
	
	// Map the integer coordinates to -1,1 normalized device coordinates
	const float numGroupsX	= float( screenWidth ) / float( WORK_GROUP_SIZE );
	const float xWidth		= float( WORK_GROUP_SIZE ) / float( screenWidth );
	float x					= 2.0 * ( float( gl_WorkGroupID.x ) + 0.5 ) / numGroupsX - 1.0;	// map from 0,1 to -1,1
	float xmin				= x - xWidth;
	float xmax				= x + xWidth;
	
	// Map the integer coordinates to -1,1 normalized device coordinates
	const float numGroupsY	= float( screenHeight ) / float( WORK_GROUP_SIZE );
	const float yWidth		= float( WORK_GROUP_SIZE ) / float( screenHeight );
	float y					= 2.0 * ( float( gl_WorkGroupID.y ) + 0.5 ) / numGroupsY - 1.0;	// map from 0,1 to -1,1
	float ymin				= y - yWidth;
	float ymax				= y + yWidth;
	
	// Calculate bounds in -1,1 normalized device coordinates
	vec3 boundsMin	= vec3( xmin, ymin, minDepthZ );
	vec3 boundsMax	= vec3( xmax, ymax, maxDepthZ );
	
	// Transform the bounds from NDC to view space
	vec4 mins	= matProjInv * vec4( boundsMin, 1.0 );
	vec4 maxs	= matProjInv * vec4( boundsMax, 1.0 );
	
	boundsMin	= mins.xyz / mins.w;
	boundsMax	= maxs.xyz / maxs.w;

	// Hack fix for ensuring the bounds aren't backwards
	vec3 center	= 0.5 * ( boundsMin + boundsMax );
	vec3 diff	= 0.5 * abs( boundsMax - boundsMin );
	
	boundsMin	= center - diff;
	boundsMax	= center + diff;
	
	// Convert the bounding box to a bounding sphere
	//vec4 boundsSphere = vec4( center.xyz, length( diff ) );

	const int threadsPerWorkGroup = WORK_GROUP_SIZE * WORK_GROUP_SIZE;
	
	int threadID = int( gl_LocalInvocationID.x + gl_LocalInvocationID.y * WORK_GROUP_SIZE );
	int tileID = int( gl_WorkGroupID.x + gl_WorkGroupID.y * screenWidth / WORK_GROUP_SIZE );
	
	//
	//	Determine the lights that intersect with this tile
	//
	for ( int i = threadID; i < maxLights; i += threadsPerWorkGroup ) {
		vec4 lightPointViewSpace	= matView * vec4( pointLights[ i ].mSphere.xyz, 1.0 );
		vec4 lightSphere			= vec4( lightPointViewSpace.xyz, pointLights[ i ].mSphere.w );

		// Skip this light if the tile is not intersecting the sphere
		if ( false == IsPointInBox( lightSphere.xyz, boundsMin - vec3( lightSphere.w ), boundsMax + vec3( lightSphere.w ) ) ) {
			continue;
		}
		
		int id = atomicAdd( lightLists[ tileID ].mNumLights, 1 );
		lightLists[ tileID ].mLightIds[ id ] = i;
	}
 }
 
 
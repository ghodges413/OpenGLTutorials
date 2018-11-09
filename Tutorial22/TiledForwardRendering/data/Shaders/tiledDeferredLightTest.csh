 #version 430
 
struct pointLight_t {
	vec4 sphere;	// xyz = position, w = radius, these are in world space coordinates
	vec4 color;
};

// Destination Render Texture
layout ( binding = 0 ) uniform writeonly image2D destTex;

// gimage* requires binding via glBindImageTexture
// sampler* requires binding via glBindTexture

// ImageStore/Load only works for pure textures.  It does not work for textures attached to an FBO.
// Textures that are attached to an FBO must be passed in as a sampler instead of as an image.
//layout( binding = 1, rgba32f ) uniform readonly image2D textureDiffuse;
//layout( rgba16f ) uniform readonly image2D textureNormal;	// should be in view space
//layout( rgba8ui ) uniform readonly uimage2D textureSpecular;
//layout( rgba32f ) uniform readonly image2D texturePosition;	// should be in view space

// The incoming G-Buffer
layout( binding = 1 ) uniform sampler2D textureDiffuse;
layout( binding = 2 ) uniform sampler2D texturePosition;
layout( binding = 3 ) uniform sampler2D textureNormal;
layout( binding = 4 ) uniform sampler2D textureSpecular;
layout( binding = 5 ) uniform sampler2D textureDepth;

layout( std430, binding = 6 ) buffer bufferLights {
	pointLight_t pointLights[];
};

uniform mat4 matView;
uniform mat4 matProjInv;

uniform int screenWidth;
uniform int screenHeight;

const int maxLights = 4096;

#define WORK_GROUP_SIZE 32

layout ( local_size_x = WORK_GROUP_SIZE, local_size_y = WORK_GROUP_SIZE, local_size_z = 1 ) in;

#define MAX_UNSIGNED_INT 0xFFFFFFFF
#define MAX_INT 0x7FFFFFFF

// This shares memory between the threads in the current workgroup
shared uint minDepth			= MAX_UNSIGNED_INT;
shared uint maxDepth		= 0;
shared int pointLightCount	= 0;
shared int pointLightIndex[ maxLights ];

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
	//gl_LocalInvocationID
	//gl_WorkGroupID
	ivec2 storePos	= ivec2( gl_GlobalInvocationID.xy );
	vec2 st				= ( vec2( gl_GlobalInvocationID.xy ) + vec2( 0.5 ) ) / vec2( screenWidth, screenHeight );
	
	vec4 finalColor	= vec4( 0.0, 0.0, 0.0, 1.0 );
	vec4 fragDiffuse	= texture( textureDiffuse,	st );
	vec4 fragPos		= texture( texturePosition,	st );
	vec4 fragSpec	= texture( textureSpecular,	st );
	vec4 fragNorm	= texture( textureNormal,	st );
	float depth			= texture( textureDepth,		st ).r;
	
	uint uDepth = uint( depth * MAX_UNSIGNED_INT );

	atomicMin( minDepth,	uDepth );
	atomicMax( maxDepth,	uDepth );
	
	// Block until all threads finish the above code
	barrier();

	// Map the depths to -1,1 normalized device coordinates
	float minDepthZ				= 2.0 * float( minDepth ) / float( MAX_UNSIGNED_INT ) - 1.0;
	float maxDepthZ			= 2.0 * float( maxDepth ) / float( MAX_UNSIGNED_INT ) - 1.0;
	
	// Map the integer coordinates to -1,1 normalized device coordinates
	const float numGroupsX	= float( screenWidth ) / float( WORK_GROUP_SIZE );
	const float xWidth			= float( WORK_GROUP_SIZE ) / float( screenWidth );
	float x							= 2.0 * ( float( gl_WorkGroupID.x ) + 0.5 ) / numGroupsX - 1.0;	// map from 0,1 to -1,1
	float xmin						= x - xWidth;
	float xmax						= x + xWidth;
	
	// Map the integer coordinates to -1,1 normalized device coordinates
	const float numGroupsY	= float( screenHeight ) / float( WORK_GROUP_SIZE );
	const float yWidth			= float( WORK_GROUP_SIZE ) / float( screenHeight );
	float y							= 2.0 * ( float( gl_WorkGroupID.y ) + 0.5 ) / numGroupsY - 1.0;	// map from 0,1 to -1,1
	float ymin						= y - yWidth;
	float ymax						= y + yWidth;
	
	// Calculate bounds in -1,1 normalized device coordinates
	vec3 boundsMin		= vec3( xmin, ymin, minDepthZ );
	vec3 boundsMax	= vec3( xmax, ymax, maxDepthZ );
	
	// Transform the bounds from NDC to view space
	vec4 mins		= matProjInv * vec4( boundsMin, 1.0 );
	vec4 maxs	= matProjInv * vec4( boundsMax, 1.0 );
	
	boundsMin	= mins.xyz / mins.w;
	boundsMax	= maxs.xyz / maxs.w;

	// Hack fix for ensuring the bounds aren't backwards
	vec3 center	= 0.5 * ( boundsMin + boundsMax );
	vec3 diff		= 0.5 * abs( boundsMax - boundsMin );
	
	boundsMin	= center - diff;
	boundsMax	= center + diff;

	const int threadsPerWorkGroup = WORK_GROUP_SIZE * WORK_GROUP_SIZE;
	
	int threadID = int( gl_LocalInvocationID.x + gl_LocalInvocationID.y * WORK_GROUP_SIZE );
	
	//
	//	Determine the lights that intersect with this tile
	//
	for ( int i = threadID; i < maxLights; i += threadsPerWorkGroup ) {
		vec4 lightPointViewSpace	= matView * vec4( pointLights[ i ].sphere.xyz, 1.0 );
		vec4 lightSphere				= vec4( lightPointViewSpace.xyz, pointLights[ i ].sphere.w );

		// Skip this light if the fragment is not intersecting the sphere
		if ( false == IsPointInBox( lightSphere.xyz, boundsMin - vec3( lightSphere.w ), boundsMax + vec3( lightSphere.w ) ) ) {
			continue;
		}
		
		// Add this light to the light list
		int id = atomicAdd( pointLightCount, 1 );
		pointLightIndex[ id ] = i;
	}
	
	// Block until all threads finish the above code
	barrier();
	
	//
	//	Light the fragment with all the lights that affect it
	//
	for ( int i = 0; i < pointLightCount; ++i ) {
		int id = pointLightIndex[ i ];
		vec4 lightPointViewSpace	= matView * vec4( pointLights[ id ].sphere.xyz, 1.0 );
		vec4 lightSphere				= vec4( lightPointViewSpace.xyz, pointLights[ id ].sphere.w );
		vec3 rayFragToLight			= lightSphere.xyz - fragPos.xyz;
		float fragDistance				= length( rayFragToLight );		
		
		// Skip this light if the fragment is not intersecting the sphere
		if ( fragDistance > lightSphere.w ) {
			continue;
		}
		rayFragToLight	= normalize( rayFragToLight );
		
		// Calculate the linear falloff
		float linearFalloff	= clamp( ( 0.9 - fragDistance / lightSphere.w ), 0.0, 1.0 );
		
		//
		// Accumulate the blinn color
		//
		float blinnIntensity	= max( 0.0, dot( rayFragToLight, fragNorm.xyz ) );
		finalColor.rgb			+= fragDiffuse.rgb * pointLights[ id ].color.rgb * blinnIntensity * linearFalloff;

		//
		// Accumulate the specular color
		//
		vec3 vSurfaceToCamera		= -1.0 * normalize( fragPos.xyz );
		vec3 vReflectedLightRay		= ( 2.0 * dot( fragNorm.xyz, rayFragToLight ) * fragNorm.xyz ) - rayFragToLight;
		
		// Add the specular color intensity
		float   specularBrightness	= dot( pointLights[ id ].color.rgb, fragSpec.rgb );
		specularBrightness			*= max( 0.0, dot( vSurfaceToCamera, vReflectedLightRay ) );
		
		// Add the light's color
		finalColor.rgb += pointLights[ id ].color.rgb * specularBrightness * linearFalloff;
	}
	
	//finalColor.r = float( pointLightCount ) / 32.0;
	imageStore( destTex, storePos, finalColor );
 }
 
 
#version 450

// This is the incoming vertex data
layout( location = 0 ) in vec3 position;
layout( location = 1 ) in vec2 st;
layout( location = 2 ) in vec4 normal;
layout( location = 3 ) in vec4 tangent;
layout( location = 4 ) in vec4 color;


uniform mat4 mvp;

uniform mat4 matBias;
uniform mat4 projectorMat;
uniform vec3 projectorPos;
uniform float oceanPlane;

layout( binding = 0, rgba32f ) uniform readonly image2D oceanHeights0;
layout( binding = 1, rgba32f ) uniform readonly image2D oceanHeights1;
layout( binding = 2, rgba32f ) uniform readonly image2D oceanHeights2;
layout( binding = 3, rgba32f ) uniform readonly image2D oceanHeights3;

uniform vec2 dimensions[ 4 ];

out vec2 v_texCoord;
out vec3 v_worldSpacPos;
out vec2 v_texCoords[ 4 ];
out vec4 v_weights;

/*
 ==========================
 GetDisplacementVector
 ==========================
 */
vec3 GetDisplacementVector( in vec3 oceanPos, in int oceanIdx, in vec2 dimension, out vec2 normalizedST ) {
	int dimSamples = int( dimension.x );
	float dimPhysical = dimension.y;
	
	// get the normalized sample coordinates
	vec2 st = fract( oceanPos.xy / dimPhysical );
	if ( st.x < 0.0f ) {
		st.x += 1.0f;
	}
	if ( st.y < 0.0f ) {
		st.y += 1.0f;
	}
	normalizedST = ( oceanPos.xy / dimPhysical );

	// Get the non-normalized sample coordinates
	st *= dimSamples;
	ivec2 sti[ 4 ];
	sti[ 0 ] = ivec2( st.xy ) + ivec2( 0, 0 );
	sti[ 1 ] = ivec2( st.xy ) + ivec2( 1, 0 );
	sti[ 2 ] = ivec2( st.xy ) + ivec2( 1, 1 );
	sti[ 3 ] = ivec2( st.xy ) + ivec2( 0, 1 );
	
	sti[ 0 ].x %= dimSamples;
	sti[ 0 ].y %= dimSamples;	
	sti[ 1 ].x %= dimSamples;
	sti[ 1 ].y %= dimSamples;	
	sti[ 2 ].x %= dimSamples;
	sti[ 2 ].y %= dimSamples;	
	sti[ 3 ].x %= dimSamples;
	sti[ 3 ].y %= dimSamples;
	
	// Calculate the weights
	st = fract( st );
	float weights[ 4 ];
	weights[ 0 ] = ( 1.0f - st.x ) * ( 1.0f - st.y );
	weights[ 1 ] = ( st.x ) * ( 1.0f - st.y );
	weights[ 2 ] = ( st.x ) * ( st.y );
	weights[ 3 ] = ( 1.0f - st.x ) * ( st.y );	
	
	// Get the weighted displacements
	vec3 displacements = vec3( 0.0f );
	if ( oceanIdx == 0 ) {
		displacements += imageLoad( oceanHeights0, sti[ 0 ] ).xyz * weights[ 0 ];
		displacements += imageLoad( oceanHeights0, sti[ 1 ] ).xyz * weights[ 1 ];
		displacements += imageLoad( oceanHeights0, sti[ 2 ] ).xyz * weights[ 2 ];
		displacements += imageLoad( oceanHeights0, sti[ 3 ] ).xyz * weights[ 3 ];
	} else if ( oceanIdx == 1 ) {
		displacements += imageLoad( oceanHeights1, sti[ 0 ] ).xyz * weights[ 0 ];
		displacements += imageLoad( oceanHeights1, sti[ 1 ] ).xyz * weights[ 1 ];
		displacements += imageLoad( oceanHeights1, sti[ 2 ] ).xyz * weights[ 2 ];
		displacements += imageLoad( oceanHeights1, sti[ 3 ] ).xyz * weights[ 3 ];
	} else if ( oceanIdx == 2 ) {
		displacements += imageLoad( oceanHeights2, sti[ 0 ] ).xyz * weights[ 0 ];
		displacements += imageLoad( oceanHeights2, sti[ 1 ] ).xyz * weights[ 1 ];
		displacements += imageLoad( oceanHeights2, sti[ 2 ] ).xyz * weights[ 2 ];
		displacements += imageLoad( oceanHeights2, sti[ 3 ] ).xyz * weights[ 3 ];
	} else {
		displacements += imageLoad( oceanHeights3, sti[ 0 ] ).xyz * weights[ 0 ];
		displacements += imageLoad( oceanHeights3, sti[ 1 ] ).xyz * weights[ 1 ];
		displacements += imageLoad( oceanHeights3, sti[ 2 ] ).xyz * weights[ 2 ];
		displacements += imageLoad( oceanHeights3, sti[ 3 ] ).xyz * weights[ 3 ];
	}
	
	return displacements;
}

/*
 ==========================
 CalculateWeights
 ==========================
 */
vec4 CalculateWeights( in vec3 oceanPos, in vec3 camPos ) {
	vec4 weights = vec4( 1.0f, 0.0f, 0.0f, 0.0f );
	
	vec4 distances;
	for ( int i = 0; i < 4; i++ ) {
		distances[ i ] = dimensions[ i ].y * 0.25f;
	}
	
	float dist = length( oceanPos.xy - camPos.xy );
	weights[ 0 ] = clamp( distances[ 0 ] - dist / distances[ 0 ] * 4.0f, 0.0f, 1.0f );
	weights[ 1 ] = min( dist / distances[ 1 ], 1.0f );
	weights[ 2 ] = min( dist / distances[ 2 ], 1.0f );
	weights[ 3 ] = min( dist / distances[ 3 ], 1.0f );
	weights = normalize( weights );
	//weights = vec4( 0.0f, 0.0f, 0.0f, 1.0f );
	
	return weights;
}

/*
 ==========================
 main
 ==========================
 */
void main() {
	// grab vertex position
	vec4 vert = vec4( position.xyz, 1.0f );	
	vert = matBias * vert;

	//
	// Get the world space coordinate of the screen space grid
	//
	vec4 worldSpaceGridPos;
	{
		vec4 pos = projectorMat * vert;
		pos.x /= pos.w;
		pos.y /= pos.w;
		pos.z /= pos.w;
		worldSpaceGridPos = vec4( pos.xyz, 1.0f );
	}
	
	//
	// Ray cast this vert onto the ocean plane
	//
	vec4 oceanPos;
	{
		vec3 ray = normalize( worldSpaceGridPos.xyz - projectorPos.xyz );
		//z = projectorPos.z + t * ray.z = oceanPlane
		float t = ( oceanPlane - projectorPos.z ) / ray.z;
		oceanPos.xyz = t * ray + projectorPos.xyz;
		oceanPos.w = 1.0f;
	}
	
	// Find the displacements
	vec2 textureCoords[ 4 ];
	vec3 displacements[ 4 ];
	displacements[ 0 ] = GetDisplacementVector( oceanPos.xyz, 0, dimensions[ 0 ], textureCoords[ 0 ] );
	displacements[ 1 ] = GetDisplacementVector( oceanPos.xyz, 1, dimensions[ 1 ], textureCoords[ 1 ] );
	displacements[ 2 ] = GetDisplacementVector( oceanPos.xyz, 2, dimensions[ 2 ], textureCoords[ 2 ] );
	displacements[ 3 ] = GetDisplacementVector( oceanPos.xyz, 3, dimensions[ 3 ], textureCoords[ 3 ] );
	
	vec4 weights = CalculateWeights( oceanPos.xyz, projectorPos.xyz );
	
	// Displace the ocean vert
	for ( int i = 0; i < 4; i++ ) {
		oceanPos.xyz += displacements[ i ] * weights[ i ];
	}
	
	// transform vertex
	gl_Position = mvp * oceanPos;
	
	// copy texture coordinate
	v_worldSpacPos = oceanPos.xyz;
	v_texCoords[ 0 ] = textureCoords[ 0 ];
	v_texCoords[ 1 ] = textureCoords[ 1 ];
	v_texCoords[ 2 ] = textureCoords[ 2 ];
	v_texCoords[ 3 ] = textureCoords[ 3 ];
	v_weights = weights;
}

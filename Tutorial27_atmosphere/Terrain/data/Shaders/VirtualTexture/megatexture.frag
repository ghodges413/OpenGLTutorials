// This line tells OpenGL which version of GLSL we're using
#version 430

in vec2 v_texCoord;
in vec3 v_normal;

uniform sampler2D pageCoordTexture;
uniform sampler2D scaleBiasTexture;
uniform sampler2D physicalTexture;

layout(location = 0) out vec4 diffuseColor;

/*
==========================
ComputeMipLevel
==========================
*/
float ComputeMipLevel( in vec2 virtCoords ) {
	const float maxAniso = 4;
	const float maxAnisoLog2 = log2( maxAniso );
	const float virtPagesWide = 1024;
	const float pageWidth = 128;
	const float pageBorder = 4;
	const float virtTexelsWide = virtPagesWide * ( pageWidth - 2 * pageBorder );

	vec2 texcoords = virtCoords.xy * virtTexelsWide;
	
	vec2 dx = dFdx( texcoords );
	vec2 dy = dFdy( texcoords );
	
	float px = dot( dx, dx );
	float py = dot( dy, dy );

	float maxLod = 0.5 * log2( max( px, py ) ); // log2(sqrt()) = 0.5*log2()
	float minLod = 0.5 * log2( min( px, py ) );

	float anisoLOD = maxLod - min( maxLod - minLod, maxAnisoLog2 );
	return anisoLOD;
}

// ======================================================
// Converts the virtual texture coordinates
//	to the appropriate physical texture coordinates
// ======================================================
vec2 PhysicalCoordinates( in vec2 virtCoord ) {
	// The pageCoordinate will give a [0, 0.125] range.
	// This is because the phyiscal texture is only 32 slots wide, 8 bits per id, so 32/256 is 0.125
	// So we need to multiply by 256/32 (or 8) to expand it to the full [0,1] range
	vec2 pageCoord = texture2D( pageCoordTexture, virtCoord ).zw * 256.0f / 32.0f;
	vec4 scaleBias = texture2D( scaleBiasTexture, pageCoord );
	vec2 physCoord = virtCoord.xy * scaleBias.x + scaleBias.zw;
	return physCoord;
}

vec2 PhysicalCoordinates( in vec2 virtCoord, in float mip ) {
	// The pageCoordinate will give a [0, 0.125] range.
	// This is because the phyiscal texture is only 32 slots wide, 8 bits per id, so 32/256 is 0.125
	// So we need to multiply by 256/32 (or 8) to expand it to the full [0,1] range
	vec2 pageCoord = textureLod( pageCoordTexture, virtCoord, mip ).zw * 256.0f / 32.0f;
	vec4 scaleBias = texture2D( scaleBiasTexture, pageCoord );
	vec2 physCoord = virtCoord.xy * scaleBias.x + scaleBias.zw;
	return physCoord;
}

/*
==========================
main
==========================
*/
void main() {
#if 1
	vec2 physCoords = PhysicalCoordinates( v_texCoord );
#else	
	float mipLevel = ComputeMipLevel( v_texCoord );
	vec2 physCoords = PhysicalCoordinates( v_texCoord, mipLevel );
#endif
	diffuseColor = texture2D( physicalTexture, physCoords );

	float light = dot( normalize( vec3( 1, 1, 1 ) ), v_normal ) * 0.75f + 0.25f;
	//light = 1.0;

	diffuseColor.rgb *= light;
}

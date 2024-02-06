// This line tells OpenGL which version of GLSL we're using
#version 430

in vec2 v_texCoord;
in float v_distance;	// distance to camera

uniform int virtualPagesWide;
uniform int pageWidth;
//uniform int pageCount;
uniform float virtualDistance;

layout( location = 0 ) out vec4 diffuseColor;

/*
==========================
ComputeMipLevel
==========================
*/
float ComputeMipLevel( in vec2 virtCoords ) {
	const float maxAniso = 4;
	const float maxAnisoLog2 = log2( maxAniso );
	//const float virtPagesWide = 1024;
	//const float pageWidth = 128;
	const int pageBorder = 4;
	const float virtTexelsWide = virtualPagesWide * ( pageWidth - 2 * pageBorder );

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

/*
==========================
main
==========================
*/
void main() {
	// Check if this fragment is too far from the camera
	if ( v_distance > virtualDistance ) {
		discard;
	}

	// Compute mip-level and virtual page coords:	
	float mipLevel = ComputeMipLevel( v_texCoord );
	vec2 pageCoords = floor( v_texCoord * float( virtualPagesWide ) ); // VT page coordinates [0,1023]

	// 8-bits is not enough for 1024 pages.
	// So we pack the lower bits together into the blue color channel.
	int x = int( pageCoords.x );
	int y = int( pageCoords.y );

	int xhigh = x >> 4;	// top 8 bits
	int xlow = x & 15; // lower 4 bits

	int yhigh = y >> 4;	// top 8 bits
	int ylow = y & 15; // lower 4 bits

	int mixed = ( xlow << 4 ) | ylow;

	vec4 page_id = vec4( xhigh, yhigh, mixed, mipLevel );
	diffuseColor = page_id / 255.0;

	//vec4 page_id = vec4( x, y, 0, mipLevel );
	//diffuseColor = page_id / 65535.0;
}

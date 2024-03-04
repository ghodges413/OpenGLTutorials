//
//  BlockCompression.cpp
//
#include "Graphics/BlockCompression.h"
#include "Graphics/Graphics.h"
#include "Math/Math.h"
#include "Math/Vector.h"
#include "Math/Matrix.h"
#include "Math/Bounds.h"
#include <stdio.h>

/*
==================================================================================================

From JP Waveren's papers
DXT1/BC1

==================================================================================================
*/


//stypedef unsigned char uint8;
// typedef unsigned short uint16;
// typedef unsigned int uint32;
uint8 *globalOutData;

#define INSET_SHIFT 4 // inset the bounding box with ( range >> shift )

#define C565_5_MASK 0xF8 // 0xFF minus last three bits
#define C565_6_MASK 0xFC // 0xFF minus last two bits

#define MAX_INT 0x7FFFFFFF;


void ExtractBlock( const uint8 *inPtr, int width, uint8 *colorBlock ) {
	for ( int j = 0; j < 4; j++ ) { 
		memcpy( &colorBlock[ j * 4 * 4 ], inPtr, 4 * 4 );
		inPtr += width * 4;
	}
}

uint16 ColorTo565( const uint8 *color ) { 
	return ( ( color[ 0 ] >> 3 ) << 11 ) | ( ( color[ 1 ] >> 2 ) << 5 ) | ( color[ 2 ] >> 3 ); 
}

void EmitByte( uint8 b ) {
	globalOutData[0] = b;
	globalOutData += 1;
}
void EmitWord( uint16 s ) {
	globalOutData[0] = ( s >> 0 ) & 255;
	globalOutData[1] = ( s >> 8 ) & 255;
	globalOutData += 2;
}
void EmitDoubleWord( uint32 i ) {
	globalOutData[0] = ( i >> 0 ) & 255;
	globalOutData[1] = ( i >> 8 ) & 255;
	globalOutData[2] = ( i >> 16 ) & 255;
	globalOutData[3] = ( i >> 24 ) & 255;
	globalOutData += 4;
}

int ColorDistance( const uint8 *c1, const uint8 *c2 ) {
	return ( ( c1[0] - c2[0] ) * ( c1[0] - c2[0] ) ) +
	( ( c1[1] - c2[1] ) * ( c1[1] - c2[1] ) ) +
	( ( c1[2] - c2[2] ) * ( c1[2] - c2[2] ) );
}
void SwapColors( uint8 *c1, uint8 *c2 ) {
	uint8 tm[3];
	memcpy( tm, c1, 3 );
	memcpy( c1, c2, 3 );
	memcpy( c2, tm, 3 );
}

#if 0
void GetMinMaxColors3( const uint8 *colorBlock, uint8 *minColor, uint8 *maxColor ) {
	int maxDistance = -1;
	for ( int i = 0; i < 64 - 4; i += 4 ) {
		for ( int j = i + 4; j < 64; j += 4 ) {
			int distance = ColorDistance( &colorBlock[i], &colorBlock[j] );
			if ( distance > maxDistance ) {
				maxDistance = distance;
				memcpy( minColor, colorBlock+i, 3 );
				memcpy( maxColor, colorBlock+j, 3 );
			}
		}
	}
	if ( ColorTo565( maxColor ) < ColorTo565( minColor ) ) {
		SwapColors( minColor, maxColor );
	}
}

#elif 0
int ColorLuminance( const uint8 *color ) {
	return ( color[0] + color[1] * 2 + color[2] );
}
void GetMinMaxColors3( const uint8 *colorBlock, uint8 *minColor, uint8 *maxColor ) {
	int maxLuminance = -1;
	int minLuminance = MAX_INT;
	for ( int i = 0; i < 16; i++ ) {
		int luminance = ColorLuminance( colorBlock+i*4 );
		if ( luminance > maxLuminance ) {
			maxLuminance = luminance;
			memcpy( maxColor, colorBlock+i*4, 3 );
		}
		if ( luminance < minLuminance ) {
			minLuminance = luminance;
			memcpy( minColor, colorBlock+i*4, 3 );
		}
	}
	if ( ColorTo565( maxColor ) < ColorTo565( minColor ) ) {
		SwapColors( minColor, maxColor );
	}
}

#elif 1
void GetMinMaxColors3( const uint8 *colorBlock, uint8 *minColor, uint8 *maxColor ) {
	int i;
	uint8 inset[3];
	minColor[0] = minColor[1] = minColor[2] = 255;
	maxColor[0] = maxColor[1] = maxColor[2] = 0;
	for ( i = 0; i < 16; i++ ) {
		if ( colorBlock[i*4+0] < minColor[0] ) { minColor[0] = colorBlock[i*4+0]; }
		if ( colorBlock[i*4+1] < minColor[1] ) { minColor[1] = colorBlock[i*4+1]; }
		if ( colorBlock[i*4+2] < minColor[2] ) { minColor[2] = colorBlock[i*4+2]; }
		if ( colorBlock[i*4+0] > maxColor[0] ) { maxColor[0] = colorBlock[i*4+0]; }
		if ( colorBlock[i*4+1] > maxColor[1] ) { maxColor[1] = colorBlock[i*4+1]; }
		if ( colorBlock[i*4+2] > maxColor[2] ) { maxColor[2] = colorBlock[i*4+2]; }
	}
	inset[0] = ( maxColor[0] - minColor[0] ) >> INSET_SHIFT;
	inset[1] = ( maxColor[1] - minColor[1] ) >> INSET_SHIFT;
	inset[2] = ( maxColor[2] - minColor[2] ) >> INSET_SHIFT;

	minColor[0] = ( minColor[0] + inset[0] <= 255 ) ? minColor[0] + inset[0] : 255;
	minColor[1] = ( minColor[1] + inset[1] <= 255 ) ? minColor[1] + inset[1] : 255;
	minColor[2] = ( minColor[2] + inset[2] <= 255 ) ? minColor[2] + inset[2] : 255;
	maxColor[0] = ( maxColor[0] >= inset[0] ) ? maxColor[0] - inset[0] : 0;
	maxColor[1] = ( maxColor[1] >= inset[1] ) ? maxColor[1] - inset[1] : 0;
	maxColor[2] = ( maxColor[2] >= inset[2] ) ? maxColor[2] - inset[2] : 0;
}
#endif

#if 1
void EmitColorIndices3( const uint8 *colorBlock, const uint8 *minColor, const uint8 *maxColor ) {
	uint8 colors[4][4];
	unsigned int indices[16];
	colors[0][0] = ( maxColor[0] & C565_5_MASK ) | ( maxColor[0] >> 5 );
	colors[0][1] = ( maxColor[1] & C565_6_MASK ) | ( maxColor[1] >> 6 );
	colors[0][2] = ( maxColor[2] & C565_5_MASK ) | ( maxColor[2] >> 5 );
	colors[1][0] = ( minColor[0] & C565_5_MASK ) | ( minColor[0] >> 5 );
	colors[1][1] = ( minColor[1] & C565_6_MASK ) | ( minColor[1] >> 6 );
	colors[1][2] = ( minColor[2] & C565_5_MASK ) | ( minColor[2] >> 5 );
	colors[2][0] = ( 2 * colors[0][0] + 1 * colors[1][0] ) / 3;
	colors[2][1] = ( 2 * colors[0][1] + 1 * colors[1][1] ) / 3;
	colors[2][2] = ( 2 * colors[0][2] + 1 * colors[1][2] ) / 3;
	colors[3][0] = ( 1 * colors[0][0] + 2 * colors[1][0] ) / 3;
	colors[3][1] = ( 1 * colors[0][1] + 2 * colors[1][1] ) / 3;
	colors[3][2] = ( 1 * colors[0][2] + 2 * colors[1][2] ) / 3;

	for ( int i = 0; i < 16; i++ ) {
		unsigned int minDistance = INT_MAX;
		for ( int j = 0; j < 4; j++ ) {
			unsigned int dist = ColorDistance( &colorBlock[i*4], &colors[j][0] );
			if ( dist < minDistance ) {
				minDistance = dist;
				indices[i] = j;
			}
		}
	}

	uint32 result = 0;
	for ( int i = 0; i < 16; i++ ) {
		result |= ( indices[i] << (unsigned int)( i << 1 ) );
	}

	EmitDoubleWord( result );
}

#else
void EmitColorIndices3( const uint8 *colorBlock, const uint8 *minColor, const uint8 *maxColor ) {
	uint16 colors[4][4];
	uint32 result = 0;
	colors[0][0] = ( maxColor[0] & C565_5_MASK ) | ( maxColor[0] >> 5 );
	colors[0][1] = ( maxColor[1] & C565_6_MASK ) | ( maxColor[1] >> 6 );
	colors[0][2] = ( maxColor[2] & C565_5_MASK ) | ( maxColor[2] >> 5 );
	colors[1][0] = ( minColor[0] & C565_5_MASK ) | ( minColor[0] >> 5 );
	colors[1][1] = ( minColor[1] & C565_6_MASK ) | ( minColor[1] >> 6 );
	colors[1][2] = ( minColor[2] & C565_5_MASK ) | ( minColor[2] >> 5 );
	colors[2][0] = ( 2 * colors[0][0] + 1 * colors[1][0] ) / 3;
	colors[2][1] = ( 2 * colors[0][1] + 1 * colors[1][1] ) / 3;
	colors[2][2] = ( 2 * colors[0][2] + 1 * colors[1][2] ) / 3;
	colors[3][0] = ( 1 * colors[0][0] + 2 * colors[1][0] ) / 3;
	colors[3][1] = ( 1 * colors[0][1] + 2 * colors[1][1] ) / 3;
	colors[3][2] = ( 1 * colors[0][2] + 2 * colors[1][2] ) / 3;
	for ( int i = 15; i >= 0; i-- ) {
		int c0 = colorBlock[i*4+0];
		int c1 = colorBlock[i*4+1];
		int c2 = colorBlock[i*4+2];
		int d0 = abs( colors[0][0] - c0 ) + abs( colors[0][1] - c1 ) + abs( colors[0][2] - c2 );
		int d1 = abs( colors[1][0] - c0 ) + abs( colors[1][1] - c1 ) + abs( colors[1][2] - c2 );
		int d2 = abs( colors[2][0] - c0 ) + abs( colors[2][1] - c1 ) + abs( colors[2][2] - c2 );
		int d3 = abs( colors[3][0] - c0 ) + abs( colors[3][1] - c1 ) + abs( colors[3][2] - c2 );
		int b0 = d0 > d3;
		int b1 = d1 > d2;
		int b2 = d0 > d2;
		int b3 = d1 > d3;
		int b4 = d2 > d3;
		int x0 = b1 & b2;
		int x1 = b0 & b3;
		int x2 = b0 & b4;
		result |= ( x2 | ( ( x0 | x1 ) << 1 ) ) << ( i << 1 );
	}
	EmitDoubleWord( result );
}
#endif

void CompressImageDXT1( const uint8 *inBuf, uint8 *outBuf, int width, int height, int &outputBytes ) {
// 	ALIGN16( uint8 block[64] ); 
// 	ALIGN16( uint8 minColor[4] ); 
// 	ALIGN16( uint8 maxColor[4] ); 
	uint8 block[64];
	uint8 minColor[4];
	uint8 maxColor[4];

	globalOutData = outBuf;
	for ( int j = 0; j < height; j += 4, inBuf += width * 4 * 4 ) {
		for ( int i = 0; i < width; i += 4 ) {
			ExtractBlock( inBuf + i * 4, width, block );
			GetMinMaxColors3( block, minColor, maxColor );
			EmitWord( ColorTo565( maxColor ) );
			EmitWord( ColorTo565( minColor ) );
			EmitColorIndices3( block, minColor, maxColor );
		}
	}
	outputBytes = globalOutData - outBuf;
}


/*
==================================================================================================

DXT5/BC3

==================================================================================================
*/


void GetMinMaxColors4( const uint8 *colorBlock, uint8 *minColor, uint8 *maxColor ) {
	int i;
	uint8 inset[4];
	minColor[0] = minColor[1] = minColor[2] = minColor[3] = 255;
	maxColor[0] = maxColor[1] = maxColor[2] = maxColor[3] = 0;
	for ( i = 0; i < 16; i++ ) {
		if ( colorBlock[i*4+0] < minColor[0] ) { minColor[0] = colorBlock[i*4+0]; }
		if ( colorBlock[i*4+1] < minColor[1] ) { minColor[1] = colorBlock[i*4+1]; }
		if ( colorBlock[i*4+2] < minColor[2] ) { minColor[2] = colorBlock[i*4+2]; }
		if ( colorBlock[i*4+3] < minColor[3] ) { minColor[3] = colorBlock[i*4+3]; }
		if ( colorBlock[i*4+0] > maxColor[0] ) { maxColor[0] = colorBlock[i*4+0]; }
		if ( colorBlock[i*4+1] > maxColor[1] ) { maxColor[1] = colorBlock[i*4+1]; }
		if ( colorBlock[i*4+2] > maxColor[2] ) { maxColor[2] = colorBlock[i*4+2]; }
		if ( colorBlock[i*4+3] > maxColor[3] ) { maxColor[3] = colorBlock[i*4+3]; }
	}
	inset[0] = ( maxColor[0] - minColor[0] ) >> INSET_SHIFT;
	inset[1] = ( maxColor[1] - minColor[1] ) >> INSET_SHIFT;
	inset[2] = ( maxColor[2] - minColor[2] ) >> INSET_SHIFT;
	inset[3] = ( maxColor[3] - minColor[3] ) >> INSET_SHIFT;
	minColor[0] = ( minColor[0] + inset[0] <= 255 ) ? minColor[0] + inset[0] : 255;
	minColor[1] = ( minColor[1] + inset[1] <= 255 ) ? minColor[1] + inset[1] : 255;
	minColor[2] = ( minColor[2] + inset[2] <= 255 ) ? minColor[2] + inset[2] : 255;
	minColor[3] = ( minColor[3] + inset[3] <= 255 ) ? minColor[3] + inset[3] : 255;
	maxColor[0] = ( maxColor[0] >= inset[0] ) ? maxColor[0] - inset[0] : 0;
	maxColor[1] = ( maxColor[1] >= inset[1] ) ? maxColor[1] - inset[1] : 0;
	maxColor[2] = ( maxColor[2] >= inset[2] ) ? maxColor[2] - inset[2] : 0;
	maxColor[3] = ( maxColor[3] >= inset[3] ) ? maxColor[3] - inset[3] : 0;
}


#if 1
void EmitAlphaIndices( const uint8 *colorBlock, const uint8 minAlpha, const uint8 maxAlpha ) {
	uint8 indices[16];
	uint8 alphas[8];
	alphas[0] = maxAlpha;
	alphas[1] = minAlpha;
	alphas[2] = ( 6 * maxAlpha + 1 * minAlpha ) / 7;
	alphas[3] = ( 5 * maxAlpha + 2 * minAlpha ) / 7;
	alphas[4] = ( 4 * maxAlpha + 3 * minAlpha ) / 7;
	alphas[5] = ( 3 * maxAlpha + 4 * minAlpha ) / 7;
	alphas[6] = ( 2 * maxAlpha + 5 * minAlpha ) / 7;
	alphas[7] = ( 1 * maxAlpha + 6 * minAlpha ) / 7;
	colorBlock += 3;
	for ( int i = 0; i < 16; i++ ) {
		int minDistance = INT_MAX;
		uint8 a = colorBlock[i*4];
		for ( int j = 0; j < 8; j++ ) {
			int dist = abs( a - alphas[j] );
			if ( dist < minDistance ) {
				minDistance = dist;
				indices[i] = j;
			}
		}
	}
	EmitByte( (indices[ 0] >> 0) | (indices[ 1] << 3) | (indices[ 2] << 6) );
	EmitByte( (indices[ 2] >> 2) | (indices[ 3] << 1) | (indices[ 4] << 4) | (indices[ 5] << 7) );
	EmitByte( (indices[ 5] >> 1) | (indices[ 6] << 2) | (indices[ 7] << 5) );
	EmitByte( (indices[ 8] >> 0) | (indices[ 9] << 3) | (indices[10] << 6) );
	EmitByte( (indices[10] >> 2) | (indices[11] << 1) | (indices[12] << 4) | (indices[13] << 7) );
	EmitByte( (indices[13] >> 1) | (indices[14] << 2) | (indices[15] << 5) );
}
#else
void EmitAlphaIndices( const uint8 *colorBlock, const uint8 minAlpha, const uint8 maxAlpha ) {
	assert( maxAlpha > minAlpha );
	uint8 indices[16];
	uint8 mid = ( maxAlpha - minAlpha ) / ( 2 * 7 );
	uint8 ab1 = minAlpha + mid;
	uint8 ab2 = ( 6 * maxAlpha + 1 * minAlpha ) / 7 + mid;
	uint8 ab3 = ( 5 * maxAlpha + 2 * minAlpha ) / 7 + mid;
	uint8 ab4 = ( 4 * maxAlpha + 3 * minAlpha ) / 7 + mid;
	uint8 ab5 = ( 3 * maxAlpha + 4 * minAlpha ) / 7 + mid;
	uint8 ab6 = ( 2 * maxAlpha + 5 * minAlpha ) / 7 + mid;
	uint8 ab7 = ( 1 * maxAlpha + 6 * minAlpha ) / 7 + mid;
	colorBlock += 3;
	for ( int i = 0; i < 16; i++ ) {
		uint8 a = colorBlock[i*4];
		int b1 = ( a <= ab1 );
		int b2 = ( a <= ab2 );
		int b3 = ( a <= ab3 );
		int b4 = ( a <= ab4 );
		int b5 = ( a <= ab5 );
		int b6 = ( a <= ab6 );
		int b7 = ( a <= ab7 );
		int index = ( b1 + b2 + b3 + b4 + b5 + b6 + b7 + 1 ) & 7;
		indices[i] = index ^ ( 2 > index );
	}
	EmitByte( (indices[ 0] >> 0) | (indices[ 1] << 3) | (indices[ 2] << 6) );
	EmitByte( (indices[ 2] >> 2) | (indices[ 3] << 1) | (indices[ 4] << 4) | (indices[ 5] << 7) );
	EmitByte( (indices[ 5] >> 1) | (indices[ 6] << 2) | (indices[ 7] << 5) );
	EmitByte( (indices[ 8] >> 0) | (indices[ 9] << 3) | (indices[10] << 6) );
	EmitByte( (indices[10] >> 2) | (indices[11] << 1) | (indices[12] << 4) | (indices[13] << 7) );
	EmitByte( (indices[13] >> 1) | (indices[14] << 2) | (indices[15] << 5) );
}
#endif


void CompressImageDXT5( const uint8 *inBuf, uint8 *outBuf, int width, int height, int &outputBytes ) {
// 	ALIGN16( uint8 block[64] );
// 	ALIGN16( uint8 minColor[4] );
// 	ALIGN16( uint8 maxColor[4] );
	uint8 block[64];
	uint8 minColor[4];
	uint8 maxColor[4];

	globalOutData = outBuf;
	for ( int j = 0; j < height; j += 4, inBuf += width * 4*4 ) {
		for ( int i = 0; i < width; i += 4 ) {
			ExtractBlock( inBuf + i * 4, width, block );
			GetMinMaxColors4( block, minColor, maxColor );
			EmitByte( maxColor[3] );
			EmitByte( minColor[3] );
			EmitAlphaIndices( block, minColor[3], maxColor[3] );
			EmitWord( ColorTo565( maxColor ) );
			EmitWord( ColorTo565( minColor ) );
			EmitColorIndices3( block, minColor, maxColor );
		}
	}
	outputBytes = globalOutData - outBuf;
}

















// Covariance is calculated to determine the best of the two diagonals in the 2d CoCg space
void SelectYCoCgDiagonal( const uint8 *colorBlock, uint8 *minColor, uint8 *maxColor ) {
	uint8 mid0 = ( (int) minColor[0] + maxColor[0] + 1 ) >> 1;
	uint8 mid1 = ( (int) minColor[1] + maxColor[1] + 1 ) >> 1;
	int covariance = 0;
	for ( int i = 0; i < 16; i++ ) {
		int b0 = colorBlock[i*4+0] - mid0;
		int b1 = colorBlock[i*4+1] - mid1;
		covariance += ( b0 * b1 );
	}
	if ( covariance < 0 ) {
		uint8 t = minColor[1];
		minColor[1] = maxColor[1];
		maxColor[1] = t;
	}
}


#define INSET_COLOR_SHIFT 4 // inset color bounding box 
#define INSET_ALPHA_SHIFT 5 // inset alpha bounding box 
#define C565_5_MASK 0xF8 // 0xFF minus last three bits 
#define C565_6_MASK 0xFC // 0xFF minus last two bits 
void InsetYCoCgBBox( uint8 *minColor, uint8 *maxColor ) {
	int inset[4];
	int mini[4];
	int maxi[4];
	inset[0] = ( maxColor[0] - minColor[0] ) - ( ( 1 << INSET_COLOR_SHIFT ) + inset[0] ) >> INSET_COLOR_SHIFT;
	mini[1] = ( ( minColor[1] << INSET_COLOR_SHIFT ) + inset[1] ) >> INSET_COLOR_SHIFT;
	mini[3] = ( ( minColor[3] << INSET_ALPHA_SHIFT ) + inset[3] ) >> INSET_ALPHA_SHIFT;
	maxi[0] = ( ( maxColor[0] << INSET_COLOR_SHIFT ) - inset[0] ) >> INSET_COLOR_SHIFT;
	maxi[1] = ( ( maxColor[1] << INSET_COLOR_SHIFT ) - inset[1] ) >> INSET_COLOR_SHIFT;
	maxi[3] = ( ( maxColor[3] << INSET_ALPHA_SHIFT ) - inset[3] ) >> INSET_ALPHA_SHIFT;
	mini[0] = ( mini[0] >= 0 ) ? mini[0] : 0;
	mini[1] = ( mini[1] >= 0 ) ? mini[1] : 0;
	mini[3] = ( mini[3] >= 0 ) ? mini[3] : 0;
	maxi[0] = ( maxi[0] <= 255 ) ? maxi[0] : 255;
	maxi[1] = ( maxi[1] <= 255 ) ? maxi[1] : 255;
	maxi[3] = ( maxi[3] <= 255 ) ? maxi[3] : 255;
	minColor[0] = ( mini[0] & C565_5_MASK ) | ( mini[0] >> 5 );
	minColor[1] = ( mini[1] & C565_6_MASK ) | ( mini[1] >> 6 );
	minColor[3] = mini[3];
	maxColor[0] = ( maxi[0] & C565_5_MASK ) | ( maxi[0] >> 5 );
	maxColor[1] = ( maxi[1] & C565_6_MASK ) | ( maxi[1] >> 6 );
	maxColor[3] = maxi[3];
}

void ScaleYCoCg( uint8 *colorBlock, uint8 *minColor, uint8 *maxColor ) {
	int m0 = abs( minColor[0] - 128 );
	int m1 = abs( minColor[1] - 128 );
	int m2 = abs( maxColor[0] - 128 );
	int m3 = abs( maxColor[1] - 128 );
	if ( m1 > m0 ) m0 = m1;
	if ( m3 > m2 ) m2 = m3;
	if ( m2 > m0 ) m0 = m2;
	const int s0 = 128 / 2 - 1;
	const int s1 = 128 / 4 - 1;
	int mask0 = -( m0 <= s0 );
	int mask1 = -( m0 <= s1 );
	int scale = 1 + ( 1 & mask0 ) + ( 2 & mask1 );
	minColor[0] = ( minColor[0] - 128 ) * scale + 128;
	minColor[1] = ( minColor[1] - 128 ) * scale + 128;
	minColor[2] = ( scale - 1 ) << 3;
	maxColor[0] = ( maxColor[0] - 128 ) * scale + 128;
	maxColor[1] = ( maxColor[1] - 128 ) * scale + 128;
	maxColor[2] = ( scale - 1 ) << 3;
	for ( int i = 0; i < 16; i++ ) {
		colorBlock[i*4+0] = ( colorBlock[i*4+0] - 128 ) * scale + 128;
		colorBlock[i*4+1] = ( colorBlock[i*4+1] - 128 ) * scale + 128;
	}
}

// Vec3 RGB2YCoCg( Vec3 rgb ) {
// 	Mat3 mat;
// 	mat.rows[ 0 ] = Vec3( 0.25f, 0.5f, 0.25f );
// 	mat.rows[ 1 ] = Vec3( 0.5f, 0, -0.5f );
// 	mat.rows[ 2 ] = Vec3( -0.25f, 0.5f, -0.25f );
// 	Vec3 yog = mat * rgb;
// 	return yog;
// }

// Normal conversion from CoCg_Y to RGB
// Co = color.x - ( 0.5 * 256.0 / 255.0 ) 
// Cg = color.y - ( 0.5 * 256.0 / 255.0 ) 
// Y = color.w 
// R = Y + Co - Cg 
// G = Y + Cg 
// B = Y - Co - Cg

// Greyscaled conversion from CoCg_Y to RGB (the version we will be using)
// scale = ( color.z * ( 255.0 / 8.0 ) ) + 1.0 
// Co = ( color.x - ( 0.5 * 256.0 / 255.0 ) ) / scale 
// Cg = ( color.y - ( 0.5 * 256.0 / 255.0 ) ) / scale 
// Y = color.w 
// R = Y + Co - Cg 
// G = Y + Cg 
// B = Y - Co - Cg
void BlockToCoCg_Y( uint8 * block ) {
	// input is RGBA, converts to CoCg_Y
	for ( int y = 0; y < 4; y++ ) {
		for ( int x = 0; x < 4; x++ ) {
			int idx = x + y * 4;
			int R = block[ idx * 4 + 0 ];
			int G = block[ idx * 4 + 1 ];
			int B = block[ idx * 4 + 2 ];
			//int A = block[ idx * 4 + 3 ];	// unused for our application

			int Y = ( R >> 2 ) + ( G >> 1 ) + ( B >> 2 );
			int Co = ( R >> 1 ) - ( B >> 1 );
			int Cg = -( R >> 2 ) + ( G >> 1 ) - ( B >> 2 );
			Co += 128;
			Cg += 128;

			block[ idx * 4 + 0 ] = (uint8)Co;
			block[ idx * 4 + 1 ] = (uint8)Cg;
			block[ idx * 4 + 2 ] = 0;
			block[ idx * 4 + 3 ] = (uint8)Y;
		}
	}
}

void CompressImageDXT5_YCoCg( const uint8 *inBuf, uint8 *outBuf, int width, int height, int &outputBytes ) {
	uint8 block[64];
	uint8 minColor[4];
	uint8 maxColor[4];

	globalOutData = outBuf;
	for ( int j = 0; j < height; j += 4, inBuf += width * 4*4 ) {
		for ( int i = 0; i < width; i += 4 ) {
			ExtractBlock( inBuf + i * 4, width, block );
			BlockToCoCg_Y( block );
			GetMinMaxColors4( block, minColor, maxColor );
			SelectYCoCgDiagonal( block, minColor, maxColor );
			ScaleYCoCg( block, minColor, maxColor );
			InsetYCoCgBBox( minColor, maxColor );
			
			EmitByte( maxColor[3] );
			EmitByte( minColor[3] );
			EmitAlphaIndices( block, minColor[3], maxColor[3] );
			EmitWord( ColorTo565( maxColor ) );
			EmitWord( ColorTo565( minColor ) );
			EmitColorIndices3( block, minColor, maxColor );
		}
	}
	outputBytes = globalOutData - outBuf;
}
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
====================================================
TestF32toF16
====================================================
*/
void TestF32toF16() {
	const int numFloats = 6;
	float floats[ numFloats ];
	floats[ 0 ] = 0.0f;
	floats[ 1 ] = 1.0f;
	floats[ 2 ] = -1.0f;
	floats[ 3 ] = 3.1415f;
	floats[ 4 ] = 0.5f;
	floats[ 5 ] = -100.01f;

	for ( int i = 0; i < numFloats; i++ ) {
		unsigned short halfHalf = Math::F32toF16( floats[ i ] );
		float newFloat = Math::F16toF32( halfHalf );
		printf( "float test %i: %f to %f\n", i, floats[ i ], newFloat );
	}
}

#if 0

//
//	This HLSL code is from the "Real-Time BC6H Compression on GPU" article in GPU Pro 7
//


float Quantize( float x ) {
	return ( f32tof16( x ) << 10 ) / ( 0x7bff + 1.0f );
}

Vec3d Quantize( Vec3d x ) {
	return ( f32tof16( x ) << 10 ) / ( 0x7bff + 1.0f );
}

uint ComputeIndex( float texelPos, float endpoint0Pos, float endpoint1Pos ) {
	float endpointDelta = endpoint1Pos - endpoint0Pos;
	float r = ( texelPos - endpoint0Pos ) / endpointDelta;
	return clamp( r * 14.933f + 0.0333f + 0.5f, 0.0f, 15.0f );
}

void main() {
	// Compute endpoints (min/max RGB bbox)
	Vec3d blockMin = texels[ 0 ];
	Vec3d blockMax = texels[ 0 ];
	for ( uint i = 1; i < 16; i++ ) {
		blockMin = min( blockMin, texels[ i ] );
		blockMax = max( blockMax, texels[ i ] );
	}

	// Refine endpoints
	Vec3d refinedBlockMin = blockMax;
	Vec3d refinedBlockMax = blockMin;
	for ( uint i = 0; i < 16; i++ ) {
		refinedBlockMin = min( refinedBlockMin, texels[ i ] == blockMin ? refinedBlockMin : texels[ i ] );
		refinedBlockMax = max( refiendBlockMax, texels[ i ] == blockMax ? refinedBlockMax : texels[ i ] );
	}

	Vec3d deltaMax = ( blockMax - blockMin ) * ( 1.0f / 16.0f );
	blockMin += min( refinedBlockMin - blockMin, deltaMax );
	blockMax -= min( blockMax - refinedBlockMax, deltaMax );

	flaot3 blockDir = blockMax - blockMin;
	blockDir = blockDir / ( blockDir.x + blockDir.y + blockDir.z );

	Vec3d endpoint0 = Quantize( blockMin );
	Vec3d endpoint1 = Quantize( blockMax );
	float endpoint0Pos = f32tof16( dot( blockMin, blockDir ) );
	float endpoint1Pos = f32tof16( dot( blockmax, blockDir ) );

	// Check if endpoint swap is required
	float texelPos = f32tof16( dot( texels[ 0 ], blockDir ) );
	indices[ 0 ] = ComputeIndex( texelPos, endpoint0Pos, endpoint1Pos );
	if ( indices[ 0 ] > 7 ) {
		Swap( endpoint0Pos, endpoint1Pos );
		Swap( endpoint0, endpoint1 );
		indices[ 0 ] = 15 - indices[ 0 ];
	}

	// Compute indices
	for ( uint j = 1; j < 16; j++ ) {
		float texelPos = f32tof16( dot( texels[ j ], blockDir ) );
		indices[ j ] = ComputeIndex( texelPos, endpoint0Pos, endpoint1Pos );
	}
}

#endif

float Quantize( float x ) {
	return ( Math::F32toF16( x ) << 10 ) / ( 0x7bff + 1.0f );
}

Vec3d Quantize( Vec3d v ) {
	Vec3d t;
	t.x = Quantize( v.x );
	t.y = Quantize( v.y );
	t.z = Quantize( v.z );
	return t;
}

uint32 ComputeIndex( float texelPos, float endpoint0Pos, float endpoint1Pos ) {
	float endpointDelta = endpoint1Pos - endpoint0Pos;
	float r = ( texelPos - endpoint0Pos ) / endpointDelta;
	return Math::Clampf( r * 14.933f + 0.0333f + 0.5f, 0.0f, 15.0f );
}

void MainBC6H( Vec3d * texels, uint32 * indices ) {
	// Compute endpoints (min/max RGB bbox)
	Vec3d blockMin = texels[ 0 ];
	Vec3d blockMax = texels[ 0 ];
	for ( uint32 i = 1; i < 16; i++ ) {
		blockMin = Min( blockMin, texels[ i ] );
		blockMax = Max( blockMax, texels[ i ] );
	}

	// Refine endpoints
	Vec3d refinedBlockMin = blockMax;
	Vec3d refinedBlockMax = blockMin;
	for ( uint32 i = 0; i < 16; i++ ) {
		refinedBlockMin = Min( refinedBlockMin, texels[ i ] == blockMin ? refinedBlockMin : texels[ i ] );
		refinedBlockMax = Max( refinedBlockMax, texels[ i ] == blockMax ? refinedBlockMax : texels[ i ] );
	}

	Vec3d deltaMax = ( blockMax - blockMin ) * ( 1.0f / 16.0f );
	blockMin += Min( refinedBlockMin - blockMin, deltaMax );
	blockMax -= Min( blockMax - refinedBlockMax, deltaMax );

	Vec3d blockDir = blockMax - blockMin;
	blockDir = blockDir / ( blockDir.x + blockDir.y + blockDir.z );

	Vec3d endpoint0 = Quantize( blockMin );
	Vec3d endpoint1 = Quantize( blockMax );
	float endpoint0Pos = Math::F32toF16( dot( blockMin, blockDir ) );
	float endpoint1Pos = Math::F32toF16( dot( blockMax, blockDir ) );

	// Check if endpoint swap is required
	float texelPos = Math::F32toF16( dot( texels[ 0 ], blockDir ) );
	indices[ 0 ] = ComputeIndex( texelPos, endpoint0Pos, endpoint1Pos );
	if ( indices[ 0 ] > 7 ) {
		Swap( endpoint0Pos, endpoint1Pos );
		Swap( endpoint0, endpoint1 );
		indices[ 0 ] = 15 - indices[ 0 ];
	}

	// Compute indices
	for ( uint32 j = 1; j < 16; j++ ) {
		float texelPos = Math::F32toF16( dot( texels[ j ], blockDir ) );
		indices[ j ] = ComputeIndex( texelPos, endpoint0Pos, endpoint1Pos );
	}
}

/*
 ===============================
 BlockCompression::CompressBC6H
 ===============================
 */
void * BlockCompression::CompressBC6H( int width, float * data ) {
	return NULL;
}


Vec3d RGB2YCoCg( Vec3d rgb ) {
	Mat3 mat;
	mat.rows[ 0 ] = Vec3d( 0.25f, 0.5f, 0.25f );
	mat.rows[ 1 ] = Vec3d( 0.5f, 0, -0.5f );
	mat.rows[ 2 ] = Vec3d( -0.25f, 0.5f, -0.25f );
	Vec3d yog = mat * rgb;
	return yog;
}

Vec3d YCoCg2RGB( Vec3d color ) {
#if 0
	// Code from paper, must be shader code
	float Co = color.x - ( 0.5f * 256.0f / 255.0f );
	float Cg = color.y - ( 0.5f * 256.0f / 255.0f );
	float Y = color.z;
	//float Y = color.w;

	float R = Y + Co - Cg;
	float G = Y + Cg;
	float B = Y - Co - Cg;
	return Vec3d( R, G, B );
#endif
	Mat3 mat;
	mat.rows[ 0 ] = Vec3d( 1, 1, -1 );
	mat.rows[ 1 ] = Vec3d( 1, 0, 1 );
	mat.rows[ 2 ] = Vec3d( 1, -1, -1 );
	Vec3d rgb = mat * color;
	return rgb;

	// Equivalent:
// tmp = Y   - Cg;
// R   = tmp + Co;
// G   = Y   + Cg;
// B   = tmp - Co;
}
#if 0
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

void SelectYCoCgDiagonal( const uint8 *colorBlock, uint8 *minColor, uint8 *maxColor ) {
	uint8 mid0 = ( (int) minColor[0] + maxColor[0] + 1 ) >> 1;
	uint8 mid1 = ( (int) minColor[1] + maxColor[1] + 1 ) >> 1;
	uint8 side = 0;
	for ( int i = 0; i < 16; i++ ) {
		uint8 b0 = colorBlock[i*4+0] >= mid0;
		uint8 b1 = colorBlock[i*4+1] >= mid1;
		side += ( b0 ^ b1 );
	}
	uint8 mask = -( side > 8 );
	uint8 c0 = minColor[1];
	uint8 c1 = maxColor[1];
	c0 ^= c1 ^= mask &= c0 ^= c1;
	minColor[1] = c0;
	maxColor[1] = c1;
}
#endif
/*
 ===============================
 BlockCompression::CompressBC3
 ===============================
 */
void * BlockCompression::CompressBC3( int width, unsigned char * data ) {
	return NULL;
}














struct pixel8888_t {
	uint8 r;
	uint8 g;
	uint8 b;
	uint8 a;
};

uint16 RGBAto565( const pixel8888_t & px ) {
	float r = (float)px.r / 255.0f;
	float g = (float)px.g / 255.0f;
	float b = (float)px.b / 255.0f;

	r *= 1 << 5;
	g *= 1 << 6;
	b *= 1 << 5;

	uint8 r2 = (uint8)r;
	uint8 g2 = (uint8)g;
	uint8 b2 = (uint8)b;

	uint16 color = ( r2 << 11 ) | ( g2 << 5 ) | b2;
	return color;
}

Vec3d Pixels2Vec3d( const pixel8888_t & px ) {
	Vec3d color;
	color.x = px.r;
	color.y = px.g;
	color.z = px.b;
	return color;
}

pixel8888_t Vec3d2Pixels( const Vec3d & v ) {
	pixel8888_t px;
	px.r = (uint8)v.x;
	px.g = (uint8)v.y;
	px.b = (uint8)v.z;
	px.a = 255;
	return px;
}

void FindEndPoints( Vec3d colors[ 16 ], Vec3d & color0, Vec3d & color1 ) {
	Bounds bounds;
	bounds.Clear();
	for ( int i = 0; i < 16; i++ ) {
		bounds.AddPoint( colors[ i ] );
	}
	color0 = bounds.min;
	color1 = bounds.max;

	color0 = colors[ 0 ];
	color1 = colors[ 0 ];
	for ( int i = 1; i < 16; i++ ) {
		const Vec3d & c = colors[ i ];
		if ( c.x < color0.x && c.y < color0.y && c.z < color0.z ) {
			color0 = c;
		}
		if ( c.x > color1.x && c.y > color1.y && c.z > color1.z ) {
			color1 = c;
		}
	}
}

/*
================================
RGBAtoBC1

This is a pretty lame way to convert rgb to bc1.
This builds a bounding box in color space, and then fits each color to the diagonal of the box.
A better method would be to use principle component analysis or simple linear regression.
But this should be good enough for now.
================================
*/
uint8 * RGBAtoBC1( const uint8 * colors, int width, int height ) {
	bc1_t * bc1 = (bc1_t *)malloc( width * height );
	if ( NULL == bc1 ) {
		return NULL;
	}
	memset( bc1, 0, width * height );

	const pixel8888_t * pixels = (const pixel8888_t *)colors;

	int blockIdx = 0;
	for ( int y = 0; y < height; y += 4 ) {
		for ( int x = 0; x < width; x += 4 ) {
			const int idx = x + height * y;	// top left corner of a 4 x 4 block

			// Build a bounding box in color space
			pixel8888_t color_0 = pixels[ idx ];
			pixel8888_t color_1 = pixels[ idx ];

			Vec3d colors[ 16 ];

			int idx3 = 0;
			for ( int suby = 0; suby < 4; suby++ ) {
				for ( int subx = 0; subx < 4; subx++ ) {
					int xx = x + subx;
					int yy = y + suby;

					const int idx2 = xx + height * yy;
					const pixel8888_t & px = pixels[ idx2 ];
					colors[ idx3 ] = Pixels2Vec3d( px );
					idx3++;
#if 1
					// Build the color min
					if ( color_0.r > px.r ) {
						color_0.r = px.r;
					}
					if ( color_0.g > px.g ) {
						color_0.r = px.g;
					}
					if ( color_0.b > px.b ) {
						color_0.b = px.b;
					}

					// Build the color max
					if ( color_1.r < px.r ) {
						color_1.r = px.r;
					}
					if ( color_1.g < px.g ) {
						color_1.r = px.g;
					}
					if ( color_1.b < px.b ) {
						color_1.b = px.b;
					}
#else
					if ( px.r < color_0.r && px.g < color_0.g && px.b < color_0.b ) {
						color_0.r = px.r;
						color_0.g = px.g;
						color_0.b = px.b;
					}
					if ( px.r > color_1.r && px.g > color_1.g && px.b > color_1.b ) {
						color_1.r = px.r;
						color_1.g = px.g;
						color_1.b = px.b;
					}
#endif
				}
			}

			// Determine the colors closest to the mins/maxs
			pixel8888_t mins = color_0;
			pixel8888_t maxs = color_1;
			int distMins = 100000;
			int distMaxs = 100000;
			for ( int suby = 0; suby < 4; suby++ ) {
				for ( int subx = 0; subx < 4; subx++ ) {
					int xx = x + subx;
					int yy = y + suby;

					const int idx2 = xx + height * yy;
					const pixel8888_t & px = pixels[ idx2 ];
					
					int dist0 = abs( px.r - color_0.r ) + abs( px.g - color_0.g ) + abs( px.b - color_0.b );
					int dist1 = abs( px.r - color_1.r ) + abs( px.g - color_1.g ) + abs( px.b - color_1.b );
					if ( dist0 < distMins ) {
						distMins = dist0;
						mins = px;
					}
					if ( dist1 < distMaxs ) {
						distMaxs = dist1;
						maxs = px;
					}
				}
			}
			color_0 = mins;
			color_1 = maxs;

			Vec3d colorA;
			Vec3d colorB;
			FindEndPoints( colors, colorA, colorB );
			color_0 = Vec3d2Pixels( colorA );
			color_1 = Vec3d2Pixels( colorB );

			// Now project each color onto the line
			Vec3d a = Vec3d( color_0.r, color_0.g, color_0.b );
			Vec3d b = Vec3d( color_1.r, color_1.g, color_1.b );
			Vec3d ab = b - a;
			ab.Normalize();

			bc1_t & block = bc1[ blockIdx ];
			block.color_0 = RGBAto565( color_0 );
			block.color_1 = RGBAto565( color_1 );
			for ( int suby = 0; suby < 4; suby++ ) {
				for ( int subx = 0; subx < 4; subx++ ) {
					int xx = x + subx;
					int yy = y + suby;

					const int idx2 = xx + height * yy;
					const pixel8888_t & px = pixels[ idx2 ];
					Vec3d c = Vec3d( px.r, px.g, px.b );
					Vec3d ac = c - a;
					ac.Normalize();

					float t = ab.Dot( ac );

					uint8 tt = 0;	// default color0
					if ( t >= 0.833f ) {
						tt = 1;		// choose color1
					} else if ( t >= 0.5f ) {
						tt = 3;		// choose 1/3 color0 + 2/3 color1
					} else if ( t >= 0.167f ) {
						tt = 2;		// choose 2/3 color0 + 1/3 color1
					}

					block.bitInidces[ suby ] |= ( tt << ( subx * 2 ) );
				}
			}
			blockIdx++;
		}
	}
	return (uint8 *)bc1;
}


























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


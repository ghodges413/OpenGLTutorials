//
//  BlockCompression.cpp
//
#include "Graphics/BlockCompression.h"
#include "Graphics/Graphics.h"
#include "Math/Math.h"
#include "Math/Vector.h"
#include "Math/Matrix.h"
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
void SelectYCoCgDiagonal( const byte *colorBlock, byte *minColor, byte *maxColor ) {
	byte mid0 = ( (int) minColor[0] + maxColor[0] + 1 ) >> 1;
	byte mid1 = ( (int) minColor[1] + maxColor[1] + 1 ) >> 1;
	int covariance = 0;
	for ( int i = 0; i < 16; i++ ) {
		int b0 = colorBlock[i*4+0] - mid0;
		int b1 = colorBlock[i*4+1] - mid1;
		covariance += ( b0 * b1 );
	}
	if ( covariance < 0 ) {
		byte t = minColor[1];
		minColor[1] = maxColor[1];
		maxColor[1] = t;
	}
}

void SelectYCoCgDiagonal( const byte *colorBlock, byte *minColor, byte *maxColor ) {
	byte mid0 = ( (int) minColor[0] + maxColor[0] + 1 ) >> 1;
	byte mid1 = ( (int) minColor[1] + maxColor[1] + 1 ) >> 1;
	byte side = 0;
	for ( int i = 0; i < 16; i++ ) {
		byte b0 = colorBlock[i*4+0] >= mid0;
		byte b1 = colorBlock[i*4+1] >= mid1;
		side += ( b0 ^ b1 );
	}
	byte mask = -( side > 8 );
	byte c0 = minColor[1];
	byte c1 = maxColor[1];
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

/*
================================
RGBAtoBC1

This is a pretty lame way to convert rgb to bc1.
This builds a bounding box in color space, and then fits each color to the diagonal of the box.
A better method would be to use principle component analysis.  But this should be good enough for now.
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

			for ( int suby = 0; suby < 4; suby++ ) {
				for ( int subx = 0; subx < 4; subx++ ) {
					int xx = x + subx;
					int yy = y + suby;

					const int idx2 = xx + height * yy;
					const pixel8888_t & px = pixels[ idx2 ];
#if 0
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
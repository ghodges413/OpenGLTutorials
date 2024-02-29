//
//  BlockCompression.h
//
#pragma once
#include "Miscellaneous/Types.h"

struct bc1_t {
	uint16 color_0;			// 2 bytes
	uint16 color_1;			// 2 bytes
	uint8 bitInidces[ 4 ];	// 4 bytes
};	// 8 bytes per 16 texels = 4 bits per texel


/*
 ===============================
 BlockCompression
 ===============================
 */
class BlockCompression {
private:
	BlockCompression();

public:
	void * CompressBC6H( int width, float * data );
	void * CompressBC3( int width, unsigned char * data );
};


/*
================================
RGBAtoBC1
================================
*/
uint8 * RGBAtoBC1( const uint8 * colors, int width, int height );

void CompressImageDXT1( const uint8 *inBuf, uint8 *outBuf, int width, int height, int &outputBytes );
void CompressImageDXT5( const uint8 *inBuf, uint8 *outBuf, int width, int height, int &outputBytes );
void CompressImageDXT5_YCoCg( const uint8 *inBuf, uint8 *outBuf, int width, int height, int &outputBytes );
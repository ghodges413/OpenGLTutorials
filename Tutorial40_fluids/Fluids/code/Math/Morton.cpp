//
//	Morton.cpp
//
#include "Morton.h"
#include <algorithm>

/*
====================================================
Morton::IsPowerOfTwo
====================================================
*/
bool Morton::IsPowerOfTwo( const unsigned int num ) {
	return ( ( num & ( num - 1 ) ) == 0 ) && num > 0;
}

/*
====================================================
Morton::IsPowerOfFour
====================================================
*/
bool Morton::IsPowerOfFour( const unsigned int num ) {
	if ( num <= 0 ) {
		return false;
	}
	if ( ( num & ( num - 1 ) ) != 0 ) {
		return false;
	}
	return ( num & 0x55555555 ) > 0;
}

/*
====================================================
Morton::IsPowerOfEight
====================================================
*/
bool Morton::IsPowerOfEight( const unsigned int num ) {
	if ( num <= 0 ) {
		return false;
	}
	if ( ( num & ( num - 1 ) ) != 0 ) {
		return false;
	}
	return ( num & 0x9249249 ) > 0;
}

/*
====================================================
Morton::ExpandBits1
====================================================
*/
unsigned int Morton::ExpandBits1( unsigned int x ) {
	x &= 0x0000ffff;
	x = ( x ^ ( x << 8 ) ) & 0x00ff00ff;
	x = ( x ^ ( x << 4 ) ) & 0x0f0f0f0f;
	x = ( x ^ ( x << 2 ) ) & 0x33333333;
	x = ( x ^ ( x << 1 ) ) & 0x55555555;
	return x;
}

/*
====================================================
Morton::ExpandBits2
====================================================
*/
unsigned int Morton::ExpandBits2( unsigned int x ) {
	x &= 0x000003ff;
	x = ( x ^ ( x << 16 ) ) & 0xff0000ff;
	x = ( x ^ ( x <<  8 ) ) & 0x0300f00f;
	x = ( x ^ ( x <<  4 ) ) & 0x030c30c3;
	x = ( x ^ ( x <<  2 ) ) & 0x09249249;
	return x;
}

/*
====================================================
Morton::CompactBits1
====================================================
*/
unsigned int Morton::CompactBits1( unsigned int x ) {
	x &= 0x55555555;
	x = ( x ^ ( x >> 1 ) ) & 0x33333333;
	x = ( x ^ ( x >> 2 ) ) & 0x0f0f0f0f;
	x = ( x ^ ( x >> 4 ) ) & 0x00ff00ff;
	x = ( x ^ ( x >> 8 ) ) & 0x0000ffff;
	return x;
}

/*
====================================================
Morton::CompactBits2
====================================================
*/
unsigned int Morton::CompactBits2( unsigned int x ) {
	x &= 0x09249249;
	x = ( x ^ ( x >>  2 ) ) & 0x030c30c3;
	x = ( x ^ ( x >>  4 ) ) & 0x0300f00f;
	x = ( x ^ ( x >>  8 ) ) & 0xff0000ff;
	x = ( x ^ ( x >> 16 ) ) & 0x000003ff;
	return x;
}

/*
====================================================
Morton::MortonOrder2D
====================================================
*/
unsigned int Morton::MortonOrder2D( unsigned int x, unsigned int y ) {
	return ( ExpandBits1( x ) + ( ExpandBits1( y ) << 1 ) );
}

/*
====================================================
Morton::MortonOrder3D
====================================================
*/
unsigned int Morton::MortonOrder3D( unsigned int x, unsigned int y, unsigned int z ) {
	return ( ExpandBits2( x ) + ( ExpandBits2( y ) << 1 ) + ( ExpandBits2( z ) << 2 ) );
}

/*
====================================================
Morton::MortonOrder3D
v is expected to be in the [0,1] range
====================================================
*/
unsigned int Morton::MortonOrder3D( const Vec3 & v ) {
	// 1024 = 2^10 ( we only get 10 bits of precision per dimension, since there's 32 bits in an integer )
	float x = std::min( std::max( v.x * 1024.0f, 0.0f ), 1023.0f );
	float y = std::min( std::max( v.y * 1024.0f, 0.0f ), 1023.0f );
	float z = std::min( std::max( v.z * 1024.0f, 0.0f ), 1023.0f );

	unsigned int ux = (unsigned int)x;
	unsigned int uy = (unsigned int)y;
	unsigned int uz = (unsigned int)z;

	return MortonOrder3D( ux, uy, uz );	
}

/*
====================================================
Morton::MortonOrder2D
====================================================
*/
void Morton::MortonOrder2D( const unsigned int idx, unsigned int & x, unsigned int & y ) {
	x = CompactBits1( idx >> 0 );
	y = CompactBits1( idx >> 1 );
}

/*
====================================================
Morton::MortonOrder3D
====================================================
*/
void Morton::MortonOrder3D( const unsigned int idx, unsigned int & x, unsigned int & y, unsigned int & z ) {
	x = CompactBits2( idx >> 0 );
	y = CompactBits2( idx >> 1 );
	y = CompactBits2( idx >> 2 );
}
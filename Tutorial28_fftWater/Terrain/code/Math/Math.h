//
//	Math.h
//
#pragma once
//#define MATH_PI ( 3.141592653589793f )

#include "Miscellaneous/Types.h"

#include <math.h>
// #include "Vector.h"
// #include "Matrix.h"
// #include "Complex.h"
// #include "Quat.h"
// #include "Random.h"
// #include "MatrixOps.h"
// #include "Fourier.h"

/*
 ================================
 Math
 ================================
 */
class Math {
private:
	Math();
	Math( const Math & math );
	Math & operator = ( const Math & math );

public:
	static const float PI;

	static bool IsPowerOfTwo( const int32 n );
	static bool IsPowerOfFour( const int32 n );
	static bool IsPowerOfEight( const int32 n );

	static int32 ExpandBits1( const int32 num );
	static int32 ExpandBits2( const int32 num );
	static int32 CompactBits1( const int32 num );
	static int32 CompactBits2( const int32 num );
	static int32 MortonOrder2D( const int32 x, const int32 y );
	static int32 MortonOrder3D( const int32 x, const int32 y, const int32 z );
	static void MortonOrder2D( const int32 idx, int32 & x, int32 & y );
	static void MortonOrder3D( const int32 idx, int32 & x, int32 & y, int32 & z );

	static float32 Cos( const float32 a ) { return cosf( a ); }
	static float32 Sin( const float32 a ) { return sinf( a ); }

	static float32 Sqrt( const float32 a ) { return sqrtf( a ); }
	static float32 Exp( const float32 a ) { return expf( a ); }
	static float32 J0( const float32 a ) { return (float32)j0( (float64)a );}
};

/*
================================
Math::IsPowerOfTwo
================================
*/
inline bool Math::IsPowerOfTwo( const int num ) {
	return ( ( num & ( num - 1 ) ) == 0 ) && num > 0;
}

/*
================================
Math::IsPowerOfFour
================================
*/
inline bool Math::IsPowerOfFour( const int num ) {
	if ( num <= 0 ) {
		return false;
	}
	if ( ( num & ( num - 1 ) ) != 0 ) {
		return false;
	}
	return ( num & 0x55555555 ) > 0;
}

/*
================================
Math::IsPowerOfEight
================================
*/
inline bool Math::IsPowerOfEight( const int num ) {
	if ( num <= 0 ) {
		return false;
	}
	if ( ( num & ( num - 1 ) ) != 0 ) {
		return false;
	}
	return ( num & 0x9249249 ) > 0;
}

/*
================================
Math::ExpandBits1
================================
*/
inline int32 Math::ExpandBits1( int32 x ) {
	x &= 0x0000ffff;
	x = ( x ^ ( x << 8 ) ) & 0x00ff00ff;
	x = ( x ^ ( x << 4 ) ) & 0x0f0f0f0f;
	x = ( x ^ ( x << 2 ) ) & 0x33333333;
	x = ( x ^ ( x << 1 ) ) & 0x55555555;
	return x;
}

/*
================================
Math::ExpandBits2
================================
*/
inline int32 Math::ExpandBits2( int32 x ) {
	x &= 0x000003ff;
	x = ( x ^ ( x << 16 ) ) & 0xff0000ff;
	x = ( x ^ ( x <<  8 ) ) & 0x0300f00f;
	x = ( x ^ ( x <<  4 ) ) & 0x030c30c3;
	x = ( x ^ ( x <<  2 ) ) & 0x09249249;
	return x;
}

/*
================================
Math::CompactBits1
================================
*/
inline int32 Math::CompactBits1( int32 x ) {
	x &= 0x55555555;
	x = ( x ^ ( x >> 1 ) ) & 0x33333333;
	x = ( x ^ ( x >> 2 ) ) & 0x0f0f0f0f;
	x = ( x ^ ( x >> 4 ) ) & 0x00ff00ff;
	x = ( x ^ ( x >> 8 ) ) & 0x0000ffff;
	return x;
}

/*
================================
Math::CompactBits2
================================
*/
inline int32 Math::CompactBits2( int32 x ) {
	x &= 0x09249249;
	x = ( x ^ ( x >>  2 ) ) & 0x030c30c3;
	x = ( x ^ ( x >>  4 ) ) & 0x0300f00f;
	x = ( x ^ ( x >>  8 ) ) & 0xff0000ff;
	x = ( x ^ ( x >> 16 ) ) & 0x000003ff;
	return x;
}

/*
================================
Math::MortonOrder2D
================================
*/
inline int32 Math::MortonOrder2D( int32 x, int32 y ) {
	return ( ExpandBits1( x ) + ( ExpandBits1( y ) << 1 ) );
}

/*
================================
Math::MortonOrder3D
================================
*/
inline int32 Math::MortonOrder3D( int32 x, int32 y, int32 z ) {
	return ( ExpandBits2( x ) + ( ExpandBits2( y ) << 1 ) + ( ExpandBits2( z ) << 2 ) );
}

/*
================================
Math::MortonOrder2D
================================
*/
inline void Math::MortonOrder2D( const int32 idx, int32 & x, int32 & y ) {
	x = CompactBits1( idx >> 0 );
	y = CompactBits1( idx >> 1 );
}

/*
================================
Math::MortonOrder3D
================================
*/
inline void Math::MortonOrder3D( const int32 idx, int32 & x, int32 & y, int32 & z ) {
	x = CompactBits2( idx >> 0 );
	y = CompactBits2( idx >> 1 );
	y = CompactBits2( idx >> 2 );
}

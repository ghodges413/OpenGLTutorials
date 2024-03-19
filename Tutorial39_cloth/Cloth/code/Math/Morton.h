//
//	Morton.h
//
#pragma once
#include "Vector.h"

/*
====================================================
Morton
====================================================
*/
class Morton {
private:
	Morton() {}
	Morton( const Morton & rhs );
	Morton & operator = ( const Morton & rhs );

public:
	static bool IsPowerOfTwo( const unsigned int n );
	static bool IsPowerOfFour( const unsigned int n );
	static bool IsPowerOfEight( const unsigned int n );

	static unsigned int ExpandBits1( const unsigned int num );
	static unsigned int ExpandBits2( const unsigned int num );
	static unsigned int CompactBits1( const unsigned int num );
	static unsigned int CompactBits2( const unsigned int num );
	static unsigned int MortonOrder2D( const unsigned int x, const unsigned int y );
	static unsigned int MortonOrder3D( const unsigned int x, const unsigned int y, const unsigned int z );
	static void MortonOrder2D( const unsigned int idx, unsigned int & x, unsigned int & y );
	static void MortonOrder3D( const unsigned int idx, unsigned int & x, unsigned int & y, unsigned int & z );

	static unsigned int MortonOrder3D( const Vec3 & v );
};

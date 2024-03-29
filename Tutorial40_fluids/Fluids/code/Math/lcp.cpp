//
//	lcp.cpp
//
#include "lcp.h"

#pragma optimize( "", off )

/*
================================
LCP_GaussSeidel
================================
*/
Vec3 LCP_GaussSeidel( const Mat3 & A, const Vec3 & b ) {
	Vec3 x( 0.0f );
	for ( int iter = 0; iter < 3; iter++ ) {
		for ( int i = 0; i < 3; i++ ) {
			float dx = ( b[ i ] - A.rows[ i ].Dot( x ) ) / A.rows[ i ][ i ];
			if ( dx * 0.0f == dx * 0.0f ) {
				x[ i ] = x[ i ] + dx;
			}
		}
	}
	return x;
}

/*
================================
LCP_GaussSeidel
================================
*/
VecN LCP_GaussSeidel( const MatN & A, const VecN & b ) {
	const int N = b.N;
	VecN x( N );
	x.Zero();

	for ( int iter = 0; iter < N; iter++ ) {
		for ( int i = 0; i < N; i++ ) {
			float dx = ( b[ i ] - A.rows[ i ].Dot( x ) ) / A.rows[ i ][ i ];
			if ( dx * 0.0f == dx * 0.0f ) {
				x[ i ] = x[ i ] + dx;
			}
		}
	}
	return x;
}



void LCP_Test() {
	{
		// ( 2, 1 )
		// ( 5, 7 )
		MatN A( 2 );
		A.rows[ 0 ][ 0 ] = 2;
		A.rows[ 0 ][ 1 ] = 1;
		A.rows[ 1 ][ 0 ] = 5;
		A.rows[ 1 ][ 1 ] = 7;

		VecN b( 2 );
		b[ 0 ] = 11;
		b[ 1 ] = 13;

		VecN x = LCP_GaussSeidel( A, b );
		for ( int i = 0; i < x.N; i++ ) {
			printf( "%i: %f\n", i, x[ i ] );
		}
	}

	{
		// ( 10, -1, 2, 0 )
		// ( -1, 11, -1, 3 )
		// ( 2, -1, 10, -1 )
		// ( 0, 3, -1, 8 )
		MatN A( 4 );
		A.rows[ 0 ][ 0 ] = 10;
		A.rows[ 0 ][ 1 ] = -1;
		A.rows[ 0 ][ 2 ] = 2;
		A.rows[ 0 ][ 3 ] = 0;
		
		A.rows[ 1 ][ 0 ] = -1;
		A.rows[ 1 ][ 1 ] = 11;
		A.rows[ 1 ][ 2 ] = -1;
		A.rows[ 1 ][ 3 ] = 3;

		A.rows[ 2 ][ 0 ] = 2;
		A.rows[ 2 ][ 1 ] = -1;
		A.rows[ 2 ][ 2 ] = 10;
		A.rows[ 2 ][ 3 ] = -1;

		A.rows[ 3 ][ 0 ] = 0;
		A.rows[ 3 ][ 1 ] = 3;
		A.rows[ 3 ][ 2 ] = -1;
		A.rows[ 3 ][ 3 ] = 8;

		VecN b( 4 );
		b[ 0 ] = 6;
		b[ 1 ] = 25;
		b[ 2 ] = -11;
		b[ 3 ] = 15;

		VecN x = LCP_GaussSeidel( A, b );
		for ( int i = 0; i < x.N; i++ ) {
			printf( "%i: %f\n", i, x[ i ] );
		}
	}
}
//
//	Perlin.cpp
//
#include "Math/Random.h"
#include "Math/Perlin.h"

Vec3d * Perlin::s_randomVector = NULL;
float * Perlin::s_randomFloat = NULL;
int * Perlin::s_permX = NULL;
int * Perlin::s_permY = NULL;
int * Perlin::s_permZ = NULL;

/*
====================================================
Perlin::Turbulence
====================================================
*/
float Perlin::Turbulence( const Vec3d & p, const int depth ) {
	float accumulation = 0;
	Vec3d temp = p;
	float weight = 1.0f;
	for ( int i = 0; i < depth; i++ ) {
		accumulation += weight * Noise2( temp );
		weight *= 0.5f;
		temp *= 2.0f;
	}
	return fabsf( accumulation );
}

/*
====================================================
PerlinInterpolate
====================================================
*/
float PerlinInterpolate( Vec3d c[ 2 ][ 2 ][ 2 ], float u, float v, float w ) {
	const float uu = u * u * ( 3.0f - 2.0f * u );
	const float vv = v * v * ( 3.0f - 2.0f * v );
	const float ww = w * w * ( 3.0f - 2.0f * w );

	float accumulation = 0;
	for ( int i = 0; i < 2; i++ ) {
		for ( int j = 0; j < 2; j++ ) {
			for ( int k = 0; k < 2; k++ ) {
				const float fi = i;
				const float fj = j;
				const float fk = k;
				const Vec3d weights( u - i, v - j, w - k );

				accumulation +=
					( fi * uu + ( 1.0f - fi ) ) *
					( fj * vv + ( 1.0f - fj ) * ( 1.0f - vv ) ) *
					( fk * ww + ( 1.0f - fk ) * ( 1.0f - ww ) ) *
					c[ i ][ j ][ k ].Dot( weights );
			}
		}
	}

	return accumulation;
}

/*
====================================================
Perlin::Initialize
====================================================
*/
void Perlin::Initialize( RandomMersenne & rnd ) {
	s_randomVector = PerlinGenerateVec3d( rnd );
	s_randomFloat = PerlinGenerate( rnd );
	s_permX = PerlinGeneratePerm( rnd );
	s_permY = PerlinGeneratePerm( rnd );
	s_permZ = PerlinGeneratePerm( rnd );
}

/*
====================================================
Perlin::Noise
====================================================
*/
float Perlin::Noise( const Vec3d & p ) {
	float u = p.x - floor( p.x );
	float v = p.y - floor( p.y );
	float w = p.z - floor( p.z );
	u = u * u * ( 3.0f - 2.0f * u );
	v = v * v * ( 3.0f - 2.0f * v );
	w = w * w * ( 3.0f - 2.0f * w );
	int i = int( 4.0f * p.x ) & 255;
	int j = int( 4.0f * p.y ) & 255;
	int k = int( 4.0f * p.z ) & 255;
	return s_randomFloat[ s_permX[ i ] ^ s_permY[ j ] ^ s_permZ[ k ] ];
}
float Perlin::Noise2( const Vec3d & p ) {
	float u = p.x - floor( p.x );
	float v = p.y - floor( p.y );
	float w = p.z - floor( p.z );
	int i = floor( p.x );
	int j = floor( p.y );
	int k = floor( p.z );
	Vec3d c[ 2 ][ 2 ][ 2 ];
	for ( int di = 0; di < 2; di++ ) {
		for ( int dj = 0; dj < 2; dj++ ) {
			for ( int dk = 0; dk < 2; dk++ ) {
				c[ di ][ dj ][ dk ] = s_randomVector[
					s_permX[ ( i + di ) & 255 ] ^
					s_permY[ ( j + dj ) & 255 ] ^
					s_permZ[ ( k + dk ) & 255 ]
				];
			}
		}
	}
	return PerlinInterpolate( c, u, v, w );
}

/*
====================================================
Perlin::PerlinGenerateVec3d
====================================================
*/
Vec3d * Perlin::PerlinGenerateVec3d( RandomMersenne & rnd ) {
	Vec3d * p = new Vec3d[ 256 ];
	for ( int i = 0; i < 256; i++ ) {
		p[ i ] = Vec3d( -1.0f + 2.0f * rnd.Get(), -1.0f + 2.0f * rnd.Get(), -1.0f + 2.0f * rnd.Get() );
		p[ i ].Normalize();
	}
	return p;
}

/*
====================================================
Perlin::PerlinGenerate
====================================================
*/
float * Perlin::PerlinGenerate( RandomMersenne & rnd ) {
	float * p = new float[ 256 ];
	for ( int i = 0; i < 256; i++ ) {
		p[ i ] = rnd.Get();
	}
	return p;
}

/*
====================================================
Perlin::Permute
====================================================
*/
void Perlin::Permute( int * p, int n, RandomMersenne & rnd ) {
	for ( int i = n - 1; i > 0; i-- ) {
		int target = int( rnd.Get() * ( i + 1 ) );
		int tmp = p[ i ];
		p[ i ] = p[ target ];
		p[ target ] = tmp;
	}
}

/*
====================================================
Perlin::PerlinGeneratePerm
====================================================
*/
int * Perlin::PerlinGeneratePerm( RandomMersenne & rnd ) {
	int * p = new int[ 256 ];
	for ( int i = 0; i < 256; i++ ) {
		p[ i ] = i;
	}
	Permute( p, 256, rnd );
	return p;
}
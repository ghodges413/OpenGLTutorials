//
//	Random.cpp
//
#include "Random.h"
#include "Vector.h"

std::mt19937 Random::m_generator( 0 );
std::uniform_real_distribution< float > Random::m_distribution( 0.0f, 1.0f );

/*
====================================================
Random::RandomInUnitSphere
====================================================
*/
Vec3 Random::RandomInUnitSphere() {
#if 1
	Vec3 xyz = RandomOnSphereSurface();
	xyz *= Get();

	return xyz;
#else
	Vec3 p;
	do {
		p = Vec3( Get(), Get(), Get() ) * 2.0f - Vec3( 1.0f, 1.0f, 1.0f );
	} while ( p.Dot( p ) >= 1.0f );
	return p;
#endif
}

/*
====================================================
Random::RandomOnSphereSurface
====================================================
*/
Vec3 Random::RandomOnSphereSurface() {
#if 1
	const float pi = acosf( -1.0f );

	float u = Get();
	float v = Get();
	float phi = acosf( 2.0f * v - 1.0f );
	float theta = 2.0f * pi * u;

	Vec3 xyz;
	xyz.x = cosf( theta ) * sinf( phi );
	xyz.y = sinf( theta ) * sinf( phi );
	xyz.z = cosf( phi );

	return xyz;
#else
	Vec3 p;
	do {
		p = Vec3( Get(), Get(), Get() ) * 2.0f - Vec3( 1.0f, 1.0f, 1.0f );
	} while ( p.Dot( p ) >= 1.0f );
	p.Normalize();
	return p;
#endif
}

/*
====================================================
Random::RandomCosineDirection
====================================================
*/
Vec3 Random::RandomCosineDirection() {
	const float pi = acosf( -1.0f );

	float r1 = Get();
	float r2 = Get();
	float z = sqrtf( 1.0f - r2 );
	float phi = 2.0f * pi * r1;
	float x = cosf( phi ) * 2.0f * sqrtf( r2 );
	float y = sinf( phi ) * 2.0f * sqrtf( r2 );

	return Vec3( x, y, z );
}

/*
====================================================
Random::RandomToSphere
====================================================
*/
Vec3 Random::RandomToSphere( float radius, float distSquared ) {
	const float pi = acosf( -1.0f );

	float r1 = Get();
	float r2 = Get();
	float z = 1.0f + r2 * ( sqrtf( 1.0f - radius * radius / distSquared ) - 1.0f );
	float phi = 2.0f * pi * r1;
	float x = cosf( phi ) * sqrtf( 1.0f - z * z );
	float y = sinf( phi ) * sqrtf( 1.0f - z * z );
	return Vec3( x, y, z );
}

/*
================================
hbRandom::Gaussian

// Generating gaussian random number with mean 0 and standard deviation 1.
// Basic Box-Muller transform
================================
*/
float Random::Gaussian( const float mean, const float stdDeviation, const float epsilon ) {
	const float pi = acosf( -1.0f );

	float u1 = Get();//(float)rand() / (float)RAND_MAX;
	float u2 = Get();//(float)rand() / (float)RAND_MAX;

	while ( u1 <= epsilon ) {
		u1 = Get();//(float)rand() / (float)RAND_MAX;
		u2 = Get();//(float)rand() / (float)RAND_MAX;
	}

	float z0 = sqrtf( -2.0f * logf( u1 ) ) * cosf( 2.0f * pi * u2 );
	z0 = z0 * stdDeviation + mean;

	return z0;
}

float Pi_Uniform( int samplesX, int samplesY ) {
	int counter = 0;
	for ( int y = 0; y < samplesY; y++ ) {
		for ( int x = 0; x < samplesX; x++ ) {
			float xx = float( x ) / float( samplesX );
			float yy = float( y ) / float( samplesY );
			float x2 = xx * xx;
			float y2 = yy * yy;
			if ( x2 + y2 < 1.0f ) {
				counter++;
			}
		}
	}
	float pi = 4.0f * float( counter ) / float( samplesX * samplesY );
	return pi;
}
float Pi_MonteCarlo( int numSamples ) {
	int counter = 0;
	for ( int i = 0; i < numSamples; i++ ) {
		float xx = Random::Get();
		float yy = Random::Get();
		float x2 = xx * xx;
		float y2 = yy * yy;
		if ( x2 + y2 < 1.0f ) {
			counter++;
		}
	}
	float pi = 4.0f * float( counter ) / float( numSamples );
	return pi;
}
void TestPI() {
	int samples[ 4 ] = { 3, 10, 100, 1000 };
	for ( int i = 0; i < 4; i++ ) {
		float pi_uniform = Pi_Uniform( samples[ i ], samples[ i ] );
		float pi_monte = Pi_MonteCarlo( samples[ i ] * samples[ i ] );
		int totalSamples = samples[ i ] * samples[ i ];
		printf( "unifomrm: %f     monte carlo: %f     Total Samples: %i\n", pi_uniform, pi_monte, totalSamples );
	}
	printf( "---------------------------------\n" );
}
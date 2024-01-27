//
//	Random.cpp
//
#include "Math/Random.h"
#include <stdlib.h>
#include "Math/Math.h"

/*
 ================================
 Random::RangedNumber
 ================================
 */
float Random::RangedNumber( const float & min, const float & max ) {
    assert( max > min );
    float num = rand();
    num /= RAND_MAX;
    const float diff = max - min;
    const float finale = min + diff * num;
    return finale;
}

/*
 ================================
 Random::GetMaxFloat
 ================================
 */
float Random::GetMaxFloat() {
    return GetMaxInt();
}

/*
 ================================
 Random::GetMaxInt
 ================================
 */
int Random::GetMaxInt() {
    return RAND_MAX;
}

/*
 ================================
 Random::GetInt
 
 Returns a value in the range [0,RAND_MAX]
 ================================
 */
int Random::GetInt() {
    return rand();
}

/*
================================
Random::Gaussian

// Generating gaussian random number with mean 0 and standard deviation 1.
// Basic Box-Muller transform
================================
*/
float Random::Gaussian( const float mean, const float stdDeviation, const float epsilon ) {
	float u1 = (float)rand() / (float)RAND_MAX;
	float u2 = (float)rand() / (float)RAND_MAX;

	while ( u1 <= epsilon ) {
		u1 = (float)rand() / (float)RAND_MAX;
		u2 = (float)rand() / (float)RAND_MAX;
	}

	float z0 = sqrtf( -2.0f * logf( u1 ) ) * cosf( 2.0f * Math::PI * u2 );
	z0 = z0 * stdDeviation + mean;

	return z0;
}













std::mt19937 RandomMersenne::m_generator( 0 );
std::uniform_real_distribution< float > RandomMersenne::m_distribution( 0.0f, 1.0f );

/*
====================================================
RandomMersenne::RandomInUnitSphere
====================================================
*/
Vec3d RandomMersenne::RandomInUnitSphere() {
	Vec3d p = Vec3d( Get(), Get(), Get() ) * 2.0f - Vec3d( 1.0f, 1.0f, 1.0f );
	return p;
}

//
//  Random.h
//
#pragma once
#include <random>
#include "Math/Vector.h"

/*
 ================================
 Random
 ================================
 */
class Random {
public:
                        ~Random();
	
    static  float       Get() { return RangedNumber( 0.0f, 1.0f ); }
    static  float       RangedNumber( const float & min, const float & max );
    static  float       GetMaxFloat();
    static  int         GetMaxInt();
    static  int         GetInt();

	// Generating gaussian random number with mean 0 and standard deviation 1.
	static	float		Gaussian( const float mean = 0.0f, const float stdDeviation = 1.0f, const float epsilon = 1e-6f );
    
private:
                        Random();
                        Random( const Random & rhs );
    const   Random &  operator = ( const Random & rhs );
};

/*
====================================================
RandomMersenne
====================================================
*/
class RandomMersenne {
public:
	static float Get() { return m_distribution( m_generator ); }	// Gets a uniform distribution in the range [ 0, 1 )
	static Vec3d RandomInUnitSphere();

private:
	static std::mt19937 m_generator;
	static std::uniform_real_distribution< float > m_distribution;
};

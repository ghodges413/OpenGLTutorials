//
//	Random.h
//
#pragma once
#include <random>
#include "Vector.h"

/*
====================================================
Random
====================================================
*/
class Random {
public:
	static float Get() { return m_distribution( m_generator ); }	// Gets a uniform distribution in the range [ 0, 1 )
	static Vec3 RandomInUnitSphere();
	static Vec3 RandomOnSphereSurface();
	static Vec3 RandomCosineDirection();
	static Vec3 RandomToSphere( float radius, float distSquared );

	// Generating gaussian random number with mean 0 and standard deviation 1.
	static float Gaussian( const float mean = 0.0f, const float stdDeviation = 1.0f, const float epsilon = 1e-6f );

private:
	static std::mt19937 m_generator;
	static std::uniform_real_distribution< float > m_distribution;
};
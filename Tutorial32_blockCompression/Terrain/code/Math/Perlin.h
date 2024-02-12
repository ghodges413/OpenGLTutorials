//
//	Perlin.h
//
#pragma once
#include "Math/Vector.h"

class RandomMersenne;

/*
====================================================
Perlin

The code for this perlin noise generator is taken from
Peter Shirley's "Ray Tracing In One Weekend"
====================================================
*/
class Perlin {
public:
	static void Initialize( RandomMersenne & rnd );
	static float Noise( const Vec3d & p );
	static float Noise2( const Vec3d & p );
	static float Turbulence( const Vec3d & p, const int depth = 7 );

private:
	static Vec3d * PerlinGenerateVec3d( RandomMersenne & rnd );
	static float * PerlinGenerate( RandomMersenne & rnd );
	static void Permute( int * p, int n, RandomMersenne & rnd );
	static int * PerlinGeneratePerm( RandomMersenne & rnd );

	static Vec3d * s_randomVector;
	static float * s_randomFloat;
	static int * s_permX;
	static int * s_permY;
	static int * s_permZ;
};

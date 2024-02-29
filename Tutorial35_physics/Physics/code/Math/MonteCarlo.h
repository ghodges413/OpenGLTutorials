/*
 *  MonteCarlo.h
 *
 */
#pragma once
#include "Vector.h"

typedef double hbFunctionSampler( const float theta, const float phi );
double hbFunctionUnit( const float theta, const float phi );
double hbFunctionSphericalHarmonic0( const float theta, const float phi );
double hbLegendre00( const float theta, const float phi );
double hbLegendre1N1( const float theta, const float phi );
double hbLegendre10( const float theta, const float phi );
double hbLegendre11( const float theta, const float phi );
double hbLegendre2N2( const float theta, const float phi );
double hbLegendre2N1( const float theta, const float phi );
double hbLegendre20( const float theta, const float phi );
double hbLegendre21( const float theta, const float phi );
double hbLegendre22( const float theta, const float phi );

/*
 ================================
 hbSphericalSample
 ================================
 */
class hbSphericalSample {
public:
	virtual float Sample( const float theta, const float phi ) const = 0;
	virtual float Sample( const Vec3 & dir ) const = 0;
};


class hbMonteCarlo {
public:
	static double IntegrateSphere( hbSphericalSample * sampler, hbFunctionSampler * function, const int numSamples );
};

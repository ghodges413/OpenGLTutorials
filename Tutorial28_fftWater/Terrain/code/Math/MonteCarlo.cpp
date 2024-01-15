//
//	MonteCarlo.cpp
//
#include "Math/MonteCarlo.h"
#include "Math/Random.h"
#include "Math/Math.h" // for Math::PI

double hbFunctionUnit( const float theta, const float phi ) {
	return 1.0;
}

double hbFunctionSphericalHarmonic0( const float theta, const float phi ) {
	return sin( theta );
}

double hbLegendre00( const float theta, const float phi ) {
	return 0.5 * sqrt( 1.0 / Math::PI );
}
/*
double hbLegendreN11( const float theta, const float phi ) {
	exp(
}
double hbLegendre01( const float theta, const float phi );
double hbLegendre11( const float theta, const float phi );
double hbLegendreN22( const float theta, const float phi );
double hbLegendreN12( const float theta, const float phi );
double hbLegendre02( const float theta, const float phi );
double hbLegendre12( const float theta, const float phi );
double hbLegendre22( const float theta, const float phi );
*/

/*
 ================================
 hbMonteCarlo::IntegrateSphere
 ================================
 */
double hbMonteCarlo::IntegrateSphere( hbSphericalSample * sampler, hbFunctionSampler * function, const int numSamples ) {
	assert( numSamples > 0 );
	if ( numSamples < 0 ) {
		return 0.0f;
	}

	double value = 0;
	
	const double weightFunction = 4.0 * Math::PI;

	// Peform the monte carlo integration over a sphere here
	for ( int i = 0; i < numSamples; ++i ) {
		Vec2d random;
		random.x = hbRandom::RangedNumber( 0, 1 );
		random.y = hbRandom::RangedNumber( 0, 1 );

		// phi is in the xy-plane
		const double phi	= 2.0 * Math::PI * random.y;

		// theta is the zenith angle
		const double theta	= 2.0 * acos( sqrt( 1.0 - random.x ) );

		// Sample the sphere
		const double sample	= sampler->Sample( theta, phi );

		// Sum the integration
		value += sample * weightFunction * function( theta, phi );
	}
	value /= (double)numSamples;

	return value;
}
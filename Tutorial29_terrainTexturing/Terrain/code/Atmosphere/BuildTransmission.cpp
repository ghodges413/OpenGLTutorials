//
//  BuildTransmission.cpp
//
#include "Atmosphere/BuildTransmission.h"
#include "Math/Matrix.h"
#include "Miscellaneous/Time.h"
#include <stdio.h>

/*
 ===============================
 ExtinctionOverPath
 ===============================
 */
Vec3d ExtinctionOverPath( const float scaleHeight, const Vec3d & pos, const Vec3d & view, const Vec3d & betaExtinction, const BrunetonData_t & data ) {
	// If we hit the planet, return an opaque value
	if ( DoesCollideGround( pos, view, data.radiusGround ) ) {
		float bigValue = 1e18;
		return Vec3d( bigValue );
	}
	
	// Get the distance for this ray
	const float pathLength = IntersectGroundTop( pos, view, data );
	const float ds = pathLength / float( data.numSamplesExtinction );

	const float radius = pos.GetMagnitude();

	// Calculate the initial sample
    float prevDensityRatio = exp( -( radius - data.radiusGround ) / scaleHeight );

	// Integrate over the path for the extinction
	float result = 0.0f;
    for ( int i = 1; i <= data.numSamplesExtinction; ++i ) {
		// Get the pathlength
        float s = float( i ) * ds;
		
		// Accurately calculate the height at this location
		const Vec3d pt = pos + s * view;
		const float r = pt.GetMagnitude();

		// Get the density ratio
        float densityRatio = expf( -( r - data.radiusGround ) / scaleHeight );

		// Trapezoidal integration
        result += ( prevDensityRatio + densityRatio ) * 0.5f * ds;

		// Save off the density ratio for trapezoidal integration
        prevDensityRatio = densityRatio;
    }

	assert( result == result );
    return result * betaExtinction;
}

/*
 ===============================
 BuildTransmission
 ===============================
 */
void BuildTransmission( const BrunetonData_t & data, Vec4d * image ) {
	for ( int y = 0; y < data.dimTransmission.y; ++y ) {
		float cosAngle = GetAngleExtinction( y, data.dimTransmission.y );
		float sinAngle = sqrtf( 1.0f - cosAngle * cosAngle );	// get the sine of the angle
		const Vec3d view = Vec3d( sinAngle, 0.0f, cosAngle );

		for ( int x = 0; x < data.dimTransmission.x; ++x ) {
			float radius = GetRadiusExtinction( x, data.dimTransmission.x, data.radiusTop, data.radiusGround );
			const Vec3d pos = Vec3d( 0, 0, radius );

			// Calculate the optical depth for rayleigh and mie
			Vec3d extinctionRayleigh = ExtinctionOverPath( data.scaleHeightRayleigh, pos, view, data.betaRayleighExtinction, data );
			Vec3d extinctionMie = ExtinctionOverPath( data.scaleHeightMie, pos, view, data.betaMieExtinction, data );

			// Calculate the final extinction
			Vec4d finalExtinction( 0 );
			for ( int i = 0; i < 3; ++i ) {
				finalExtinction[ i ] = expf( -extinctionRayleigh[ i ] - extinctionMie[ i ] );
				assert( finalExtinction[ i ] == finalExtinction[ i ] );
			}
	
			// Store the final extinction into the transmittance table
			const int idx = data.dimTransmission.x * y + x;
			image[ idx ] = finalExtinction;
		}
	}
}

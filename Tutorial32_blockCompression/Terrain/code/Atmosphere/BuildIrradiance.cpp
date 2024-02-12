//
//  BuildIrradiance.cpp
//
#include "Atmosphere/BuildIrradiance.h"
#include "Math/Matrix.h"
#include "Miscellaneous/Time.h"
#include "Miscellaneous/Comparison.h"
#include <stdio.h>



/*
 ===============================
 InitIrradiance
 ===============================
 */
void InitIrradiance( const BrunetonData_t & data, const Vec4d * transmittance, Vec4d * image ) {
	const float pi = acosf( -1.0f );
	const float invPi = 1.0f / pi;

	for ( int y = 0; y < data.dimIrradiance.y; ++y ) {
		float cosAngle = GetAngleIrradiance( y, data.dimIrradiance.y );

		for ( int x = 0; x < data.dimIrradiance.x; ++x ) {
			float radius = GetRadiusIrradiance( x, data.dimIrradiance.x, data.radiusTop, data.radiusGround );
	
			// Lookup the transmittance for this height and solar direction
			Vec4d transmission = Transmittance( data, radius, cosAngle, transmittance );

			// Perform a dot product with the normal of the spherical ground
			Vec4d finalColor = transmission * Max( cosAngle, 0.0f );

			// Store the final color
			const int idx = data.dimIrradiance.x * y + x;
			image[ idx ] = finalColor * invPi;
		}
	}
}

/*
 ===============================
 DeltaGroundIrradiance
 ===============================
 */
void DeltaGroundIrradiance( const BrunetonData_t & data, const Vec4d * deltaScatter, Vec4d * image ) {
	const float pi = acosf( -1.0f );
	const float invPi = 1.0f / pi;

	for ( int y = 0; y < data.dimIrradiance.y; ++y ) {
		printf( "DeltaGroundIrradiance:  %i of [%i]\n", y, (int)data.dimIrradiance.y );
		float cosAngleSun = GetAngleIrradiance( y, data.dimIrradiance.y );

		for ( int x = 0; x < data.dimIrradiance.x; ++x ) {
			float radius = GetRadiusIrradiance( x, data.dimIrradiance.x, data.radiusTop, data.radiusGround );
	
			// Calculate the ray to the sun
			float sinAngleSun = sqrtf( 1.0f - cosAngleSun * cosAngleSun );
			Vec3d sunRay = Vec3d( sinAngleSun, 0.0f, cosAngleSun );

			float deltaTheta = 0.5f * pi / float( data.numSamplesIrradiance );		// zenith angle (only integrate over half the sphere.. we only need the upper half for calculating ground irradiance)
			float deltaPhi = 2.0f * pi / float( data.numSamplesIrradiance );		// azimuth angle

			//
			// Integrate over the entire view sphere for this height and solar angle ( equation 15 in Bruneton2008 )
			//
			Vec4d irradiance = Vec4d( 0.0f );
			for ( int i = 0; i <= data.numSamplesIrradiance; ++i ) {
				// Current azimuth angle for the view
				float phi = float( i ) * deltaPhi;
		
				for ( int j = 0; j <= data.numSamplesIrradiance; ++j ) {
					// Current zenith angle for the view
					float theta = float( j ) * deltaTheta;
			
					// The solid angle for this part of the integral
					float dOmega = sinf( theta ) * deltaTheta * deltaPhi;

					// Calculate the view ray direction
					Vec3d raySample = Vec3d( cosf( phi ) * sinf( theta ), sinf( phi ) * sinf( theta ), cosf( theta ) );
			
					// Calculate the angle between the ray to the sun and view direction ray
					float cosAngleViewSun = sunRay.Dot( raySample );

					// Lookup the change in the scattering
					Vec4d scatter = SampleScatter( data, radius, raySample.z, cosAngleSun, cosAngleViewSun, deltaScatter );
			
					// Sum the integral
					irradiance += scatter * raySample.z * dOmega;
				}
			}

			// Store the final color
			const int idx = data.dimIrradiance.x * y + x;
			image[ idx ] = irradiance * invPi;
		}
	}
}


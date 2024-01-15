//
//  BuildScatter.cpp
//

#include "Atmosphere/BuildScatter.h"
#include "Math/Matrix.h"
#include "Miscellaneous/Time.h"
#include "Miscellaneous/Comparison.h"
#include <stdio.h>



/*
 ===============================
 GetViewAndSunRaysFromAngles
 ===============================
 */
void GetViewAndSunRaysFromAngles( const float cosAngleView, const float cosAngleSun, const float cosAngleViewSun, Vec3d & view, Vec3d & sun ) {
	float sinAngleView = sqrtf( 1.0f - cosAngleView * cosAngleView );	// get the sine of the angle
	float sinAngleSun = sqrtf( 1.0f - cosAngleSun * cosAngleSun );	// get the sine of the angle

	// Build a ray for the view
    view = Vec3d( sinAngleView, 0.0f, cosAngleView );
	
	// Build a ray for the sun
	sun = Vec3d( sinAngleSun, 0.0f, cosAngleSun );
	
	// If the view is directly up, then it doesn't matter if the sun is in the same plane... it's all symmetric.
	// In the event that the view isn't directly up, we need to calculate the 3D direction to the sun.
	if ( sinAngleView > 0.0f ) {
		// cosAngleViewSun is the dot product between the sun and view
		// cosAngleViewSun = sx * vx + sy * vy + sz + vz
		// = sx * vx + sz + vz
		// => sx * vx = cosAngleViewSun - sz * vz
		sun.x = ( cosAngleViewSun - sun.z * view.z ) / view.x;
		
		// x2 + y2 + z2 = 1.0
		// => y = sqrt( 1.0 - x2 - z2 )
		float ySqr = 1.0f - sun.x * sun.x - sun.z * sun.z;
		if ( ySqr > 0.0f ) {
			sun.y = sqrtf( ySqr );
		}
	}
}

/*
 ===============================
 TransmittanceBetweenPoints
 ===============================
 */
Vec4d TransmittanceBetweenPoints( const BrunetonData_t & data, Vec3d pt0, Vec3d pt1, const Vec4d * sampler ) {
	float radius0 = pt0.GetMagnitude();
	float radius1 = pt1.GetMagnitude();
	
	Vec3d ray = ( pt1 - pt0 ).Normalize();

	Vec3d zenith0 = pt0.Normalize();
	Vec3d zenith1 = pt1.Normalize();
	
	float cosAngle0 = zenith0.DotProduct( ray );
	float cosAngle1 = zenith1.DotProduct( ray );
	
	Vec4d transmission0 = Transmittance( data, radius0, cosAngle0, sampler );
	Vec4d transmission1 = Transmittance( data, radius1, cosAngle1, sampler );
	
	Vec4d result;
	for ( int i = 0; i < 4; ++i ) {
		result[ i ] = Min( transmission0[ i ] / transmission1[ i ], 1.0f );
		assert( result[ i ] == result[ i ] );
	}
    return result;
}

/*
 ===============================
 ScatterAtPointFromSunIntoView
 ===============================
 */
Vec3d ScatterAtPointFromSunIntoView( const BrunetonData_t & data, Vec3d pos, Vec3d pt, Vec3d sun, const float scaleHeight, const Vec4d * transmittance ) {
	const float radiusGround = data.radiusGround;
	
	// Get the extinction for the view and for the sun
	Vec4d extinctionView = TransmittanceBetweenPoints( data, pos, pt, transmittance );
	Vec4d extinctionSun = Transmittance( data, pt, sun, transmittance );

	// Multiply the extinction from the camera to this point and the extinction from this point to the sun
	Vec3d totalExtinction;
	for ( int i = 0; i < 3; ++i ) {
		totalExtinction[ i ] = extinctionView[ i ] * extinctionSun[ i ];
		assert( totalExtinction[ i ] == totalExtinction[ i ] );
	}

	const float radiusPoint = pt.GetMagnitude();
	
	// Calculate the scattering
	float heightPoint = radiusPoint - radiusGround;
	Vec3d scatter = expf( -heightPoint / scaleHeight ) * totalExtinction;
	return scatter;
}

/*
 ===============================
 CalcluateScattering
 ===============================
 */
Vec3d CalcluateScattering( const BrunetonData_t & data, Vec3d pos, Vec3d view, Vec3d sun, const Vec3d & betaScatter, const float scaleHeight, const Vec4d * transmittance ) {
	// Get the total path length and the delta path length
	const float pathLength = IntersectGroundTop( pos, view, data );
	const float ds = pathLength / float( data.numSamplesScatter );
	
	// Integrate over the path
	Vec3d scatter = Vec3d( 0 );
	Vec3d prevScatter = Vec3d( 0 );
    for ( int i = 0; i < data.numSamplesScatter; ++i ) {
		// Get the path length to this sample
        const float s = float( i ) * ds;
		
		// Accurately calculate the height at this location
		const Vec3d pt = pos + s * view;
		
		// Get the scattering over the path for this height and solar angle
		const Vec3d samplePointScatter = ScatterAtPointFromSunIntoView( data, pos, pt, sun, scaleHeight, transmittance );
		
		// Trapezoidal integration
        scatter += ( prevScatter + samplePointScatter ) * 0.5f * ds;
		
		// Store this iteration to the previous for trapezoidal integration
        prevScatter = samplePointScatter;
    }
	
	// Finalize the scattering
	for ( int i = 0; i < 3; ++i ) {
		scatter[ i ] *= betaScatter[ i ];
		assert( scatter[ i ] == scatter[ i ] );
	}
	return scatter;
}

/*
 ===============================
 SingleScattering
 ===============================
 */
void SingleScattering( const BrunetonData_t & data, const Vec4d * transmittance, Vec4d * image, Vec4d * deltaScatter ) {
	for ( int w = 0; w < data.dimScatter.w; ++w ) {
		float radius = GetRadius( w, data.dimScatter.w, data.radiusTop, data.radiusGround );
		printf( "Single Scattering:  %i of [%i]\n", w, (int)data.dimScatter.w );

		for ( int z = 0; z < data.dimScatter.z; ++z ) {
			float cosAngleView = GetCosAngleView( z, data.dimScatter.z, radius, data.radiusTop, data.radiusGround );

			for ( int y = 0; y < data.dimScatter.y; ++y ) {
				float cosAngleViewSun = GetCosAngleViewSun( y, data.dimScatter.y );

				for ( int x = 0; x < data.dimScatter.x; ++x ) {
					float cosAngleSun = GetCosAngleSun( x, data.dimScatter.x );

					// Clamp the angle between the view and the sun.  Otherwise we get invalid values.
					// Obviously it's not physically possible for the angle between the view and sun to
					// be outside a range... dependent on the current view and sun vectors.
					// Really, these are wasted texels in the 4D texture anyway.  We could skip them,
					// but then we might have a problem with sampling border texels... as they'll look
					// up a linear interpolation with a valid value and a bad value.
					cosAngleViewSun = ClampCosAngleViewSun( cosAngleViewSun, cosAngleView, cosAngleSun );

					Vec3d view;
					Vec3d sun;
					GetViewAndSunRaysFromAngles( cosAngleView, cosAngleSun, cosAngleViewSun, view, sun );
					Vec3d pos = Vec3d( 0, 0, radius );

					Vec3d scatterRayleigh = CalcluateScattering( data, pos, view, sun, data.betaRayleighScatter, data.scaleHeightRayleigh, transmittance );
					Vec3d scatterMie = CalcluateScattering( data, pos, view, sun, data.betaMieScatter, data.scaleHeightMie, transmittance );
	
					// This is simple single scattering.  Likewise from Schafhitzel 2007, we are to defer the phase function to later.
					Vec4d finalScatter;
					finalScatter.x = scatterRayleigh.x;
					finalScatter.y = scatterRayleigh.y;
					finalScatter.z = scatterRayleigh.z;
					finalScatter.w = scatterMie.x;

					assert( finalScatter.x == finalScatter.x );
					assert( finalScatter.y == finalScatter.y );
					assert( finalScatter.z == finalScatter.z );
					assert( finalScatter.w == finalScatter.w );
	
					// Store off the scattering
					int idx = x;
					idx += data.dimScatter.x * y;
					idx += data.dimScatter.x * data.dimScatter.y * z;
					idx += data.dimScatter.x * data.dimScatter.y * data.dimScatter.z * w;
					image[ idx ] = finalScatter;

					// We need to also initialize the deltaScatter table... so that we can properly estimate the change in scattering between multi-scattering calculations.
					// Since we use the deltaScatter table to calculate the multi-scattering, and we need the phase function information then... we go ahead and include it
					// in our delta scatter calculation here.  This must be why Bruneton 2008 used a 4D texture to store the scattering information... multiple scattering requires it.
					for ( int n = 0; n < 3; ++n ) {
						const float deltaScatterRayleigh = finalScatter[ n ] * ScatterPhaseFunctionRayleigh( cosAngleViewSun );
						const float deltaScatterMie = finalScatter.w * ScatterPhaseFunctionMie( cosAngleViewSun, data.mieG );
				
						finalScatter[ n ] = deltaScatterRayleigh + deltaScatterMie;
					}
					deltaScatter[ idx ] = finalScatter;
				}
			}
		}
	}
}

/*
 =============================================================================================

 DeltaLightIrradiance

 =============================================================================================
 */

/*
 ===============================
 CalculateInscatter
 ===============================
 */
Vec3d CalculateInscatter( const BrunetonData_t & data, Vec3d pos, Vec3d rayView, Vec3d raySun, const Vec4d * transmittance, const Vec4d * deltaIrradiance, const Vec4d * deltaScatterTable ) {
	const float radiusGround = data.radiusGround;
	const float radiusTop = data.radiusTop;

	const float pi = acosf( -1.0f );
	float deltaTheta = pi / float( data.numSamplesScatterSpherical );	// zenith angle
	float deltatPhi = 2.0f * pi / float( data.numSamplesScatterSpherical );		// azimuth angle

    //
	//	Integrate over the sphere
	//
	Vec3d scatter = Vec3d( 0.0f );
    for ( int i = 0; i <= data.numSamplesScatterSpherical; ++i ) {
		// Current zenith angle for the sample
		float theta = float( i ) * deltaTheta;
        float cosTheta = cosf( theta );
		float sinTheta = sinf( theta );

		// Integrate over the azimuth
        for ( int j = 0; j <= data.numSamplesScatterSpherical; ++j ) {
			// Current azimuth angle for the sample
			float phi = float( j ) * deltatPhi;
			
			// The solid angle for the sample
            float dOmega = deltaTheta * deltatPhi * sinTheta * 0.25f;
			
			// The ray pointing at the sample
            Vec3d raySample = Vec3d( cosf( phi ) * sinTheta, sinf( phi ) * sinTheta, cosTheta );

			//
			// Calculate the light scattering into the sample from the ground
			//
			Vec3d groundScatter = Vec3d( 0.0f );
			if ( DoesCollideGround( pos, raySample, radiusGround ) ) {
				// Get the distance to the ground
				float distanceToGround = IntersectGroundTop( pos, raySample, data );
				
				// Get the vector to the location of the ground that was hit
				Vec3d groundPos = pos + distanceToGround * raySample;
				
				// Get the extinction between here and the ground
				Vec4d groundTransmittance = TransmittanceBetweenPoints( data, pos, groundPos, transmittance );

				// Get the normal of the ground (spherical surface, so just normalize)
				Vec3d groundNormal = groundPos;
				groundNormal.Normalize();

				// Calculate the angle between the ground and the sun
				float cosAngleGroundSun = groundNormal.DotProduct( raySun );
				
				// Get the light coming off of the ground
				Vec4d groundIrradiance = Irradiance( data, radiusGround, cosAngleGroundSun, deltaIrradiance );
				
				// Mulitply the ground irradiance by the extinction from the ground to the view position
				for ( int n = 0; n < 3; ++n ) {
					groundScatter[ n ] = data.averageGroundReflectence * groundIrradiance[ n ] * groundTransmittance[ n ];
				}
			}
			
			//
			//	Calculate the deltaS term
			//
			
			const float cosAngleSunSample = raySun.DotProduct( raySample );
			const float cosAngleViewSample = rayView.DotProduct( raySample );

			// Lookup the scattering entering this point from this direction.
			const Vec4d scatterElementIn = SampleScatter( data, pos.GetMagnitude(), raySample.z, raySun.z, cosAngleSunSample, deltaScatterTable );

			//
			//	Calculate the final light entering our sample
			//
			
			// Get the height
			const float height = pos.GetMagnitude() - radiusGround;
			
			// Get the density ratios
			const float densityRatioRayleigh = expf( -height / data.scaleHeightRayleigh );
			const float densityRatioMie = expf( -height / data.scaleHeightMie );
			
			for ( int n = 0; n < 3; ++n ) {
				// Get the scattering
				const float scatterRayleigh = data.betaRayleighScatter[ n ] * densityRatioRayleigh * ScatterPhaseFunctionRayleigh( cosAngleViewSample );
				const float scatterMie = data.betaMieScatter[ n ] * densityRatioMie * ScatterPhaseFunctionMie( cosAngleViewSample, data.mieG );
			
				// Integrate
				scatter[ n ] += ( groundScatter[ n ] + scatterElementIn[ n ] ) * ( scatterRayleigh + scatterMie ) * dOmega;
				assert( scatter[ n ] == scatter[ n ] );
			}
        }
    }

	return scatter;
}


#ifdef WINDOWS
#include <Windows.h>
#define ThreadReturnType_t DWORD WINAPI
//typedef DWORD WINAPI ThreadReturnType_t;
typedef LPVOID ThreadInputType_t;
#else
#include <pthread.h>
typedef void* ThreadReturnType_t;
typedef void* ThreadInputType_t;
#endif

#define NUM_THREADS 8

struct LightIrradianceData_t {
	BrunetonData_t data;
	const Vec4d * transmittance;
	const Vec4d * deltaIrradiance;
	const Vec4d * deltaScatter;
	Vec4d * image;
	int w;
};
ThreadReturnType_t InnerLightLoops( ThreadInputType_t threadData ) {
	LightIrradianceData_t * lightData = (LightIrradianceData_t *)threadData;
	const BrunetonData_t & data = lightData->data;
	const Vec4d * transmittance = lightData->transmittance;
	const Vec4d * deltaIrradiance = lightData->deltaIrradiance;
	const Vec4d * deltaScatter = lightData->deltaScatter;
	Vec4d * image = lightData->image;
	int w = lightData->w;

	float radius = GetRadius( w, data.dimScatter.w, data.radiusTop, data.radiusGround );
		
	for ( int z = 0; z < data.dimScatter.z; ++z ) {
		float cosAngleView = GetCosAngleView( z, data.dimScatter.z, radius, data.radiusTop, data.radiusGround );

		for ( int y = 0; y < data.dimScatter.y; ++y ) {
			float cosAngleViewSun = GetCosAngleViewSun( y, data.dimScatter.y );

			for ( int x = 0; x < data.dimScatter.x; ++x ) {
				float cosAngleSun = GetCosAngleSun( x, data.dimScatter.x );

				// Clamp the angle between the view and the sun.  Otherwise we get invalid values.
				// Obviously it's not physically possible for the angle between the view and sun to
				// be outside a range... dependent on the current view and sun vectors.
				// Really, these are wasted texels in the 4D texture anyway.  We could skip them,
				// but then we might have a problem with sampling border texels... as they'll look
				// up a linear interpolation with a valid value and a bad value.
				cosAngleViewSun = ClampCosAngleViewSun( cosAngleViewSun, cosAngleView, cosAngleSun );

				//
				//	Build the rays of the View and to the Sun
				//
				Vec3d rayView;
				Vec3d raySun;
				GetViewAndSunRaysFromAngles( cosAngleView, cosAngleSun, cosAngleViewSun, rayView, raySun );
				Vec3d pos = Vec3d( 0, 0, radius );
				
				Vec3d inscatter = CalculateInscatter( data, pos, rayView, raySun, transmittance, deltaIrradiance, deltaScatter );

				Vec4d finalScatter;
				finalScatter.x = inscatter.x;
				finalScatter.y = inscatter.y;
				finalScatter.z = inscatter.z;
				finalScatter.w = 0.0f;

				assert( finalScatter.x == finalScatter.x );
				assert( finalScatter.y == finalScatter.y );
				assert( finalScatter.z == finalScatter.z );

				// Store off the scattering
				int idx = x;
				idx += data.dimScatter.x * y;
				idx += data.dimScatter.x * data.dimScatter.y * z;
				idx += data.dimScatter.x * data.dimScatter.y * data.dimScatter.z * w;
				image[ idx ] = finalScatter;
			}
		}
	}
	return 0;
}

void DeltaLightIrradianceMT( const BrunetonData_t & data, const Vec4d * transmittance, const Vec4d * deltaIrradiance, const Vec4d * deltaScatter, Vec4d * image ) {
#ifdef WINDOWS
	LightIrradianceData_t lightData[ NUM_THREADS ];
	HANDLE hThreadArray[ NUM_THREADS ];
	DWORD dwThreadIdArray[ NUM_THREADS ];
#else
	pthread_t threads[ NUM_THREADS ];
    LightIrradianceData_t data[ NUM_THREADS ];
    int rc = 0;
#endif
	for ( int w = 0; w < data.dimScatter.w; w += NUM_THREADS ) {
		printf( "DeltaLightIrradiance:  %i of [%i]\n", w, (int)data.dimScatter.w );

		for ( int i = 0; i < NUM_THREADS; ++i ) {
				lightData[ i ].data = data;
				lightData[ i ].transmittance = transmittance;
				lightData[ i ].deltaIrradiance = deltaIrradiance;
				lightData[ i ].deltaScatter = deltaScatter;
				lightData[ i ].image = image;
				lightData[ i ].w = w + i;

#ifdef WINDOWS
				hThreadArray[ i ] = CreateThread( 
										NULL,						// default security attributes
										0,							// use default stack size  
										InnerLightLoops,			// thread function name
										lightData + i,					// argument to thread function 
										0,							// use default creation flags 
										&dwThreadIdArray[ i ] );	// returns the thread identifier
				assert( hThreadArray[ i ] != NULL );
#else
				rc = pthread_create( &threads[ i ], NULL, ThreadFunctionWide, (void *) &lightData[ i ] );
				assert( 0 == rc );
#endif
		}

#ifdef WINDOWS
		// Wait until all threads have terminated.
		WaitForMultipleObjects( NUM_THREADS, hThreadArray, TRUE, INFINITE );

		for ( int i = 0; i < NUM_THREADS; ++i ) {
			CloseHandle( hThreadArray[ i ] );
		}
#else
		// wait for each thread to complete
		for ( i = 0; i < NUM_THREADS; ++i ) {
			// block until thread i completes
			rc = pthread_join( threads[ i ], NULL );
			assert( 0 == rc );
		}
#endif
	}
}

/*
 ===============================
 DeltaLightIrradiance
 ===============================
 */
void DeltaLightIrradiance( const BrunetonData_t & data, const Vec4d * transmittance, const Vec4d * deltaIrradiance, const Vec4d * deltaScatter, Vec4d * image ) {
	for ( int w = 0; w < data.dimScatter.w; ++w ) {
		float radius = GetRadius( w, data.dimScatter.w, data.radiusTop, data.radiusGround );
		printf( "DeltaLightIrradiance:  %i of [%i]\n", w, (int)data.dimScatter.w );

		for ( int z = 0; z < data.dimScatter.z; ++z ) {
			float cosAngleView = GetCosAngleView( z, data.dimScatter.z, radius, data.radiusTop, data.radiusGround );

			for ( int y = 0; y < data.dimScatter.y; ++y ) {
				float cosAngleViewSun = GetCosAngleViewSun( y, data.dimScatter.y );

				for ( int x = 0; x < data.dimScatter.x; ++x ) {
					float cosAngleSun = GetCosAngleSun( x, data.dimScatter.x );

					// Clamp the angle between the view and the sun.  Otherwise we get invalid values.
					// Obviously it's not physically possible for the angle between the view and sun to
					// be outside a range... dependent on the current view and sun vectors.
					// Really, these are wasted texels in the 4D texture anyway.  We could skip them,
					// but then we might have a problem with sampling border texels... as they'll look
					// up a linear interpolation with a valid value and a bad value.
					cosAngleViewSun = ClampCosAngleViewSun( cosAngleViewSun, cosAngleView, cosAngleSun );

					//
					//	Build the rays of the View and to the Sun
					//
					Vec3d rayView;
					Vec3d raySun;
					GetViewAndSunRaysFromAngles( cosAngleView, cosAngleSun, cosAngleViewSun, rayView, raySun );
					Vec3d pos = Vec3d( 0, 0, radius );
				
					Vec3d inscatter = CalculateInscatter( data, pos, rayView, raySun, transmittance, deltaIrradiance, deltaScatter );

					Vec4d finalScatter;
					finalScatter.x = inscatter.x;
					finalScatter.y = inscatter.y;
					finalScatter.z = inscatter.z;
					finalScatter.w = 0.0f;

					assert( finalScatter.x == finalScatter.x );
					assert( finalScatter.y == finalScatter.y );
					assert( finalScatter.z == finalScatter.z );

					// Store off the scattering
					int idx = x;
					idx += data.dimScatter.x * y;
					idx += data.dimScatter.x * data.dimScatter.y * z;
					idx += data.dimScatter.x * data.dimScatter.y * data.dimScatter.z * w;
					image[ idx ] = finalScatter;
				}
			}
		}
	}
}

/*
 =============================================================================================

 CalculateInscatter

 =============================================================================================
 */

/*
 ===============================
 CalculateScatterAtPoint
 ===============================
 */
Vec4d CalculateScatterAtPoint( const BrunetonData_t & data, Vec3d pos, Vec3d view, Vec3d sun, float s, const Vec4d * transmittance, const Vec4d * deltaLightScatterTable ) {
	// Get the next position
	Vec3d pt = pos + s * view;
	float radiusScatterPoint = pt.GetMagnitude();

	// Get the view angle at the next position
	float cosAngleViewScatterPoint = pt.DotProduct( view );

	// Get the sun angle at the next position
	float cosAngleSunScatterPoint = pt.DotProduct( sun );

	// Get the angle between the view and sun
	float cosAngleSunView = view.DotProduct( sun );
	
	// Lookup the change in the light scattering
	Vec4d deltaLightScatter = SampleScatter( data, radiusScatterPoint, cosAngleViewScatterPoint, cosAngleSunScatterPoint, cosAngleSunView, deltaLightScatterTable );

	// Look up the extinction between these two points
	Vec4d transmission = TransmittanceBetweenPoints( data, pos, pt, transmittance );
	
	// Return the amount of light scattered into the position
	Vec4d scatter;
	for ( int i = 0; i < 4; ++i ) {
		scatter[ i ] = transmission[ i ] * deltaLightScatter[ i ];
		assert( scatter[ i ] == scatter[ i ] );
	}
    return scatter;
}

/*
 ===============================
 DeltaScatter
 ===============================
 */
void DeltaScatter( const BrunetonData_t & data, const Vec4d * transmittance, const Vec4d * deltaLightScatterTable, Vec4d * image ) {
	for ( int w = 0; w < data.dimScatter.w; ++w ) {
		float radius = GetRadius( w, data.dimScatter.w, data.radiusTop, data.radiusGround );
		printf( "DeltaScatter:  %i of [%i]\n", w, (int)data.dimScatter.w );

		for ( int z = 0; z < data.dimScatter.z; ++z ) {
			float cosAngleView = GetCosAngleView( z, data.dimScatter.z, radius, data.radiusTop, data.radiusGround );

			for ( int y = 0; y < data.dimScatter.y; ++y ) {
				float cosAngleViewSun = GetCosAngleViewSun( y, data.dimScatter.y );

				for ( int x = 0; x < data.dimScatter.x; ++x ) {
					float cosAngleSun = GetCosAngleSun( x, data.dimScatter.x );

					// Clamp the angle between the view and the sun.  Otherwise we get invalid values.
					// Obviously it's not physically possible for the angle between the view and sun to
					// be outside a range... dependent on the current view and sun vectors.
					// Really, these are wasted texels in the 4D texture anyway.  We could skip them,
					// but then we might have a problem with sampling border texels... as they'll look
					// up a linear interpolation with a valid value and a bad value.
					cosAngleViewSun = ClampCosAngleViewSun( cosAngleViewSun, cosAngleView, cosAngleSun );

					
					Vec3d view;
					Vec3d sun;
					GetViewAndSunRaysFromAngles( cosAngleView, cosAngleSun, cosAngleViewSun, view, sun );
					Vec3d pos = Vec3d( 0, 0, radius );

					// Get the total path length and the delta path length
					const float pathLength = IntersectGroundTop( pos, view, data );
					const float ds = pathLength / float( data.numSamplesScatter );
	
					// Get the initial scatering for trapezoidal integration
					Vec4d prevScatter = CalculateScatterAtPoint( data, pos, view, sun, 0.0f, transmittance, deltaLightScatterTable );
	
					// Integrate over the path to find all the new light scattering into this position
					Vec4d scatter = Vec4d( 0.0f );
					for ( int i = 1; i <= data.numSamplesScatter; ++i ) {
						// Get the path length for this sample
						float s = float( i ) * ds;
		
						// Get the change in the inscattering at this point
						Vec4d samplePointScatter = CalculateScatterAtPoint( data, pos, view, sun, s, transmittance, deltaLightScatterTable );
		
						// Trapezoidal integration
						scatter += ( prevScatter + samplePointScatter ) * 0.5f * ds;

						// Store off this sample for trapezoidal integration
						prevScatter = samplePointScatter;
					}

					Vec4d finalScatter;
					finalScatter.x = scatter.x;
					finalScatter.y = scatter.y;
					finalScatter.z = scatter.z;
					finalScatter.w = 0.0f;

					assert( finalScatter.x == finalScatter.x );
					assert( finalScatter.y == finalScatter.y );
					assert( finalScatter.z == finalScatter.z );

					// Store off the scattering
					int idx = x;
					idx += data.dimScatter.x * y;
					idx += data.dimScatter.x * data.dimScatter.y * z;
					idx += data.dimScatter.x * data.dimScatter.y * data.dimScatter.z * w;
					image[ idx ] = finalScatter;
				}
			}
		}
	}
}
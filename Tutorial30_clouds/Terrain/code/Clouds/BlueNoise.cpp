//
//  BlueNoise.cpp
//
#include "Clouds/BlueNoise.h"
#include "Math/Complex.h"
#include "Math/Random.h"
#include "Math/Fourier.h"

/*
=====================================
GenerateBlueNoise32
Generates a 32 x 32 blue noise image by using a random generator and a high pass filter
=====================================
*/
void GenerateBlueNoise32( float * data ) {
	const int width = 32;

	Complex tmp[ width * width ];
	Complex tmp2[ width * width ];

	for ( int y = 0; y < width; y++ ) {
		for ( int x = 0; x < width; x++ ) {
			const int idx = x + y * width;

			tmp[ idx ].x = RandomMersenne::Get();
			tmp[ idx ].y = 0.0f;
		}
	}

	int dim[ 2 ] = { width, width };
	Fourier::Radix2_2D( tmp, dim, tmp2 );

	// High pass filter - set the low frequencies to zero
	for ( int y = 0; y < width / 2; y++ ) {
		for ( int x = 0; x < width / 2; x++ ) {
			const int idx = x + y * width;

			tmp2[ idx ].x = 0.0f;
			tmp2[ idx ].y = 0.0f;
		}
	}

	Fourier::InverseRadix2_2D( tmp2, dim, tmp );

	for ( int y = 0; y < width; y++ ) {
		for ( int x = 0; x < width; x++ ) {
			const int idx = x + y * width;

			data[ idx ] = fabsf( tmp[ idx ].x );
		}
	}
}
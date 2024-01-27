//
//	Fourier.h
//
#pragma once
#include "Math/Complex.h"
#include "Math/Math.h"


typedef void FFTFunction_t( const Complex * data_in, const int num, Complex * data_out );

/*
 ================================
 Fourier
 ================================
 */
class Fourier {
public:
	// performs the discrete fourier transform
	static void DFT( const Complex * data_in, const int num, Complex * data_out );
	static void InverseDFT( const Complex * data_in, const int num, Complex * data_out );

	// Radix 2 FFT
	static void Radix2( const Complex * data_in, const int num, Complex * data_out );
	static void InverseRadix2( const Complex * data_in, const int num, Complex * data_out );

	// Radix 4 FFT
	static void Radix4( const Complex * data_in, const int num, Complex * data_out );
	static void InverseRadix4( const Complex * data_in, const int num, Complex * data_out );

	// Radix 8 FFT
	static void Radix8( const Complex * data_in, const int num, Complex * data_out );
	static void InverseRadix8( const Complex * data_in, const int num, Complex * data_out );

	// dft on 2D data
	static void DFT_2D( const Complex * data_int, const int dim[ 2 ], Complex * data_out );
	static void InverseDFT_2D( const Complex * data_int, const int dim[ 2 ], Complex * data_out );
	static void DFT_3D( const Complex * data_int, const int dim[ 3 ], Complex * data_out );
	static void InverseDFT_3D( const Complex * data_int, const int dim[ 3 ], Complex * data_out );

	static void Radix2_2D( const Complex * data_in, const int dim[ 2 ], Complex * data_out );
	static void Radix4_2D( const Complex * data_in, const int dim[ 2 ], Complex * data_out );
	static void Radix8_2D( const Complex * data_in, const int dim[ 2 ], Complex * data_out );
	static void InverseRadix2_2D( const Complex * data_in, const int dim[ 2 ], Complex * data_out );
	static void InverseRadix4_2D( const Complex * data_in, const int dim[ 2 ], Complex * data_out );
	static void InverseRadix8_2D( const Complex * data_in, const int dim[ 2 ], Complex * data_out );

	// very special for helping us get fourier on gpu working
	static void Radix8_512_0( Complex * data );
	static void Radix8_512_1( Complex * data );
	static void Radix8_512_2( Complex * data );
	static void Radix8_512_3( Complex * data );
	static void Reorder8_512( Complex * data );
	static void Radix8_512( const Complex * data_in, const int num, Complex * data_out );
	static void Radix8_512_2D( const Complex * data_in, Complex * data_out );

	static void Radix2_3D( const Complex * data_in, const int dim[ 3 ], Complex * data_out );
	static void Radix4_3D( const Complex * data_in, const int dim[ 3 ], Complex * data_out );
	static void Radix8_3D( const Complex * data_in, const int dim[ 3 ], Complex * data_out );
	static void InverseRadix2_3D( const Complex * data_in, const int dim[ 3 ], Complex * data_out );
	static void InverseRadix4_3D( const Complex * data_in, const int dim[ 3 ], Complex * data_out );
	static void InverseRadix8_3D( const Complex * data_in, const int dim[ 3 ], Complex * data_out );

	static bool FFT( const bool isForward, const int m, Complex * data );

	static void Test();

private:
	static void Test1D();
	static void Test2D();
	static void Test3D();
	static void PrintReorder8_512();

	static Complex W( const int nk, const int N );
	static Complex invW( const int nk, const int N );

	static Complex W_2D( const int nk[ 2 ], const int N[ 2 ] );
	static Complex invW_2D( const int nk[ 2 ], const int N[ 2 ] );

	static Complex W_3D( const int nk[ 3 ], const int N[ 3 ] );
	static Complex invW_3D( const int nk[ 3 ], const int N[ 3 ] );

	static void Radix2_r( Complex * data, const int power, Complex * pingPong );
	static void InverseRadix2_r( Complex * data, const int power, Complex * pingPong );
	static void Reorder2_r( Complex * data, const int power, Complex * pingPong );

	static void Radix4_r( Complex * data, const int power, Complex * pingPong );
	static void InverseRadix4_r( Complex * data, const int power, Complex * pingPong );
	static void Reorder4_r( Complex * data, const int power, Complex * pingPong );

	static void Radix8_r( Complex * data, const int power, Complex * pingPong );
	static void InverseRadix8_r( Complex * data, const int power, Complex * pingPong );
	static void Reorder8_r( Complex * data, const int power, Complex * pingPong );

	static void FFT_2D( const Complex * data_in, const int dim[ 2 ], Complex * data_out, FFTFunction_t * functor );
	static void FFT_3D( const Complex * data_in, const int dim[ 3 ], Complex * data_out, FFTFunction_t * functor );

	static void RotateData2D( Complex * data, const int dim[ 2 ] );
	static void RotateData3D( Complex * data, const int dim[ 3 ] );

	static bool FactorLame( const int num, const int start, int factors[ 2 ] );
};

/*
================================
Fourier::FactorLame
================================
*/
inline bool Fourier::FactorLame( const int num, const int start, int factors[ 2 ] ) {
	const int end = (int)sqrtf( (float)num );

	for ( int i = start; i <= end; i++ ) {
		int j = num / i;
		if ( i * j == num ) {
			factors[ 0 ] = i;
			factors[ 1 ] = j;
			return false;
		}
	}

	// If no primes are found then the number is prime
	return true;
}

/*
================================
Fourier::W
================================
*/
inline Complex Fourier::W( const int nk, const int N ) {
	const float angle = -2.0f * Math::PI * (float)nk / (float)N;

	return Complex( 1.0f, angle );
}

/*
================================
Fourier::invW
================================
*/
inline Complex Fourier::invW( const int nk, const int N ) {
	return Fourier::W( -nk, N );
}

/*
================================
Fourier::W_2D
================================
*/
inline Complex Fourier::W_2D( const int nk[ 2 ], const int N[ 2 ] ) {
	const float angle = -2.0f * Math::PI * ( (float)nk[ 0 ] / (float)N[ 0 ] + (float)nk[ 1 ] / (float)N[ 1 ] );

	return Complex( 1.0f, angle );
}

/*
================================
Fourier::invW_2D
================================
*/
inline Complex Fourier::invW_2D( const int nk[ 2 ], const int N[ 2 ] ) {
	int nk2[ 2 ] = { -nk[ 0 ], -nk[ 1 ] };
	return Fourier::W_2D( nk2, N );
}

/*
================================
Fourier::W_3D
================================
*/
inline Complex Fourier::W_3D( const int nk[ 3 ], const int N[ 3 ] ) {
	const float angle = -2.0f * Math::PI * ( (float)nk[ 0 ] / (float)N[ 0 ] + (float)nk[ 1 ] / (float)N[ 1 ] + (float)nk[ 2 ] / (float)N[ 2 ] );

	return Complex( 1.0f, angle );
}

/*
================================
Fourier::invW_3D
================================
*/
inline Complex Fourier::invW_3D( const int nk[ 3 ], const int N[ 3 ] ) {
	int nk2[ 3 ] = { -nk[ 0 ], -nk[ 1 ], -nk[ 2 ] };
	return Fourier::W_3D( nk2, N );
}

/*
================================
Fourier::Radix_2D
================================
*/
inline void Fourier::Radix2_2D( const Complex * data_in, const int dim[ 2 ], Complex * data_out ) {
	FFT_2D( data_in, dim, data_out, Radix2 );
}
inline void Fourier::Radix4_2D( const Complex * data_in, const int dim[ 2 ], Complex * data_out ) {
	FFT_2D( data_in, dim, data_out, Radix4 );
}
inline void Fourier::Radix8_2D( const Complex * data_in, const int dim[ 2 ], Complex * data_out ) {
	FFT_2D( data_in, dim, data_out, Radix8 );
}
inline void Fourier::InverseRadix2_2D( const Complex * data_in, const int dim[ 2 ], Complex * data_out ) {
	FFT_2D( data_in, dim, data_out, InverseRadix2 );
}
inline void Fourier::InverseRadix4_2D( const Complex * data_in, const int dim[ 2 ], Complex * data_out ) {
	FFT_2D( data_in, dim, data_out, InverseRadix4 );
}
inline void Fourier::InverseRadix8_2D( const Complex * data_in, const int dim[ 2 ], Complex * data_out ) {
	FFT_2D( data_in, dim, data_out, InverseRadix8 );
}


inline void Fourier::Radix8_512_2D( const Complex * data_in, Complex * data_out ) {
	const int dim[ 2 ] = { 512, 512 };
	FFT_2D( data_in, dim, data_out, Radix8_512 );
}

/*
================================
Fourier::Radix_3D
================================
*/
inline void Fourier::Radix2_3D( const Complex * data_in, const int dim[ 3 ], Complex * data_out ) {
	FFT_3D( data_in, dim, data_out, Radix2 );
}
inline void Fourier::Radix4_3D( const Complex * data_in, const int dim[ 3 ], Complex * data_out ) {
	FFT_3D( data_in, dim, data_out, Radix4 );
}
inline void Fourier::Radix8_3D( const Complex * data_in, const int dim[ 3 ], Complex * data_out ) {
	FFT_3D( data_in, dim, data_out, Radix8 );
}
inline void Fourier::InverseRadix2_3D( const Complex * data_in, const int dim[ 3 ], Complex * data_out ) {
	FFT_3D( data_in, dim, data_out, InverseRadix2 );
}
inline void Fourier::InverseRadix4_3D( const Complex * data_in, const int dim[ 3 ], Complex * data_out ) {
	FFT_3D( data_in, dim, data_out, InverseRadix4 );
}
inline void Fourier::InverseRadix8_3D( const Complex * data_in, const int dim[ 3 ], Complex * data_out ) {
	FFT_3D( data_in, dim, data_out, InverseRadix8 );
}

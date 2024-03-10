//
//	Fourier.cpp
//
#include "Fourier.h"
#include "Math.h"
#include "Morton.h"
#include <algorithm>

template <class T>
void SquareTranspose( T * data, const int width ) {
	for ( int x = 0; x < width; x++ ) {
		for ( int y = x + 1; y < width; y++ ) {
			const int idx0 = x + width * y;
			const int idx1 = y + width * x;
			std::swap( data[ idx0 ], data[ idx1 ] );
		}
	}
}

/*
================================
Fourier::DFT

Fourier transform from time domain to frequency domain or real space to momentum space
================================
*/
void Fourier::DFT( const Complex * data_in, const int num, Complex * data_out ) {
	for ( int k = 0; k < num; ++k ) {
		data_out[ k ].Zero();

		for ( int n = 0; n < num; ++n ) {
			const Complex w = Fourier::W( n * k, num );

			data_out[ k ] += data_in[ n ] * w;
		}
	}
}

/*
================================
Fourier::InverseDFT
================================
*/
void Fourier::InverseDFT( const Complex * data_in, const int num, Complex * data_out ) {
	const float fN = float( num );
	const float invN = 1.0f / fN;

	for ( int n = 0; n < num; ++n ) {
		data_out[ n ].Zero();

		for ( int k = 0; k < num; ++k ) {
			const Complex w = Fourier::invW( n * k, num );

			data_out[ n ] += data_in[ k ] * w;
		}
		data_out[ n ] *= invN;
	}
}

#pragma optimize( "", off )

/*
================================
Fourier::FractionalDFT

The fractional fourier transform is defined as:

sqrt( 1 - i cot( alpha ) ) * e^( i pi cot( alpha ) u^2 ) * Integral( e^( -i 2 pi ( csc( alpha ) u x - cot( alpha ) x^2 / 2 ) ) )

http://www.ee.bilkent.edu.tr/~haldun/publications/ozaktas166.pdf
"The Discrete Fractional Fourier Transform" by Kutay, Ozaktas, 2000
This is a very good description

https://www.researchgate.net/publication/3316077_Discrete_rotational_Fourier_transform

https://github.com/giulianograziani/FractionalFastFourier/blob/master/ReadMe/Implementation%20Details.pdf

https://arxiv.org/pdf/1909.13691.pdf  "A novel description and mathematical analysis of the Fractional Discrete Fourier Transform"

http://kilyos.ee.bilkent.edu.tr/~haldun/wileybook.html
// The Fractional Fourier Transform with Applications in Optics and Signal Processing by Ozaktas, Zalevsky, Kutay, published by Wiley & Sons
// might be a good idea to read that book
================================
*/
float DeltaFunction( int x ) {
	if ( 0 == x ) {
		return 1.0f;
	}
	return 0.0f;
}
Complex KappaA( Complex a0, Complex a1, Complex a2, Complex a3, int n, int k, int N ) {
	Complex W = Fourier::W( n * k, N );
	Complex invW = Fourier::invW( n * k, N );

	float invRootN = 1.0f / sqrtf( float( N ) );

	Complex kappa = a0 * DeltaFunction( n - k ) + a1 * invRootN * W + a2 * DeltaFunction( ( n + k ) % N ) + a3 * invRootN * invW;
	return kappa;
}
void Fourier::FractionalDFT( float alpha, const Complex * data_in, const int num, Complex * data_out ) {
	// This implementation is directly from https://www.researchgate.net/publication/3316077_Discrete_rotational_Fourier_transform
	// "Discrete rotational Fourier transform" by Santhanam, McClellan
	alpha *= HB_PI / 2.0f;

	const Complex J = Complex( Vec2( 0, 1 ) );
	const Complex eja = Complex( 1.0f, alpha );
	const Complex one = Complex( Vec2( 1, 0 ) );

	const Complex a0 = one + eja;
	const Complex a1 = one - J * eja;
	const Complex a2 = eja - one;
	const Complex a3 = ( one * -1.0f ) - J * eja;

	const Complex fa0 = a0 * cosf( alpha ) * 0.5f;
	const Complex fa1 = a1 * sinf( alpha ) * 0.5f;
	const Complex fa2 = a2 * cosf( alpha ) * 0.5f;
	const Complex fa3 = a3 * sinf( alpha ) * 0.5f;

	for ( int k = 0; k < num; ++k ) {
		data_out[ k ].Zero();

		for ( int n = 0; n < num; ++n ) {
			const Complex kappa = KappaA( fa0, fa1, fa2, fa3, n, k, num );

			data_out[ k ] += data_in[ n ] * kappa;
		}
	}
}

/*
================================
Fourier::DFT_2D
================================
*/
void Fourier::DFT_2D( const Complex * data_in, const int dim[ 2 ], Complex * data_out ) {
	int k[ 2 ] = { 0, 0 };
	int n[ 2 ] = { 0, 0 };

	for ( int kx = 0; kx < dim[ 0 ]; ++kx ) {
		for ( int ky = 0; ky < dim[ 1 ]; ++ky ) {
			const int kidx = kx + dim[ 0 ] * ky;
			data_out[ kidx ].Zero();

			for ( int nx = 0; nx < dim[ 0 ]; ++nx ) {
				for ( int ny = 0; ny < dim[ 1 ]; ++ny ) {
					const int nidx = nx + dim[ 0 ] * ny;
					const int nk[ 2 ] = { nx * kx, ny * ky };
					const Complex w = Fourier::W_2D( nk, dim );

					data_out[ kidx ] += data_in[ nidx ] * w;
				}
			}
		}
	}
}

/*
 ================================
 Fourier::InverseDFT_2D
 ================================
 */
void Fourier::InverseDFT_2D( const Complex * data_in, const int dim[ 2 ], Complex * data_out ) {
	int k[ 2 ] = { 0, 0 };
	int n[ 2 ] = { 0, 0 };

	for ( int nx = 0; nx < dim[ 0 ]; ++nx ) {
		for ( int ny = 0; ny < dim[ 1 ]; ++ny ) {
			const int nidx = nx + dim[ 0 ] * ny;
			data_out[ nidx ].Zero();

			for ( int kx = 0; kx < dim[ 0 ]; ++kx ) {
				for ( int ky = 0; ky < dim[ 1 ]; ++ky ) {		
					const int kidx = kx + dim[ 0 ] * ky;
					
					const int nk[ 2 ] = { nx * kx, ny * ky };
					const Complex w = Fourier::invW_2D( nk, dim );

					data_out[ nidx ] += data_in[ kidx ] * w;
				}
			}

			data_out[ nidx ] *= 1.0f / (float)( dim[ 0 ] * dim[ 1 ] );
		}
	}
}

/*
================================
Fourier::FractionalDFT_2D
================================
*/
void Fourier::FractionalDFT_2D( float alpha, const Complex * data_in, const int dim[ 2 ], Complex * data_out ) {
	Complex * tmpData = (Complex *)malloc( dim[ 0 ] * dim[ 1 ] * sizeof( Complex ) );

	for ( int y = 0; y < dim[ 1 ]; y++ ) {
		int x = 0;
		int offset = x + dim[ 0 ] * y;
		FractionalDFT( alpha, data_in + offset, dim[ 0 ], tmpData + offset );
		//Radix2( data_in + offset, dim[ 0 ], data_out + offset );
	}

	RotateData2D( tmpData, dim );

	for ( int x = 0; x < dim[ 0 ]; x++ ) {
		int y = 0;
		int offset = y + dim[ 1 ] * x;
		FractionalDFT( alpha, tmpData + offset, dim[ 1 ], data_out + offset );
		//Radix2( data_out + offset, dim[ 1 ], data_out + offset );
	}

	const int dim2[ 2 ] = { dim[ 1 ], dim[ 0 ] };
	RotateData2D( data_out, dim2 );

	free( tmpData );
}

/*
 ================================
 Fourier::DFT_3D
 ================================
 */
void Fourier::DFT_3D( const Complex * data_in, const int dim[ 3 ], Complex * data_out ) {
	int k[ 3 ] = { 0, 0, 0 };
	int n[ 3 ] = { 0, 0, 0 };

	for ( int kx = 0; kx < dim[ 0 ]; ++kx ) {
		for ( int ky = 0; ky < dim[ 1 ]; ++ky ) {
			for ( int kz = 0; kz < dim[ 2 ]; ++kz ) {
				const int kidx = kx + dim[ 0 ] * ky + dim[ 0 ] * dim[ 1 ] * kz;
				data_out[ kidx ].Zero();

				for ( int nx = 0; nx < dim[ 0 ]; ++nx ) {
					for ( int ny = 0; ny < dim[ 1 ]; ++ny ) {
						for ( int nz = 0; nz < dim[ 2 ]; ++nz ) {
							const int nidx = nx + dim[ 0 ] * ny + dim[ 0 ] * dim[ 1 ] * nz;
							const int nk[ 3 ] = { nx * kx, ny * ky, nz * kz };
							const Complex w = Fourier::W_3D( nk, dim );

							data_out[ kidx ] += data_in[ nidx ] * w;
						}
					}
				}
			}
		}
	}
}

/*
 ================================
 Fourier::InverseDFT_3D
 ================================
 */
void Fourier::InverseDFT_3D( const Complex * data_in, const int dim[ 3 ], Complex * data_out ) {
	int k[ 3 ] = { 0, 0, 0 };
	int n[ 3 ] = { 0, 0, 0 };

	for ( int nx = 0; nx < dim[ 0 ]; ++nx ) {
		for ( int ny = 0; ny < dim[ 1 ]; ++ny ) {
			for ( int nz = 0; nz < dim[ 2 ]; ++nz ) {
				const int nidx = nx + dim[ 0 ] * ny + dim[ 0 ] * dim[ 1 ] * nz;
				data_out[ nidx ].Zero();

				for ( int kx = 0; kx < dim[ 0 ]; ++kx ) {
					for ( int ky = 0; ky < dim[ 1 ]; ++ky ) {
						for ( int kz = 0; kz < dim[ 2 ]; ++kz ) {
							const int kidx = kx + dim[ 0 ] * ky + dim[ 0 ] * dim[ 1 ] * kz;
					
							const int nk[ 3 ] = { nx * kx, ny * ky, nz * kz };
							const Complex w = Fourier::invW_3D( nk, dim );

							data_out[ nidx ] += data_in[ kidx ] * w;
						}
					}
				}

				data_out[ nidx ] *= 1.0f / (float)( dim[ 0 ] * dim[ 1 ] * dim[ 2 ] );
			}
		}
	}
}

/*
================================
TransposeInPlace
Follow the cycles
================================
*/
void TransposeInPlace( Complex * data, const int dim[ 2 ] ) {
	const int w = dim[ 0 ];
	const int h = dim[ 1 ];

	for ( int start = 0; start <= w * h - 1; ++start ) {
		int next = start;
		int i = 0;

		do {
			++i;
			next = ( next % h ) * w + next / h;
		} while ( next > start );

		if ( next >= start && i != 1 ) {
			const Complex tmp = data[ start ];
			next = start;
			do {
				i = ( next % h ) * w + next / h;
				data[ next ] = ( i == start ) ? tmp : data[ i ];
				next = i;
			} while ( next > start );
		}
	}
}

/*
================================
Fourier::RotateData2D
================================
*/
void Fourier::RotateData2D( Complex * data, const int dim[ 2 ] ) {
	if ( dim[ 0 ] == dim[ 1 ] ) {
		for ( int x = 0; x < dim[ 0 ]; x++ ) {
			for ( int y = x + 1; y < dim[ 1 ]; y++ ) {
				const int idx0 = x + dim[ 0 ] * y;
				const int idx1 = y + dim[ 1 ] * x;
				std::swap( data[ idx0 ], data[ idx1 ] );
			}
		}
	} else {
#if 0
		const int num = dim[ 0 ] * dim[ 1 ];
		Complex * buffer = new Complex[ num ];

		for ( int i = 0; i < num; i++ ) {
			buffer[ i ] = data[ i ];
		}

		for ( int x = 0; x < dim[ 0 ]; x++ ) {
			for ( int y = 0; y < dim[ 1 ]; y++ ) {
				const int idx0 = x + dim[ 0 ] * y;
				const int idx1 = y + dim[ 1 ] * x;
				data[ idx0 ] = buffer[ idx1 ];
			}
		}

		delete[] buffer;
#else
		TransposeInPlace( data, dim );
#endif
	}
}

/*
================================
Fourier::FFT_2D
================================
*/
void Fourier::FFT_2D( const Complex * data_in, const int dim[ 2 ], Complex * data_out, FFTFunction_t * functor ) {
	for ( int y = 0; y < dim[ 1 ]; y++ ) {
		int x = 0;
		int offset = x + dim[ 0 ] * y;
		functor( data_in + offset, dim[ 0 ], data_out + offset );
	}

	RotateData2D( data_out, dim );

	for ( int x = 0; x < dim[ 0 ]; x++ ) {
		int y = 0;
		int offset = y + dim[ 1 ] * x;
		functor( data_out + offset, dim[ 1 ], data_out + offset );
	}

	const int dim2[ 2 ] = { dim[ 1 ], dim[ 0 ] };
	RotateData2D( data_out, dim2 );
}

/*
================================
Fourier::RotateData3D
================================
*/
void Fourier::RotateData3D( Complex * data, const int dim[ 3 ] ) {
	const int num = dim[ 0 ] * dim[ 1 ] * dim[ 2 ];
	Complex * buffer = new Complex[ num ];

	for ( int i = 0; i < num; i++ ) {
		buffer[ i ] = data[ i ];
	}

	for ( int x = 0; x < dim[ 0 ]; x++ ) {
		for ( int y = 0; y < dim[ 1 ]; y++ ) {
			for ( int z = 0; z < dim[ 2 ]; z++ ) {
				int idx0 = x + dim[ 0 ] * y + dim[ 0 ] * dim[ 1 ] * z;
				int idx1 = y + dim[ 1 ] * z + dim[ 1 ] * dim[ 2 ] * x;
				data[ idx0 ] = buffer[ idx1 ];
			}
		}
	}

	delete[] buffer;
}

/*
================================
Fourier::FFT_3D
================================
*/
void Fourier::FFT_3D( const Complex * data_in, const int dim[ 3 ], Complex * data_out, FFTFunction_t * functor ) {
	for ( int y = 0; y < dim[ 1 ]; y++ ) {
		for ( int z = 0; z < dim[ 2 ]; z++ ) {
			int x = 0;
			int offset = x + dim[ 0 ] * y + dim[ 0 ] * dim[ 1 ] * z;
			functor( data_in + offset, dim[ 0 ], data_out + offset );
		}
	}

	RotateData3D( data_out, dim );

	for ( int z = 0; z < dim[ 2 ]; z++ ) {
		for ( int x = 0; x < dim[ 0 ]; x++ ) {
			int y = 0;
			int offset = y + dim[ 1 ] * z + dim[ 1 ] * dim[ 2 ] * x;
			functor( data_out + offset, dim[ 1 ], data_out + offset );
		}
	}

	const int dim2[ 3 ] = { dim[ 1 ], dim[ 2 ], dim[ 0 ] };
	RotateData3D( data_out, dim2 );

	for ( int x = 0; x < dim[ 0 ]; x++ ) {
		for ( int y = 0; y < dim[ 1 ]; y++ ) {
			int z = 0;
			int offset = z + dim[ 2 ] * x + dim[ 2 ] * dim[ 0 ] * y;
			functor( data_out + offset, dim[ 2 ], data_out + offset );
		}
	}

	const int dim3[ 3 ] = { dim[ 2 ], dim[ 0 ], dim[ 1 ] };
	RotateData3D( data_out, dim3 );
}

/*
================================
Fourier::Radix2
================================
*/
void Fourier::Radix2( const Complex * data_in, const int num, Complex * data_out ) {
	if ( num < 2 ) {
		return;
	}

	// Make sure power of 2
	if ( !Morton::IsPowerOfTwo( num ) ) {
		return;
	}

	// Determine the power
	int power = 0;
	{
		int n = num;
		while ( ( n & 1 ) == 0 ) {
			power++;
			n = n >> 1;
		}
	}

	// Copy data
	if ( data_in != data_out ) {
		for ( int n = 0; n < num; n++ ) {
			data_out[ n ] = data_in[ n ];
		}
	}

	Complex * pingpong = new Complex[ num ];

	// Perform the FFT
	Radix2_r( data_out, power, pingpong );
	Reorder2_r( data_out, power, pingpong );

	delete[] pingpong;
}

/*
================================
Fourier::Radix2_r
================================
*/
void Fourier::Radix2_r( Complex * data, const int power, Complex * pingPong ) {
	const int num = ( 1 << power );
	const int halfNum = num >> 1;
#if 0
	if ( num > 2 ) {
		// Swap the first half odds and the second half evens
		for ( int n = 0; n < num; n++ ) {
			pingPong[ n ] = data[ n ];
		}
		for ( int n = 0; n < halfNum; n++ ) {
			data[ n + 0 * halfNum ] = pingPong[ 2 * n + 0 ];
			data[ n + 1 * halfNum ] = pingPong[ 2 * n + 1 ];
		}
	}

	if ( power > 1 ) {
		Radix2_r( data + 0 * halfNum, power - 1, pingPong );
		Radix2_r( data + 1 * halfNum, power - 1, pingPong );
	}

	// Perform the simplified DFT here
	for ( int n = 0; n < halfNum; n++ ) {
#if 1	// Amazingly, both of these work equally as well
		Complex w0 = Fourier::W( n + 0 * halfNum, num );
		Complex w1 = Fourier::W( n + 1 * halfNum, num );
		//Complex w1 = -1.0f * w0;

		Complex tmp0 = data[ n + 0 * halfNum ];
		Complex tmp1 = data[ n + 1 * halfNum ];

		data[ n + 0 * halfNum ] = tmp0 + w0 * tmp1;
		data[ n + 1 * halfNum ] = tmp0 + w1 * tmp1;
#else
		Complex w0 = Fourier::W( n * 0, num );
		Complex w1 = Fourier::W( n * 1, num );
		//Complex w1 = -1.0f * w0;

		Complex tmp0 = data[ n + 0 * halfNum ];
		Complex tmp1 = data[ n + 1 * halfNum ];

		data[ n + 0 * halfNum ] = ( tmp0 + tmp1 ) * w0;
		data[ n + 1 * halfNum ] = ( tmp0 - tmp1 ) * w1;
#endif
	}
#else
	// Perform the simplified DFT here
	for ( int n = 0; n < halfNum; n++ ) {
		Complex W0 = Fourier::W( 0 * n, num );
		Complex W1 = Fourier::W( 1 * n, num );

		Complex data0 = data[ n + 0 * halfNum ];
		Complex data1 = data[ n + 1 * halfNum ];

		data[ n + 0 * halfNum ] = ( data0 + data1 ) * W0;
		data[ n + 1 * halfNum ] = ( data0 - data1 ) * W1;
	}

	if ( power > 1 ) {
		Radix2_r( data + 0 * halfNum, power - 1, pingPong );
		Radix2_r( data + 1 * halfNum, power - 1, pingPong );
	}
#endif
}

/*
================================
Fourier::InverseRadix2
================================
*/
void Fourier::InverseRadix2( const Complex * data_in, const int num, Complex * data_out ) {
	if ( num < 2 ) {
		return;
	}

	// Make sure power of 2
	if ( !Morton::IsPowerOfTwo( num ) ) {
		return;
	}

	// Determine the power
	int power = 0;
	{
		int n = num;
		while ( ( n & 1 ) == 0 ) {
			power++;
			n = n >> 1;
		}
	}

	// Copy data
	for ( int n = 0; n < num; n++ ) {
		data_out[ n ] = data_in[ n ] * ( 1.0f / (float)num );
	}

	Complex * pingpong = new Complex[ num ];

	// Perform the FFT
	InverseRadix2_r( data_out, power, pingpong );
	Reorder2_r( data_out, power, pingpong );

	delete[] pingpong;
}

/*
================================
Fourier::InverseRadix2_r
================================
*/
void Fourier::InverseRadix2_r( Complex * data, const int power, Complex * pingPong ) {
#if 0
	const int num = ( 1 << power );
	const int halfNum = num >> 1;

	// Sort
	if ( num > 2 ) {
		for ( int n = 0; n < num; n++ ) {
			pingPong[ n ] = data[ n ];
		}
		for ( int n = 0; n < halfNum; n++ ) {
			data[ n ] = pingPong[ 2 * n ];
			data[ n + halfNum ] = pingPong[ 2 * n + 1 ];
		}
	}

	// Do the little bits first
	if ( power > 1 ) {
		InverseRadix2_r( data, power - 1, pingPong );
		InverseRadix2_r( data + halfNum, power - 1, pingPong );
	}

	// Perform the simplified DFT here
	for ( int n = 0; n < halfNum; n++ ) {
		const Complex w0 = Fourier::invW( n, num );
		const Complex w1 = -1.0f * w0;

		Complex tmp0 = data[ n ];
		Complex tmp1 = data[ n + halfNum ];

		data[ n ] = tmp0 + w0 * tmp1;
		data[ n + halfNum ] = tmp0 + w1 * tmp1;
	}
#else
	const int num = ( 1 << power );
	const int halfNum = num >> 1;

	// Perform the simplified DFT here
	for ( int n = 0; n < halfNum; n++ ) {
		Complex W0 = Fourier::invW( 0 * n, num );
		Complex W1 = Fourier::invW( 1 * n, num );

		Complex data0 = data[ n + 0 * halfNum ];
		Complex data1 = data[ n + 1 * halfNum ];

		data[ n + 0 * halfNum ] = ( data0 + data1 ) * W0;
		data[ n + 1 * halfNum ] = ( data0 - data1 ) * W1;
	}

	if ( power > 1 ) {
		InverseRadix2_r( data + 0 * halfNum, power - 1, pingPong );
		InverseRadix2_r( data + 1 * halfNum, power - 1, pingPong );
	}
#endif
}

/*
================================
Fourier::Reorder2_r
================================
*/
void Fourier::Reorder2_r( Complex * data, const int power, Complex * pingPong ) {
	const int num = 1 << power;
	const int halfNum = num / 2;

	if ( num > 2 ) {
#if 0
		// Swap the first half odds and the second half evens
		for ( int n = 0; n < num; n++ ) {
			pingPong[ n ] = data[ n ];
		}
		for ( int n = 0; n < halfNum; n++ ) {
			data[ n + 0 * halfNum ] = pingPong[ 2 * n + 0 ];
			data[ n + 1 * halfNum ] = pingPong[ 2 * n + 1 ];
		}
#elif 0
		// Swap the first half odds and the second half evens
		for ( int p = 1; p < halfNum; p++ ) {
			int start = p;
			int end = num - start;

			for ( int n = start; n < end; n += 2 ) {
				hbSwap( data[ n ] , data[ n + 1 ] );
			}
		}
#elif 0
		// Swap the first half odds and the second half evens
		for ( int p = 1; p < halfNum; p++ ) {
			int start = p - 1;
			int end = num - start - 2;

			for ( int n = start; n < end; n += 2 ) {
				SquareTranspose( data + n, 2 );
			}
		}
#else
		int dim[ 2 ];
		dim[ 0 ] = 2;
		dim[ 1 ] = num / 2;
		RotateData2D( data, dim );
#endif

		Reorder2_r( data + 0 * halfNum, power - 1, pingPong );
		Reorder2_r( data + 1 * halfNum, power - 1, pingPong );
	}
}

/*
================================
Fourier::Radix4
================================
*/
void Fourier::Radix4( const Complex * data_in, const int num, Complex * data_out ) {
	if ( num < 4 ) {
		return;
	}

	// Make sure power of 4
	if ( !Morton::IsPowerOfFour( num ) ) {
		return;
	}

	// Determine the power
	int power = 0;
	{
		int n = num;
		while ( ( n & 1 ) == 0 ) {
			power++;
			n = n >> 1;
		}
	}

	// Copy data
	if ( data_in != data_out ) {
		for ( int n = 0; n < num; n++ ) {
			data_out[ n ] = data_in[ n ];
		}
	}

	Complex * pingpong = new Complex[ num ];

	// Perform the FFT
	Radix4_r( data_out, power, pingpong );
	Reorder4_r( data_out, power, pingpong );

	delete[] pingpong;
}

/*
================================
Fourier::Radix4_r
================================
*/
void Fourier::Radix4_r( Complex * data, const int power, Complex * pingPong ) {
	const int num = ( 1 << power );
	const int quarterNum = num >> 2;

	// Perform the simplified DFT here
	for ( int n = 0; n < quarterNum; n++ ) {
		const Complex j = Complex( Vec2( 0, 1 ) );

		Complex W0 = Fourier::W( 0 * n, num );
		Complex W1 = Fourier::W( 1 * n, num );
		Complex W2 = Fourier::W( 2 * n, num );
		Complex W3 = Fourier::W( 3 * n, num );

		Complex data0 = data[ n + 0 * quarterNum ];
		Complex data1 = data[ n + 1 * quarterNum ];
		Complex data2 = data[ n + 2 * quarterNum ];
		Complex data3 = data[ n + 3 * quarterNum ];

		data[ n + 0 * quarterNum ] = ( data0 +     data1 + data2 +     data3 ) * W0;
		data[ n + 1 * quarterNum ] = ( data0 - j * data1 - data2 + j * data3 ) * W1;
		data[ n + 2 * quarterNum ] = ( data0 -     data1 + data2 -     data3 ) * W2;
		data[ n + 3 * quarterNum ] = ( data0 + j * data1 - data2 - j * data3 ) * W3;
	}

	if ( power > 2 ) {
		Radix4_r( data + 0 * quarterNum, power - 2, pingPong );
		Radix4_r( data + 1 * quarterNum, power - 2, pingPong );
		Radix4_r( data + 2 * quarterNum, power - 2, pingPong );
		Radix4_r( data + 3 * quarterNum, power - 2, pingPong );
	}
#if 0
	const int num = ( 1 << power );
	const int halfNum = num >> 1;

	// Perform the simplified DFT here
	for ( int n = 0; n < halfNum; n++ ) {
		Complex W0 = Fourier::invW( 0 * n, num );
		Complex W1 = Fourier::invW( 1 * n, num );

		Complex data0 = data[ n + 0 * halfNum ];
		Complex data1 = data[ n + 1 * halfNum ];

		data[ n + 0 * halfNum ] = ( data0 + data1 ) * W0;
		data[ n + 1 * halfNum ] = ( data0 - data1 ) * W1;
	}

	if ( power > 1 ) {
		InverseRadix2_r( data + 0 * halfNum, power - 1, pingPong );
		InverseRadix2_r( data + 1 * halfNum, power - 1, pingPong );
	}
#endif
}

/*
================================
Fourier::InverseRadix4
================================
*/
void Fourier::InverseRadix4( const Complex * data_in, const int num, Complex * data_out ) {
	if ( num < 4 ) {
		return;
	}

	// Make sure power of 4
	if ( !Morton::IsPowerOfFour( num ) ) {
		return;
	}

	// Determine the power
	int power = 0;
	{
		int n = num;
		while ( ( n & 1 ) == 0 ) {
			power++;
			n = n >> 1;
		}
	}

	// Copy data
	for ( int n = 0; n < num; n++ ) {
		data_out[ n ] = data_in[ n ] * ( 1.0f / (float)num );
	}

	Complex * pingpong = new Complex[ num ];

	// Perform the FFT
	InverseRadix4_r( data_out, power, pingpong );
	Reorder4_r( data_out, power, pingpong );

	delete[] pingpong;
}

/*
================================
Fourier::InverseRadix4_r
================================
*/
void Fourier::InverseRadix4_r( Complex * data, const int power, Complex * pingPong ) {
	const int num = ( 1 << power );
	//const int halfNum = num >> 1;
	const int quarterNum = num >> 2;

	// Perform the simplified DFT here
	for ( int n = 0; n < quarterNum; n++ ) {
		const Complex j = Complex( Vec2( 0, 1 ) );

		Complex W0 = Fourier::invW( 0 * n, num );
		Complex W1 = Fourier::invW( 1 * n, num );
		Complex W2 = Fourier::invW( 2 * n, num );
		Complex W3 = Fourier::invW( 3 * n, num );

		Complex data0 = data[ n + 0 * quarterNum ];
		Complex data1 = data[ n + 1 * quarterNum ];
		Complex data2 = data[ n + 2 * quarterNum ];
		Complex data3 = data[ n + 3 * quarterNum ];

		data[ n + 0 * quarterNum ] = ( data0 +     data1 + data2 +     data3 ) * W0;
		data[ n + 1 * quarterNum ] = ( data0 + j * data1 - data2 - j * data3 ) * W1;
		data[ n + 2 * quarterNum ] = ( data0 -     data1 + data2 -     data3 ) * W2;
		data[ n + 3 * quarterNum ] = ( data0 - j * data1 - data2 + j * data3 ) * W3;
	}

	if ( power > 2 ) {
		InverseRadix4_r( data + 0 * quarterNum, power - 2, pingPong );
		InverseRadix4_r( data + 1 * quarterNum, power - 2, pingPong );
		InverseRadix4_r( data + 2 * quarterNum, power - 2, pingPong );
		InverseRadix4_r( data + 3 * quarterNum, power - 2, pingPong );
	}
}

/*
================================
Fourier::Reorder4_r
================================
*/
void Fourier::Reorder4_r( Complex * data, const int power, Complex * pingPong ) {
	const int num = 1 << power;
	const int quarterNum = num / 4;

	if ( num > 4 ) {
#if 0
		// Swap the first half odds and the second half evens
		// Put the 0-mod4 in the first quarter
		// put the 1-mod4 in the second
		// put the 2-mod4 in the third
		// put the 3-mod4 in the fourth
		for ( int n = 0; n < num; n++ ) {
			pingPong[ n ] = data[ n ];
		}
		for ( int n = 0; n < quarterNum; n++ ) {
			data[ n + 0 * quarterNum ] = pingPong[ 4 * n + 0 ];
			data[ n + 1 * quarterNum ] = pingPong[ 4 * n + 1 ];
			data[ n + 2 * quarterNum ] = pingPong[ 4 * n + 2 ];
			data[ n + 3 * quarterNum ] = pingPong[ 4 * n + 3 ];
		}
#else
		int dim[ 2 ];
		dim[ 0 ] = 4;
		dim[ 1 ] = num / 4;
		RotateData2D( data, dim );
#endif
	}

	if ( power > 2 ) {

		Reorder4_r( data + 0 * quarterNum, power - 2, pingPong );
		Reorder4_r( data + 1 * quarterNum, power - 2, pingPong );
		Reorder4_r( data + 2 * quarterNum, power - 2, pingPong );
		Reorder4_r( data + 3 * quarterNum, power - 2, pingPong );
	}
}

/*
================================
Fourier::Radix8
================================
*/
void Fourier::Radix8( const Complex * data_in, const int num, Complex * data_out ) {
	if ( num < 8 ) {
		return;
	}

	// Make sure power of 8
	if ( !Morton::IsPowerOfEight( num ) ) {
		return;
	}

	// Determine the power
	int power = 0;
	{
		int n = num;
		while ( ( n & 1 ) == 0 ) {
			power++;
			n = n >> 1;
		}
	}

	// Copy data
	if ( data_in != data_out ) {
		for ( int n = 0; n < num; n++ ) {
			data_out[ n ] = data_in[ n ];
		}
	}

	Complex * pingpong = new Complex[ num ];

	// Perform the FFT
	Radix8_r( data_out, power, pingpong );
	Reorder8_r( data_out, power, pingpong );

	delete[] pingpong;
}

/*
================================
Fourier::Radix8_r
================================
*/
void Fourier::Radix8_r( Complex * data, const int power, Complex * pingPong ) {
	const int num = ( 1 << power );
	const int eighthNum = num >> 3;
	
	// TODO: Precompute this
	Complex twiddles[ 8 ][ 8 ];
	for ( int x = 0; x < 8; x++ ) {
		for ( int y = 0; y < 8; y++ ) {
			const float angle = HB_PI * 2.0f * float( x * y ) / 8.0f;
			twiddles[ x ][ y ] = Complex( 1.0f, -angle );
		}
	}

	// Perform the simplified DFT here
	for ( int n = 0; n < eighthNum; n++ ) {
		Complex butterFly[ 8 ];
		for ( int i = 0; i < 8; i++ ) {
			butterFly[ i ].Zero();

			for ( int j = 0; j < 8; j++ ) {
				butterFly[ i ] += twiddles[ i ][ j ] * data[ n + j * eighthNum ];
			}
		}

		data[ n + 0 * eighthNum ] = butterFly[ 0 ] * W( 0 * n, num );
		data[ n + 1 * eighthNum ] = butterFly[ 1 ] * W( 1 * n, num );
		data[ n + 2 * eighthNum ] = butterFly[ 2 ] * W( 2 * n, num );
		data[ n + 3 * eighthNum ] = butterFly[ 3 ] * W( 3 * n, num );
		data[ n + 4 * eighthNum ] = butterFly[ 4 ] * W( 4 * n, num );
		data[ n + 5 * eighthNum ] = butterFly[ 5 ] * W( 5 * n, num );
		data[ n + 6 * eighthNum ] = butterFly[ 6 ] * W( 6 * n, num );
		data[ n + 7 * eighthNum ] = butterFly[ 7 ] * W( 7 * n, num );
	}

	if ( power > 3 ) {
		Radix8_r( data + 0 * eighthNum, power - 3, pingPong );
		Radix8_r( data + 1 * eighthNum, power - 3, pingPong );
		Radix8_r( data + 2 * eighthNum, power - 3, pingPong );
		Radix8_r( data + 3 * eighthNum, power - 3, pingPong );
		Radix8_r( data + 4 * eighthNum, power - 3, pingPong );
		Radix8_r( data + 5 * eighthNum, power - 3, pingPong );
		Radix8_r( data + 6 * eighthNum, power - 3, pingPong );
		Radix8_r( data + 7 * eighthNum, power - 3, pingPong );
	}
}

void Fourier::Radix8_512_1( Complex * data ) {
	const int eighthNum = 1;
	const int num = 8;

	// TODO: Precompute this
	Complex twiddles[ 8 ][ 8 ];
	for ( int x = 0; x < 8; x++ ) {
		for ( int y = 0; y < 8; y++ ) {
			const float angle = HB_PI * 2.0f * float( x * y ) / 8.0f;
			twiddles[ x ][ y ] = Complex( 1.0f, -angle );
		}
	}

	// Perform the simplified DFT here
	for ( int n = 0; n < eighthNum; n++ ) {
		Complex butterFly[ 8 ];
		for ( int i = 0; i < 8; i++ ) {
			butterFly[ i ].Zero();

			for ( int j = 0; j < 8; j++ ) {
				butterFly[ i ] += twiddles[ i ][ j ] * data[ n + j * eighthNum ];
			}
		}

		data[ n + 0 * eighthNum ] = butterFly[ 0 ] * W( 0 * n, num );
		data[ n + 1 * eighthNum ] = butterFly[ 1 ] * W( 1 * n, num );
		data[ n + 2 * eighthNum ] = butterFly[ 2 ] * W( 2 * n, num );
		data[ n + 3 * eighthNum ] = butterFly[ 3 ] * W( 3 * n, num );
		data[ n + 4 * eighthNum ] = butterFly[ 4 ] * W( 4 * n, num );
		data[ n + 5 * eighthNum ] = butterFly[ 5 ] * W( 5 * n, num );
		data[ n + 6 * eighthNum ] = butterFly[ 6 ] * W( 6 * n, num );
		data[ n + 7 * eighthNum ] = butterFly[ 7 ] * W( 7 * n, num );
	}
}

void Fourier::Radix8_512_2( Complex * data ) {
	const int eighthNum = 8;
	const int num = 64;

	// TODO: Precompute this
	Complex twiddles[ 8 ][ 8 ];
	for ( int x = 0; x < 8; x++ ) {
		for ( int y = 0; y < 8; y++ ) {
			const float angle = HB_PI * 2.0f * float( x * y ) / 8.0f;
			twiddles[ x ][ y ] = Complex( 1.0f, -angle );
		}
	}

	// Perform the simplified DFT here
	for ( int n = 0; n < eighthNum; n++ ) {
		Complex butterFly[ 8 ];
		for ( int i = 0; i < 8; i++ ) {
			butterFly[ i ].Zero();

			for ( int j = 0; j < 8; j++ ) {
				butterFly[ i ] += twiddles[ i ][ j ] * data[ n + j * eighthNum ];
			}
		}

		data[ n + 0 * eighthNum ] = butterFly[ 0 ] * W( 0 * n, num );
		data[ n + 1 * eighthNum ] = butterFly[ 1 ] * W( 1 * n, num );
		data[ n + 2 * eighthNum ] = butterFly[ 2 ] * W( 2 * n, num );
		data[ n + 3 * eighthNum ] = butterFly[ 3 ] * W( 3 * n, num );
		data[ n + 4 * eighthNum ] = butterFly[ 4 ] * W( 4 * n, num );
		data[ n + 5 * eighthNum ] = butterFly[ 5 ] * W( 5 * n, num );
		data[ n + 6 * eighthNum ] = butterFly[ 6 ] * W( 6 * n, num );
		data[ n + 7 * eighthNum ] = butterFly[ 7 ] * W( 7 * n, num );
	}

	{
		Radix8_512_1( data + 0 * eighthNum );
		Radix8_512_1( data + 1 * eighthNum );
		Radix8_512_1( data + 2 * eighthNum );
		Radix8_512_1( data + 3 * eighthNum );
		Radix8_512_1( data + 4 * eighthNum );
		Radix8_512_1( data + 5 * eighthNum );
		Radix8_512_1( data + 6 * eighthNum );
		Radix8_512_1( data + 7 * eighthNum );
	}
}

void Fourier::Radix8_512_3( Complex * data ) {
	const int eighthNum = 64;
	const int num = 512;

	// TODO: Precompute this
	Complex twiddles[ 8 ][ 8 ];
	for ( int x = 0; x < 8; x++ ) {
		for ( int y = 0; y < 8; y++ ) {
			const float angle = HB_PI * 2.0f * float( x * y ) / 8.0f;
			twiddles[ x ][ y ] = Complex( 1.0f, -angle );
		}
	}

	// Perform the simplified DFT here
	for ( int n = 0; n < eighthNum; n++ ) {
		Complex butterFly[ 8 ];
		for ( int i = 0; i < 8; i++ ) {
			butterFly[ i ].Zero();

			for ( int j = 0; j < 8; j++ ) {
				butterFly[ i ] += twiddles[ i ][ j ] * data[ n + j * eighthNum ];
			}
		}

		data[ n + 0 * eighthNum ] = butterFly[ 0 ] * W( 0 * n, num );
		data[ n + 1 * eighthNum ] = butterFly[ 1 ] * W( 1 * n, num );
		data[ n + 2 * eighthNum ] = butterFly[ 2 ] * W( 2 * n, num );
		data[ n + 3 * eighthNum ] = butterFly[ 3 ] * W( 3 * n, num );
		data[ n + 4 * eighthNum ] = butterFly[ 4 ] * W( 4 * n, num );
		data[ n + 5 * eighthNum ] = butterFly[ 5 ] * W( 5 * n, num );
		data[ n + 6 * eighthNum ] = butterFly[ 6 ] * W( 6 * n, num );
		data[ n + 7 * eighthNum ] = butterFly[ 7 ] * W( 7 * n, num );
	}

	{
		Radix8_512_2( data + 0 * eighthNum );
		Radix8_512_2( data + 1 * eighthNum );
		Radix8_512_2( data + 2 * eighthNum );
		Radix8_512_2( data + 3 * eighthNum );
		Radix8_512_2( data + 4 * eighthNum );
		Radix8_512_2( data + 5 * eighthNum );
		Radix8_512_2( data + 6 * eighthNum );
		Radix8_512_2( data + 7 * eighthNum );
	}
}

const int gReorder8_512_Lookups[ 512 ] = { 0, 64, 128, 192, 256, 320, 384, 448,
	8, 72, 136, 200, 264, 328, 392, 456, 16, 80, 144, 208, 272, 336, 400, 464,
	24, 88, 152, 216, 280, 344, 408, 472, 32, 96, 160, 224, 288, 352, 416, 480,
	40, 104, 168, 232, 296, 360, 424, 488, 48, 112, 176, 240, 304, 368, 432, 496,
	56, 120, 184, 248, 312, 376, 440, 504, 1, 65, 129, 193, 257, 321, 385, 449, 9,
	73, 137, 201, 265, 329, 393, 457, 17, 81, 145, 209, 273, 337, 401, 465, 25, 89,
	153, 217, 281, 345, 409, 473, 33, 97, 161, 225, 289, 353, 417, 481, 41, 105, 169,
	233, 297, 361, 425, 489, 49, 113, 177, 241, 305, 369, 433, 497, 57, 121, 185, 249,
	313, 377, 441, 505, 2, 66, 130, 194, 258, 322, 386, 450, 10, 74, 138, 202, 266,
	330, 394, 458, 18, 82, 146, 210, 274, 338, 402, 466, 26, 90, 154, 218, 282, 346,
	410, 474, 34, 98, 162, 226, 290, 354, 418, 482, 42, 106, 170, 234, 298, 362, 426,
	490, 50, 114, 178, 242, 306, 370, 434, 498, 58, 122, 186, 250, 314, 378, 442, 506,
	3, 67, 131, 195, 259, 323, 387, 451, 11, 75, 139, 203, 267, 331, 395, 459, 19, 83,
	147, 211, 275, 339, 403, 467, 27, 91, 155, 219, 283, 347, 411, 475, 35, 99, 163, 227,
	291, 355, 419, 483, 43, 107, 171, 235, 299, 363, 427, 491, 51, 115, 179, 243, 307, 371,
	435, 499, 59, 123, 187, 251, 315, 379, 443, 507, 4, 68, 132, 196, 260, 324, 388, 452,
	12, 76, 140, 204, 268, 332, 396, 460, 20, 84, 148, 212, 276, 340, 404, 468, 28, 92, 156,
	220, 284, 348, 412, 476, 36, 100, 164, 228, 292, 356, 420, 484, 44, 108, 172, 236, 300,
	364, 428, 492, 52, 116, 180, 244, 308, 372, 436, 500, 60, 124, 188, 252, 316, 380, 444,
	508, 5, 69, 133, 197, 261, 325, 389, 453, 13, 77, 141, 205, 269, 333, 397, 461, 21, 85,
	149, 213, 277, 341, 405, 469, 29, 93, 157, 221, 285, 349, 413, 477, 37, 101, 165, 229,
	293, 357, 421, 485, 45, 109, 173, 237, 301, 365, 429, 493, 53, 117, 181, 245, 309, 373,
	437, 501, 61, 125, 189, 253, 317, 381, 445, 509, 6, 70, 134, 198, 262, 326, 390, 454, 14,
	78, 142, 206, 270, 334, 398, 462, 22, 86, 150, 214, 278, 342, 406, 470, 30, 94, 158, 222,
	286, 350, 414, 478, 38, 102, 166, 230, 294, 358, 422, 486, 46, 110, 174, 238, 302, 366,
	430, 494, 54, 118, 182, 246, 310, 374, 438, 502, 62, 126, 190, 254, 318, 382, 446, 510,
	7, 71, 135, 199, 263, 327, 391, 455, 15, 79, 143, 207, 271, 335, 399, 463, 23, 87, 151,
	215, 279, 343, 407, 471, 31, 95, 159, 223, 287, 351, 415, 479, 39, 103, 167, 231, 295, 359,
	423, 487, 47, 111, 175, 239, 303, 367, 431, 495, 55, 119, 183, 247, 311, 375, 439, 503, 63,
	127, 191, 255, 319, 383, 447, 511
};

void Fourier::Reorder8_512( Complex * data_out ) {
	Complex * tmp = new Complex[ 512 ];

	for ( int i = 0; i < 512; i++ ) {
		tmp[ i ] = data_out[ i ];
	}
	for ( int i = 0; i < 512; i++ ) {
		const int idx = gReorder8_512_Lookups[ i ];
		data_out[ i ] = tmp[ idx ];
	}

	delete[] tmp;
}

void Fourier::Radix8_512( const Complex * data_in, const int num, Complex * data_out ) {
	if ( data_in != data_out ) {
		for ( int i = 0; i < 512; i++ ) {
			data_out[ i ] = data_in[ i ];
		}
	}

	Radix8_512_3( data_out );
#if 1
	Reorder8_r( data_out, 9, NULL );
#else
	Reorder8_512( data_out );
#endif
}

/*
================================
Fourier::InverseRadix8
================================
*/
void Fourier::InverseRadix8( const Complex * data_in, const int num, Complex * data_out ) {
	if ( num < 8 ) {
		return;
	}

	// Make sure power of 8
	if ( !Morton::IsPowerOfEight( num ) ) {
		return;
	}

	// Determine the power
	int power = 0;
	{
		int n = num;
		while ( ( n & 1 ) == 0 ) {
			power++;
			n = n >> 1;
		}
	}

	// Copy data
	for ( int n = 0; n < num; n++ ) {
		data_out[ n ] = data_in[ n ] * ( 1.0f / (float)num );
	}

	Complex * pingpong = new Complex[ num ];

	// Perform the FFT
	InverseRadix8_r( data_out, power, pingpong );
	Reorder8_r( data_out, power, pingpong );

	delete[] pingpong;
}

/*
================================
Fourier::InverseRadix8_r
================================
*/
void Fourier::InverseRadix8_r( Complex * data, const int power, Complex * pingPong ) {
	const int num = ( 1 << power );
	const int eighthNum = num >> 3;

	// TODO: Precompute this
	Complex twiddles[ 8 ][ 8 ];
	for ( int x = 0; x < 8; x++ ) {
		for ( int y = 0; y < 8; y++ ) {
			const float angle = HB_PI * 2.0f * float( x * y ) / 8.0f;
			twiddles[ x ][ y ] = Complex( 1.0f, angle );
		}
	}

	// Perform the simplified DFT here
	for ( int n = 0; n < eighthNum; n++ ) {
		Complex butterFly[ 8 ];
		for ( int i = 0; i < 8; i++ ) {
			butterFly[ i ].Zero();

			for ( int j = 0; j < 8; j++ ) {
				butterFly[ i ] += twiddles[ i ][ j ] * data[ n + j * eighthNum ];
			}
		}

		data[ n + 0 * eighthNum ] = butterFly[ 0 ] * invW( 0 * n, num );
		data[ n + 1 * eighthNum ] = butterFly[ 1 ] * invW( 1 * n, num );
		data[ n + 2 * eighthNum ] = butterFly[ 2 ] * invW( 2 * n, num );
		data[ n + 3 * eighthNum ] = butterFly[ 3 ] * invW( 3 * n, num );
		data[ n + 4 * eighthNum ] = butterFly[ 4 ] * invW( 4 * n, num );
		data[ n + 5 * eighthNum ] = butterFly[ 5 ] * invW( 5 * n, num );
		data[ n + 6 * eighthNum ] = butterFly[ 6 ] * invW( 6 * n, num );
		data[ n + 7 * eighthNum ] = butterFly[ 7 ] * invW( 7 * n, num );
	}

	if ( power > 3 ) {
		InverseRadix8_r( data + 0 * eighthNum, power - 3, pingPong );
		InverseRadix8_r( data + 1 * eighthNum, power - 3, pingPong );
		InverseRadix8_r( data + 2 * eighthNum, power - 3, pingPong );
		InverseRadix8_r( data + 3 * eighthNum, power - 3, pingPong );
		InverseRadix8_r( data + 4 * eighthNum, power - 3, pingPong );
		InverseRadix8_r( data + 5 * eighthNum, power - 3, pingPong );
		InverseRadix8_r( data + 6 * eighthNum, power - 3, pingPong );
		InverseRadix8_r( data + 7 * eighthNum, power - 3, pingPong );
	}
}

/*
================================
Fourier::Reorder8_r
================================
*/
void Fourier::Reorder8_r( Complex * data, const int power, Complex * pingPong ) {
	const int num = 1 << power;
	const int eighthNum = num / 8;

	if ( num > 8 ) {
#if 0
		// Swap the first half odds and the second half evens
		for ( int n = 0; n < num; n++ ) {
			pingPong[ n ] = data[ n ];
		}
		for ( int n = 0; n < eighthNum; n++ ) {
			data[ n + 0 * eighthNum ] = pingPong[ 8 * n + 0 ];
			data[ n + 1 * eighthNum ] = pingPong[ 8 * n + 1 ];
			data[ n + 2 * eighthNum ] = pingPong[ 8 * n + 2 ];
			data[ n + 3 * eighthNum ] = pingPong[ 8 * n + 3 ];
			data[ n + 4 * eighthNum ] = pingPong[ 8 * n + 4 ];
			data[ n + 5 * eighthNum ] = pingPong[ 8 * n + 5 ];
			data[ n + 6 * eighthNum ] = pingPong[ 8 * n + 6 ];
			data[ n + 7 * eighthNum ] = pingPong[ 8 * n + 7 ];
		}
#else
		int dim[ 2 ];
		dim[ 0 ] = 8;
		dim[ 1 ] = num / 8;
		RotateData2D( data, dim );
#endif
	}

	if ( power > 3 ) {
		Reorder8_r( data + 0 * eighthNum, power - 3, pingPong );
		Reorder8_r( data + 1 * eighthNum, power - 3, pingPong );
		Reorder8_r( data + 2 * eighthNum, power - 3, pingPong );
		Reorder8_r( data + 3 * eighthNum, power - 3, pingPong );
		Reorder8_r( data + 4 * eighthNum, power - 3, pingPong );
		Reorder8_r( data + 5 * eighthNum, power - 3, pingPong );
		Reorder8_r( data + 6 * eighthNum, power - 3, pingPong );
		Reorder8_r( data + 7 * eighthNum, power - 3, pingPong );
	}
}

/*
   This computes an in-place complex-to-complex FFT 
   x and y are the real and imaginary arrays of 2^m points.
   dir =  1 gives forward transform
   dir = -1 gives reverse transform 
*/
/*
 ================================
 Fourier::FFT
 ================================
 */
bool Fourier::FFT( const bool isForward, const int m, Complex * data ) {
	// Calculate the number of points
	const int N = 1 << m;

	// Do the bit reversal
	const int halfN = N >> 1;
	int j = 0;
	for ( int i = 0; i < N - 1; i++ ) {
		if ( i < j ) {
			std::swap( data[ i ], data[ j ] );
		}

		int k = halfN;
		while ( k <= j ) {
			j -= k;
			k >>= 1;
		}
		j += k;
	}

	// Compute the FFT
	Complex c( Vec2( -1, 0 ) );
	const float factor = isForward ? -1.0f : 1.0f;
	for ( int l = 0; l < m; l++ ) {
		const int l1 = 1 << l;
		const int l2 = 1 << ( l + 1 );

		Complex u( Vec2( 1, 0 ) );
		for ( int j = 0; j < l1; j++ ) {
			for ( int i = j; i < N; i += l2 ) {
				const int i1 = i + l1;
				const Complex t = u * data[ i1 ];
				data[ i1 ]	= data[ i ] - t;
				data[ i ]	= data[ i ] + t;
			}
			u = u * c;
		}
		c.i = factor * sqrtf( ( 1.0f - c.r ) * 0.5f );
		c.r = sqrtf( ( 1.0f + c.r ) * 0.5f );
	}

	// Scaling for forward transform
	if ( isForward ) {
		for ( int i = 0; i < N; i++ ) {
			data[ i ] /= float( N );
		}
	}
   
	return true;
}

/*
 ================================
 Fourier::Test1D
 ================================
 */
void Fourier::Test1D() {
	const int num = 4 * 4 * 4 * 4;
	Complex data[ num ];
	Complex fft[ num ];
	Complex inv[ num ];

	printf( "-------------------- test 1 ---------------------\n" );
	for ( int i = 0; i < num; ++i ) {
		data[ i ].r = i;
		data[ i ].i = 0;
	}

	Fourier::DFT( data, num, fft );
	Fourier::InverseDFT( fft, num, inv );

	for ( int i = 0; i < num; i++ ) {
		printf( "%i:   %.3f  %.3f    %.3f  %.3f    %.3f  %.3f\n", i, data[ i ].r, data[ i ].i, fft[ i ].r, fft[ i ].i, inv[ i ].r, inv[ i ].i );
	}

	for ( int i = 0; i < num; i++ ) {
		fft[ i ] = Complex( Vec2( 0, 0 ) );
		inv[ i ] = Complex( Vec2( 0, 0 ) );
	}

	//const int us_start = hbGetTimeMicroseconds();

	const float alpha = 0.75f;
	Fourier::FractionalDFT( alpha, data, num, fft );
	Fourier::FractionalDFT( -alpha, fft, num, inv );
	
	//Fourier::Radix4( data, num, fft );
	//Fourier::InverseRadix4( fft, num, inv );

	//const int us_end = hbGetTimeMicroseconds();

	for ( int i = 0; i < num; i++ ) {
		printf( "%i:   %.3f  %.3f    %.3f  %.3f    %.3f  %.3f\n", i, data[ i ].r, data[ i ].i, fft[ i ].r, fft[ i ].i, inv[ i ].r, inv[ i ].i );
	}

	//printf( "FFT Time: %ius\n", us_end - us_start );
#if 0
	printf( "-------------------- test 2 ---------------------\n" );
	for ( int i = 0; i < num; ++i ) {
		data[ i ].Zero();
	}
	data[ 0 ].r = 1.0f;

	Fourier::DFT( data, num, fft );
	Fourier::InverseDFT( fft, num, inv );

	for ( int i = 0; i < num; i++ ) {
		printf( "%i:   %.3f  %.3f    %.3f  %.3f    %.3f  %.3f\n", i, data[ i ].r, data[ i ].i, fft[ i ].r, fft[ i ].i, inv[ i ].r, inv[ i ].i );
	}

	printf( "-------------------- test 3 ---------------------\n" );
	for ( int i = 0; i < num; ++i ) {
		data[ i ].Zero();
	}
	data[ 1 ].r = 1.0f;

	Fourier::DFT( data, num, fft );
	Fourier::InverseDFT( fft, num, inv );

	for ( int i = 0; i < num; i++ ) {
		printf( "%i:   %.3f  %.3f    %.3f  %.3f    %.3f  %.3f\n", i, data[ i ].r, data[ i ].i, fft[ i ].r, fft[ i ].i, inv[ i ].r, inv[ i ].i );
	}

	printf( "-------------------- test 4 ---------------------\n" );
	for ( int i = 0; i < num; ++i ) {
		data[ i ].Zero();
	}
	data[ 1 ].r = 1.0f;

	Fourier::Radix2( data, num, fft );
	Fourier::InverseRadix2( fft, num, inv );

	for ( int i = 0; i < num; i++ ) {
		printf( "%i:   %.3f  %.3f    %.3f  %.3f    %.3f  %.3f\n", i, data[ i ].r, data[ i ].i, fft[ i ].r, fft[ i ].i, inv[ i ].r, inv[ i ].i );
	}
#endif
	/*
	printf( "-------------------- test 4 ---------------------\n" );
	for ( int i = 0; i < num; ++i ) {
		data[ i ].Zero();
//		data[ i ].r = i;
	}
	data[ 0 ].r = 1.0f;

	for ( int i = 0; i < num; i++ ) {
		printf( "data %i:   %.3f  %.3f\n", i, data[ i ].r, data[ i ].i );
	}

	Fourier::FFT( true, 3, data );

	for ( int i = 0; i < num; i++ ) {
		printf( "fwd %i:   %.3f  %.3f\n", i, data[ i ].r, data[ i ].i );
	}

	Fourier::FFT( false, 3, data );

	for ( int i = 0; i < num; i++ ) {
		printf( "inv %i:   %.3f  %.3f\n", i, data[ i ].r, data[ i ].i );
	}
	*/
	/*
	for ( int n = 0; n < 0x7FFFFFFF; n++ ) {
		if ( IsPowerOfFour( n ) ) {
			printf( "%i\n", n );
		}
	}
	for ( int n = 0; n < 0x7FFFFFFF; n++ ) {
		if ( IsPowerOfEight( n ) ) {
			printf( "%i\n", n );
		}
	}
	*/
}

/*
 ================================
 Fourier::Test2D
 ================================
 */
#define DIM_2D 64
void Fourier::Test2D() {
// 	const int num = DIM_2D * DIM_2D;
// 	const int dim[ 2 ] = { DIM_2D, DIM_2D };
// 	Complex * data = new Complex[ num ];
// 	Complex * fft = new Complex[ num ];
// 	Complex * inv = new Complex[ num ];
// 
// 	printf( "-------------------- test 1 ---------------------\n" );
// 	for ( int i = 0; i < num; ++i ) {
// 		data[ i ].r = i;
// 		data[ i ].i = 0;
// 	}
// 
// 	const int us_start_dft = hbGetTimeMicroseconds();
// 
// 	//Fourier::DFT_2D( data, dim, fft );
// 	//Fourier::InverseDFT_2D( fft, dim, inv );
// 
// 	const int us_end_dft = hbGetTimeMicroseconds();
// 
// //	for ( int i = 0; i < num; i++ ) {
// //		printf( "%i:   %.3f  %.3f    %.3f  %.3f    %.3f  %.3f\n", i, data[ i ].r, data[ i ].i, fft[ i ].r, fft[ i ].i, inv[ i ].r, inv[ i ].i );
// //	}
// 
// 	for ( int i = 0; i < num; i++ ) {
// 		fft[ i ] = Complex( hbVec2( 0, 0 ) );
// 		inv[ i ] = Complex( hbVec2( 0, 0 ) );
// 	}
// 
// 	const int us_start_FFT = hbGetTimeMicroseconds();
// 
// 	Fourier::Radix8_2D( data, dim, fft );
// 	Fourier::InverseRadix8_2D( fft, dim, inv );
// 
// 	const int us_end_FFT = hbGetTimeMicroseconds();
// 
// 	for ( int i = 0; i < num; i += DIM_2D ) {
// 		printf( "%i:   %.3f  %.3f    %.3f  %.3f    %.3f  %.3f\n", i, data[ i ].r, data[ i ].i, fft[ i ].r, fft[ i ].i, inv[ i ].r, inv[ i ].i );
// 	}
// 
// 	printf( "DFT Times: %i\n", us_end_dft - us_start_dft );
// 	printf( "FFT Times: %i\n", us_end_FFT - us_start_FFT );
// 
// 	delete[] data;
// 	delete[] fft;
// 	delete[] inv;
}

/*
 ================================
 Fourier::Test3D
 ================================
 */
void Fourier::Test3D() {
	const int num = 64;
	const int dim[ 3 ] = { 4, 4, 4 };
	Complex data[ num ];
	Complex fft[ num ];
	Complex inv[ num ];

	printf( "-------------------- test 1 ---------------------\n" );
	for ( int i = 0; i < num; ++i ) {
		data[ i ].r = i;
		data[ i ].i = 0;
	}

	Fourier::DFT_3D( data, dim, fft );
	Fourier::InverseDFT_3D( fft, dim, inv );

	for ( int i = 0; i < num; i++ ) {
		printf( "%i:   %.3f  %.3f    %.3f  %.3f    %.3f  %.3f\n", i, data[ i ].r, data[ i ].i, fft[ i ].r, fft[ i ].i, inv[ i ].r, inv[ i ].i );
	}

	for ( int i = 0; i < num; i++ ) {
		fft[ i ] = Complex( Vec2( 0, 0 ) );
		inv[ i ] = Complex( Vec2( 0, 0 ) );
	}

	Fourier::Radix2_3D( data, dim, fft );
	Fourier::InverseRadix2_3D( fft, dim, inv );

	for ( int i = 0; i < num; i++ ) {
		printf( "%i:   %.3f  %.3f    %.3f  %.3f    %.3f  %.3f\n", i, data[ i ].r, data[ i ].i, fft[ i ].r, fft[ i ].i, inv[ i ].r, inv[ i ].i );
	}
}

Complex gData[ 256 ];
Complex gPingPong[ 256 ];

void PrintData1D( const Complex * data, const int num ) {
	for ( int i = 0; i < num; i++ ) {
		printf( "%i ", (int)data[ i ].r );
	}

	printf( "\n" );
}

void PrintDataSquare( const Complex * data, const int num ) {
	//const int width = num >> 2;
	const int width = (int)sqrtf( (float)num );

	for ( int y = 0; y < width; y++ ) {
		for ( int x = 0; x < width; x++ ) {
			printf( "%i ", (int)data[ x + width * y ].r );
		}
		printf( "\n" );
	}

	printf( "\n" );
}

void Fourier::PrintReorder8_512() {
	Complex * data = new Complex[ 512 ];

	for ( int i = 0; i < 512; i++ ) {
		data[ i ].r = i;
		data[ i ].i = 0;
	}

	Reorder8_r( data, 9, NULL );
	
	for ( int i = 0; i < 512; i++ ) {
		int idx = (int)data[ i ].r;
		printf( " %i,", idx );
	}

	delete[] data;
}

#pragma optimize( "", off )

/*
 ================================
 Fourier::Test
 ================================
 */
void Fourier::Test() {
	//PrintReorder8_512();
	Test1D();

	for ( int thread_id = 0; thread_id < 32; thread_id++ ) {
		int ostride = 512 * 512 / 8;
		int istride = ostride;
		ostride /= 8;

		int imod = thread_id & (istride - 1);
		int iaddr = ((thread_id - imod) << 3) + imod;

		int omod = thread_id & (ostride - 1);
		int oaddr = ((thread_id - omod) << 3) + omod;

		printf( "id: %i   in: %i  out: %i\n", thread_id, iaddr, oaddr );
	}

	/*
	printf( "---------------------------------\n" );
	Complex twiddles[ 8 ][ 8 ];
	for ( int x = 0; x < 8; x++ ) {
		printf( "{" );
		for ( int y = 0; y < 8; y++ ) {
			const float angle = HB_PI * 2.0f * float( x * y ) / 8.0f;
			twiddles[ x ][ y ] = Complex( 1.0f, -angle );

			printf( " vec2( %f, %f ),", twiddles[ x ][ y ].r, twiddles[ x ][ y ].i );
		}
		printf( "}\n" );
	}
	/*
	//const int power = 3;	// 8
	const int power = 4;	// 16
	//const int power = 6;	// 64
	const int num = 1 << power;
	for ( int i = 0; i < num; i++ ) {
		gData[ i ] = Complex( hbVec2( i, 0 ) );
	}

	PrintDataSquare( gData, num );
	//PrintData1D( gData, num );

	Reorder4_r( gData, power, gPingPong );

	//PrintData1D( gData, num );
	PrintDataSquare( gData, num );

	//SquareTranspose( gData, (int)sqrtf( (float)num ) );
	*/
	//PrintData( gData, num );
}
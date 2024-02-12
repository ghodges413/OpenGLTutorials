//
//  Ocean.cpp
//
#include "Ocean.h"
#include "Math/Complex.h"
#include "Math/MatrixOps.h"
#include "Math/Bounds.h"
#include "Math/Random.h"
#include "Math/Math.h"
#include "Math/MatrixOps.h"
#include "Graphics/TextureManager.h"
#include "Graphics/Mesh.h"
#include "Graphics/ShaderManager.h"
#include "Miscellaneous/String.h"
#include "Miscellaneous/Types.h"

/*
=====================================
OceanParms_t
=====================================
*/
struct OceanParms_t {
	int		m_dimSamples;
	float	m_dimPhysical;
	float	m_timeScale;
	float	m_amplitude;
	Vec2d	m_windDir;
	float	m_windSpeed;
	float	m_windDependency;
	float	m_choppiness;
	float	m_suppressionFactor;
	float	m_gravity;

	OceanParms_t() {
		m_dimSamples = 512;
		m_dimPhysical = 10;
		m_timeScale = 1.0f;
		m_amplitude = 1.0f;
		m_windDir = Vec2d( 0.4f, 0.6f );
		m_windDir.Normalize();
		m_windSpeed = 6.0f;
		m_windDependency = 1.0f;
		m_choppiness = 1.0f;
		m_suppressionFactor = 0.001f;
		m_gravity = 9.81f;
	}

	OceanParms_t( int _dimSamples,
		float _dimPhysical,
		float _timeScale,
		float _amplitude,
		Vec2d _windDir,
		float _windSpeed,
		float _windDependency,
		float _choppiness,
		float _suppressionFactor,
		float _gravity ) {
		m_dimSamples = _dimSamples;
		m_dimPhysical = _dimPhysical;
		m_timeScale = _timeScale;
		m_amplitude = _amplitude;
		m_windDir = _windDir;
		m_windDir.Normalize();
		m_windSpeed = _windSpeed;
		m_windDependency = _windDependency;
		m_choppiness = _choppiness;
		m_suppressionFactor = _suppressionFactor;
		m_gravity = _gravity;
	}
};

static const int s_numOceanParms = 4;
static float s_windSpeed = 4.0f;
static float s_choppiness = 10.0f;
static float s_supression = 0.001f;
static float s_timeScale = 20.0f;
const OceanParms_t g_OceanParms[ s_numOceanParms ] = {
	OceanParms_t( 512, 10.0f, s_timeScale, 1.0f, Vec2d( 0.4f, 0.6f ), s_windSpeed, 1.0f, s_choppiness, s_supression, 9.81f ),
	OceanParms_t(  64, 40.0f, s_timeScale, 1.0f, Vec2d( 0.4f, 0.6f ), s_windSpeed, 1.0f, s_choppiness, s_supression, 9.81f ),
	OceanParms_t(  64, 80.0f, s_timeScale, 1.0f, Vec2d( 0.4f, 0.6f ), s_windSpeed, 1.0f, s_choppiness, s_supression, 9.81f ),
	OceanParms_t(  64, 200.0f, s_timeScale, 1.0f, Vec2d( 0.4f, 0.6f ), s_windSpeed, 1.0f, s_choppiness, s_supression, 9.81f )
};

/*
=====================================
GetK
=====================================
*/
static Vec2d GetK( const int idx, const int width, const int length ) {
	const float pi2 = 3.1415f * 2.0f;
	const float invLength = 1.0f / length;

	const int i = idx % width;
	const int j = idx / width;
	const float m = float( i ) - float( width ) * 0.5f;
	const float n = float( j ) - float( width ) * 0.5f;

	Vec2d K = Vec2d( m, n ) * pi2 * invLength;
	return K;
}

/*
=====================================
GetX
=====================================
*/
static Vec2d GetX( const int idx, const int width, const int length ) {
	Vec2d X;

	const int i = idx % width;
	const int j = idx / width;
	const float m = float( i ) - float( width ) * 0.5f;
	const float n = float( j ) - float( width ) * 0.5f;

	X.x = m / float( ( width ) ) * length;
	X.y = n / float( ( width ) ) * length;
	return X;
}

/*
=====================================
PhillipsSpectrum
=====================================
*/
static float PhillipsSpectrum( const Vec2d & K, const OceanParms_t & parms ) {
	const float Ksqr = K.Dot( K );
	if ( 0.0f == Ksqr ) {
		return 0.0f;
	}

	const float L = parms.m_windSpeed * parms.m_windSpeed / parms.m_gravity;
	const float l = L * parms.m_suppressionFactor;
	float amplitude = parms.m_amplitude * expf( -Ksqr * l * l );

	Vec2d k = K;
	k.Normalize();
	const float kwind = k.Dot( parms.m_windDir );
	if ( kwind < 0.0f ) {
		amplitude *= parms.m_windDependency;
	}

	const float phillips = amplitude * expf( -1.0f / ( Ksqr * L * L ) ) * ( kwind * kwind ) / ( Ksqr * Ksqr );
	return phillips;
}

/*
=====================================
H0
=====================================
*/
static Complex H0( const Vec2d & K, const OceanParms_t & parms ) {
	const float xir = Random::Gaussian();
	const float xii = Random::Gaussian();

	const float invRoot2 = 1.0f / sqrtf( 2.0f );
	const float ph = PhillipsSpectrum( K, parms );
	const float rootPH = sqrtf( ph );

	Complex c;
	c.r = xir * invRoot2 * rootPH;
	c.i = xii * invRoot2 * rootPH;
	return c;
}

/*
=====================================
BuildSpectrum
Builds the k-space Phillip's spectrum
=====================================
*/
static void BuildSpectrum( Complex * gH0, const OceanParms_t & parms ) {
	const float pi2 = 3.1415f * 2.0f;
	const float invLength = 1.0f / parms.m_dimPhysical;

	//
	//	Build the initial spectrum
	//
	for ( int i = 0; i < parms.m_dimSamples; ++i ) {
		for ( int j = 0; j < parms.m_dimSamples; ++j ) {
			const int idx = i + parms.m_dimSamples * j;

			float real = 0;
			float imaginary = 0;

			Vec2d K = GetK( idx, parms.m_dimSamples, parms.m_dimPhysical );
			Complex c = H0( K, parms );
			real = c.r;
			imaginary = c.i;
			if ( real != real ) {
				real = 0;
			}
			if ( imaginary != imaginary ) {
				imaginary = 0;
			}

			gH0[ idx ].r = real;
			gH0[ idx ].i = imaginary;
		}
	}
}

Texture * g_OceanSpectrumH0[ s_numOceanParms ] = { NULL };
Texture * g_OceanSpectrumH02[ s_numOceanParms ] = { NULL };
Texture * g_OceanSpectrumHT[ s_numOceanParms ] = { NULL };
Texture * g_OceanFFT[ s_numOceanParms ] = { NULL };

Texture * g_OceanHeights[ s_numOceanParms ] = { NULL };
Texture * g_OceanNormals[ s_numOceanParms ] = { NULL };

Complex data[ 512 * 512 * 3 ];

Mesh g_OceanGridModel;

/*
=====================================
OceanInitialize
=====================================
*/
static bool g_IsOceanInitialized = false;
static void OceanInitialize() {
	if ( g_IsOceanInitialized ) {
		return;
	}

	//
	//	Generate ocean texture buffers
	//
	TextureOpts_t opts;
	opts.wrapS = WM_REPEAT;
	opts.wrapT = WM_REPEAT;
	opts.wrapR = WM_REPEAT;
	opts.minFilter = FM_LINEAR;
	opts.magFilter = FM_LINEAR;
	opts.dimX = 512;
	opts.dimY = 512;
	opts.dimZ = 0;
	opts.type = TT_TEXTURE_2D;
	opts.format = FMT_RG32F;

	TextureOpts_t optsAlt;
	optsAlt = opts;
	optsAlt.format = FMT_RGBA32F;

	for ( int i = 0; i < s_numOceanParms; i++ ) {
		BuildSpectrum( data, g_OceanParms[ i ] );

		opts.dimX = g_OceanParms[ i ].m_dimSamples;
		opts.dimY = g_OceanParms[ i ].m_dimSamples;

		optsAlt.dimX = g_OceanParms[ i ].m_dimSamples;
		optsAlt.dimY = g_OceanParms[ i ].m_dimSamples;

		String str;
		str = String::va( "_OceanH0_%i", i );
		g_OceanSpectrumH0[ i ] = g_textureManager->GetTexture( str.cstr(), opts, data );

		str = String::va( "_OceanH02_%i", i );
		g_OceanSpectrumH02[ i ] = g_textureManager->GetTexture( str.cstr(), opts, data );

		str = String::va( "_OceanHeights_%i", i );
		g_OceanHeights[ i ] = g_textureManager->GetTexture( str.cstr(), optsAlt, NULL );

		str = String::va( "_OceanNormals_%i", i );
		g_OceanNormals[ i ] = g_textureManager->GetTexture( str.cstr(), optsAlt, NULL );

		opts.dimY *= 3;
		str = String::va( "_OceanHT_%i", i );
		g_OceanSpectrumHT[ i ] = g_textureManager->GetTexture( str.cstr(), opts, NULL );

		str = String::va( "_OceanFFT_%i", i );
		g_OceanFFT[ i ] = g_textureManager->GetTexture( str.cstr(), opts, NULL );
	}

	g_IsOceanInitialized = true;

	//
	//	Build the ocean screen space grid
	//
	const int dimSize = 256;
	Array< vert_t > verts;
	Array< vert_t > vertsPlane;
	Array< uint16 > indices;
	verts.Resize( dimSize * dimSize );
	verts.Clear();
	vertsPlane.Resize( dimSize * dimSize );
	vertsPlane.Clear();
	for ( int i = 0; i < dimSize * dimSize; i++ ) {
		vert_t vert;
		
		int x;
		int y;
		Math::MortonOrder2D( i, x, y );

		Vec2d st = Vec2d( x, y );
		st /= float( dimSize - 1 );

		vert.st = st;

		st *= 2.0f;
		st += Vec2d( -1, -1 );

		Vec3d pos = Vec3d( st.x, st.y, -1.0f );
		vert.pos = pos;

		Vec3dToByte4_n11( vert.norm, Vec3d( 0, 0, 1 ) );
		Vec3dToByte4_n11( vert.tang, Vec3d( 1, 0, 0 ) );

		verts.Append( vert );

		vert.pos.z = 0.0f;
		vert.pos *= 10000.0f;
		vertsPlane.Append( vert );
	}

	indices.Resize( dimSize * dimSize * 3 * 2 );
	for ( int x = 0; x < dimSize - 1; x++ ) {
		for ( int y = 0; y < dimSize - 1; y++ ) {
			uint16 idx0 = (uint16)Math::MortonOrder2D( x + 0, y + 0 );
			uint16 idx1 = (uint16)Math::MortonOrder2D( x + 1, y + 0 );
			uint16 idx2 = (uint16)Math::MortonOrder2D( x + 1, y + 1 );

			uint16 idx4 = (uint16)Math::MortonOrder2D( x + 0, y + 0 );
			uint16 idx5 = (uint16)Math::MortonOrder2D( x + 1, y + 1 );
			uint16 idx6 = (uint16)Math::MortonOrder2D( x + 0, y + 1 );

			indices.Append( idx0 );
			indices.Append( idx1 );
			indices.Append( idx2 );

			indices.Append( idx4 );
			indices.Append( idx5 );
			indices.Append( idx6 );
		}
	}
	g_OceanGridModel.LoadFromData( verts.ToPtr(), verts.Num(), indices.ToPtr(), indices.Num() );
}

/*
=====================================
FFT8
=====================================
*/
static int FFT8( const OceanParms_t & parms, const int idx ) {
	Shader * shader = NULL;

	int numRuns = 0;
	int dim = parms.m_dimSamples;
	while ( dim > 1 ) {
		dim >>= 3;
		++numRuns;
	}

	int numIters = 0;

	const int workGroupSize = 64;
	const int Num = parms.m_dimSamples;

	const int numThreads = Num * Num / 8;

	//
	//	Perform FFTs on rows
	//
	{
		shader = g_shaderManager->GetAndUseShader( "Water/FFTRadix8_rows" );
		for ( int i = 0; i < numRuns; i++ ) {
			Texture * bufferA = ( ( numIters & 1 ) == 0 ) ? g_OceanSpectrumHT[ idx ] : g_OceanFFT[ idx ];
			Texture * bufferB = ( ( numIters & 1 ) == 1 ) ? g_OceanSpectrumHT[ idx ] : g_OceanFFT[ idx ];

			glBindImageTexture( 0, bufferA->GetName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F );
			glBindImageTexture( 1, bufferB->GetName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F );

			shader->SetUniform1i( "NumSamples", 1, &Num );
			shader->SetUniform1i( "Iteration", 1, &i );

			shader->DispatchCompute( numThreads / workGroupSize, 1, 1 );
			glFlush();

			++numIters;
		}
	}

	//
	//	Perform FFTs on cols
	//
	{
		shader = g_shaderManager->GetAndUseShader( "Water/FFTRadix8_cols" );
		for ( int i = 0; i < numRuns; i++ ) {
			Texture * bufferA = ( ( numIters & 1 ) == 0 ) ? g_OceanSpectrumHT[ idx ] : g_OceanFFT[ idx ];
			Texture * bufferB = ( ( numIters & 1 ) == 1 ) ? g_OceanSpectrumHT[ idx ] : g_OceanFFT[ idx ];

			glBindImageTexture( 0, bufferA->GetName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F );
			glBindImageTexture( 1, bufferB->GetName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F );

			shader->SetUniform1i( "NumSamples", 1, &Num );
			shader->SetUniform1i( "Iteration", 1, &i );

			shader->DispatchCompute( numThreads / workGroupSize, 1, 1 );
			glFlush();

			++numIters;
		}
	}

	return numIters;
}

/*
=====================================
OceanUpdate
=====================================
*/
void OceanUpdate( const OceanUpdateParms_t & parms ) {
	OceanInitialize();

	const float time = parms.m_timeMS * 0.0005f;

	//
	//	Update the time
	//
	for ( int i = 0; i < s_numOceanParms; i++ ) {
		Shader * shader = g_shaderManager->GetAndUseShader( "Water/SpectrumUpdate" );

		glBindImageTexture( 0, g_OceanSpectrumH0[ i ]->GetName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F );
		glBindImageTexture( 1, g_OceanSpectrumHT[ i ]->GetName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F );

		shader->SetUniform1i( "patch_dim",		1, &g_OceanParms[ i ].m_dimSamples );
		shader->SetUniform1f( "patch_length",	1, &g_OceanParms[ i ].m_dimPhysical );
		shader->SetUniform1f( "gravity",		1, &g_OceanParms[ i ].m_gravity );
		shader->SetUniform1f( "time",			1, &time );

		const int workGroupSize = 32;
		const int dispatchX = g_OceanParms[ i ].m_dimSamples / workGroupSize;
		const int dispatchY = g_OceanParms[ i ].m_dimSamples / workGroupSize;
		shader->DispatchCompute( dispatchX, dispatchY, 1 );
		glFlush();
	}

	//
	//	Perform the FFT and copy the height data over
	//
	for ( int i = 0; i < s_numOceanParms; i++ ) {
		// Perform the fancy FFT
		const int numIters = FFT8( g_OceanParms[ i ], i );

		//
		//	Now do the heights
		//
		Shader * shader = g_shaderManager->GetAndUseShader( "Water/CopyHeights" );

		Texture * buffer = ( ( numIters & 1 ) == 0 ) ? g_OceanSpectrumHT[ i ] : g_OceanFFT[ i ];
		glBindImageTexture( 0, buffer->GetName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F );
		glBindImageTexture( 1, g_OceanHeights[ i ]->GetName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F );

		shader->SetUniform1i( "patch_dim",		1, &g_OceanParms[ i ].m_dimSamples );
		shader->SetUniform1f( "patch_length",	1, &g_OceanParms[ i ].m_dimPhysical );

		const float choppiness = 1.0f;
		shader->SetUniform1f( "choppiness",		1, &choppiness );

		const int workGroupSize = 32;
		const int dispatchX = g_OceanParms[ i ].m_dimSamples / workGroupSize;
		const int dispatchY = g_OceanParms[ i ].m_dimSamples / workGroupSize;
		shader->DispatchCompute( dispatchX, dispatchY, 1 );
		glFlush();
	}

	//
	//	Build the normals from the height data
	//
	for ( int i = 0; i < s_numOceanParms; i++ ) {
		Shader * shader = g_shaderManager->GetAndUseShader( "Water/BuildNormals" );

		glBindImageTexture( 0, g_OceanHeights[ i ]->GetName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F );
		glBindImageTexture( 1, g_OceanNormals[ i ]->GetName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F );

		shader->SetUniform1i( "patch_dim",		1, &g_OceanParms[ i ].m_dimSamples );
		shader->SetUniform1f( "patch_length",	1, &g_OceanParms[ i ].m_dimPhysical );

		const int workGroupSize = 32;
		const int dispatchX = g_OceanParms[ i ].m_dimSamples / workGroupSize;
		const int dispatchY = g_OceanParms[ i ].m_dimSamples / workGroupSize;
		shader->DispatchCompute( dispatchX, dispatchY, 1 );
		glFlush();
	}
}

bool IntersectSegmentPlaneZ( const Vec3d & ptA, const Vec3d & ptB, const float planeZ, Vec3d & ptOut ) {
	if ( ptA.z >= planeZ && ptB.z >= planeZ ) {
		return false;
	}
	if ( ptA.z <= planeZ && ptB.z <= planeZ ) {
		return false;
	}

	Vec3d dir = ptB - ptA;
	dir.Normalize();

	// z = ptA.z + t * dir.z = planeZ
	float t = ( planeZ - ptA.z ) / dir.z;
	ptOut = ptA + t * dir;

	return true;
}

/*
=====================================
IntersectOceanPlaneViewFrustum
=====================================
*/
bool IntersectOceanPlaneViewFrustum( const Matrix & matView, const Matrix & matProj, const float oceanPlaneZ, Matrix & matProjector, Matrix & matBias, Vec3d & projectorPos ) {
	Vec4d pts[ 8 ] = {
		Vec4d(-1,-1, 1, 1 ),
		Vec4d( 1,-1, 1, 1 ),
		Vec4d( 1, 1, 1, 1 ),
		Vec4d(-1, 1, 1, 1 ),

		Vec4d(-1,-1,-1, 1 ),
		Vec4d( 1,-1,-1, 1 ),
		Vec4d( 1, 1,-1, 1 ),
		Vec4d(-1, 1,-1, 1 ),
	};

	struct edge_t {
		int a;
		int b;
		edge_t( int _a, int _b ) : a( _a ), b( _b ) {}
	};

	const edge_t edges[ 12 ] = {
		edge_t( 0, 1 ),
		edge_t( 1, 2 ),
		edge_t( 2, 3 ),
		edge_t( 3, 0 ),

		edge_t( 4, 5 ),
		edge_t( 5, 6 ),
		edge_t( 6, 7 ),
		edge_t( 7, 4 ),

		edge_t( 0, 4 ),
		edge_t( 1, 5 ),
		edge_t( 2, 6 ),
		edge_t( 3, 7 ),
	};

	// Transform the points into the world spaces
	Matrix invView = matView;
	myMatrixInverse4x4( matView.ToPtr(), invView.ToPtr() );

	Matrix invProj = matProj;
	myMatrixInverse4x4( matProj.ToPtr(), invProj.ToPtr() );

	Matrix invViewProj = invView * invProj;
	myMatrixMultiply( invProj.ToPtr(), invView.ToPtr(), invViewProj.ToPtr() );

	Matrix matViewProj;
	myMatrixMultiply( matView.ToPtr(), matProj.ToPtr(), matViewProj.ToPtr() );

	// Get the inverse of the view projection matrix
	Matrix matInverse;
	myMatrixInverse4x4( matViewProj.ToPtr(), matInverse.ToPtr() );

	for ( int i = 0; i < 8; i++ ) {
		Vec4d corner;
		myTransformVector4D( matInverse.ToPtr(), pts[ i ].ToPtr(), corner.ToPtr() );
		corner /= corner.w;
		pts[ i ] = corner;
		pts[ i ].w = 1.0f;
	}

	Vec3d oceanPts[ 24 * 3 ];
	int numIntersections = 0;
	for ( int z = -1; z <= 1; z++ ) {
		const float dz = 1.0f;//g_oceanPlaneDisplacement.GetFloat();
		const float planeZ = float( z ) * dz + oceanPlaneZ;

		for ( int i = 0; i < 12; i++ ) {
			const int a = edges[ i ].a;
			const int b = edges[ i ].b;
			const Vec3d ptA = pts[ a ].xyz();
			const Vec3d ptB = pts[ b ].xyz();

			const bool doesIntersect = IntersectSegmentPlaneZ( ptA, ptB, planeZ, oceanPts[ numIntersections ] );
			if ( doesIntersect ) {
				++numIntersections;
			}
		}
	}

	if ( numIntersections < 3 ) {
		return false;
	}
	
	// Put the ocean pts back into screen space
	Vec4d screenSpacePts[ 24 * 3 ];
	Matrix mvp = matProj * matView;
	for ( int i = 0; i < numIntersections; i++ ) {
		screenSpacePts[ i ] = Vec4d( oceanPts[ i ].x, oceanPts[ i ].y, oceanPts[ i ].z, 1.0f );
		
		Vec4d corner;
		myTransformVector4D( matViewProj.ToPtr(), screenSpacePts[ i ].ToPtr(), corner.ToPtr() );
		corner /= corner.w;
		screenSpacePts[ i ] = corner;
	}

	// Make the bounds
	Bounds bounds;
	for ( int i = 0; i < numIntersections; i++ ) {
		Vec2d pt2d = Vec2d( screenSpacePts[ i ].x, screenSpacePts[ i ].y );
		bounds.AddPoint( Vec3d( pt2d.x, pt2d.y, 0.0f ) );
	}
	if ( bounds.min.x <= -0.99f ) {
		bounds.min.x = -1.2f;
	}
	if ( bounds.min.y <= -0.99f ) {
		bounds.min.y = -1.2f;
	}
	if ( bounds.max.x >= 0.99f ) {
		bounds.max.x = 1.2f;
	}

	// Calculate the bias matrix: (we might need to transpose it)
	matBias.SetIdentity();
	matBias[ 0 ][ 0 ] = ( bounds.max.x - bounds.min.x ) * 0.5f;
	matBias[ 1 ][ 1 ] = ( bounds.max.y - bounds.min.y ) * 0.5f;
	matBias[ 0 ][ 3 ] = ( bounds.max.x + bounds.min.x ) * 0.5f;
	matBias[ 1 ][ 3 ] = ( bounds.max.y + bounds.min.y ) * 0.5f;
	matBias.Transpose();


	matProjector = invViewProj;
	return true;
}

/*
=====================================
OceanFillGBuffer
=====================================
*/
void OceanFillGBuffer( const OceanFillGBufferParms_t & parms ) {
	OceanInitialize();

	const float oceanPlaneZ = 0.0f;

	Matrix matBias;
	Matrix matProjector;
	matProjector.SetIdentity();

	Matrix invView = parms.m_matView;
	myMatrixInverse4x4( parms.m_matView.ToPtr(), invView.ToPtr() );

	Matrix invProj = parms.m_matProj;
	myMatrixInverse4x4( parms.m_matProj.ToPtr(), invProj.ToPtr() );

	Vec3d projectorPos = parms.m_camPos;

	const bool doesIntersect = IntersectOceanPlaneViewFrustum( parms.m_matView, parms.m_matProj, oceanPlaneZ, matProjector, matBias, projectorPos );
	if ( !doesIntersect ) {
		return;
	}

	Shader * shader = g_shaderManager->GetAndUseShader( "Water/depthOnly" );

 	shader->SetUniformMatrix4f( "matModelViewProj", 1, false, parms.m_matViewProj.ToPtr() );

	glBindImageTexture( 0, g_OceanHeights[ 0 ]->GetName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F );
	glBindImageTexture( 1, g_OceanHeights[ 1 ]->GetName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F );
	glBindImageTexture( 2, g_OceanHeights[ 2 ]->GetName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F );
	glBindImageTexture( 3, g_OceanHeights[ 3 ]->GetName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F );

	shader->SetUniformMatrix4f( "matBias", 1, false, matBias.ToPtr() );
	shader->SetUniformMatrix4f( "projectorMat", 1, false, matProjector.ToPtr() );
	shader->SetUniform3f( "projectorPos", 1, projectorPos.ToPtr() );
	shader->SetUniform1f( "oceanPlane", 1, &oceanPlaneZ );

	Vec2d dimensions[ 4 ];
	dimensions[ 0 ] = Vec2d( g_OceanParms[ 0 ].m_dimSamples, g_OceanParms[ 0 ].m_dimPhysical );
	dimensions[ 1 ] = Vec2d( g_OceanParms[ 1 ].m_dimSamples, g_OceanParms[ 1 ].m_dimPhysical );
	dimensions[ 2 ] = Vec2d( g_OceanParms[ 2 ].m_dimSamples, g_OceanParms[ 2 ].m_dimPhysical );
	dimensions[ 3 ] = Vec2d( g_OceanParms[ 3 ].m_dimSamples, g_OceanParms[ 3 ].m_dimPhysical );
	shader->SetUniform2f( "dimensions", 4, dimensions[ 0 ].ToPtr() );

	g_OceanGridModel.Draw();
}

/*
=====================================
OceanDraw
=====================================
*/
void OceanDraw( const OceanDrawParms_t & parms ) {
	OceanInitialize();

	const float oceanPlaneZ = 0.0f;

	Shader * shader = g_shaderManager->GetAndUseShader( "Water/DrawProjected" );

	Matrix matBias;
	Matrix matProjector;
	matProjector.SetIdentity();

	Matrix invView = parms.m_matView;
	myMatrixInverse4x4( parms.m_matView.ToPtr(), invView.ToPtr() );

	Matrix invProj = parms.m_matProj;
	myMatrixInverse4x4( parms.m_matProj.ToPtr(), invProj.ToPtr() );

	Vec3d projectorPos = parms.m_camPos;

	const bool doesIntersect = IntersectOceanPlaneViewFrustum( parms.m_matView, parms.m_matProj, oceanPlaneZ, matProjector, matBias, projectorPos );
	if ( !doesIntersect ) {
		return;
	}

	glBindImageTexture( 0, g_OceanHeights[ 0 ]->GetName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F );
	glBindImageTexture( 1, g_OceanHeights[ 1 ]->GetName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F );
	glBindImageTexture( 2, g_OceanHeights[ 2 ]->GetName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F );
	glBindImageTexture( 3, g_OceanHeights[ 3 ]->GetName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F );

	shader->SetUniformMatrix4f( "mvp", 1, false, parms.m_matViewProj.ToPtr() );
	shader->SetUniformMatrix4f( "matBias", 1, false, matBias.ToPtr() );
	shader->SetUniformMatrix4f( "projectorMat", 1, false, matProjector.ToPtr() );
	shader->SetUniform3f( "projectorPos", 1, projectorPos.ToPtr() );
	shader->SetUniform1f( "oceanPlane", 1, &oceanPlaneZ );

	shader->SetUniform3f( "camPos", 1, parms.m_camPos.ToPtr() );
	shader->SetUniform3f( "dirToSun", 1, parms.m_dirToSun.ToPtr() );

	Vec2d dimensions[ 4 ];
	dimensions[ 0 ] = Vec2d( g_OceanParms[ 0 ].m_dimSamples, g_OceanParms[ 0 ].m_dimPhysical );
	dimensions[ 1 ] = Vec2d( g_OceanParms[ 1 ].m_dimSamples, g_OceanParms[ 1 ].m_dimPhysical );
	dimensions[ 2 ] = Vec2d( g_OceanParms[ 2 ].m_dimSamples, g_OceanParms[ 2 ].m_dimPhysical );
	dimensions[ 3 ] = Vec2d( g_OceanParms[ 3 ].m_dimSamples, g_OceanParms[ 3 ].m_dimPhysical );
	shader->SetUniform2f( "dimensions", 4, dimensions[ 0 ].ToPtr() );

	shader->SetAndBindUniformTexture( "s_normals0", 4, GL_TEXTURE_2D, g_OceanNormals[ 0 ]->GetName() );
	shader->SetAndBindUniformTexture( "s_normals1", 5, GL_TEXTURE_2D, g_OceanNormals[ 1 ]->GetName() );
	shader->SetAndBindUniformTexture( "s_normals2", 6, GL_TEXTURE_2D, g_OceanNormals[ 2 ]->GetName() );
	shader->SetAndBindUniformTexture( "s_normals3", 7, GL_TEXTURE_2D, g_OceanNormals[ 3 ]->GetName() );

	g_OceanGridModel.Draw();
}

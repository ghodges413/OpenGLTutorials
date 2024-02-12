//
//  BrunetonUtils.cpp
//
#include "Atmosphere/BrunetonUtils.h"
#include "Math/MatrixOps.h"
#include "Math/Sphere.h"
#include "Miscellaneous/Comparison.h"



/*
 ===============================
 GetRadiusExtinction
 ===============================
 */
float GetRadiusExtinction( const int coord, const int dim, const float radiusTop, const float radiusGround ) {
	float t = float( coord ) / float( dim - 1 );
	float radius = radiusGround + t * ( radiusTop - radiusGround );
	return radius;
}

/*
 ===============================
 GetAngleExtinction
 ===============================
 */
float GetAngleExtinction( const int coord, const int dim ) {
	float t = float( coord ) / float( dim - 1 );
	float cosAngle = 2.0f * t - 1.0f;
	return cosAngle;
}

/*
 ===============================
 Transmittance
 // Returns the calculated transmittance for the height/angle.  Ignores ground intersections.
 ===============================
 */
Vec4d Transmittance( const BrunetonData_t & data, const float radius, const float cosAngle, const Vec4d * sampler ) {
	const float radiusGround = data.radiusGround;
	const float radiusTop = data.radiusTop;

	Vec2d st;
	st.x = ( radius - radiusGround ) / ( radiusTop - radiusGround );
	st.y = 0.5f * ( cosAngle + 1.0f );

	Vec4d sample = SampleTexture2DLinear( sampler, st.x, st.y, data.dimTransmission.x, data.dimTransmission.y );
	assert( sample.x == sample.x );
	assert( sample.y == sample.y );
	assert( sample.z == sample.z );
	assert( sample.w == sample.w );
	return sample;
}
Vec4d Transmittance( const BrunetonData_t & data, Vec3d pos, Vec3d view, const Vec4d * sampler ) {
	const float radius = pos.GetMagnitude();
	pos.Normalize();
	view.Normalize();
	const float cosAngle = pos.Dot( view );
	return Transmittance( data, radius, cosAngle, sampler );
}


// Used for widening/narrowing the range of the cosAngle
const float deltaIrradiance = 1.0f;//0.2f;

/*
 ===============================
 GetRadiusIrradiance
 ===============================
 */
float GetRadiusIrradiance( const int coord, const int dim, const float radiusTop, const float radiusGround ) {
	float t = float( coord ) / float( dim - 1 );
	float radius = radiusGround + t * ( radiusTop - radiusGround );
	return radius;
}

/*
 ===============================
 GetAngleIrradiance
 ===============================
 */
float GetAngleIrradiance( const int coord, const int dim ) {
	float t = float( coord ) / float( dim - 1 );
	float cosAngle = 2.0f * t - 1.0f;
	return cosAngle;
}

/*
 ===============================
 Irradiance
 ===============================
 */
Vec4d Irradiance( const BrunetonData_t & data, const float radius, const float cosAngle, const Vec4d * sampler ) {
	const float radiusGround = data.radiusGround;
	const float radiusTop = data.radiusTop;

	Vec2d st;
	st.x = ( radius - radiusGround ) / ( radiusTop - radiusGround );
	st.y = 0.5f * ( cosAngle + 1.0f );
    
	return SampleTexture2DLinear( sampler, st.x, st.y, data.dimIrradiance.x, data.dimIrradiance.y );
}









/*
 ===============================
 DoesCollideGround
 ===============================
 */
bool DoesCollideGround( const const Vec3d & pt, const Vec3d & ray, const float ground ) {
	const Sphere sphere = Sphere( Vec3d( 0 ), ground );
	float t0 = -1;
	float t1 = -1;
	hbIntersectRaySphere( pt, ray, sphere, t0, t1 );
	if ( t0 > 0 || t1 > 0 ) {
		return true;
	}
	return false;
}

/*
 ===============================
 IntersectGroundTop
 // Intersect the ground or the top, whichever is nearest, and return the distance
 ===============================
 */
float IntersectGroundTop( const Vec3d & pt, const Vec3d & ray, const BrunetonData_t & data ) {
	const float radiusTop = data.radiusTop;
	const float radiusGround = data.radiusGround;

	const Sphere sphereGround = Sphere( Vec3d( 0 ), radiusGround );
	const Sphere sphereTop = Sphere( Vec3d( 0 ), radiusTop + 1.0f );

	float tout;
	float t[ 4 ] = { -1 };
	hbIntersectRaySphere( pt, ray, sphereGround, t[ 0 ], t[ 1 ] );
	hbIntersectRaySphere( pt, ray, sphereTop, t[ 2 ], t[ 3 ] );
	if ( t[ 0 ] >= 0 && t[ 1 ] >= 0 ) {
		tout = Min( t[ 0 ], t[ 1 ] );
	} else {
		tout = t[ 3 ];
		// This shouldn't be necessary since we should always be inside the atmosphere during precompute
		if ( t[ 2 ] >= 0.0f ) {
			tout = Min( tout, t[ 2 ] );
		}
	}
	
    return tout;
}









/*
==========================
ScatterPhaseFunctionRayleigh
// Equation 2 from Bruneton2008
==========================
*/
float ScatterPhaseFunctionRayleigh( const float cosTheta ) {
	const float pi = acosf( -1.0f );

	float phase = ( 3.0f / ( 16.0f * pi ) ) * ( 1.0f + cosTheta * cosTheta );
	return phase;
}

/*
==========================
ScatterPhaseFunctionMie
// Equation 4 from Bruneton2008
==========================
*/
float ScatterPhaseFunctionMie( const float cosTheta, const float mieG ) {
	const float pi = acosf( -1.0f );

	float g = mieG;
	float g2 = g * g;
	
	float phase = ( 3.0f / ( 8.0f * pi ) ) * ( 1.0f - g2 ) * ( 1.0f + cosTheta * cosTheta ) / ( ( 2.0f + g2 ) * powf( ( 1.0f + g2 - 2.0f * g * cosTheta ), 1.5f ) );
	return phase;
}


/*
========================================================================================================

4D Table Lookups

========================================================================================================
*/


/*
 ===============================
 GetRadius
 fragCoord is in the range [0, width], [0, height], [0, depth]
 ===============================
 */
float GetRadius( const int coord, const int dim, const float radiusTop, const float radiusGround ) {
	// ur = rho / H
	// H = sqrt( rt * rt - rg * rg )
	// rho = sqrt( r * r - rg * rg )

	// r = sqrt( rho * rho + rg * rg )
	// rho = ur * H
	// H = sqrt( rt * rt - rg * rg )
	float ur = float( coord ) / float( dim - 1 );
	const float H = sqrtf( radiusTop * radiusTop - radiusGround * radiusGround );
	const float rho = ur * H;
	float radius = sqrtf( rho * rho + radiusGround * radiusGround );
	Clamp( radius, radiusGround + 0.01f, radiusTop - 0.001f );
	return radius;
}

/*
 ===============================
 GetCosAngleViewSun
 fragCoord is in the range [0, width], [0, height]
 ===============================
 */
float GetCosAngleViewSun( const int coord, const int dim ) {
	const float Unu = float( coord ) / float( dim - 1 );
	// Unu = ( 1 + nu ) / 2
	// nu = 2 * Unu - 1
	float nu = 2.0f * Unu - 1.0f;
	Clamp( nu, -1.0f, 1.0f );
	return nu;
}
float GetCosAngleSun( const int coord, const int dim ) {
	const float Umus = ( float( coord ) + 0.5f ) / float( dim - 1 );
	// Umus = ( 1 - e( -3mus - 0.6 ) / ( 1 - e( -3.6 ) )
	// Umus * ( 1 - e( -3.6 ) ) = ( 1 - e( -3mus - 0.6 )
	// -3mus - 0.6 = log( 1 - Umus * ( 1 - e( -3.6 ) ) )

	float cosAngleSun = -( 0.6f + logf( 1.0f - Umus * ( 1.0f -  exp( -3.6f ) ) ) ) / 3.0f;
	Clamp( cosAngleSun, -1.0f, 1.0f );
	return cosAngleSun;
}
float ClampCosAngleViewSun( const float cosAngleViewSun, const float cosAngleView, const float cosAngleSun ) {
	float sinAngleView = sqrtf( 1.0f - cosAngleView * cosAngleView );
	float sinAngleSun = sqrtf( 1.0f - cosAngleSun * cosAngleSun );
		
	// The angle between the sun and view need to range between the min and max angles away from
	// the view vector that the sun can possible be.  The min and max sun vectors will be co-planar with
	// the view vector.
	Vec3d view = Vec3d( sinAngleView, 0.0f, cosAngleView );
	Vec3d sun0 = Vec3d( sinAngleSun, 0.0f, cosAngleSun );
	Vec3d sun1 = Vec3d( -sinAngleSun, 0.0f, cosAngleSun );
		
	// TODO: make sure these are the right limits (ie that sun0 really is the min and sun1 really is the max)
	float min = view.Dot( sun0 );
	float max = view.Dot( sun1 );
		
	// This probably isn't necessary
	if ( max < min ) {
		float tmp = min;
		min = max;
		max = tmp;
	}

	// Clamp the angle between the view and sun
	if ( cosAngleViewSun < min ) {
		return min;
	}
	if ( cosAngleViewSun > max ) {
		return max;
	}
	return cosAngleViewSun;
}
float GetCosAngleView( const int coord, const int dim, const float radius, const float radiusTop, const float radiusGround ) {
	const int dimHalf = dim >> 1;

	const float Umu = float( coord ) / float( dim - 1 );

	const float rho = sqrtf( radius * radius - radiusGround * radiusGround );
	const float H = sqrtf( radiusTop * radiusTop - radiusGround * radiusGround );
	
	float cosAngleView;
	if ( coord < dimHalf ) {
		const float beta = ( 2.0f * Umu - 1.0f ) * rho;
		cosAngleView = ( beta * beta + rho * rho ) / ( 2.0f * beta * radius );
    } else {
		const float beta = ( 2.0f * Umu - 1.0f ) * ( rho + H );
		cosAngleView = ( H * H - rho * rho - beta * beta ) / ( 2.0f * beta * radius );
    }
	Clamp( cosAngleView, -1.0f, 1.0f );

	return cosAngleView;
}

/*
 ===============================
 GetCoords4D
 Uses the specialized coordinate look ups from the paper
 ===============================
 */
Vec4d GetCoords4D( const BrunetonData_t & data, const float radius, const float cosAngleView, const float cosAngleSun, const float cosAngleViewSun ) {
	const float radiusTop = data.radiusTop;
	const float radiusGround = data.radiusGround;

	// Helper values
	const float r = ( radius > radiusGround ) ? radius : ( radiusGround + 0.01f );
	const float mu = cosAngleView;

	const float rmu = r * mu;
	const float H = sqrtf( radiusTop * radiusTop - radiusGround * radiusGround );
	const float rho = sqrtf( r * r - radiusGround * radiusGround );
	const float delta = rmu * rmu - rho * rho;

	// Coordinate lookups from the paper
	float uMu = 0.5f;
	if ( rmu < 0.0f && delta > 0.0f ) {
		uMu += ( rmu + sqrtf( delta ) ) / ( 2.0f * rho );
	} else {
		uMu -= ( rmu - sqrtf( delta + H * H ) ) / ( 2.0f * rho + 2.0f * H );
	}
	
	// Coordinate lookups from the paper
	const float uR = rho / H;
	const float uMuS = ( 1.0f - expf( -3.0f * cosAngleSun - 0.6f ) ) / ( 1.0f - expf( -3.6f ) );
	const float uNu = ( 1.0f + cosAngleViewSun ) * 0.5f;

	assert( uMuS == uMuS );
	assert( uNu == uNu );
	assert( uMu == uMu );
	assert( uR == uR );
	return Vec4d( uMuS, uNu, uMu, uR );
}

/*
 ===============================
 SampleScatter
 ===============================
 */
Vec4d SampleScatter( const BrunetonData_t & data, const float radius, const float cosAngleView, const float cosAngleSun, const float cosAngleViewSun, const Vec4d * table ) {
	const Vec4d coords4D = GetCoords4D( data, radius, cosAngleView, cosAngleSun, cosAngleViewSun );
	Vec4d scatter = SampleTexture4DLinear( table, coords4D.x, coords4D.y, coords4D.z, coords4D.w, data.dimScatter.x, data.dimScatter.y, data.dimScatter.z, data.dimScatter.w );
	return scatter;
}

	

/*
========================================================================================================

Texture Sampling

========================================================================================================
*/

/*
==========================
SampleTexture1D
==========================
*/
Vec4d SampleTexture1D( const Vec4d * texture, float s, const int dimX ) {
	Clamp( s, 0.0f, 1.0f );

	int x = s * dimX;
	Clamp( x, 0, dimX - 1 );
	return texture[ x ];
}
Vec4d SampleTexture1DLinear( const Vec4d * texture, float s, const int dimX ) {
	Clamp( s, 0.0f, 1.0f );

	int x0 = s * dimX;
	Clamp( x0, 0, dimX - 1 );
	const Vec4d sample0 = texture[ x0 ];

	int x1 = x0 + 1;
	Clamp( x1, 0, dimX - 1 );
	const Vec4d sample1 = texture[ x1 ];

	const float fx = s * (float)dimX;
	const float fracX = fx - (int)fx;
	Vec4d sample = hbLerp( sample0, sample1, fracX );
	return sample;
}

/*
==========================
SampleTexture2D
==========================
*/
Vec4d SampleTexture2D( const Vec4d * texture, float s, float t, const int dimX, const int dimY ) {
	Clamp( s, 0.0f, 1.0f );
	Clamp( t, 0.0f, 1.0f );

	int x = s * dimX;
	int y = t * dimY;

	Clamp( x, 0, dimX - 1 );
	Clamp( y, 0, dimY - 1 );

	const int index = x + y * dimX;
	return texture[ index ];
}
Vec4d SampleTexture2DLinear( const Vec4d * texture, float s, float t, const int dimX, const int dimY ) {
	Clamp( s, 0.0f, 1.0f );
	Clamp( t, 0.0f, 1.0f );

	int x0 = s * dimX;
	int y0 = t * dimY;

	Clamp( x0, 0, dimX - 1 );
	Clamp( y0, 0, dimY - 1 );

	int x1 = x0 + 1;
	int y1 = y0 + 1;

	Clamp( x1, 0, dimX - 1 );
	Clamp( y1, 0, dimY - 1 );

	//
	// Get Samples
	//

	int indices[ 4 ];
	indices[ 0 ] = x0 + y0 * dimX;
	indices[ 1 ] = x0 + y1 * dimX;
	indices[ 2 ] = x1 + y0 * dimX;
	indices[ 3 ] = x1 + y1 * dimX;

	Vec4d samples[ 4 ];
	for ( int i = 0; i < 4; ++i ) {
		samples[ i ] = texture[ indices[ i ] ];
	}

	//
	//	Calculate weights
	//

	const float fx = s * (float)dimX;
	const float fy = t * (float)dimY;

	const float fracX = fx - (int)fx;
	const float fracY = fy - (int)fy;

	float weights[ 4 ];
	weights[ 0 ] = ( 1.0f - fracX ) * ( 1.0f - fracY );
	weights[ 1 ] = ( 1.0f - fracX ) * ( fracY );
	weights[ 2 ] = ( fracX ) * ( 1.0f - fracY );
	weights[ 3 ] = ( fracX ) * ( fracY );

	//
	//	Calculate final sample from weights
	//

	Vec4d finalSample = Vec4d( 0.0f );
	for ( int i = 0; i < 4; ++i ) {
		finalSample += weights[ i ] * samples[ i ];
	}
	assert( finalSample.x == finalSample.x );
	assert( finalSample.y == finalSample.y );
	assert( finalSample.z == finalSample.z );
	assert( finalSample.w == finalSample.w );
	return finalSample;
}

/*
==========================
SampleTexture3D
==========================
*/
Vec4d SampleTexture3D( const Vec4d * texture, float s, float t, float r, const int dimX, const int dimY, const int dimZ ) {
	Clamp( s, 0.0f, 1.0f );
	Clamp( t, 0.0f, 1.0f );
	Clamp( r, 0.0f, 1.0f );

	int x = s * dimX;
	int y = t * dimY;
	int z = r * dimZ;

	Clamp( x, 0, dimX - 1 );
	Clamp( y, 0, dimY - 1 );
	Clamp( z, 0, dimZ - 1 );

	const int index = x + y * dimX + z * dimX * dimY;
	Vec4d sample = texture[ index ];
	return sample;
}
Vec4d SampleTexture3DLinear( const Vec4d * texture, float s, float t, float r, const int dimX, const int dimY, const int dimZ ) {
	Clamp( s, 0.0f, 1.0f );
	Clamp( t, 0.0f, 1.0f );
	Clamp( r, 0.0f, 1.0f );

	int x0 = s * dimX;
	int y0 = t * dimY;
	int z0 = r * dimZ;

	Clamp( x0, 0, dimX - 1 );
	Clamp( y0, 0, dimY - 1 );
	Clamp( z0, 0, dimZ - 1 );

	int x1 = x0 + 1;
	int y1 = y0 + 1;
	int z1 = z0 + 1;

	Clamp( x1, 0, dimX - 1 );
	Clamp( y1, 0, dimY - 1 );
	Clamp( z1, 0, dimZ - 1 );

	//
	// Get Samples
	//

	int indices[ 8 ];
	indices[ 0 ] = x0 + y0 * dimX + z0 * dimX * dimY;
	indices[ 1 ] = x0 + y1 * dimX + z0 * dimX * dimY;
	indices[ 2 ] = x1 + y0 * dimX + z0 * dimX * dimY;
	indices[ 3 ] = x1 + y1 * dimX + z0 * dimX * dimY;
	indices[ 4 ] = x0 + y0 * dimX + z1 * dimX * dimY;
	indices[ 5 ] = x0 + y1 * dimX + z1 * dimX * dimY;
	indices[ 6 ] = x1 + y0 * dimX + z1 * dimX * dimY;
	indices[ 7 ] = x1 + y1 * dimX + z1 * dimX * dimY;

	Vec4d samples[ 8 ];
	for ( int i = 0; i < 8; ++i ) {
		samples[ i ] = texture[ indices[ i ] ];
	}

	//
	//	Calculate weights
	//

	const float fx = s * (float)dimX;
	const float fy = t * (float)dimY;
	const float fz = r * (float)dimZ;

	const float fracX = fx - (int)fx;
	const float fracY = fy - (int)fy;
	const float fracZ = fz - (int)fz;

	float weights[ 8 ];
	weights[ 0 ] = ( 1.0f - fracX ) * ( 1.0f - fracY ) * ( 1.0f - fracZ );
	weights[ 1 ] = ( 1.0f - fracX ) * ( fracY ) * ( 1.0f - fracZ );
	weights[ 2 ] = ( fracX ) * ( 1.0f - fracY ) * ( 1.0f - fracZ );
	weights[ 3 ] = ( fracX ) * ( fracY ) * ( 1.0f - fracZ );
	weights[ 4 ] = ( 1.0f - fracX ) * ( 1.0f - fracY ) * ( fracZ );
	weights[ 5 ] = ( 1.0f - fracX ) * ( fracY ) * ( fracZ );
	weights[ 6 ] = ( fracX ) * ( 1.0f - fracY ) * ( fracZ );
	weights[ 7 ] = ( fracX ) * ( fracY ) * ( fracZ );

	//
	//	Calculate final sample from weights
	//

	Vec4d finalSample = Vec4d( 0.0f );
	for ( int i = 0; i < 8; ++i ) {
		finalSample += weights[ i ] * samples[ i ];
	}
	return finalSample;
}

/*
==========================
SampleTexture4D
==========================
*/
Vec4d SampleTexture4D( const Vec4d * texture, float s, float t, float r, float q, const int dimX, const int dimY, const int dimZ, const int dimW ) {
	Clamp( s, 0.0f, 1.0f );
	Clamp( t, 0.0f, 1.0f );
	Clamp( r, 0.0f, 1.0f );
	Clamp( q, 0.0f, 1.0f );

	int x = s * dimX;
	int y = t * dimY;
	int z = r * dimZ;
	int w = r * dimW;

	Clamp( x, 0, dimX - 1 );
	Clamp( y, 0, dimY - 1 );
	Clamp( z, 0, dimZ - 1 );
	Clamp( w, 0, dimW - 1 );

	const int index = x + y * dimX + z * dimX * dimY + w * dimX * dimY * dimZ;
	Vec4d sample = texture[ index ];
	return sample;
}
Vec4d SampleTexture4DLinear( const Vec4d * texture, float s, float t, float r, float q, const int dimX, const int dimY, const int dimZ, const int dimW ) {
	Clamp( s, 0.0f, 1.0f );
	Clamp( t, 0.0f, 1.0f );
	Clamp( r, 0.0f, 1.0f );
	Clamp( q, 0.0f, 1.0f );

	int x0 = s * dimX;
	int y0 = t * dimY;
	int z0 = r * dimZ;
	int w0 = q * dimW;

	Clamp( x0, 0, dimX - 1 );
	Clamp( y0, 0, dimY - 1 );
	Clamp( z0, 0, dimZ - 1 );
	Clamp( w0, 0, dimW - 1 );

	int x1 = x0 + 1;
	int y1 = y0 + 1;
	int z1 = z0 + 1;
	int w1 = w0 + 1;

	Clamp( x1, 0, dimX - 1 );
	Clamp( y1, 0, dimY - 1 );
	Clamp( z1, 0, dimZ - 1 );
	Clamp( w1, 0, dimW - 1 );

	//
	// Get Samples
	//

	int indices[ 16 ];
	indices[ 0 ] = x0 + y0 * dimX + z0 * dimX * dimY + w0 * dimX * dimY * dimZ;
	indices[ 1 ] = x0 + y1 * dimX + z0 * dimX * dimY + w0 * dimX * dimY * dimZ;
	indices[ 2 ] = x1 + y0 * dimX + z0 * dimX * dimY + w0 * dimX * dimY * dimZ;
	indices[ 3 ] = x1 + y1 * dimX + z0 * dimX * dimY + w0 * dimX * dimY * dimZ;
	indices[ 4 ] = x0 + y0 * dimX + z1 * dimX * dimY + w0 * dimX * dimY * dimZ;
	indices[ 5 ] = x0 + y1 * dimX + z1 * dimX * dimY + w0 * dimX * dimY * dimZ;
	indices[ 6 ] = x1 + y0 * dimX + z1 * dimX * dimY + w0 * dimX * dimY * dimZ;
	indices[ 7 ] = x1 + y1 * dimX + z1 * dimX * dimY + w0 * dimX * dimY * dimZ;

	indices[ 8 ] = x0 + y0 * dimX + z0 * dimX * dimY + w1 * dimX * dimY * dimZ;
	indices[ 9 ] = x0 + y1 * dimX + z0 * dimX * dimY + w1 * dimX * dimY * dimZ;
	indices[10 ] = x1 + y0 * dimX + z0 * dimX * dimY + w1 * dimX * dimY * dimZ;
	indices[11 ] = x1 + y1 * dimX + z0 * dimX * dimY + w1 * dimX * dimY * dimZ;
	indices[12 ] = x0 + y0 * dimX + z1 * dimX * dimY + w1 * dimX * dimY * dimZ;
	indices[13 ] = x0 + y1 * dimX + z1 * dimX * dimY + w1 * dimX * dimY * dimZ;
	indices[14 ] = x1 + y0 * dimX + z1 * dimX * dimY + w1 * dimX * dimY * dimZ;
	indices[15 ] = x1 + y1 * dimX + z1 * dimX * dimY + w1 * dimX * dimY * dimZ;

	Vec4d samples[ 16 ];
	for ( int i = 0; i < 16; ++i ) {
		samples[ i ] = texture[ indices[ i ] ];
	}

	//
	//	Calculate weights
	//

	const float fx = s * (float)dimX;
	const float fy = t * (float)dimY;
	const float fz = r * (float)dimZ;
	const float fw = q * (float)dimW;

	const float fracX = fx - (int)fx;
	const float fracY = fy - (int)fy;
	const float fracZ = fz - (int)fz;
	const float fracW = fw - (int)fw;

	float weights[ 16 ];
	weights[ 0 ] = ( 1.0f - fracX ) * ( 1.0f - fracY ) * ( 1.0f - fracZ ) * ( 1.0f - fracW );
	weights[ 1 ] = ( 1.0f - fracX ) * ( fracY ) * ( 1.0f - fracZ ) * ( 1.0f - fracW );
	weights[ 2 ] = ( fracX ) * ( 1.0f - fracY ) * ( 1.0f - fracZ ) * ( 1.0f - fracW );
	weights[ 3 ] = ( fracX ) * ( fracY ) * ( 1.0f - fracZ ) * ( 1.0f - fracW );
	weights[ 4 ] = ( 1.0f - fracX ) * ( 1.0f - fracY ) * ( fracZ ) * ( 1.0f - fracW );
	weights[ 5 ] = ( 1.0f - fracX ) * ( fracY ) * ( fracZ ) * ( 1.0f - fracW );
	weights[ 6 ] = ( fracX ) * ( 1.0f - fracY ) * ( fracZ ) * ( 1.0f - fracW );
	weights[ 7 ] = ( fracX ) * ( fracY ) * ( fracZ ) * ( 1.0f - fracW );

	weights[ 8 ] = ( 1.0f - fracX ) * ( 1.0f - fracY ) * ( 1.0f - fracZ ) * ( fracW );
	weights[ 9 ] = ( 1.0f - fracX ) * ( fracY ) * ( 1.0f - fracZ ) * ( fracW );
	weights[10 ] = ( fracX ) * ( 1.0f - fracY ) * ( 1.0f - fracZ ) * ( fracW );
	weights[11 ] = ( fracX ) * ( fracY ) * ( 1.0f - fracZ ) * ( fracW );
	weights[12 ] = ( 1.0f - fracX ) * ( 1.0f - fracY ) * ( fracZ ) * ( fracW );
	weights[13 ] = ( 1.0f - fracX ) * ( fracY ) * ( fracZ ) * ( fracW );
	weights[14 ] = ( fracX ) * ( 1.0f - fracY ) * ( fracZ ) * ( fracW );
	weights[15 ] = ( fracX ) * ( fracY ) * ( fracZ ) * ( fracW );

	//
	//	Calculate final sample from weights
	//

	Vec4d finalSample = Vec4d( 0.0f );
	for ( int i = 0; i < 16; ++i ) {
		finalSample += weights[ i ] * samples[ i ];
	}
	return finalSample;
}
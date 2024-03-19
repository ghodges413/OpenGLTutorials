//
//  ShapeCapsule.cpp
//
#include "Physics/Shapes/ShapeCapsule.h"

/*
========================================================================================================

ShapeCapsule

========================================================================================================
*/

/*
====================================================
ShapeCapsule::Support
====================================================
*/
Vec3 ShapeCapsule::Support( const Vec3 & dir, const Vec3 & pos, const Quat & orient, const float bias ) const {
	Vec3 a = Vec3( 0.0f, 0.0f, -m_height * 0.5f );
	Vec3 b = Vec3( 0.0f, 0.0f, m_height * 0.5f );

	a = pos + orient.RotatePoint( a );
	b = pos + orient.RotatePoint( b );

	a = a + dir * ( m_radius + bias );
	b = b + dir * ( m_radius + bias );

	float distA = dir.Dot( a );
	float distB = dir.Dot( b );

	if ( distA > distB ) {
		return a;
	}
	return b;
}

static bool IsExternal( float radius, float height, Vec3 pt ) {
	Vec3 a = Vec3( 0, 0, -height * 0.5f );
	Vec3 b = Vec3( 0, 0, height * 0.5f );

	// If it's in the center/cylinder section return false
	if ( pt.z >= a.z && pt.z <= b.z && ( pt.x * pt.x + pt.y * pt.y ) <= radius * radius ) {
		return false;
	}
	
	// Check the spherical caps
	Vec3 ptA = pt - a;
	Vec3 ptB = pt - b;

	if ( ptA.GetLengthSqr() <= radius * radius ) {
		return false;
	}
	if( ptB.GetLengthSqr() <= radius * radius ) {
		return false;
	}

	// If we made it here, then we're not inside the capsule
	return true;
}


static void PrintTensor( const Mat3 & mat ) {
	for ( int j = 0; j < 3; j++ ) {
		printf( "%i:   %.3f %.3f %.3f\n", j, mat.rows[ j ].x, mat.rows[ j ].y, mat.rows[ j ].z );
	}
}

/*
====================================================
ShapeCapsule::InertiaTensor
====================================================
*/
Mat3 ShapeCapsule::CalculateInertiaTensor( float radius, float height ) const {
	const int numSamples = 100;

	const Bounds bounds = GetBounds();

	Mat3 tensor;
	tensor.Zero();

	const Vec3 cm = Vec3( 0, 0, 0 );

	const float dx = bounds.WidthX() / (float)numSamples;
	const float dy = bounds.WidthY() / (float)numSamples;
	const float dz = bounds.WidthZ() / (float)numSamples;

	int sampleCount = 0;
	for ( float x = bounds.mins.x; x < bounds.maxs.x; x += dx ) {
		for ( float y = bounds.mins.y; y < bounds.maxs.y; y += dy ) {
			for ( float z = bounds.mins.z; z < bounds.maxs.z; z += dz ) {
				Vec3 pt( x, y, z );

				if ( IsExternal( radius, height, pt ) ) {
					continue;
				}

				// Get the point relative to the center of mass
				pt -= cm;

				tensor.rows[ 0 ][ 0 ] += pt.y * pt.y + pt.z * pt.z;
				tensor.rows[ 1 ][ 1 ] += pt.z * pt.z + pt.x * pt.x;
				tensor.rows[ 2 ][ 2 ] += pt.x * pt.x + pt.y * pt.y;

				tensor.rows[ 0 ][ 1 ] += -1.0f * pt.x * pt.y;
				tensor.rows[ 0 ][ 2 ] += -1.0f * pt.x * pt.z;
				tensor.rows[ 1 ][ 2 ] += -1.0f * pt.y * pt.z;

				tensor.rows[ 1 ][ 0 ] += -1.0f * pt.x * pt.y;
				tensor.rows[ 2 ][ 0 ] += -1.0f * pt.x * pt.z;
				tensor.rows[ 2 ][ 1 ] += -1.0f * pt.y * pt.z;

				sampleCount++;
			}
		}
	}

	tensor *= 1.0f / (float)sampleCount;

	PrintTensor( tensor );
	InertiaTensor();

	return tensor;
}

/*
====================================================
ShapeCapsule::InertiaTensor
====================================================
*/
Mat3 ShapeCapsule::InertiaTensor() const {
	const float h = m_height;
	const float r = m_radius;
	const float h2 = h * h;
	const float r2 = r * r;
	const float r3 = r * r * r;

	const float pi = acosf( -1.0f );
	float mcy = h * r2 * pi;
	float mhs = 2.0f * r3 * pi / 3.0f;
	const float totalMass = mcy + 2.0f * mhs;
	mcy /= totalMass;
	mhs /= totalMass;

	const float xx = mcy * ( h2 / 12.0f + r2 / 4.0f ) + 2.0f * mhs * ( 2.0f * r2 / 5.0f + h2 / 4.0f + 3.0f * h * r / 8.0f );
	const float yy = xx;
	const float zz = mcy * ( r2 / 2.0f ) + 2.0f * mhs * ( 2.0f * r2 / 5.0f );

	Mat3 tensor;
	tensor.Zero();
	tensor.rows[ 0 ][ 0 ] = xx;
	tensor.rows[ 1 ][ 1 ] = yy;
	tensor.rows[ 2 ][ 2 ] = zz;
	return tensor;
}

/*
====================================================
ShapeCapsule::GetBounds
====================================================
*/
Bounds ShapeCapsule::GetBounds( const Vec3 & pos, const Quat & orient ) const {
	Vec3 a = Vec3( 0.0f, 0.0f, -m_height * 0.5f );
	Vec3 b = Vec3( 0.0f, 0.0f, m_height * 0.5f );

	a = pos + orient.RotatePoint( a );
	b = pos + orient.RotatePoint( b );

	Bounds tmp;
	tmp.Expand( a + Vec3( m_radius ) );
	tmp.Expand( a - Vec3( m_radius ) );

	tmp.Expand( b + Vec3( m_radius ) );
	tmp.Expand( b - Vec3( m_radius ) );
	return tmp;
}

/*
====================================================
ShapeCapsule::GetBounds
====================================================
*/
Bounds ShapeCapsule::GetBounds() const {
	Vec3 a = Vec3( 0.0f, 0.0f, -m_height * 0.5f );
	Vec3 b = Vec3( 0.0f, 0.0f, m_height * 0.5f );

	Bounds tmp;
	tmp.Expand( a + Vec3( m_radius ) );
	tmp.Expand( a - Vec3( m_radius ) );

	tmp.Expand( b + Vec3( m_radius ) );
	tmp.Expand( b - Vec3( m_radius ) );
	return tmp;
}
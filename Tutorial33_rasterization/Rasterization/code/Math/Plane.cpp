//
//  Plane.cpp
//
#include "Math/Plane.h"

/*
 =======================================
 Plane::Plane
 =======================================
 */
Plane::Plane() :
mD( 0 ) {
}

/*
 =======================================
 Plane::Plane
 =======================================
 */
Plane::Plane( const Plane & rhs ) :
mNormal( rhs.mNormal ),
mD( rhs.mD ) {
}

/*
 =======================================
 Plane::Plane
 =======================================
 */
Plane::Plane( const Vec3d & a, const Vec3d & b, const Vec3d & c ) {
	// assume that the triangle is CCW
	Vec3d ab = a - b;
	Vec3d ac = a - c;
	mNormal = ab.Cross( ac );
	mNormal.Normalize();
    
	//mNormal = hbGetNormalFromCCWTriangle( a, b, c );
    
	mD = -1.0f * a.Dot( mNormal );
    
	mPlanarPoint = a;
	// equation of a plane:   n.(r-r0)=0
	// or Nx*X + Ny*Y + Nz * Z + D = 0
	// where D = -R0x*Nx - R0y*Ny - R0z*Nz
}

/*
 =======================================
 Plane::Plane
 =======================================
 */
Plane::Plane( const float a, const float b, const float c, const float d ) {
    // assume that the triangle is CCW
    //    Vec3d ab = a - b;
    //    Vec3d ac = a - c;
    //    mNormal = ab.Cross( ac );
    //    mNormal.Normalize();
    mD = d;
    mNormal = Vec3d( a, b, c );
    const float t = mNormal.GetMagnitude();
    assert( t > 0 );
    mD /= t;
    mNormal.Normalize();
    
    // Calculate the planar point
    int axis = 0;
    const float x = fabsf( mNormal.x );
    const float y = fabsf( mNormal.y );
    const float z = fabsf( mNormal.z );
    if ( y > x && y > z ) {
        axis = 1;
    } else if ( z > x && z > y ) {
        axis = 2;
    }
    mPlanarPoint = Vec3d( 0, 0, 0 );
    mPlanarPoint[ axis ] = -mD / mNormal[ axis ];
    
    // equation of a plane:   n.(r-r0)=0
    // or Nx*X + Ny*Y + Nz * Z + D = 0
    // where D = -R0x*Nx - R0y*Ny - R0z*Nz
}

/*
 =======================================
 Plane::Plane
 =======================================
 */
Plane::Plane( const Vec3d & point, const Vec3d & normal ) :
mPlanarPoint( point ),
mNormal( normal ) {
    mD = -mPlanarPoint.Dot( mNormal );
}

/*
 =======================================
 Plane::operator =
 =======================================
 */
const Plane & Plane::operator = ( const Plane & rhs ) {
    mNormal = rhs.mNormal;
    mD = rhs.mD;
    mPlanarPoint = rhs.mPlanarPoint;
    return *this;
}

/*
 =======================================
 Plane::SideDistribution
 =======================================
 */
Plane::planeSide_t Plane::SideDistribution( const Vec3d * const points, const int numPoints ) const {
	int numPositive( 0 );
	int numNegative( 0 );

    for ( int i = 0; i < numPoints; ++i ) {
		const Vec3d & pt = points[ i ];

		// project the point onto the line coming out of the plane
		const float distance = DistanceFromPlane( pt );
		if ( distance < 0.0f ) {
			++numNegative;
		}
		if ( distance > 0.0f ) {
			++numPositive;
		}

		if ( numNegative > 0 && numPositive > 0 ) {
			return Plane::PS_STRADDLE;
		}
	}

	if ( 0 == numPositive && 0 == numNegative ) {
		return Plane::PS_ON;
	}

	if ( numPositive > 0 ) {
		return Plane::PS_POSITIVE;
	}

	return Plane::PS_NEGATIVE;
}

/*
 =======================================
 Plane::SideDistribution
 =======================================
 */
// Plane::planeSide_t Plane::SideDistribution( const hbArray< Vec3d > & points ) const {
// 	return SideDistribution( points.ToPtr(), points.Num() );
// }

/*
 =======================================
 Plane::SideDistribution
 =======================================
 */
// Plane::planeSide_t Plane::SideDistribution( const hbArray< Vec3d * > & points ) const {
// 	int numPositive( 0 );
// 	int numNegative( 0 );
// 
//     for ( int i = 0; i < points.Num(); ++i ) {
// 		const Vec3d & pt = *points[ i ];
// 
// 		// project the point onto the line coming out of the plane
// 		const float distance = DistanceFromPlane( pt );
// 		if ( distance < 0.0f ) {
// 			++numNegative;
// 		}
// 		if ( distance > 0.0f ) {
// 			++numPositive;
// 		}
// 
// 		if ( numNegative > 0 && numPositive > 0 ) {
// 			return Plane::PS_STRADDLE;
// 		}
// 	}
// 
// 	if ( 0 == numPositive && 0 == numNegative ) {
// 		return Plane::PS_ON;
// 	}
// 
// 	if ( numPositive > 0 ) {
// 		return Plane::PS_POSITIVE;
// 	}
// 
// 	return Plane::PS_NEGATIVE;
// }

static bool Solve(	const float x1, const float y1, const float d1,
					const float x2, const float y2, const float d2,
					float & solx, float & soly ) {
	// x1 * x + y1 * y + d1 = 0
	// x2 * x + y2 * y + d2 = 0
	// x = - 1 / x1 * ( y1 * y + d1 )
	// -x2 / x1 * ( y1 * y + d1 ) + y2 * y + d2 = 0
	// y * ( y2 - x2 / x1 * y1 ) + d2 - x2 / x1 * d1 = 0
	// y = ( ( x2 / x1 ) * d1 - d2 ) / ( y2 - ( x2 / x1 ) * y1 )

	const float epsilon = 0.000001f;

	// if x1 happens to be zero
	if ( fabsf( x1 ) <= epsilon && fabsf( y1 ) > epsilon && fabsf( x2 ) > epsilon ) {
		soly = -1.0f * d1 / y1;
		solx = ( -1.0f / x2 ) * ( y2 * soly + d2 );
		return true;
	}

	// if x2 happens to be zero
	if ( fabsf( x2 ) <= epsilon && fabsf( y2 ) > epsilon && fabsf( x1 ) > epsilon ) {
		soly = -1.0f * d2 / y2;
		solx = ( -1.0f / x1 ) * ( y1 * soly + d1 );
		return true;
	}

	// if y1 happens to be zero
	if ( fabsf( y1 ) <= epsilon && fabsf( x1 ) > epsilon && fabsf( y2 ) > epsilon ) {
		solx = -1.0f * d1 / x1;
		soly = ( -1.0f / y2 ) * ( x2 * solx + d2 );
		return true;
	}

	// if y2 happens to be zero
	if ( fabsf( y2 ) <= epsilon && fabsf( x2 ) > epsilon && fabsf( y1 ) > epsilon ) {
		solx = -1.0f * d2 / x2;
		soly = ( -1.0f / y1 ) * ( x1 * solx + d1 );
		return true;
	}
		
	if ( fabsf( x1 ) > epsilon ) {
		const float denom = ( y2 - ( x2 / x1 ) * y1 );
		if ( fabsf( denom ) > epsilon ) {
			soly = ( ( x2 / x1 ) * d1 - d2 ) / denom;
			solx = ( -1.0f / x1 ) * ( y1 * soly + d1 );
			return true;
		}
	}

	if ( fabsf( x2 ) > epsilon ) {
		const float denom = ( y1 - ( x1 / x2 ) * y2 );
		if ( fabsf( denom ) > epsilon ) {
			soly = ( ( x1 / x2 ) * d2 - d1 ) / denom;
			solx = ( -1.0f / x2 ) * ( y2 * soly + d2 );
			return true;
		}
	}

	return false;
}

/*
 =======================================
 Plane::IntersectPlane
 This may not be the most numerically robust function.
 There's probably also a faster way of calculating the
 intersecting line, but this is a nice geometric
 easy to understand solution.
 =======================================
 */
bool Plane::IntersectPlane( const Plane & rhs, Vec3d & start, Vec3d & dir ) const {
	const float absoluteDot = mNormal.Dot( rhs.mNormal );
	if ( fabsf( absoluteDot ) > 0.99f ) {
		// planes are very nearly parallel and probably intersect very very far away.
		return false;
	}

	// The direction of the line of intersection is the cross product
	// of the normals of the two planes
	const Vec3d & n1	= mNormal;
	const Vec3d & n2	= rhs.mNormal;
	const Vec3d n3	= (n1.Cross( n2 )).Normalize();

	// Store out the direction of the line of intersection
	dir = n3;

	// believe it or not, this actually does happen
	if ( mPlanarPoint == rhs.mPlanarPoint ) {
		start = mPlanarPoint;
		return true;
	}

#if 1
	if ( fabsf( n3.x ) > 0.5f ) {
		// set x = 0, since we know the line will pass through x = 0 at some point
		start.x = 0;
		return Solve( n1.y, n1.z, mD, n2.y, n2.z, rhs.mD, start.y, start.z );
	} else if ( fabsf( n3.y ) > 0.5f ) {
		// set y = 0, since we know the line will pass through y = 0 at some point
		start.y = 0;
		return Solve( n1.x, n1.z, mD, n2.x, n2.z, rhs.mD, start.x, start.z );
	} else if ( fabsf( n3.z ) > 0.5f ) {
		// set z = 0, since we know the line will pass through z = 0 at some point
		start.z = 0;
		return Solve( n1.x, n1.y, mD, n2.x, n2.y, rhs.mD, start.x, start.y );
	}
#else
	// Calculate the ray directions, in the two planes, that point
	// point towards the line of intersection.
	const Vec3d ray1 = n1.Cross( n3 ).Normalize();
	const Vec3d ray2 = n2.Cross( n3 ).Normalize();

	// The two closest points of these two lines that are in each plane
	// and orthogonal to the line of intersection, will have their closest points
	// where they intersect the line of intersection.
	float t1( 0.0f );
	float t2( 0.0f );
	Vec3d p1;
	Vec3d p2;
	hbClosestPointsRayRay( mPlanarPoint, ray1, rhs.mPlanarPoint, ray2, t1, t2, p1, p2 );

	// Store out a point in the line of intersection
	start = p1;
#endif
	return true;
}

/*
 =======================================
 Plane::IntersectSegment
 =======================================
 */
// bool Plane::IntersectSegment( const Vec3d & a, const Vec3d & b, float & t, Vec3d & p ) const {
// 	return hbIntersectSegmentPlane( a, b, *this, t, p );
// }

/*
 =======================================
 Plane::IntersectRay
 =======================================
 */
bool Plane::IntersectRay( const Vec3d & start, const Vec3d & dir, float & t, Vec3d & p ) const {
	const Vec3d normDir = dir / dir.GetMagnitude();

	const float cosTheta = mNormal.Dot( normDir );
	if ( fabsf( cosTheta ) < 0.0001f ) {
		// ray runs perpindicular to the normal... intersection at infinity
		return false;
	}

#if 0
	t = ( mD - mNormal.DotProduct( start ) ) / alpha;

	p = start + t * dir;
#else
	// get the distance from the plane
	const float distance = DistanceFromPlane( start );

	// get the ray distance to point of intersection
	float rayT = distance / cosTheta;

	if ( distance < 0 && cosTheta > 0 ) {
		// start is in back and points towards plane
		rayT = -distance / cosTheta;
	} else if ( distance > 0 && cosTheta < 0 ) {
		// start is in front and points towards plane
		rayT = distance / cosTheta;
	} else if ( distance < 0 && cosTheta < 0 ) {
		// start is in back and points away
		rayT = distance / cosTheta;
	} else if ( distance > 0 && cosTheta > 0 ) {
		// start is in front and points away
		rayT = -distance / cosTheta;
	}

	p = start + rayT * normDir;

	// rayT * normDir = t * dir
	t = rayT / dir.GetMagnitude();
	
#endif
	return false;
}




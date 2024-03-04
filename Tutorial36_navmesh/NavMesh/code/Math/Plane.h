//
//  Plane.h
//
#pragma once
#include "Math/Vector.h"

template < typename T > class hbArray;

/*
=======================================
Plane
=======================================
*/
class Plane {
public:
    Plane();
    Plane( const Plane & rhs );
    Plane( const Vec3 & a, const Vec3 & b, const Vec3 & c );
    Plane( const float a, const float b, const float c, const float d );
    Plane( const Vec3 & point, const Vec3 & normal );
    const Plane & operator = ( const Plane & rhs );
    ~Plane() {}

	float DistanceFromPlane( const Vec3 & pt ) const;

	enum planeSide_t {
		PS_POSITIVE,
		PS_NEGATIVE,
		PS_ON,
		PS_STRADDLE,
	};
// 	planeSide_t SideDistribution( const hbArray< Vec3 > & points ) const;
// 	planeSide_t SideDistribution( const hbArray< Vec3 * > & points ) const;
	planeSide_t SideDistribution( const Vec3 * const points, const int numPoints ) const;

	bool IntersectRay( const Vec3 & start, const Vec3 & dir, float & t, Vec3 & p ) const;
	bool IntersectPlane( const Plane & rhs, Vec3 & start, Vec3 & dir ) const;
//	bool IntersectSegment( const Vec3 & a, const Vec3 & b, float & t, Vec3 & p ) const;
    
public:
    Vec3 mNormal;
    float   mD;
    Vec3 mPlanarPoint;
};

/*
=======================================
Plane::DistanceFromPlane
=======================================
*/
inline float Plane::DistanceFromPlane( const Vec3 & pt ) const {
	const Vec3 ray = pt - mPlanarPoint;
	return ( ray.Dot( mNormal ) );
}


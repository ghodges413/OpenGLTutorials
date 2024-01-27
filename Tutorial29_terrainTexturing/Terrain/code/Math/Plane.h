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
    Plane( const Vec3d & a, const Vec3d & b, const Vec3d & c );
    Plane( const float a, const float b, const float c, const float d );
    Plane( const Vec3d & point, const Vec3d & normal );
    const Plane & operator = ( const Plane & rhs );
    ~Plane() {}

	float DistanceFromPlane( const Vec3d & pt ) const;

	enum planeSide_t {
		PS_POSITIVE,
		PS_NEGATIVE,
		PS_ON,
		PS_STRADDLE,
	};
// 	planeSide_t SideDistribution( const hbArray< Vec3d > & points ) const;
// 	planeSide_t SideDistribution( const hbArray< Vec3d * > & points ) const;
	planeSide_t SideDistribution( const Vec3d * const points, const int numPoints ) const;

	bool IntersectRay( const Vec3d & start, const Vec3d & dir, float & t, Vec3d & p ) const;
	bool IntersectPlane( const Plane & rhs, Vec3d & start, Vec3d & dir ) const;
//	bool IntersectSegment( const Vec3d & a, const Vec3d & b, float & t, Vec3d & p ) const;
    
public:
    Vec3d mNormal;
    float   mD;
    Vec3d mPlanarPoint;
};

/*
=======================================
Plane::DistanceFromPlane
=======================================
*/
inline float Plane::DistanceFromPlane( const Vec3d & pt ) const {
	const Vec3d ray = pt - mPlanarPoint;
	return ( ray.DotProduct( mNormal ) );
}


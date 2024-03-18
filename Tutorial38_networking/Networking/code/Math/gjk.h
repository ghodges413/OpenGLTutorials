//
//  gjk.h
//
#pragma once
#include "Vector.h"
#include "Quat.h"
#include "../Physics/Body.h"

struct point_t {
	Vec3 xyz;	// The point on the minkowski sum
	Vec3 ptA;	// The point on bodyA
	Vec3 ptB;	// The point on bodyB

	point_t() : xyz( 0.0f ), ptA( 0.0f ), ptB( 0.0f ) {}

	const point_t & operator = ( const point_t & rhs ) {
		xyz = rhs.xyz;
		ptA = rhs.ptA;
		ptB = rhs.ptB;
		return *this;
	}

	bool operator == ( const point_t & rhs ) const {
		return ( ( ptA == rhs.ptA ) && ( ptB == rhs.ptB ) && ( xyz == rhs.xyz ) );
	}
};

point_t Support( const Body * bodyA, const Body * bodyB, Vec3 dir, const float bias );

void GJK_ClosestPoints( const Body * bodyA, const Body * bodyB, Vec3 & ptOnA, Vec3 & ptOnB );
bool GJK_DoesIntersect( const Body * bodyA, const Body * bodyB, const float bias, Vec3 & ptOnA, Vec3 & ptOnB );
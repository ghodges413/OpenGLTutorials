//
//	Bounds.h
//
#pragma once
#include "Vector.h"

/*
================================
Bounds
================================
*/
class Bounds {
public:
	Bounds() {
		Clear();
	}
	Bounds( Vec3d _min, Vec3d _max ) : min( _min ), max( _max ) {}
	
	void Clear();
	bool IsInitialized() const;
	void AddPoint( Vec3d pt );
	bool HasPoint( Vec3d pt ) const;
	bool HasPoint2D( Vec3d pt ) const;
	bool IntersectBounds( const Bounds & rhs ) const;
	bool IntersectBounds2D( const Bounds & rhs ) const;

	Vec3d min;
	Vec3d max;
};

/*
================================
Bounds::Clear
================================
*/
inline void Bounds::Clear() {
	const float val = 1e6;
	min = Vec3d( val, val, val );
	max = Vec3d( -val, -val, -val );
}

/*
================================
Bounds::IsInitialized
================================
*/
inline bool Bounds::IsInitialized() const {
	if ( max.x > min.x ) {
		return true;
	}
	if ( max.y > min.y ) {
		return true;
	}
	if ( max.z > min.z ) {
		return true;
	}
	return false;
}

/*
================================
Bounds::AddPoint
================================
*/
inline void Bounds::AddPoint( Vec3d pt ) {
	if ( pt.x < min.x ) {
		min.x = pt.x;
	}
	if ( pt.y < min.y ) {
		min.y = pt.y;
	}
	if ( pt.z < min.z ) {
		min.z = pt.z;
	}

	if ( pt.x > max.x ) {
		max.x = pt.x;
	}
	if ( pt.y > max.y ) {
		max.y = pt.y;
	}
	if ( pt.z > max.z ) {
		max.z = pt.z;
	}
}

/*
================================
Bounds::HasPoint
================================
*/
inline bool Bounds::HasPoint( Vec3d pt ) const {
	if ( pt.x < min.x || pt.y < min.y || pt.z < min.z ) {
		return false;
	}
	if ( pt.x > max.x || pt.y > max.y || pt.z > max.z ) {
		return false;
	}
	return true;
}

/*
================================
Bounds::HasPoint2D
================================
*/
inline bool Bounds::HasPoint2D( Vec3d pt ) const {
	if ( pt.x < min.x || pt.y < min.y ) {
		return false;
	}
	if ( pt.x > max.x || pt.y > max.y ) {
		return false;
	}
	return true;
}

/*
================================
Bounds::IntersectBounds
================================
*/
inline bool Bounds::IntersectBounds( const Bounds & rhs ) const {
	if ( max.x < rhs.min.x || min.x > rhs.max.x ) {
		return false;
	}
	if ( max.y < rhs.min.y || min.y > rhs.max.y ) {
		return false;
	}
	if ( max.z < rhs.min.z || min.z > rhs.max.z ) {
		return false;
	}
	return true;
}

/*
================================
Bounds::IntersectBounds2D
================================
*/
inline bool Bounds::IntersectBounds2D( const Bounds & rhs ) const {
	if ( max.x < rhs.min.x || min.x > rhs.max.x ) {
		return false;
	}
	if ( max.y < rhs.min.y || min.y > rhs.max.y ) {
		return false;
	}
	return true;
}
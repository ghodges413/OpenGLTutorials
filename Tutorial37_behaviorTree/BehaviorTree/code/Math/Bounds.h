//
//	Bounds.h
//
#pragma once
#include <math.h>
#include <assert.h>
#include "Vector.h"
#include <vector>

/*
====================================================
Bounds
====================================================
*/
class Bounds {
public:
	Bounds() { Clear(); }
	Bounds( const Bounds & rhs ) : mins( rhs.mins ), maxs( rhs.maxs ) {}
	Bounds( const Vec3 & mins_, const Vec3 & maxs_ ) : mins( mins_ ), maxs( maxs_ ) {}
	const Bounds & operator = ( const Bounds & rhs );
	~Bounds() {}

	void Clear() { mins = Vec3( 1e6 ); maxs = Vec3( -1e6 ); }
	bool DoesIntersect( const Bounds & rhs ) const;
	bool DoesIntersect( const Vec3 & pt ) const;
	void Expand( const Vec3 * pts, const int num );
	void Expand( const Vec3 & rhs );
	void Expand( const Bounds & rhs );

	float WidthX() const { return maxs.x - mins.x; }
	float WidthY() const { return maxs.y - mins.y; }
	float WidthZ() const { return maxs.z - mins.z; }

	float SurfaceArea() const;
	Vec3 Center() const { return ( mins + maxs ) * 0.5f; }

	bool IsValid() const;

public:
	Vec3 mins;
	Vec3 maxs;
};
//
//  Brush.h
//
#pragma once
#include "Math/Vector.h"
#include "Math/Bounds.h"
#include "Graphics/Mesh.h"
#include "Graphics/RenderSurface.h"
#include "Graphics/Shader.h"
#include "Miscellaneous/Types.h"
#include <vector>

struct winding_t {
	// A winding is a counter clockwise ordered set of points.
	// There is an edge between adjacent points.
	std::vector< Vec3 > pts;
	bool useForSplitting;
};

struct plane_t {
	Vec3 pts[ 3 ];
	Vec3 normal;
};

#define s_maxPlanes 36
struct brush_t {
	int numPlanes;
	plane_t planes[ s_maxPlanes ];
	winding_t windings[ s_maxPlanes ];
	Bounds bounds;
};

void BuildBrush( brush_t & brush );
winding_t BuildOversizedWinding( const plane_t * p );

/*
================================
CalculateNormal
================================
*/
inline Vec3 CalculateNormal( const Vec3 & a, const Vec3 & b, const Vec3 & c ) {
	Vec3 norm = ( b - a ).Cross( c - a );
	norm.Normalize();
	return norm;
}

/*
================================
PlaneNormal
================================
*/
inline Vec3 PlaneNormal( const plane_t & a ) {
	return CalculateNormal( a.pts[ 0 ], a.pts[ 1 ], a.pts[ 2 ] );
}
//
//  Brush.cpp
//
#include "BSP/Brush.h"
#include "Math/MatrixOps.h"
#include "Math/Matrix.h"
#include "Math/Bounds.h"
#include "Graphics/Graphics.h"
#include "Graphics/TextureManager.h"
#include "Graphics/ShaderManager.h"
#include "Graphics/Targa.h"
#include "Miscellaneous/Fileio.h"
#include <math.h>
#include <algorithm>
#include <stdlib.h>
#include <assert.h>
#include <vector>

/*
================================================================
 
Brush
 
================================================================
*/

/*
================================
BuildOversizedWinding
================================
*/
winding_t BuildOversizedWinding( const plane_t * p ) {
	// Choose a semi-tangent vector to the face
	Vec3 up = Vec3( 0, 0, 1 );
	if ( fabsf( p->normal.z ) > fabsf( p->normal.y ) && fabsf( p->normal.z ) > fabsf( p->normal.x ) ) {
		up = Vec3( 1, 0, 0 );
	}

	// Project the origin onto the plane
	Vec3 ao = Vec3( 0.0f ) - p->pts[ 0 ];
	float dist = -p->normal.Dot( ao );
	Vec3 org = p->normal * dist;

	// Build the tangent vectors on this plane
	//Vec3 org = p->normal * p->dist;
	up = up - p->normal * up.Dot( p->normal );
	up.Normalize();	
	Vec3 right = up.Cross( p->normal );

	up *= 65536;
	right *= 65536;

	// Build a large 2d box projected onto the plane
	winding_t winding;
// 	winding.pts.push_back( org - right + up );
// 	winding.pts.push_back( org + right + up );
// 	winding.pts.push_back( org + right - up );
// 	winding.pts.push_back( org - right - up );

	winding.pts.push_back( org + right + up );
	winding.pts.push_back( org - right + up );
	winding.pts.push_back( org - right - up );
	winding.pts.push_back( org + right - up );
	return winding;
}

/*
================================
ClipWinding
================================
*/
void ClipWinding( winding_t & winding, const plane_t * split ) {
	winding_t tmp;
	for ( int idx0 = 0; idx0 < winding.pts.size() ; idx0++ ) {
		int idx1 = ( idx0 + 1 ) % winding.pts.size();
		Vec3 p0 = winding.pts[ idx0 ];
		Vec3 p1 = winding.pts[ idx1 ];

		// vector from plane's surface to point
		Vec3 pp0 = p0 - split->pts[ 0 ];
		Vec3 pp1 = p1 - split->pts[ 0 ];

		float dist0 = split->normal.Dot( pp0 );
		float dist1 = split->normal.Dot( pp1 );

		// Keep points that are on the slicing plane
		if ( fabsf( dist0 ) < 0.001f ) {
			tmp.pts.push_back( p0 );
			continue;
		}

		// Keep points that are behind the slicing plane
		if ( dist0 < 0.0f ) {
			tmp.pts.push_back( p0 );
		}

		// If the second point is on the plane, then there's no need to split this segment
		if ( fabsf( dist1 ) < 0.001f ) {
			continue;
		}

		// If both points are on the same side of the plane, then there's no need to split this segment
		if ( dist0 * dist1 > 0.0f ) {
			continue;
		}

		// Split this segment
		float t = dist0 / ( dist0 - dist1 );
		Vec3 mid = p0 + ( p1 - p0 ) * t;
		tmp.pts.push_back( mid );
	}

	winding = tmp;
}

/*
================================
IsCoplanar
================================
*/
bool IsCoplanar( const plane_t * a, const plane_t * b ) {
	float dot = a->normal.Dot( b->normal );
	if ( dot < 0.999f ) {
		return false;
	}

	float distA = a->pts[ 0 ].Dot( a->normal );
	float distB = b->pts[ 0 ].Dot( b->normal );

	Vec3 ab = b->pts[ 0 ] - a->pts[ 0 ];
	float dist = a->normal.Dot( ab );

	if ( fabsf( dist ) > 0.001f ) {
		return false;
	}

	return true;
}

/*
================================
BuildWinding
================================
*/
winding_t BuildWinding( brush_t * brush, const int index ) {
	const plane_t * p = &brush->planes[ index ];

	// Build an over-sized winding that we can slice up
	winding_t winding = BuildOversizedWinding( p );

	// Clip the winding by the other faces of the brush
	for ( int i = 0; i < brush->numPlanes; i++ ) {
		if ( index == i ) {
			continue;
		}

		const plane_t * clipPlane = &brush->planes[ i ];
		if ( IsCoplanar( p, clipPlane ) ) {
			continue;
		}

		ClipWinding( winding, clipPlane );
	}

	if ( winding.pts.size() < 3 ) {
		winding.pts.clear();
	}

	return winding;
}

/*
================================
BuildBrush
================================
*/
void BuildBrush( brush_t & brush ) {
	for ( int i = 0; i < brush.numPlanes; i++ ) {
		brush.windings[ i ] = BuildWinding( &brush, i );
	}
}

//
//  epa.cpp
//
#include "epa.h"
#include "SignedVolumes.h"
#include "lcp.h"

/*
========================================================================================================

EPA - shape version of the epa

========================================================================================================
*/
Vec3 BarycentericCoordinatesSignedVolume( Vec3 s1, Vec3 s2, Vec3 s3, const Vec3 & pt ) {
	s1 = s1 - pt;
	s2 = s2 - pt;
	s3 = s3 - pt;

	Vec3 normal = ( s2 - s1 ).Cross( s3 - s1 );
	Vec3 p0 = normal * s1.Dot( normal ) / normal.GetLengthSqr();

	// Find the axis with the greatest projected area
	int idx = 0;
	float area_max = 0;
	for ( int i = 0; i < 3; i++ ) {
		int j = ( i + 1 ) % 3;
		int k = ( i + 2 ) % 3;

		Vec2 a = Vec2( s1[ j ], s1[ k ] );
		Vec2 b = Vec2( s2[ j ], s2[ k ] );
		Vec2 c = Vec2( s3[ j ], s3[ k ] );
		Vec2 ab = b - a;
		Vec2 ac = c - a;

		float area = ab.x * ac.y - ab.y * ac.x;
		if ( area * area > area_max * area_max ) {
			idx = i;
			area_max = area;
		}
	}

	// Project onto the appropriate axis
	int x = ( idx + 1 ) % 3;
	int y = ( idx + 2 ) % 3;
	Vec2 s[ 3 ];
	s[ 0 ] = Vec2( s1[ x ], s1[ y ] );
	s[ 1 ] = Vec2( s2[ x ], s2[ y ] );
	s[ 2 ] = Vec2( s3[ x ], s3[ y ] );
	Vec2 p = Vec2( p0[ x ], p0[ y ] );

	// Get the sub-areas of the triangles formed from the projected origin and the edges
	Vec3 areas;
	for ( int i = 0; i < 3; i++ ) {
		int j = ( i + 1 ) % 3;
		int k = ( i + 2 ) % 3;

		Vec2 a = p;
		Vec2 b = s[ j ];
		Vec2 c = s[ k ];
		Vec2 ab = b - a;
		Vec2 ac = c - a;

		areas[ i ] = ab.x * ac.y - ab.y * ac.x;
	}

	Vec3 lambdas = areas / area_max;
	if ( !lambdas.IsValid() ) {
		printf( "oof bary signed volume\n" );
		lambdas = Vec3( 1, 0, 0 );
	}
	return lambdas;
}

float TriangleArea( const Vec3 & a, const Vec3 & b, const Vec3 & c ) {
	const Vec3 ab = b - a;
	const Vec3 ac = c - a;
	const Vec3 norm = ab.Cross( ac );
	return norm.GetMagnitude() * 0.5f;
}

Vec3 BarycentricCoordinatesLine( const Vec3 & a, const Vec3 & b, const Vec3 & c, const Vec3 & pt ) {
	Vec3 furthestPts[ 2 ];
	int furthestIdxs[ 2 ];

	float ab = ( b - a ).GetLengthSqr();
	float ac = ( c - a ).GetLengthSqr();
	float bc = ( c - b ).GetLengthSqr();

	if ( ab > ac && ab > bc ) {
		furthestPts[ 0 ] = a;
		furthestPts[ 1 ] = b;

		furthestIdxs[ 0 ] = 0;
		furthestIdxs[ 1 ] = 1;
	} else if ( ac > ab && ac > bc ) {
		furthestPts[ 0 ] = a;
		furthestPts[ 1 ] = c;

		furthestIdxs[ 0 ] = 0;
		furthestIdxs[ 1 ] = 2;
	} else {
		furthestPts[ 0 ] = b;
		furthestPts[ 1 ] = c;

		furthestIdxs[ 0 ] = 1;
		furthestIdxs[ 1 ] = 2;
	}

	Vec3 coords;
	coords.Zero();

	// Get the normalize ray [a,b] and the distance between
	Vec3 ray = furthestPts[ 1 ] - furthestPts[ 0 ];
	float rayLength = ray.GetMagnitude();
	ray.Normalize();

	// Project ray to pt onto the ray between [a,b]
	Vec3 rayPt = pt - furthestPts[ 0 ];
	float dist = rayPt.Dot( ray );

	// Normalize projection
	float t = dist / rayLength;
	if ( 0.0f * t != 0.0f * t ) {
		coords[ 0 ] = 1.0f;
		coords[ 1 ] = 0.0f;
		return coords;
	}

	if ( t < 0.0f ) {
		t = 0.0f;
	}
	if ( t > 1.0f ) {
		t = 1.0f;
	}

	coords[ furthestIdxs[ 0 ] ] = 1.0f - t;
	coords[ furthestIdxs[ 1 ] ] = t;
}

// From Real-time collision detection
Vec3 BarycentricCoordinates2( const Vec3 & a, const Vec3 & b, const Vec3 & c, const Vec3 & pt, int recurssion = 0 ) {
	// Solves for the barycentric coordinates of pt on abc:
	Vec3 v0 = b - a;
	Vec3 v1 = c - a;
	Vec3 v2 = pt - a;
	float d00 = v0.Dot( v0 );
	float d01 = v0.Dot( v1 );
	float d11 = v1.Dot( v1 );
	float d20 = v2.Dot( v0 );
	float d21 = v2.Dot( v1 );
	float denom = d00 * d11 - d01 * d01;

	Vec3 coords;
	coords.y = ( d11 * d20 - d01 * d21 ) / denom;
	coords.z = ( d00 * d21 - d01 * d20 ) / denom;
	coords.x = 1.0f - coords.y - coords.z;
	if ( !coords.IsValid() ) {
		if ( recurssion >= 2 ) {
			coords = BarycentricCoordinatesLine( a, b, c, pt );
		} else {
			Vec3 tmp = BarycentricCoordinates2( b, c, a, pt, recurssion + 1 );
			coords[ 0 ] = tmp[ 2 ];
			coords[ 1 ] = tmp[ 0 ];
			coords[ 2 ] = tmp[ 1 ];
		}
	}
	return coords;
}

Vec3 BarycentricCoordinates( const Vec3 & a, const Vec3 & b, const Vec3 & c, const Vec3 & pt ) {
#if 1
	return BarycentericCoordinatesSignedVolume( a, b, c, pt );
#elif 1
	return BarycentricCoordinates2( a, b, c, pt );
#elif 1
	Vec3 ab = b - a;
	Vec3 ac = c - a;
	Vec3 normal = ab.Cross( ac );
	Vec3 ap = pt - a;
	Vec3 projPt = pt - normal * ap.Dot( normal ) / normal.GetLengthSqr();

	float areaABC = TriangleArea( a, b, c );
	float areaPBC = TriangleArea( projPt, b, c );
	float areaPCA = TriangleArea( projPt, c, a );

	Vec3 coords;
	coords.x = areaPBC / areaABC;
	coords.y = areaPCA / areaABC;
	coords.z = 1.0f - coords.x - coords.y;
	if ( !coords.IsValid() ) {
		printf( "oof bary\n" );
		coords = Vec3( 1, 0, 0 );
}
	return coords;
#else
	float areaABC = TriangleArea( a, b, c );
	float areaPBC = TriangleArea( pt, b, c );
	float areaPCA = TriangleArea( pt, c, a );

	Vec3 coords;
	coords.x = areaPBC / areaABC;
	coords.y = areaPCA / areaABC;
	coords.z = 1.0f - coords.x - coords.y;
	if ( !coords.IsValid() ) {
		printf( "oof bary\n" );
		coords = Vec3( 1, 0, 0 );
	}
	return coords;
#endif
}

/*
====================================================
NormalDirection
====================================================
*/
Vec3 NormalDirection( const tri_t & tri, const std::vector< point_t > & points ) {
	const Vec3 & a = points[ tri.a ].xyz;
	const Vec3 & b = points[ tri.b ].xyz;
	const Vec3 & c = points[ tri.c ].xyz;

	Vec3 ab = b - a;
	Vec3 ac = c - a;
	Vec3 normal = ab.Cross( ac );
	normal.Normalize();
	return normal;
}

/*
====================================================
SignedDistanceToTriangle
====================================================
*/
float SignedDistanceToTriangle( const tri_t & tri, const Vec3 & pt, const std::vector< point_t > & points ) {
	const Vec3 normal = NormalDirection( tri, points );
	const Vec3 & a = points[ tri.a ].xyz;
	const Vec3 a2pt = pt - a;
	const float dist = normal.Dot( a2pt );
	return dist;
}

/*
====================================================
ClosestTriangle
====================================================
*/
int ClosestTriangle( const std::vector< tri_t > & triangles, const std::vector< point_t > & points ) {
	float minDistSqr = 1e10;

	int idx = -1;
	for ( int i = 0; i < triangles.size(); i++ ) {
		const tri_t & tri = triangles[ i ];

		float dist = SignedDistanceToTriangle( tri, Vec3( 0.0f ), points );
		float distSqr = dist * dist;
		if ( distSqr < minDistSqr ) {
			idx = i;
			minDistSqr = distSqr;
		}
	}

	return idx;
}

/*
====================================================
HasPoint
====================================================
*/
bool HasPoint( const Vec3 & w, const std::vector< tri_t > triangles, const std::vector< point_t > & points ) {
	const float epsilons = 0.001f * 0.001f;
	Vec3 delta;

	for ( int i = 0; i < triangles.size(); i++ ) {
		const tri_t & tri = triangles[ i ];

		delta = w - points[ tri.a ].xyz;
		if ( delta.GetLengthSqr() < epsilons ) {
			return true;
		}
		delta = w - points[ tri.b ].xyz;
		if ( delta.GetLengthSqr() < epsilons ) {
			return true;
		}
		delta = w - points[ tri.c ].xyz;
		if ( delta.GetLengthSqr() < epsilons ) {
			return true;
		}
	}
	return false;
}

/*
====================================================
RemoveTrianglesFacingPoint
====================================================
*/
int RemoveTrianglesFacingPoint( const Vec3 & pt, std::vector< tri_t > & triangles, const std::vector< point_t > & points ) {
	int numRemoved = 0;
	for ( int i = 0; i < triangles.size(); i++ ) {
		const tri_t & tri = triangles[ i ];

		float dist = SignedDistanceToTriangle( tri, pt, points );
		if ( dist > 0.0f ) {
			// This triangle faces the point.  Remove it.
			triangles.erase( triangles.begin() + i );
			i--;
			numRemoved++;
		}
	}
	return numRemoved;
}

/*
====================================================
FindDanglingEdges
====================================================
*/
void FindDanglingEdges( std::vector< edge_t > & danglingEdges, const std::vector< tri_t > & triangles ) {
	danglingEdges.clear();

	for ( int i = 0; i < triangles.size(); i++ ) {
		const tri_t & tri = triangles[ i ];

		edge_t edges[ 3 ];
		edges[ 0 ].a = tri.a;
		edges[ 0 ].b = tri.b;

		edges[ 1 ].a = tri.b;
		edges[ 1 ].b = tri.c;

		edges[ 2 ].a = tri.c;
		edges[ 2 ].b = tri.a;

		int counts[ 3 ];
		counts[ 0 ] = 0;
		counts[ 1 ] = 0;
		counts[ 2 ] = 0;

		for ( int j = 0; j < triangles.size(); j++ ) {
			if ( j == i ) {
				continue;
			}

			const tri_t & tri2 = triangles[ j ];

			edge_t edges2[ 3 ];
			edges2[ 0 ].a = tri2.a;
			edges2[ 0 ].b = tri2.b;

			edges2[ 1 ].a = tri2.b;
			edges2[ 1 ].b = tri2.c;

			edges2[ 2 ].a = tri2.c;
			edges2[ 2 ].b = tri2.a;

			for ( int k = 0; k < 3; k++ ) {
				if ( edges[ k ] == edges2[ 0 ] ) {
					counts[ k ]++;
				}
				if ( edges[ k ] == edges2[ 1 ] ) {
					counts[ k ]++;
				}
				if ( edges[ k ] == edges2[ 2 ] ) {
					counts[ k ]++;
				}		
			}
		}

		// An edge that isn't shared, is dangling 
		for ( int k = 0; k < 3; k++ ) {
			if ( 0 == counts[ k ] ) {
				danglingEdges.push_back( edges[ k ] );
			}
		}		
	}
}

/*
====================================================
EPA_Expand
====================================================
*/
float EPA_Expand( const Body * bodyA, const Body * bodyB, const float bias, const point_t simplexPoints[ 4 ], Vec3 & ptOnA, Vec3 & ptOnB ) {
	std::vector< point_t > points;
	std::vector< tri_t > triangles;
	std::vector< edge_t > danglingEdges;

	Vec3 center( 0.0f );
	for ( int i = 0; i < 4; i++ ) {
		points.push_back( simplexPoints[ i ] );
		center += simplexPoints[ i ].xyz;
	}
	center *= 0.25f;

	// Build the triangles
	for ( int i = 0; i < 4; i++ ) {
		int j = ( i + 1 ) % 4;
		int k = ( i + 2 ) % 4;
		tri_t tri;
		tri.a = i;
		tri.b = j;
		tri.c = k;

		int unusedPt = ( i + 3 ) % 4;
		float dist = SignedDistanceToTriangle( tri, points[ unusedPt ].xyz, points );

		// The unused point is always on the negative/inside of the triangle.. make sure the normal points away
		if ( dist > 0.0f ) {
			std::swap( tri.a, tri.b );
		}

		triangles.push_back( tri );
	}

	//
	//	Expand the simplex to find the closest face of the CSO to the origin
	//
	while ( 1 ) {
		const int idx = ClosestTriangle( triangles, points );
		Vec3 normal = NormalDirection( triangles[ idx ], points );

		const point_t newPt = Support( bodyA, bodyB, normal, bias );

		// if w already exists, then just stop
		// because it means we can't expand any further
		if ( HasPoint( newPt.xyz, triangles, points ) ) {
			break;
		}

		float dist = SignedDistanceToTriangle( triangles[ idx ], newPt.xyz, points );
		if ( dist <= 0.0f ) {
			break;	// can't expand
		}

		const int newIdx = (int)points.size();
		points.push_back( newPt );

		// Remove Triangles that face this point
		int numRemoved = RemoveTrianglesFacingPoint( newPt.xyz, triangles, points );
		if ( 0 == numRemoved ) {
			break;
		}

		// Find Dangling Edges
		danglingEdges.clear();
		FindDanglingEdges( danglingEdges, triangles );
		if ( 0 == danglingEdges.size() ) {
			break;
		}

		// In theory the edges should be a proper CCW order
		// So we only need to add the new point as 'a' in order
		// to create new triangles that face away from origin
		for ( int i = 0; i < danglingEdges.size(); i++ ) {
			const edge_t & edge = danglingEdges[ i ];

			tri_t triangle;
			triangle.a = newIdx;
			triangle.b = edge.b;
			triangle.c = edge.a;

			// Make sure it's oriented properly
			float dist = SignedDistanceToTriangle( triangle, center, points );
			if ( dist > 0.0f ) {
				std::swap( triangle.b, triangle.c );
			}

			triangles.push_back( triangle );
		}
	}

	// Get the projection of the origin on the closest triangle
	const int idx = ClosestTriangle( triangles, points );
	const tri_t & tri = triangles[ idx ];
	Vec3 ptA_w = points[ tri.a ].xyz;
	Vec3 ptB_w = points[ tri.b ].xyz;
	Vec3 ptC_w = points[ tri.c ].xyz;
	Vec3 lambdas = BarycentricCoordinates( ptA_w, ptB_w, ptC_w, Vec3( 0.0f ) );

	// Get the point on shape A
	Vec3 ptA_a = points[ tri.a ].ptA;
	Vec3 ptB_a = points[ tri.b ].ptA;
	Vec3 ptC_a = points[ tri.c ].ptA;
	ptOnA = ptA_a * lambdas[ 0 ] + ptB_a * lambdas[ 1 ] + ptC_a * lambdas[ 2 ];

	// Get the point on shape B
	Vec3 ptA_b = points[ tri.a ].ptB;
	Vec3 ptB_b = points[ tri.b ].ptB;
	Vec3 ptC_b = points[ tri.c ].ptB;
	ptOnB = ptA_b * lambdas[ 0 ] + ptB_b * lambdas[ 1 ] + ptC_b * lambdas[ 2 ];

	// Return the penetration distance
	Vec3 delta = ptOnB - ptOnA;
	return delta.GetMagnitude();
}
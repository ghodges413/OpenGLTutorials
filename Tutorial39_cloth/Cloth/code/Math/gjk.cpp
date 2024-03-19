//
//  gjk.cpp
//
#include <string>
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <algorithm>
#include "Vector.h"
#include <vector>
#include "gjk.h"
#include "epa.h"
#include "SignedVolumes.h"

/*
========================================================================================================

GJK - This is the generic shape version of the algorithm

========================================================================================================
*/

/*
====================================================
Support
The support function returns the point in the minkowski
"difference" that is furthest in a particular direction.
In this case, our minkowski sum is A - B
====================================================
*/
point_t Support( const Body * bodyA, const Body * bodyB, Vec3 dir, const float bias ) {
	dir.Normalize();

	point_t point;

	// Find the point in A furthest in direction
	point.ptA = bodyA->m_shape->Support( dir, bodyA->m_position, bodyA->m_orientation, bias );

	dir *= -1.0f;

	// Find the point in B furthest in the opposite direction
	point.ptB = bodyB->m_shape->Support( dir, bodyB->m_position, bodyB->m_orientation, bias );

	// Return the point, in the minkowski sum, furthest in the direction
	point.xyz = point.ptA - point.ptB;
	return point;
}

/*
====================================================
Simplex

Projects the origin onto the simplex to acquire the new search direction
====================================================
*/
bool SimplexSignedVolumes( point_t * pts, const int num, Vec3 & newDir, Vec4 & lambdasOut ) {
	const float epsilonf = 0.0001f * 0.0001f;
	lambdasOut.Zero();

	bool doesIntersect = false;
	switch ( num ) {
		default:
		case 2: {
			Vec2 lambdas = SignedVolume1D( pts[ 0 ].xyz, pts[ 1 ].xyz );
			Vec3 v( 0.0f );
			for ( int i = 0; i < 2; i++ ) {
				v += pts[ i ].xyz * lambdas[ i ];
			}
			newDir = v * -1.0f;
			doesIntersect = ( v.GetLengthSqr() < epsilonf );
			lambdasOut[ 0 ] = lambdas[ 0 ];
			lambdasOut[ 1 ] = lambdas[ 1 ];
		} break;
		case 3: {
			Vec3 lambdas = SignedVolume2D( pts[ 0 ].xyz, pts[ 1 ].xyz, pts[ 2 ].xyz );
			Vec3 v( 0.0f );
			for ( int i = 0; i < 3; i++ ) {
				v += pts[ i ].xyz * lambdas[ i ];
			}
			newDir = v * -1.0f;
			doesIntersect = ( v.GetLengthSqr() < epsilonf );
			lambdasOut[ 0 ] = lambdas[ 0 ];
			lambdasOut[ 1 ] = lambdas[ 1 ];
			lambdasOut[ 2 ] = lambdas[ 2 ];
		} break;
		case 4: {
			Vec4 lambdas = SignedVolume3D( pts[ 0 ].xyz, pts[ 1 ].xyz, pts[ 2 ].xyz, pts[ 3 ].xyz );
			Vec3 v( 0.0f );
			for ( int i = 0; i < 4; i++ ) {
				v += pts[ i ].xyz * lambdas[ i ];
			}
			newDir = v * -1.0f;
			doesIntersect = ( v.GetLengthSqr() < epsilonf );
			lambdasOut[ 0 ] = lambdas[ 0 ];
			lambdasOut[ 1 ] = lambdas[ 1 ];
			lambdasOut[ 2 ] = lambdas[ 2 ];
			lambdasOut[ 3 ] = lambdas[ 3 ];
		} break;
	};

	return doesIntersect;
}

/*
================================
HasPoint

Checks whether the new point already exists in the simplex
================================
*/
bool HasPoint( const point_t simplexPoints[ 4 ], const point_t & newPt ) {
	const float precision = 1e-6f;

	for ( int i = 0; i < 4; i++ ) {
		Vec3 delta = simplexPoints[ i ].xyz - newPt.xyz;
		if ( delta.GetLengthSqr() < precision * precision ) {
			return true;
		}
	}
	return false;
}

/*
================================
SortValids

Sorts the valid support points to the beginning of the array
================================
*/
void SortValids( point_t simplexPoints[ 4 ], Vec4 & lambdas ) {
	bool valids[ 4 ];
	for ( int i = 0; i < 4; i++ ) {
		valids[ i ] = true;
		if ( lambdas[ i ] == 0.0f ) {
			valids[ i ] = false;
		}
	}

	Vec4 validLambdas( 0.0f );
	int validCount = 0;
	point_t validPts[ 4 ];
	memset( validPts, 0, sizeof( point_t ) * 4 );
	for ( int i = 0; i < 4; i++ ) {
		if ( valids[ i ] ) {
			validPts[ validCount ] = simplexPoints[ i ];
			validLambdas[ validCount ] = lambdas[ i ];
			validCount++;
		}
	}

	// Copy the valids back into simplexPoints
	for ( int i = 0; i < 4; i++ ) {
		simplexPoints[ i ] = validPts[ i ];
		lambdas[ i ] = validLambdas[ i ];
	}
}

/*
================================
NumValids
================================
*/
static int NumValids( const Vec4 & lambdas ) {
	int num = 0;
	for ( int i = 0; i < 4; i++ ) {
		if ( 0.0f != lambdas[ i ] ) {
			num++;
		}
	}
	return num;
}

/*
================================
ValidateTetrahedron
================================
*/
bool ValidateTetrahedron( const point_t simplexPoints[ 4 ] ) {
	Vec3 ab = simplexPoints[ 1 ].xyz - simplexPoints[ 0 ].xyz;
	Vec3 ac = simplexPoints[ 2 ].xyz - simplexPoints[ 0 ].xyz;
	Vec3 ad = simplexPoints[ 3 ].xyz - simplexPoints[ 0 ].xyz;

	Vec3 n = ab.Cross( ac );
	float v = fabsf( n.Dot( ad ) );

	float tol = 1e-3f;
	if ( v < tol * tol * tol ) {
		return false;
	}

	return true;
}

/*
================================
GJK_ClosestPoints
Gilbert-Johnson-Keerthi

Johnson algorithm replaced with Signed Volumes
================================
*/
void GJK_ClosestPoints( const Body * bodyA, const Body * bodyB, Vec3 & ptOnA, Vec3 & ptOnB ) {
	const Vec3 origin( 0.0f );

	float closestDist = 1e10f;
	const float bias = 0.0f;

	int numPts = 1;
	point_t simplexPoints[ 4 ];
	simplexPoints[ 0 ] = Support( bodyA, bodyB, Vec3( 1, 1, 1 ), bias );

	Vec4 lambdas = Vec4( 1, 0, 0, 0 );
	Vec3 newDir = simplexPoints[ 0 ].xyz * -1.0f;
	do {
		// Get the new point to check on
		point_t newPt = Support( bodyA, bodyB, newDir, bias );

		// If the new point is the same as a previous point, then we can't expand any further
		if ( HasPoint( simplexPoints, newPt ) ) {
			break;
		}

		// Add point and get new search direction
		simplexPoints[ numPts ] = newPt;
		numPts++;

		SimplexSignedVolumes( simplexPoints, numPts, newDir, lambdas );
		SortValids( simplexPoints, lambdas );
		numPts = NumValids( lambdas );

		// Check that the new projection of the origin onto the simplex is closer than the previous
		float dist = newDir.GetLengthSqr();
		if ( dist >= closestDist ) {
			break;
		}
		closestDist = dist;
	} while ( numPts < 4 );

	ptOnA.Zero();
	ptOnB.Zero();
	for ( int i = 0; i < 4; i++ ) {
		ptOnA += simplexPoints[ i ].ptA * lambdas[ i ];
		ptOnB += simplexPoints[ i ].ptB * lambdas[ i ];
	}
}

/*
====================================================
GJK_DoesIntersect
Gilbert-Johnson-Keerthi

Johnson algorithm replaced with Signed Volumes
====================================================
*/
bool GJK_DoesIntersect( const Body * bodyA, const Body * bodyB, const float bias, Vec3 & ptOnA, Vec3 & ptOnB ) {
	const Vec3 origin( 0.0f );

	int numPts = 1;
	point_t simplexPoints[ 4 ];
	simplexPoints[ 0 ] = Support( bodyA, bodyB, Vec3( 1, 1, 1 ), 0.0f );

	float closestDist = 1e10f;
	bool doesContainOrigin = false;
	Vec3 newDir = simplexPoints[ 0 ].xyz * -1.0f;
	do {
		// Get the new point to check on
		point_t newPt = Support( bodyA, bodyB, newDir, 0.0f );

		// If the new point is the same as a previous point, then we can't expand any further
		if ( HasPoint( simplexPoints, newPt ) ) {
			break;
		}

		simplexPoints[ numPts ] = newPt;
		numPts++;

		// If this new point hasn't moved passed the origin, then the
		// origin cannot be in the set. And therefore there is no collision.
		float dotdot = newDir.Dot( newPt.xyz - origin );
		if ( dotdot < 0.0f ) {
			break;
		}

		Vec4 lambdas;
		doesContainOrigin = SimplexSignedVolumes( simplexPoints, numPts, newDir, lambdas );
		if ( doesContainOrigin ) {
			break;
		}

		// Check that the new projection of the origin onto the simplex is closer than the previous
		float dist = newDir.GetLengthSqr();
		if ( dist >= closestDist ) {
			break;
		}
		closestDist = dist;

		// Use the lambdas that support the new search direction, and invalidate any points that don't support it
		SortValids( simplexPoints, lambdas );
		numPts = NumValids( lambdas );
		doesContainOrigin = ( 4 == numPts );
	} while ( !doesContainOrigin );

	if ( !doesContainOrigin ) {
		return false;
	}

	//
	//	Check that we have a 3-simplex (EPA expects a tetrahedron)
	//
	if ( 1 == numPts ) {
		Vec3 searchDir = simplexPoints[ 0 ].xyz * -1.0f;
		point_t newPt = Support( bodyA, bodyB, searchDir, 0.0f );
		simplexPoints[ numPts ] = newPt;
		numPts++;
	}
	if ( 2 == numPts ) {
		Vec3 ab = simplexPoints[ 1 ].xyz - simplexPoints[ 0 ].xyz;
		Vec3 u, v;
		ab.GetOrtho( u, v );

		Vec3 newDir = u;
		point_t newPt = Support( bodyA, bodyB, newDir, 0.0f );
		simplexPoints[ numPts ] = newPt;
		numPts++;
	}
	if ( 3 == numPts ) {
		Vec3 ab = simplexPoints[ 1 ].xyz - simplexPoints[ 0 ].xyz;
		Vec3 ac = simplexPoints[ 2 ].xyz - simplexPoints[ 0 ].xyz;
		Vec3 norm = ab.Cross( ac );

		Vec3 newDir = norm;
		point_t newPt = Support( bodyA, bodyB, newDir, 0.0f );
		simplexPoints[ numPts ] = newPt;
		numPts++;
	}

	//
	// Expand the simplex by the bias amount
	//

	// Get the center point of the simplex
	Vec3 avg = Vec3( 0, 0, 0 );
	for ( int i = 0; i < 4; i++ ) {
		avg += simplexPoints[ i ].xyz;
	}
	avg *= 0.25f;

	// Now expand the simplex by the bias amount
	for ( int i = 0; i < numPts; i++ ) {
		point_t & pt = simplexPoints[ i ];

		Vec3 dir = pt.xyz - avg;	// ray from "center" to witness point
		dir.Normalize();
		pt.ptA += dir * bias;
		pt.ptB -= dir * bias;
		pt.xyz = pt.ptA - pt.ptB;
	}

	// Check that this tetrahedron has a volume, otherwise return false
	if ( !ValidateTetrahedron( simplexPoints ) ) {
		return false;
	}

	//
	// Perform EPA expansion of the simplex to find the closest face on the CSO
	//
	EPA_Expand( bodyA, bodyB, bias, simplexPoints, ptOnA, ptOnB );
	return true;
}
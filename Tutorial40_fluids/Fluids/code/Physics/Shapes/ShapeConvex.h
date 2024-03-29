//
//	ShapeConvex.h
//
#pragma once
#include "Physics/Shapes/ShapeBase.h"

struct tri_t {
	int a;
	int b;
	int c;

	bool operator == ( const tri_t & rhs ) const {
		if ( a != rhs.a && a != rhs.b && a != rhs.c ) {
			return false;
		}
		if ( b != rhs.a && b != rhs.b && b != rhs.c ) {
			return false;
		}
		if ( c != rhs.a && c != rhs.b && c != rhs.c ) {
			return false;
		}

		return true;
	}

	bool HasEdge( int A, int B ) const {
		if ( ( a == A && b == B ) || ( a == B && b == A ) ) {
			return true;
		}
		if ( ( b == A && c == B ) || ( b == B && c == A ) ) {
			return true;
		}
		if ( ( c == A && a == B ) || ( c == B && a == A ) ) {
			return true;
		}
		return false;			
	}

	int NumSharedEdges( const tri_t & rhs ) const {
		int num = 0;
		if ( rhs.HasEdge( a, b ) ) {
			num++;
		}
		if ( rhs.HasEdge( b, c ) ) {
			num++;
		}
		if ( rhs.HasEdge( c, a ) ) {
			num++;
		}
		return num;
	}

	bool SharesEdge( const tri_t & rhs ) const {
		return ( NumSharedEdges( rhs ) > 0 );
	}
};

struct edge_t {
	int a;
	int b;

	bool operator == ( const edge_t & rhs ) const {
		return ( ( a == rhs.a && b == rhs.b ) || ( a == rhs.b && b == rhs.a ) );
	}
};

void ExpandConvexHull( std::vector< Vec3 > & hullPoints, std::vector< tri_t > & hullTris, const std::vector< Vec3 > & verts );
void BuildConvexHull( const std::vector< Vec3 > & verts, std::vector< Vec3 > & hullPts, std::vector< tri_t > & hullTris );

/*
====================================================
ShapeConvex
====================================================
*/
class ShapeConvex : public Shape {
public:
	explicit ShapeConvex( const Vec3 * pts, const int num ) {
		Build( pts, num );
	}
	void Build( const Vec3 * pts, const int num );

	Vec3 Support( const Vec3 & dir, const Vec3 & pos, const Quat & orient, const float bias ) const override;

	Mat3 InertiaTensor() const override { return m_inertiaTensor; }

	Bounds GetBounds( const Vec3 & pos, const Quat & orient ) const override;
	Bounds GetBounds() const override { return m_bounds; }

	float FastestLinearSpeed( const Vec3 & angularVelocity, const Vec3 & dir ) const override;

	shapeType_t GetType() const override { return SHAPE_CONVEX; }

public:
	std::vector< Vec3 > m_points;
	Bounds m_bounds;
	Mat3 m_inertiaTensor;
};
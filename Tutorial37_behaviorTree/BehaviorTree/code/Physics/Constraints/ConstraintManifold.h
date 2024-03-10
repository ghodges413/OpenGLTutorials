//
//	ConstraintManifold.h
//
#pragma once
#include "Physics/Constraints/ConstraintBase.h"

/*
====================================================
ConstraintManifold
====================================================
*/
class ConstraintManifold : public Constraint {
public:
	ConstraintManifold() : Constraint(), m_cachedLambda( 3 * 4 ), m_Jacobian( 3 * 4, 12 ) {
		m_cachedLambda.Zero();
		m_baumgarte[ 0 ] = 0.0f;
		m_baumgarte[ 1 ] = 0.0f;
		m_baumgarte[ 2 ] = 0.0f;
		m_baumgarte[ 3 ] = 0.0f;
		m_friction = 0.0f;
	}

	void PreSolve( const float dt_sec ) override;
	void Solve() override;

	static const int MAX_CONTACTS = 4;

	VecN m_cachedLambda;
	Vec3 m_normal;	// In bodyA's local space
	Vec3 m_ptsOnA_LocalSpace[ MAX_CONTACTS ];
	Vec3 m_ptsOnB_LocalSpace[ MAX_CONTACTS ];
	Vec3 m_normals[ MAX_CONTACTS ];
	int m_numPoints;

	MatMN m_Jacobian;

	float m_baumgarte[ MAX_CONTACTS ];
	float m_friction;
};
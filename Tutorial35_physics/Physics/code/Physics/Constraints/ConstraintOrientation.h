//
//	ConstraintOrientation.h
//
#pragma once
#include "Physics/Constraints/ConstraintBase.h"

/*
====================================================
ConstraintOrientation
====================================================
*/
class ConstraintOrientation : public Constraint {
public:
	ConstraintOrientation() : Constraint(), m_Jacobian( 4, 12 ) {
		m_baumgarte = 0.0f;
	}

	void PreSolve( const float dt_sec ) override;
	void Solve() override;

	Quat m_q0;			// The initial relative quaternion q1^-1 * q2

	VecN m_cachedLambda;
	MatMN m_Jacobian;

	float m_baumgarte;
};

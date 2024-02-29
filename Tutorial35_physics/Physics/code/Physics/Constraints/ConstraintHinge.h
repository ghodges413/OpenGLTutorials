//
//	ConstraintHinge.h
//
#pragma once
#include "Physics/Constraints/ConstraintBase.h"

/*
====================================================
ConstraintHinge
(Also known as Revolute Constraint)
====================================================
*/
class ConstraintHingeVector : public Constraint {
public:
	void Solve() override;
};

/*
====================================================
ConstraintHingeAxis
(Also known as Revolute Constraint)
====================================================
*/
class ConstraintHingeAxis : public Constraint {
public:
	void Solve() override;
};

/*
====================================================
ConstraintHingeQuat
(Also known as Revolute Constraint)
====================================================
*/
class ConstraintHingeQuat : public Constraint {
public:
	ConstraintHingeQuat() : Constraint(), m_cachedLambda( 3 ), m_Jacobian( 3, 12 ) {
		m_cachedLambda.Zero();
		m_baumgarte = 0.0f;
	}
	void PreSolve( const float dt_sec ) override;
	void Solve() override;
	void PostSolve() override;

	Quat q0;	// The initial relative quaternion q1^-1 * q2

	VecN m_cachedLambda;
	MatMN m_Jacobian;

	float m_baumgarte;
};

/*
====================================================
ConstraintHingeQuatLimited
(Also known as Revolute Constraint)
====================================================
*/
class ConstraintHingeQuatLimited : public Constraint {
public:
	ConstraintHingeQuatLimited() : Constraint(), m_cachedLambda( 4 ), m_Jacobian( 4, 12 ) {
		m_cachedLambda.Zero();
		m_baumgarte = 0.0f;
		m_isAngleViolated = false;
		m_relativeAngle = 0.0f;
	}
	void PreSolve( const float dt_sec ) override;
	void Solve() override;
	void PostSolve() override;

	Quat m_q0;	// The initial relative quaternion q1^-1 * q2

	VecN m_cachedLambda;
	MatMN m_Jacobian;

	float m_baumgarte;

	bool m_isAngleViolated;
	float m_relativeAngle;
};

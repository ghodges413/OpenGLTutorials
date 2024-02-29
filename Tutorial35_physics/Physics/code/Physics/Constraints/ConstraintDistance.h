//
//	ConstraintDistance.h
//
#pragma once
#include "Physics/Constraints/ConstraintBase.h"

/*
====================================================
ConstraintDistance
====================================================
*/
class ConstraintDistance : public Constraint {
public:
	void Solve() override;
};

/*
====================================================
ConstraintDistance2
====================================================
*/
class ConstraintDistance2 : public Constraint {
public:
	void Solve() override;
};

/*
====================================================
ConstraintDistanceRigidBody
====================================================
*/
class ConstraintDistanceRigidBody : public Constraint {
public:
	void Solve() override;
};

/*
====================================================
ConstraintDistanceRigidBody2
====================================================
*/
class ConstraintDistanceRigidBody2 : public Constraint {
public:
	ConstraintDistanceRigidBody2() : Constraint(), m_cachedLambda( 1 ), m_cachedImpulses( 12 ), m_Jacobian( 1, 12 ) {
		m_cachedLambda.Zero();
		m_cachedImpulses.Zero();
		m_baumgarte = 0.0f;
	}

	void PreSolve( const float dt_sec ) override;
	void Solve() override;
	void PostSolve() override;

	VecN m_cachedLambda;
	VecN m_cachedImpulses;	// We've left this in here to re-enforce how bad it is to do warm starting with direct impulses
	MatMN m_Jacobian;

	float m_baumgarte;
};

//
//	ConstraintBase.h
//
#pragma once
#include "Math/Vector.h"
#include "Math/Quat.h"
#include "Math/Matrix.h"
#include "Math/Bounds.h"
#include <vector>

class Body;

/*
====================================================
Constraint
====================================================
*/
class Constraint {
public:
	virtual void PreSolve( const float dt_sec ) {}
	virtual void Solve() {}
	virtual void PostSolve() {}

protected:
	MatMN GetInverseMassMatrix() const;
	VecN GetVelocities() const;
	void ApplyImpulses( const VecN & impulses );

protected:
	static Mat4 Left( const Quat & q );
	static Mat4 Right( const Quat & q );

public:
	Body * m_bodyA;
	Body * m_bodyB;

	Vec3 m_anchorA;		// The anchor location in bodyA's space
	Vec3 m_axisA;		// The axis direction in bodyA's space

	Vec3 m_anchorB;		// The anchor location in bodyB's space
	Vec3 m_axisB;		// The axis direction in bodyB's space
};

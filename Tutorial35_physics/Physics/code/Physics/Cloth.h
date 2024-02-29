//
//	Cloth.h
//
#pragma once
#include "Math/Vector.h"
#include "Math/Quat.h"
#include "Math/Matrix.h"
#include "Math/Bounds.h"
#include "Physics/Shapes.h"
#include "Physics/Contact.h"

class ClothConstraint2 {
public:
	ClothConstraint2() : m_cachedLambda( 1 ), m_cachedImpulses( 6 ), m_Jacobian( 1, 6 ) {
		m_cachedLambda.Zero();
		m_cachedImpulses.Zero();
		m_baumgarte = 0.0f;

		m_idxA = -1;
		m_idxB = -1;

		m_invMassA = 1.0f;
		m_invMassB = 1.0f;
	}

	int m_idxA;
	int m_idxB;
	float m_restLength;

	Vec3 m_posA;
	Vec3 m_velA;
	Vec3 m_posB;
	Vec3 m_velB;

	float m_invMassA;
	float m_invMassB;

	void PreSolve( const float dt_sec );
	void Solve();

	MatMN GetInverseMassMatrix() const;
	VecN GetVelocities() const;
	void ApplyImpulses( const VecN & impulses );

	VecN m_cachedLambda;
	VecN m_cachedImpulses;	// We've left this in here to re-enforce how bad it is to do warm starting with direct impulses
	MatMN m_Jacobian;

	float m_baumgarte;
};

struct ClothConstraint {
	int m_idxA;
	int m_idxB;
	float m_restLength;
};

struct ClothParticle {
	Vec3 m_position;
	Vec3 m_positionOld;
	Vec3 m_velocity;
	Vec3 m_acceleration;	// accumulation of forces
	float m_invMass;
};

/*
====================================================
Cloth
====================================================
*/
class Cloth {
public:
	Cloth();

	void Reset();
	Bounds GetBounds() const { return m_bounds; }
	void Update( float dt_sec );

	void Collide( const ShapeSphere * shape, const Vec3 & spherePos );
	
#if 0
	static const int m_width = 64;
	static const int m_height = 64;
	static const int m_numIters = 4;
#elif 1
	static const int m_width = 16;
	static const int m_height = 16;
	static const int m_numIters = 4;
#elif 0
	static const int m_width = 32;
	static const int m_height = 32;
	static const int m_numIters = 16;
#elif 0
	static const int m_width = 128;
	static const int m_height = 128;
	static const int m_numIters = 16;
#endif
	static const float m_restDistance;

	static const float m_widthPhysical;
	static const float m_heightPhysical;

	ClothParticle m_particles[ m_width * m_height ];
	ClothConstraint m_constraints[ m_width * m_height * 4 ];
	ClothConstraint2 m_constraints2[ m_width * m_height * 4 * 2 ];
	int m_numConstraints;

	Bounds m_bounds;

	static const int m_numClamps = m_width;// / 4;
	Vec3 m_clamped[ m_numClamps ];

	// CPU coloring attempt.  While not necessary for cpu cloth, a step towards gpu cloth.
	static const int m_numColors = 8;
	int m_constraintColors[ m_numColors ][ m_width * m_height ];
	ClothConstraint m_coloredConstraints[ m_numColors ][ m_width * m_height * 4 ];
	int m_colorCounts[ m_numColors ];

private:
	void SolveConstraints( float dt_sec );
	void SolveConstraintsClassic( float dt_sec );
	void SolveConstraintsColored( float dt_sec );
	void SolveConstraintsLagrange( float dt_sec );
};

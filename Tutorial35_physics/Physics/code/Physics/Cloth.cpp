//
//  Cloth.cpp
//
#include "Physics/Cloth.h"
#include "Math/lcp.h"

const float Cloth::m_widthPhysical = 5.0f;
const float Cloth::m_heightPhysical = 5.0f;

const float Cloth::m_restDistance = Cloth::m_widthPhysical / (float)Cloth::m_width;

//
//	Useful Papers:
//	Advanced Character Physics - Jakobsen 2001
//	Cloth Simulation on GPU - Cyril Zeller 2005
//

/*
====================================================
Cloth::Cloth
====================================================
*/
Cloth::Cloth() {
	Reset();
}

/*
====================================================
Cloth::Reset
====================================================
*/
void Cloth::Reset() {
	// Reset cloth positions
	for ( int y = 0; y < m_height; y++ ) {
		for ( int x = 0; x < m_width; x++ ) {
			const int idx = x + y * m_width;
			m_particles[ idx ].m_position = Vec3( x, 0, -y );
			m_particles[ idx ].m_position = Vec3( x, y, 0 );
			m_particles[ idx ].m_position *= m_restDistance;
			m_particles[ idx ].m_position.z += 7.0f;
			m_particles[ idx ].m_position.x += -15.0f;
			m_particles[ idx ].m_positionOld = m_particles[ idx ].m_position;
			m_particles[ idx ].m_velocity.Zero();
			m_particles[ idx ].m_acceleration.Zero();
			m_particles[ idx ].m_invMass = 1.0f / float( m_width * m_height );
		}
	}

	for ( int i = 0; i < m_numClamps; i++ ) {
		int idx = ( m_width - 1 ) * i / ( m_numClamps - 1 );
		m_clamped[ i ] = m_particles[ idx ].m_position;
		m_particles[ idx ].m_invMass = 0;
	}

	// Reset constraints
	int numConstraints = 0;
	for ( int y = 0; y < m_height; y++ ) {
		for ( int x = 0; x < m_width; x++ ) {
			int idx = x + y * m_width;

			int x0 = x - 1;
			int y0 = y - 1;
			int x1 = x + 1;
			int y1 = y + 1;

			int idx00 = x0 + y0 * m_width;	// upper left
			int idx01 = x0 + y1 * m_width;	// lower left
			int idx10 = x1 + y0 * m_width;	// upper right
			int idx11 = x1 + y1 * m_width;	// lower right

			int idxY0 = x + y0 * m_width;	// above
			int idxY1 = x + y1 * m_width;	// below
			int idxX0 = x0 + y * m_width;	// left
			int idxX1 = x1 + y * m_width;	// right

			if ( x0 >= 0 ) {
				ClothConstraint constraint;
				constraint.m_idxA = idx;
				constraint.m_idxB = idxX0;
				constraint.m_restLength = m_restDistance;
				m_constraints[ numConstraints ] = constraint;

				m_constraints2[ numConstraints * 2 + 0 ].m_idxA = idx;
				m_constraints2[ numConstraints * 2 + 0 ].m_idxB = idxX0;
				m_constraints2[ numConstraints * 2 + 0 ].m_restLength = m_restDistance;
				m_constraints2[ numConstraints * 2 + 1 ].m_idxB = idx;
				m_constraints2[ numConstraints * 2 + 1 ].m_idxA = idxX0;
				m_constraints2[ numConstraints * 2 + 1 ].m_restLength = m_restDistance;
				numConstraints++;
			}
			if ( y0 >= 0 ) {
				ClothConstraint constraint;
				constraint.m_idxA = idx;
				constraint.m_idxB = idxY0;
				constraint.m_restLength = m_restDistance;
				m_constraints[ numConstraints ] = constraint;

				m_constraints2[ numConstraints * 2 + 0 ].m_idxA = idx;
				m_constraints2[ numConstraints * 2 + 0 ].m_idxB = idxY0;
				m_constraints2[ numConstraints * 2 + 0 ].m_restLength = m_restDistance;
				m_constraints2[ numConstraints * 2 + 1 ].m_idxB = idx;
				m_constraints2[ numConstraints * 2 + 1 ].m_idxA = idxY0;
				m_constraints2[ numConstraints * 2 + 1 ].m_restLength = m_restDistance;
				numConstraints++;
			}

			if ( y1 < m_height ) {
				float restLength = sqrtf( 2.0f ) * m_restDistance;

				if ( x0 >= 0 ) {
					ClothConstraint constraint;
					constraint.m_idxA = idx;
					constraint.m_idxB = idx01;
					constraint.m_restLength = restLength;
					m_constraints[ numConstraints ] = constraint;

					m_constraints2[ numConstraints * 2 + 0 ].m_idxA = idx;
					m_constraints2[ numConstraints * 2 + 0 ].m_idxB = idx01;
					m_constraints2[ numConstraints * 2 + 0 ].m_restLength = restLength;
					m_constraints2[ numConstraints * 2 + 1 ].m_idxB = idx;
					m_constraints2[ numConstraints * 2 + 1 ].m_idxA = idx01;
					m_constraints2[ numConstraints * 2 + 1 ].m_restLength = restLength;
					numConstraints++;
				}
				if ( x1 < m_width ) {
					ClothConstraint constraint;
					constraint.m_idxA = idx;
					constraint.m_idxB = idx11;
					constraint.m_restLength = restLength;
					m_constraints[ numConstraints ] = constraint;

					m_constraints2[ numConstraints * 2 + 0 ].m_idxA = idx;
					m_constraints2[ numConstraints * 2 + 0 ].m_idxB = idx11;
					m_constraints2[ numConstraints * 2 + 0 ].m_restLength = restLength;
					m_constraints2[ numConstraints * 2 + 1 ].m_idxB = idx;
					m_constraints2[ numConstraints * 2 + 1 ].m_idxA = idx11;
					m_constraints2[ numConstraints * 2 + 1 ].m_restLength = restLength;
					numConstraints++;
				}
			}
		}
	}
	m_numConstraints = numConstraints;

	//
	//	Update Bounds
	//
	m_bounds.Clear();
	for ( int i = 0; i < m_width * m_height; i++ ) {
		m_bounds.Expand( m_particles[ i ].m_position );
	}

	//
	//	Build Constraint Colors
	//
	memset( m_colorCounts, 0, sizeof( int ) * m_numColors );
	for ( int y = 0; y < m_height; y++ ) {
		// Horizontal 0
		for ( int x = 0; x < m_width - 1; x += 2 ) {
			int baseIdx = y * m_width + x;
			int nextIdx = baseIdx + 1;

			ClothConstraint constraint;
			constraint.m_idxA = baseIdx;
			constraint.m_idxB = nextIdx;
			constraint.m_restLength = m_restDistance;

			m_coloredConstraints[ 0 ][ m_colorCounts[ 0 ] ] = constraint;
			m_colorCounts[ 0 ]++;
		}

		// Horizontal 1
		for ( int x = 1; x < m_width - 1; x += 2 ) {
			int baseIdx = y * m_width + x;
			int nextIdx = baseIdx + 1;

			ClothConstraint constraint;
			constraint.m_idxA = baseIdx;
			constraint.m_idxB = nextIdx;
			constraint.m_restLength = m_restDistance;

			m_coloredConstraints[ 1 ][ m_colorCounts[ 1 ] ] = constraint;
			m_colorCounts[ 1 ]++;
		}

		// Vertical 0
		for ( int x = 0; x < m_width; x++ ) {
			int baseIdx = ( 2 * y + 0 ) * m_width + x;
			int nextIdx = baseIdx + m_width;
			if ( nextIdx >= m_width * m_height ) {
				continue;
			}

			ClothConstraint constraint;
			constraint.m_idxA = baseIdx;
			constraint.m_idxB = nextIdx;
			constraint.m_restLength = m_restDistance;

			m_coloredConstraints[ 2 ][ m_colorCounts[ 2 ] ] = constraint;
			m_colorCounts[ 2 ]++;
		}

		// Vertical 1
		for ( int x = 0; x < m_width; x++ ) {
			int baseIdx = ( 2 * y + 1 ) * m_width + x;
			int nextIdx = baseIdx + m_width;
			if ( nextIdx >= m_width * m_height ) {
				continue;
			}

			ClothConstraint constraint;
			constraint.m_idxA = baseIdx;
			constraint.m_idxB = nextIdx;
			constraint.m_restLength = m_restDistance;

			m_coloredConstraints[ 3 ][ m_colorCounts[ 3 ] ] = constraint;
			m_colorCounts[ 3 ]++;
		}

		// BackSlash 0
		for ( int x = 0; x < m_width - 1; x += 2 ) {
			int baseIdx = y * m_width + x;
			int nextIdx = baseIdx + m_width + 1;
			if ( nextIdx >= m_width * m_height ) {
				continue;
			}

			ClothConstraint constraint;
			constraint.m_idxA = baseIdx;
			constraint.m_idxB = nextIdx;
			constraint.m_restLength = m_restDistance;

			m_coloredConstraints[ 4 ][ m_colorCounts[ 4 ] ] = constraint;
			m_colorCounts[ 4 ]++;
		}

		// BackSlash 1
		for ( int x = 1; x < m_width - 1; x += 2 ) {
			int baseIdx = y * m_width + x;
			int nextIdx = baseIdx + m_width + 1;
			if ( nextIdx >= m_width * m_height ) {
				continue;
			}

			ClothConstraint constraint;
			constraint.m_idxA = baseIdx;
			constraint.m_idxB = nextIdx;
			constraint.m_restLength = m_restDistance;

			m_coloredConstraints[ 5 ][ m_colorCounts[ 5 ] ] = constraint;
			m_colorCounts[ 5 ]++;
		}

		// ForwardSlash 0
		for ( int x = 1; x < m_width; x += 2 ) {
			int baseIdx = y * m_width + x;
			int nextIdx = baseIdx + m_width - 1;
			if ( nextIdx >= m_width * m_height ) {
				continue;
			}

			ClothConstraint constraint;
			constraint.m_idxA = baseIdx;
			constraint.m_idxB = nextIdx;
			constraint.m_restLength = m_restDistance;

			m_coloredConstraints[ 6 ][ m_colorCounts[ 6 ] ] = constraint;
			m_colorCounts[ 6 ]++;
		}

		// ForwardSlash 0
		for ( int x = 2; x < m_width; x += 2 ) {
			int baseIdx = y * m_width + x;
			int nextIdx = baseIdx + m_width - 1;
			if ( nextIdx >= m_width * m_height ) {
				continue;
			}

			ClothConstraint constraint;
			constraint.m_idxA = baseIdx;
			constraint.m_idxB = nextIdx;
			constraint.m_restLength = m_restDistance;

			m_coloredConstraints[ 7 ][ m_colorCounts[ 7 ] ] = constraint;
			m_colorCounts[ 7 ]++;
		}
	}
}

/*
====================================================
Cloth::SolveConstraintsClassic
====================================================
*/
void Cloth::SolveConstraintsClassic( float dt_sec ) {
	//
	//	Satisfy constraints via projection
	//
	for ( int iter = 0; iter < m_numIters; iter++ ) {
		for ( int i = 0; i < m_numConstraints; i++ ) {
			ClothConstraint & c = m_constraints[ i ];

			Vec3 & x1 = m_particles[ c.m_idxA ].m_position;
			Vec3 & x2 = m_particles[ c.m_idxB ].m_position;

			Vec3 delta = x2 - x1;

			float deltaLength = delta.GetMagnitude();
			float diff = ( deltaLength - c.m_restLength ) / deltaLength;

			if ( diff * 0.0f == diff * 0.0f ) {
				const float invMassA = m_particles[ c.m_idxA ].m_invMass;
				const float invMassB = m_particles[ c.m_idxB ].m_invMass;

				const float tA = invMassA / ( invMassA + invMassB );
				const float tB = invMassB / ( invMassA + invMassB );

				x1 += delta * diff * tA;
				x2 -= delta * diff * tB;
			}
		}

		// Clamped cloth particles
		for ( int i = 0; i < m_numClamps; i++ ) {
			int idx = ( m_width - 1 ) * i / ( m_numClamps - 1 );
			m_particles[ idx ].m_position = m_clamped[ i ];
		}
	}
}

/*
====================================================
Cloth::SolveConstraintsColored
====================================================
*/
void Cloth::SolveConstraintsColored( float dt_sec ) {
	//
	//	Satisfy constraints via projection
	//
	for ( int iter = 0; iter < m_numIters; iter++ ) {
		for ( int colorIdx = 0; colorIdx < m_numColors; colorIdx++ ) {
			int colorCount = m_colorCounts[ colorIdx ];

			for ( int i = 0; i < colorCount; i++ ) {
				ClothConstraint & c = m_coloredConstraints[ colorIdx ][ i ];

				Vec3 & x1 = m_particles[ c.m_idxA ].m_position;
				Vec3 & x2 = m_particles[ c.m_idxB ].m_position;

				Vec3 delta = x2 - x1;

				float deltaLength = delta.GetMagnitude();
				float diff = ( deltaLength - c.m_restLength ) / deltaLength;

				if ( diff * 0.0f == diff * 0.0f ) {
					const float invMassA = m_particles[ c.m_idxA ].m_invMass;
					const float invMassB = m_particles[ c.m_idxB ].m_invMass;

					const float tA = invMassA / ( invMassA + invMassB );
					const float tB = invMassB / ( invMassA + invMassB );

					x1 += delta * diff * tA;
					x2 -= delta * diff * tB;
				}
			}
		}

		// Clamped cloth particles
		for ( int i = 0; i < m_numClamps; i++ ) {
			int idx = ( m_width - 1 ) * i / ( m_numClamps - 1 );
			m_particles[ idx ].m_position = m_clamped[ i ];
		}
	}
}

/*
====================================================
Cloth::SolveConstraintsLagrange
====================================================
*/
void Cloth::SolveConstraintsLagrange( float dt_sec ) {
	//
	//	Copy particle positions and velocities
	//
	for ( int i = 0; i < m_numConstraints; i++ ) {
		ClothConstraint2 & constraint = m_constraints2[ i ];
		const ClothParticle & a = m_particles[ constraint.m_idxA ];
		const ClothParticle & b = m_particles[ constraint.m_idxB ];

		constraint.m_posA = a.m_position;
		constraint.m_posB = b.m_position;

		constraint.m_velA = a.m_velocity;//( a.m_position - a.m_positionOld ) / dt_sec;
		constraint.m_velB = b.m_velocity;//( b.m_position - b.m_positionOld ) / dt_sec;

		constraint.m_invMassA = a.m_invMass;
		constraint.m_invMassB = b.m_invMass;
	}

	//
	//	Solve Constraints
	//
	for ( int i = 0; i < m_numConstraints; i++ ) {
		m_constraints2[ i ].PreSolve( dt_sec );
	}

	const int maxIters = 1;//5;
	for ( int iters = 0; iters < maxIters; iters++ ) {
		for ( int i = 0; i < m_numConstraints; i++ ) {
			m_constraints2[ i ].Solve();
		}
	}

	//
	//	Copy positions/velocities back
	//
	for ( int i = 0; i < m_numConstraints; i++ ) {
		const ClothConstraint2 & constraint = m_constraints2[ i ];
		ClothParticle & a = m_particles[ constraint.m_idxA ];

		a.m_position = constraint.m_posA;
		a.m_velocity = constraint.m_velA;
		//a.m_positionOld = constraint.m_posA - constraint.m_velA * dt_sec;
	}

	// Clamped cloth particles
	for ( int i = 0; i < m_numClamps; i++ ) {
		int idx = ( m_width - 1 ) * i / ( m_numClamps - 1 );
		m_particles[ idx ].m_position = m_clamped[ i ];
	}
}

/*
====================================================
Cloth::SolveConstraints
====================================================
*/
void Cloth::SolveConstraints( float dt_sec ) {
	SolveConstraintsClassic( dt_sec );
//	SolveConstraintsColored( dt_sec );
//	SolveConstraintsLagrange( dt_sec );
}

/*
====================================================
Cloth::Update
====================================================
*/
void Cloth::Update( float dt_sec ) {
#if 1
	//
	//	Accumulate Forces
	//
	for ( int i = 0; i < m_width * m_height; i++ ) {
		m_particles[ i ].m_acceleration = Vec3( 0, 0, -10.01f );
	}

	//
	//	Velocity Verlet integration:
	//
	// x1 = x0 + v0 * dt + 1/2 * a0 * dt^2
	// v = ( x0 - x_1 ) / dt
	// => x1 = x0 + dt * ( x0 - x_1 ) / dt + 1/2 * a0 * dt^2
	// => x1 = x0 + ( x0 - x_1 ) + 1/2 * a0 * dt^2
	// => x1 = 2 * x0 - x_1 + 1/2 * a0 * dt^2
	//
	// If there's a damping factor k, then:
	// => x1 = x0 + ( x0 - x_1 ) * k + 1/2 * a0 * dt^2

	for ( int i = 0; i < m_width * m_height; i++ ) {
		ClothParticle & particle = m_particles[ i ];

		if ( particle.m_invMass > 0.0f ) {
			Vec3 poo	= particle.m_position;
			Vec3 x0		= particle.m_position;
			Vec3 x_1	= particle.m_positionOld;
			Vec3 a0		= particle.m_acceleration;
			Vec3 x1		= x0 * 2.0f - x_1 + a0 * dt_sec * dt_sec * 0.5f;
		 
			const float k = 1.0f;//0.99f;
			x1 = x0 + ( x0 - x_1 ) * k + a0 * dt_sec * dt_sec * 0.5f;
		
			particle.m_position = x1;
			particle.m_positionOld = x0;
		}
	}

// 	for ( int i = 0; i < m_width * m_height; i++ ) {
// 		ClothParticle & particle = m_particles[ i ];
// 
// 		if ( particle.m_invMass > 0.0f ) {
// 			particle.m_velocity += Vec3( 0, 0, -10 ) * dt_sec;
// 		}
// 	}

	SolveConstraints( dt_sec );

// 	for ( int i = 0; i < m_width * m_height; i++ ) {
// 		ClothParticle & particle = m_particles[ i ];
// 
// 		if ( particle.m_invMass > 0.0f ) {
// 			particle.m_position += particle.m_velocity * dt_sec;
// 		}
// 	}
#endif
	//
	//	Update Bounds
	//
	m_bounds.Clear();
	for ( int i = 0; i < m_width * m_height; i++ ) {
		m_bounds.Expand( m_particles[ i ].m_position );
	}
}

/*
====================================================
Cloth::Collide
====================================================
*/
void Cloth::Collide( const ShapeSphere * shape, const Vec3 & spherePos ) {
	// Use bounds to early out
	Bounds boundsShape = shape->GetBounds();
	boundsShape.mins += spherePos;
	boundsShape.maxs += spherePos;
	if ( !m_bounds.DoesIntersect( boundsShape ) ) {
		return;
	}

	const float radius = shape->m_radius;
	for ( int i = 0; i < m_width * m_height; i++ ) {
		Vec3 & pos = m_particles[ i ].m_position;

		Vec3 delta = pos - spherePos;	// ray from center of sphere to particle
		float lengthSqr = delta.GetLengthSqr();


		if ( lengthSqr < radius * radius ) {
 			delta.Normalize();

			pos = spherePos + delta * radius;
		}
	}
}










/*
========================================================================================================

ClothConstraint2

========================================================================================================
*/







/*
====================================================
ClothConstraint2::PreSolve
====================================================
*/
void ClothConstraint2::PreSolve( const float dt_sec ) {
	const Vec3 r = m_posB - m_posA;
	Vec3 ur = r;
	ur.Normalize();
	float halfLength = m_restLength * 0.5f;
	const Vec3 a = m_posA + ur * halfLength;	// anchor position for body a
	const Vec3 b = m_posB - ur * halfLength;	// anchor position for body b

	m_Jacobian.Zero();

	Vec3 J1 = ( a - b ) * 2.0f;
	m_Jacobian.rows[ 0 ][ 0 ] = J1.x;
	m_Jacobian.rows[ 0 ][ 1 ] = J1.y;
	m_Jacobian.rows[ 0 ][ 2 ] = J1.z;

	Vec3 J3 = ( b - a ) * 2.0f;
	m_Jacobian.rows[ 0 ][ 3 ] = J3.x;
	m_Jacobian.rows[ 0 ][ 4 ] = J3.y;
	m_Jacobian.rows[ 0 ][ 5 ] = J3.z;

#define WARM_STARTING
#if defined( WARM_STARTING )
	//
	// Apply warm starting from last frame
	//
	const VecN impulses = m_Jacobian.Transpose() * m_cachedLambda;
	ApplyImpulses( impulses );
#endif

//#define BAUMGARTE_STABILIZATION
#if defined( BAUMGARTE_STABILIZATION )
	//
	//	Calculate the baumgarte stabilization
	//
	float C = r.Dot( r );
	C = std::max( 0.0f, C - 0.001f );
	const float Beta = 0.05f;
	m_baumgarte = ( Beta / dt_sec ) * C;
#else
	m_baumgarte = 0.0f;
#endif
}

/*
====================================================
ClothConstraint2::Solve
====================================================
*/
void ClothConstraint2::Solve() {
	const MatMN JacobianTranspose = m_Jacobian.Transpose();

	// Build the system of equations
	const VecN q_dt = GetVelocities();
	const MatMN invMassMatrix = GetInverseMassMatrix();
	const MatMN J_W_Jt = m_Jacobian * invMassMatrix * JacobianTranspose;
	VecN rhs = ( m_Jacobian * q_dt * -1.0f );
	rhs[ 0 ] -= m_baumgarte;
	
	// Solve for the Lagrange multipliers
	const VecN lambdaN = LCP_GaussSeidel( J_W_Jt, rhs );

	// Apply the impulses
	const VecN impulses = JacobianTranspose * lambdaN;
	ApplyImpulses( impulses );

	// Accumulate the impulses for warm starting
	m_cachedLambda += lambdaN;
	m_cachedImpulses += impulses;
}




/*
====================================================
ClothConstraint2::GetInverseMassMatrix
====================================================
*/
MatMN ClothConstraint2::GetInverseMassMatrix() const {
	MatMN invMassMatrix( 6, 6 );
	invMassMatrix.Zero();

	invMassMatrix.rows[ 0 ][ 0 ] = m_invMassA;
	invMassMatrix.rows[ 1 ][ 1 ] = m_invMassA;
	invMassMatrix.rows[ 2 ][ 2 ] = m_invMassA;

	invMassMatrix.rows[ 3 ][ 3 ] = m_invMassB;
	invMassMatrix.rows[ 4 ][ 4 ] = m_invMassB;
	invMassMatrix.rows[ 5 ][ 5 ] = m_invMassB;

	return invMassMatrix;
}

/*
====================================================
ClothConstraint2::GetVelocities
====================================================
*/
VecN ClothConstraint2::GetVelocities() const {
	VecN q_dt( 6 );

	q_dt[ 0 ] = m_velA.x;
	q_dt[ 1 ] = m_velA.y;
	q_dt[ 2 ] = m_velA.z;

	q_dt[ 3 ] = m_velB.x;
	q_dt[ 4 ] = m_velB.y;
	q_dt[ 5 ] = m_velB.z;

	return q_dt;
}

/*
====================================================
ClothConstraint2::ApplyImpulses
====================================================
*/
void ClothConstraint2::ApplyImpulses( const VecN & impulses ) {
	Vec3 forceInternalA( 0.0f );
	Vec3 forceInternalB( 0.0f );

	forceInternalA[ 0 ] = impulses[ 0 ];
	forceInternalA[ 1 ] = impulses[ 1 ];
	forceInternalA[ 2 ] = impulses[ 2 ];

	forceInternalB[ 0 ] = impulses[ 6 ];
	forceInternalB[ 1 ] = impulses[ 7 ];
	forceInternalB[ 2 ] = impulses[ 8 ];

//	m_bodyA->ApplyImpulseLinear( forceInternalA );
//	m_bodyB->ApplyImpulseLinear( forceInternalB );
	m_velA += forceInternalA * m_invMassA;
	m_velB += forceInternalB * m_invMassB;

	if ( !forceInternalA.IsValid() ) {
		printf( "oh boy\n" );
	}
	if ( !forceInternalB.IsValid() ) {
		printf( "oh boy\n" );
	}
}
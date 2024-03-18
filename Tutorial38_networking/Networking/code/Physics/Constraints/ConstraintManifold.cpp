//
//  ConstraintManifold.cpp
//
#include "Physics/Constraints/ConstraintManifold.h"
#include "Physics/Body.h"
#include "Math/lcp.h"

#pragma optimize( "", off )

/*
========================================================================================================

ConstraintManifold

========================================================================================================
*/

/*
====================================================
ConstraintManifold::PreSolve
====================================================
*/
void ConstraintManifold::PreSolve( const float dt_sec ) {
	// Make sure any missing contacts have their cached impulses zero'd out
	for ( int i = m_numPoints; i < MAX_CONTACTS; i++ ) {
		m_cachedLambda[ 3 * i + 0 ] = 0.0f;
		m_cachedLambda[ 3 * i + 1 ] = 0.0f;
		m_cachedLambda[ 3 * i + 2 ] = 0.0f;
	}

	const float frictionA = m_bodyA->m_friction;
	const float frictionB = m_bodyB->m_friction;
	m_friction = frictionA * frictionB;

	Vec3 ptsA[ 4 ];
	Vec3 ptsB[ 4 ];
	Vec3 deltas[ 4 ];
	float dots[ 4 ];
	for ( int i = 0; i < m_numPoints; i++ ) {
		ptsA[ i ] = m_bodyA->BodySpaceToWorldSpace( m_ptsOnA_LocalSpace[ i ] );
		ptsB[ i ] = m_bodyB->BodySpaceToWorldSpace( m_ptsOnB_LocalSpace[ i ] );
		deltas[ i ] = ptsA[ i ] - ptsB[ i ];
		dots[ i ] = m_normal.Dot( deltas[ i ] );
	}

	m_Jacobian.Zero();
	for ( int i = 0; i < m_numPoints; i++ ) {
		const Vec3 a = m_bodyA->BodySpaceToWorldSpace( m_ptsOnA_LocalSpace[ i ] );
		const Vec3 b = m_bodyB->BodySpaceToWorldSpace( m_ptsOnB_LocalSpace[ i ] );

		const Vec3 ra = a - m_bodyA->GetCenterOfMassWorldSpace();
		const Vec3 rb = b - m_bodyB->GetCenterOfMassWorldSpace();

		Vec3 u;
		Vec3 v;
		m_normals[ i ].GetOrtho( u, v );
		m_normal.GetOrtho( u, v );

		// Convert tangent space from model space to world space
		Vec3 normal = m_bodyA->m_orientation.RotatePoint( m_normals[ i ] );
		normal = m_bodyA->m_orientation.RotatePoint( m_normal );
		u = m_bodyA->m_orientation.RotatePoint( u );
		v = m_bodyA->m_orientation.RotatePoint( v );

		const int idx = i * 3;

		// First row is the primary distance constraint that holds the anchor points together
		Vec3 J1 = normal * -1.0f;
		m_Jacobian.rows[ idx ][ 0 ] = J1.x;
		m_Jacobian.rows[ idx ][ 1 ] = J1.y;
		m_Jacobian.rows[ idx ][ 2 ] = J1.z;

		Vec3 J2 = ra.Cross( normal * -1.0f );
		m_Jacobian.rows[ idx ][ 3 ] = J2.x;
		m_Jacobian.rows[ idx ][ 4 ] = J2.y;
		m_Jacobian.rows[ idx ][ 5 ] = J2.z;

		Vec3 J3 = normal * 1.0f;
		m_Jacobian.rows[ idx ][ 6 ] = J3.x;
		m_Jacobian.rows[ idx ][ 7 ] = J3.y;
		m_Jacobian.rows[ idx ][ 8 ] = J3.z;

		Vec3 J4 = rb.Cross( normal * 1.0f );
		m_Jacobian.rows[ idx ][ 9 ] = J4.x;
		m_Jacobian.rows[ idx ][ 10] = J4.y;
		m_Jacobian.rows[ idx ][ 11] = J4.z;

		if ( m_friction > 0.0f ) {
			Vec3 J1 = u * -1.0f;
			m_Jacobian.rows[ idx + 1 ][ 0 ] = J1.x;
			m_Jacobian.rows[ idx + 1 ][ 1 ] = J1.y;
			m_Jacobian.rows[ idx + 1 ][ 2 ] = J1.z;

			Vec3 J2 = ra.Cross( u * -1.0f );
			m_Jacobian.rows[ idx + 1 ][ 3 ] = J2.x;
			m_Jacobian.rows[ idx + 1 ][ 4 ] = J2.y;
			m_Jacobian.rows[ idx + 1 ][ 5 ] = J2.z;

			Vec3 J3 = u * 1.0f;
			m_Jacobian.rows[ idx + 1 ][ 6 ] = J3.x;
			m_Jacobian.rows[ idx + 1 ][ 7 ] = J3.y;
			m_Jacobian.rows[ idx + 1 ][ 8 ] = J3.z;

			Vec3 J4 = rb.Cross( u * 1.0f );
			m_Jacobian.rows[ idx + 1 ][ 9 ] = J4.x;
			m_Jacobian.rows[ idx + 1 ][ 10] = J4.y;
			m_Jacobian.rows[ idx + 1 ][ 11] = J4.z;
		}
		if ( m_friction > 0.0f ) {
			Vec3 J1 = v * -1.0f;
			m_Jacobian.rows[ idx + 2 ][ 0 ] = J1.x;
			m_Jacobian.rows[ idx + 2 ][ 1 ] = J1.y;
			m_Jacobian.rows[ idx + 2 ][ 2 ] = J1.z;

			Vec3 J2 = ra.Cross( v * -1.0f );
			m_Jacobian.rows[ idx + 2 ][ 3 ] = J2.x;
			m_Jacobian.rows[ idx + 2 ][ 4 ] = J2.y;
			m_Jacobian.rows[ idx + 2 ][ 5 ] = J2.z;

			Vec3 J3 = v * 1.0f;
			m_Jacobian.rows[ idx + 2 ][ 6 ] = J3.x;
			m_Jacobian.rows[ idx + 2 ][ 7 ] = J3.y;
			m_Jacobian.rows[ idx + 2 ][ 8 ] = J3.z;

			Vec3 J4 = rb.Cross( v * 1.0f );
			m_Jacobian.rows[ idx + 2 ][ 9 ] = J4.x;
			m_Jacobian.rows[ idx + 2 ][ 10] = J4.y;
			m_Jacobian.rows[ idx + 2 ][ 11] = J4.z;
		}
	}

#define WARM_STARTING
#if defined( WARM_STARTING )
	//
	// Apply warm starting from last frame
	//
	const VecN impulses = m_Jacobian.Transpose() * m_cachedLambda;
	ApplyImpulses( impulses );
#endif

#define BAUMGARTE_STABILIZATION
#if defined( BAUMGARTE_STABILIZATION )
	//
	//	Calculate the baumgarte stabilization
	//
	float invReducedMass = m_bodyA->m_invMass + m_bodyB->m_invMass;
	for ( int i = 0; i < MAX_CONTACTS; i++ ) {
		if ( i < m_numPoints ) {
			const Vec3 a = m_bodyA->BodySpaceToWorldSpace( m_ptsOnA_LocalSpace[ i ] );
			const Vec3 b = m_bodyB->BodySpaceToWorldSpace( m_ptsOnB_LocalSpace[ i ] );
			const Vec3 normal = m_bodyA->m_orientation.RotatePoint( m_normals[ i ] );

			float C = ( b - a ).Dot( normal );
			C = std::min( 0.0f, C + 0.01f );	// Add slop
			float Beta = 0.09f * invReducedMass;
			m_baumgarte[ i ] = ( Beta / dt_sec ) * C;
		} else {
			m_baumgarte[ i ] = 0.0f;
		}
	}
#else
	for ( int i = 0; i < MAX_CONTACTS; i++ ) {
		m_baumgarte[ i ] = 0.0f;
	}
#endif
}

/*
====================================================
ConstraintManifold::Solve
====================================================
*/
void ConstraintManifold::Solve() {
	const MatMN JacobianTranspose = m_Jacobian.Transpose();

	// Build the system of equations
	const VecN q_dt = GetVelocities();
	const MatMN invMassMatrix = GetInverseMassMatrix();
	const MatMN J_W_Jt = m_Jacobian * invMassMatrix * JacobianTranspose;
	VecN rhs = ( m_Jacobian * q_dt * -1.0f );
	for ( int i = 0; i < m_numPoints; i++ ) {
		rhs[ 3 * i + 0 ] -= m_baumgarte[ i ];
	}

	// Solve for the Lagrange multipliers
	VecN lambdaN = LCP_GaussSeidel( J_W_Jt, rhs );

	// Accumulate the impulses and clamp to within the constraint limits
	VecN oldLambda = m_cachedLambda;
	m_cachedLambda += lambdaN;
	for ( int i = 0; i < m_numPoints; i++ ) {
		if ( m_cachedLambda[ 3 * i + 0 ] < 0.0f ) {
			m_cachedLambda[ 3 * i + 0 ] = 0.0f;
		}
		if ( m_friction > 0.0f ) {
			const float limit = fabsf( lambdaN[ 3 * i + 0 ] * m_friction );

			if ( m_cachedLambda[ 3 * i + 1 ] > limit ) {
				m_cachedLambda[ 3 * i + 1 ] = limit;
			}
			if ( m_cachedLambda[ 3 * i + 1 ] < -limit ) {
				m_cachedLambda[ 3 * i + 1 ] = -limit;
			}

			if ( m_cachedLambda[ 3 * i + 2 ] > limit ) {
				m_cachedLambda[ 3 * i + 2 ] = limit;
			}
			if ( m_cachedLambda[ 3 * i + 2 ] < -limit ) {
				m_cachedLambda[ 3 * i + 2 ] = -limit;
			}
		}
	}
	lambdaN = m_cachedLambda - oldLambda;

	// Apply the impulses
	const VecN impulses = JacobianTranspose * lambdaN;
	ApplyImpulses( impulses );
}
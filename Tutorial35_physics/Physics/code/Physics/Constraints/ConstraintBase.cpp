//
//  ConstraintBase.cpp
//
#include "Physics/Constraints/ConstraintBase.h"
#include "Physics/Body.h"
#include "Math/lcp.h"

#pragma optimize( "", off )

/*
========================================================================================================

Constraints

========================================================================================================
*/

/*
====================================================
Constraint::GetInverseMassMatrix
====================================================
*/
MatMN Constraint::GetInverseMassMatrix() const {
	MatMN invMassMatrix( 12, 12 );
	invMassMatrix.Zero();

	invMassMatrix.rows[ 0 ][ 0 ] = m_bodyA->m_invMass;
	invMassMatrix.rows[ 1 ][ 1 ] = m_bodyA->m_invMass;
	invMassMatrix.rows[ 2 ][ 2 ] = m_bodyA->m_invMass;

	Mat3 invInertiaTensorA = m_bodyA->GetInverseInertiaTensorWorldSpace();
	for ( int i = 0; i < 3; i++ ) {
		invMassMatrix.rows[ 3 + i ][ 3 + 0 ] = invInertiaTensorA.rows[ i ][ 0 ];
		invMassMatrix.rows[ 3 + i ][ 3 + 1 ] = invInertiaTensorA.rows[ i ][ 1 ];
		invMassMatrix.rows[ 3 + i ][ 3 + 2 ] = invInertiaTensorA.rows[ i ][ 2 ];
	}

	invMassMatrix.rows[ 6 ][ 6 ] = m_bodyB->m_invMass;
	invMassMatrix.rows[ 7 ][ 7 ] = m_bodyB->m_invMass;
	invMassMatrix.rows[ 8 ][ 8 ] = m_bodyB->m_invMass;

	Mat3 invInertiaTensorB = m_bodyB->GetInverseInertiaTensorWorldSpace();
	for ( int i = 0; i < 3; i++ ) {
		invMassMatrix.rows[ 9 + i ][ 9 + 0 ] = invInertiaTensorB.rows[ i ][ 0 ];
		invMassMatrix.rows[ 9 + i ][ 9 + 1 ] = invInertiaTensorB.rows[ i ][ 1 ];
		invMassMatrix.rows[ 9 + i ][ 9 + 2 ] = invInertiaTensorB.rows[ i ][ 2 ];
	}

#if 0
	printf( "Inverse Mass Matrix:\n" );
	for ( int i = 0; i < invMassMatrix.M; i++ ) {
		for ( int j = 0; j < invMassMatrix.N; j++ ) {
			printf( "%.2f ", invMassMatrix.rows[ i ][ j ] );
		}
		printf( "\n" );
	}
#endif
	return invMassMatrix;
}

/*
====================================================
Constraint::GetVelocities
====================================================
*/
VecN Constraint::GetVelocities() const {
	VecN q_dt( 12 );

	q_dt[ 0 ] = m_bodyA->m_linearVelocity.x;
	q_dt[ 1 ] = m_bodyA->m_linearVelocity.y;
	q_dt[ 2 ] = m_bodyA->m_linearVelocity.z;

	q_dt[ 3 ] = m_bodyA->m_angularVelocity.x;
	q_dt[ 4 ] = m_bodyA->m_angularVelocity.y;
	q_dt[ 5 ] = m_bodyA->m_angularVelocity.z;

	q_dt[ 6 ] = m_bodyB->m_linearVelocity.x;
	q_dt[ 7 ] = m_bodyB->m_linearVelocity.y;
	q_dt[ 8 ] = m_bodyB->m_linearVelocity.z;

	q_dt[ 9 ] = m_bodyB->m_angularVelocity.x;
	q_dt[ 10] = m_bodyB->m_angularVelocity.y;
	q_dt[ 11] = m_bodyB->m_angularVelocity.z;

	return q_dt;
}

/*
====================================================
Constraint::ApplyImpulses
====================================================
*/
void Constraint::ApplyImpulses( const VecN & impulses ) {
	Vec3 forceInternalA( 0.0f );
	Vec3 torqueInternalA( 0.0f );
	Vec3 forceInternalB( 0.0f );
	Vec3 torqueInternalB( 0.0f );

	forceInternalA[ 0 ] = impulses[ 0 ];
	forceInternalA[ 1 ] = impulses[ 1 ];
	forceInternalA[ 2 ] = impulses[ 2 ];

	torqueInternalA[ 0 ] = impulses[ 3 ];
	torqueInternalA[ 1 ] = impulses[ 4 ];
	torqueInternalA[ 2 ] = impulses[ 5 ];

	forceInternalB[ 0 ] = impulses[ 6 ];
	forceInternalB[ 1 ] = impulses[ 7 ];
	forceInternalB[ 2 ] = impulses[ 8 ];

	torqueInternalB[ 0 ] = impulses[ 9 ];
	torqueInternalB[ 1 ] = impulses[ 10];
	torqueInternalB[ 2 ] = impulses[ 11];

	m_bodyA->ApplyImpulseLinear( forceInternalA );
	m_bodyA->ApplyImpulseAngular( torqueInternalA );

	m_bodyB->ApplyImpulseLinear( forceInternalB );
	m_bodyB->ApplyImpulseAngular( torqueInternalB );

	if ( !forceInternalA.IsValid() ) {
		printf( "oh boy\n" );
	}
	if ( !forceInternalB.IsValid() ) {
		printf( "oh boy\n" );
	}
}

#define DO_TRANSPOSE // Use to toggle the transpose matrices (our very simple example hinge is too symmetrical to know if we're correct)

Mat4 Constraint::Left( const Quat & q ) {
	Mat4 L;
	L.rows[ 0 ] = Vec4( q.w, -q.x, -q.y, -q.z );
	L.rows[ 1 ] = Vec4( q.x,  q.w, -q.z,  q.y );
	L.rows[ 2 ] = Vec4( q.y,  q.z,  q.w, -q.x );
	L.rows[ 3 ] = Vec4( q.z, -q.y,  q.x,  q.w );
#if defined( DO_TRANSPOSE )
	return L.Transpose();
#else
	return L;
#endif
}

Mat4 Constraint::Right( const Quat & q ) {
	Mat4 R;
	R.rows[ 0 ] = Vec4( q.w, -q.x, -q.y, -q.z );
	R.rows[ 1 ] = Vec4( q.x,  q.w,  q.z, -q.y );
	R.rows[ 2 ] = Vec4( q.y, -q.z,  q.w,  q.x );
	R.rows[ 3 ] = Vec4( q.z,  q.y, -q.x,  q.w );
#if defined( DO_TRANSPOSE )
	return R.Transpose();
#else
	return R;
#endif
}

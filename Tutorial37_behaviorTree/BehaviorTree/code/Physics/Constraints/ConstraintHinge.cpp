//
//  ConstraintHinge.cpp
//
#include "Physics/Constraints/ConstraintHinge.h"
#include "Physics/Body.h"
#include "Math/lcp.h"

#pragma optimize( "", off )

/*
========================================================================================================

ConstraintHingeVector

========================================================================================================
*/

/*
====================================================
ConstraintHingeVector::Solve
====================================================
*/
void ConstraintHingeVector::Solve() {
	const float dt_sec = 1.0f / 60.0f;
	if ( dt_sec <= 0.0f ) {
		return;
	}

	const float radius = 1.0f;

	// Get the world space position of the hinge from A's orientation
	const Vec3 worldAnchorA = m_bodyA->BodySpaceToWorldSpace( m_anchorA );

	// Get the world space position of the hinge from B's orientation
	const Vec3 worldAnchorB = m_bodyB->BodySpaceToWorldSpace( m_anchorB );

	// Get the hinge axis in world space
	const Vec3 worldAxisA = m_bodyA->m_orientation.RotatePoint( m_axisA );
	const Vec3 worldAxisB = m_bodyB->m_orientation.RotatePoint( m_axisB );

	const Vec3 r = worldAnchorB - worldAnchorA;
	const Vec3 ra = worldAnchorA - m_bodyA->GetCenterOfMassWorldSpace();
	const Vec3 rb = worldAnchorB - m_bodyB->GetCenterOfMassWorldSpace();
	const Vec3 a = worldAnchorA;
	const Vec3 b = worldAnchorB;

	const Vec3 n = worldAxisA;
	const Vec3 u = Vec3( worldAxisB.z, worldAxisB.x, worldAxisB.y ).Cross( worldAxisB ).Normalize();
	const Vec3 v = worldAxisB.Cross( u );

	const int numRows = 2;

	Vec3 forceInternalA( 0.0f );
	Vec3 torqueInternalA( 0.0f );
	Vec3 forceInternalB( 0.0f );
	Vec3 torqueInternalB( 0.0f );
	{
		const MatMN invMassMatrix = GetInverseMassMatrix();

		MatMN Jacobian( numRows, 12 );
		{
			Jacobian.Zero();

			// First row is the primary distance constraint that holds the anchor points together
			Vec3 J1 = ( a - b ) * 2.0f;
			Jacobian.rows[ 0 ][ 0 ] = J1.x;
			Jacobian.rows[ 0 ][ 1 ] = J1.y;
			Jacobian.rows[ 0 ][ 2 ] = J1.z;

			Vec3 J2 = ra.Cross( ( a - b ) * 2.0f );
			Jacobian.rows[ 0 ][ 3 ] = J2.x;
			Jacobian.rows[ 0 ][ 4 ] = J2.y;
			Jacobian.rows[ 0 ][ 5 ] = J2.z;

			Vec3 J3 = ( b - a ) * 2.0f;
			Jacobian.rows[ 0 ][ 6 ] = J3.x;
			Jacobian.rows[ 0 ][ 7 ] = J3.y;
			Jacobian.rows[ 0 ][ 8 ] = J3.z;

			Vec3 J4 = rb.Cross( ( b - a ) * 2.0f );
			Jacobian.rows[ 0 ][ 9 ] = J4.x;
			Jacobian.rows[ 0 ][ 10] = J4.y;
			Jacobian.rows[ 0 ][ 11] = J4.z;

			{
				// Second row will be the torsion constraint of the hinge axis
				J1.Zero();
				Jacobian.rows[ 1 ][ 0 ] = J1.x;
				Jacobian.rows[ 1 ][ 1 ] = J1.y;
				Jacobian.rows[ 1 ][ 2 ] = J1.z;

				J2 = worldAxisA.Cross( worldAxisB );
				Jacobian.rows[ 1 ][ 3 ] = J2.x;
				Jacobian.rows[ 1 ][ 4 ] = J2.y;
				Jacobian.rows[ 1 ][ 5 ] = J2.z;

				J3.Zero();
				Jacobian.rows[ 1 ][ 6 ] = J3.x;
				Jacobian.rows[ 1 ][ 7 ] = J3.y;
				Jacobian.rows[ 1 ][ 8 ] = J3.z;

				J4 = worldAxisB.Cross( worldAxisA );
				Jacobian.rows[ 1 ][ 9 ] = J4.x;
				Jacobian.rows[ 1 ][ 10] = J4.y;
				Jacobian.rows[ 1 ][ 11] = J4.z;
			}
		}
		MatMN JacobianTranspose = Jacobian.Transpose();

		const VecN q_dt = GetVelocities();

		// J = dC/dq
		// C_dt = J * q_dt
		// C_dt2 = J_dt * q_dt + J * q_dt2
		// J_dt = d( C_dt ) / dq - this works because order of operation of dq vs dt is interchangeable?
		// => C_dt2 = J_dt * q_dt + J * q_dt2
		// = J_dt * q_dt + J * W * ( Q_ext + Q_int )
		// = 0
		// => J W Q_int = - J_dt * q_dt + J W Q_ext
		// => Q_int = W_inv * J_inv * ( - J_dt * q_dt + J * W * Q_ext )
		// Q_int * x = 0, for all v st J * v = 0
		// Q_int = J_transpose * lambda
		//
		// => J W J_transpose * lambda = -J_dt * q_dt - J W Q_ext
		// For Baumgarte stabilization assume C_dt2 = -k_s * C - k_d * C_dt, instead of zero
		// => J W J_transpose * lamdba = -J_dt * q_dt - J W Q_ext - k_s * C - k_d * C_dt
		// solve for lamdba to get the internal forces
		// Boom, done.
		{
			MatMN J_W_Jt = Jacobian * invMassMatrix * JacobianTranspose;
			MatMN J_dt = Jacobian * ( 1.0f / dt_sec );	// This is more stable than hand calculating the time derivative of the jacobian
			VecN rhs = ( J_dt * q_dt * -1.0f );

			// Modified Baumgarte stabilization (need to read the original paper to find out by how much this is different)
			{
				const float C = r.Dot( r );
				const float Beta = 0.5f;	// keep it between [0,1)
				const float h = dt_sec;
				const float baumgarte = ( Beta / h ) * C;
				rhs[ 0 ] -= baumgarte;

				{
					const float C2 = worldAxisA.Dot( worldAxisB ) - 1.0f;
					const float Beta2 = 0.0f;
					const float baumgarte2 = ( Beta2 / h ) * C2;
					rhs[ 1 ] -= baumgarte2;
				}
			}

			MatN tmpJWJt = J_W_Jt;

			VecN lambdaN = LCP_GaussSeidel( tmpJWJt, rhs );
			//			float lambdaN = ( rhs - baumgarte ) / J_W_Jt;

			VecN force = JacobianTranspose * lambdaN;

			forceInternalA[ 0 ] = force[ 0 ];
			forceInternalA[ 1 ] = force[ 1 ];
			forceInternalA[ 2 ] = force[ 2 ];

			torqueInternalA[ 0 ] = force[ 3 ];
			torqueInternalA[ 1 ] = force[ 4 ];
			torqueInternalA[ 2 ] = force[ 5 ];

			forceInternalB[ 0 ] = force[ 6 ];
			forceInternalB[ 1 ] = force[ 7 ];
			forceInternalB[ 2 ] = force[ 8 ];

			torqueInternalB[ 0 ] = force[ 9 ];
			torqueInternalB[ 1 ] = force[ 10];
			torqueInternalB[ 2 ] = force[ 11];

			if ( !forceInternalA.IsValid() ) {
				printf( "oh boy\n" );
			}
			if ( !forceInternalB.IsValid() ) {
				printf( "oh boy\n" );
			}
		}
	}

// 	printf( "%.3f %.3f %.3f,    %.3f %.3f %.3f,    %.3f %.3f %.3f,    %.3f %.3f %.3f\n",
// 		forceInternalA[ 0 ],	forceInternalA[ 1 ],	forceInternalA[ 2 ],
// 		torqueInternalA[ 0 ],	torqueInternalA[ 1 ],	torqueInternalA[ 2 ],
// 		forceInternalB[ 0 ],	forceInternalB[ 1 ],	forceInternalB[ 2 ],
// 		torqueInternalB[ 0 ],	torqueInternalB[ 1 ],	torqueInternalB[ 2 ] );
	const Quat quatA = m_bodyA->m_orientation;
	const Quat quatB = m_bodyB->m_orientation;
	Quat relativeQuat = quatA * quatB.Inverse();
	printf( "%.3f %.3f %.3f %.3f     %.3f %.3f %.3f %.3f     %.3f %.3f %.3f %.3f\n",
		quatA.w, quatA.x, quatA.y, quatA.z,
		quatB.w, quatB.x, quatB.y, quatB.z,
		relativeQuat.w, relativeQuat.x, relativeQuat.y, relativeQuat.z );

	Vec3 forceExternal( 0.0f, 0.0f, -10.0f );
	forceExternal.Zero();
	Vec3 finalImpulseA = ( forceInternalA + forceExternal ) * dt_sec;
	m_bodyA->ApplyImpulseLinear( finalImpulseA );
	m_bodyA->ApplyImpulseAngular( torqueInternalA * dt_sec );

	Vec3 finalImpulseB = ( forceInternalB + forceExternal ) * dt_sec;
	m_bodyB->ApplyImpulseLinear( finalImpulseB );
	m_bodyB->ApplyImpulseAngular( torqueInternalB * dt_sec );
}

/*
========================================================================================================

ConstraintHingeAxis

========================================================================================================
*/

/*
====================================================
ConstraintHingeAxis::Solve
====================================================
*/
void ConstraintHingeAxis::Solve() {
	const float dt_sec = 1.0f / 60.0f;
	if ( dt_sec <= 0.0f ) {
		return;
	}

	const float radius = 1.0f;

	// Get the world space position of the hinge from A's orientation
	const Vec3 worldAnchorA = m_bodyA->BodySpaceToWorldSpace( m_anchorA );

	// Get the world space position of the hinge from B's orientation
	const Vec3 worldAnchorB = m_bodyB->BodySpaceToWorldSpace( m_anchorB );

	// Get the hinge axis in world space
	const Vec3 worldAxisA = m_bodyA->m_orientation.RotatePoint( m_axisA );
	const Vec3 worldAxisB = m_bodyB->m_orientation.RotatePoint( m_axisB );

	const Vec3 r = worldAnchorB - worldAnchorA;
	const Vec3 ra = worldAnchorA - m_bodyA->GetCenterOfMassWorldSpace();
	const Vec3 rb = worldAnchorB - m_bodyB->GetCenterOfMassWorldSpace();
	const Vec3 a = worldAnchorA;
	const Vec3 b = worldAnchorB;

	const Vec3 n = worldAxisA;
	const Vec3 u = Vec3( worldAxisB.z, worldAxisB.x, worldAxisB.y ).Cross( worldAxisB ).Normalize();
	const Vec3 v = worldAxisB.Cross( u );

	const int numRows = 3;

	Vec3 forceInternalA( 0.0f );
	Vec3 torqueInternalA( 0.0f );
	Vec3 forceInternalB( 0.0f );
	Vec3 torqueInternalB( 0.0f );
	{
		const MatMN invMassMatrix = GetInverseMassMatrix();

		MatMN Jacobian( numRows, 12 );
		{
			Jacobian.Zero();

			// First row is the primary distance constraint that holds the anchor points together
			Vec3 J1 = ( a - b ) * 2.0f;
			Jacobian.rows[ 0 ][ 0 ] = J1.x;
			Jacobian.rows[ 0 ][ 1 ] = J1.y;
			Jacobian.rows[ 0 ][ 2 ] = J1.z;

			Vec3 J2 = ra.Cross( ( a - b ) * 2.0f );
			Jacobian.rows[ 0 ][ 3 ] = J2.x;
			Jacobian.rows[ 0 ][ 4 ] = J2.y;
			Jacobian.rows[ 0 ][ 5 ] = J2.z;

			Vec3 J3 = ( b - a ) * 2.0f;
			Jacobian.rows[ 0 ][ 6 ] = J3.x;
			Jacobian.rows[ 0 ][ 7 ] = J3.y;
			Jacobian.rows[ 0 ][ 8 ] = J3.z;

			Vec3 J4 = rb.Cross( ( b - a ) * 2.0f );
			Jacobian.rows[ 0 ][ 9 ] = J4.x;
			Jacobian.rows[ 0 ][ 10] = J4.y;
			Jacobian.rows[ 0 ][ 11] = J4.z;

			// Axis jocabians
			{
				// Second row will be the torsion constraint of the hinge axis
				{
					J1.Zero();
					Jacobian.rows[ 1 ][ 0 ] = J1.x;
					Jacobian.rows[ 1 ][ 1 ] = J1.y;
					Jacobian.rows[ 1 ][ 2 ] = J1.z;

					J2 = n.Cross( u );
					Jacobian.rows[ 1 ][ 3 ] = J2.x;
					Jacobian.rows[ 1 ][ 4 ] = J2.y;
					Jacobian.rows[ 1 ][ 5 ] = J2.z;

					J3.Zero();
					Jacobian.rows[ 1 ][ 6 ] = J3.x;
					Jacobian.rows[ 1 ][ 7 ] = J3.y;
					Jacobian.rows[ 1 ][ 8 ] = J3.z;

					J4 = u.Cross( n );
					Jacobian.rows[ 1 ][ 9 ] = J4.x;
					Jacobian.rows[ 1 ][ 10] = J4.y;
					Jacobian.rows[ 1 ][ 11] = J4.z;
				}
				{
					J1.Zero();
					Jacobian.rows[ 2 ][ 0 ] = J1.x;
					Jacobian.rows[ 2 ][ 1 ] = J1.y;
					Jacobian.rows[ 2 ][ 2 ] = J1.z;

					J2 = n.Cross( v );
					Jacobian.rows[ 2 ][ 3 ] = J2.x;
					Jacobian.rows[ 2 ][ 4 ] = J2.y;
					Jacobian.rows[ 2 ][ 5 ] = J2.z;

					J3.Zero();
					Jacobian.rows[ 2 ][ 6 ] = J3.x;
					Jacobian.rows[ 2 ][ 7 ] = J3.y;
					Jacobian.rows[ 2 ][ 8 ] = J3.z;

					J4 = v.Cross( n );
					Jacobian.rows[ 2 ][ 9 ] = J4.x;
					Jacobian.rows[ 2 ][ 10] = J4.y;
					Jacobian.rows[ 2 ][ 11] = J4.z;
				}
			}
		}
		MatMN JacobianTranspose = Jacobian.Transpose();

		const VecN q_dt = GetVelocities();

		// J = dC/dq
		// C_dt = J * q_dt
		// C_dt2 = J_dt * q_dt + J * q_dt2
		// J_dt = d( C_dt ) / dq - this works because order of operation of dq vs dt is interchangeable?
		// => C_dt2 = J_dt * q_dt + J * q_dt2
		// = J_dt * q_dt + J * W * ( Q_ext + Q_int )
		// = 0
		// => J W Q_int = - J_dt * q_dt + J W Q_ext
		// => Q_int = W_inv * J_inv * ( - J_dt * q_dt + J * W * Q_ext )
		// Q_int * x = 0, for all v st J * v = 0
		// Q_int = J_transpose * lambda
		//
		// => J W J_transpose * lambda = -J_dt * q_dt - J W Q_ext
		// For Baumgarte stabilization assume C_dt2 = -k_s * C - k_d * C_dt, instead of zero
		// => J W J_transpose * lamdba = -J_dt * q_dt - J W Q_ext - k_s * C - k_d * C_dt
		// solve for lamdba to get the internal forces
		// Boom, done.
		{
			MatMN J_W_Jt = Jacobian * invMassMatrix * JacobianTranspose;
			MatMN J_dt = Jacobian * ( 1.0f / dt_sec );	// This is more stable than hand calculating the time derivative of the jacobian
			VecN rhs = ( J_dt * q_dt * -1.0f );

			// Modified Baumgarte stabilization (need to read the original paper to find out by how much this is different)
			{
				const float C = r.Dot( r );
				const float Beta = 0.5f;	// keep it between [0,1)
				const float h = dt_sec;
				const float baumgarte = ( Beta / h ) * C;
				rhs[ 0 ] -= baumgarte;

				{
					const float C2 = n.Dot( u );
					const float Beta2 = 0.5f;
					const float baumgarte2 = ( Beta2 / h ) * C2;
					rhs[ 1 ] -= baumgarte2;

					const float C3 = n.Dot( v );
					const float Beta3 = 0.5f;
					const float baumgarte3 = ( Beta3 / h ) * C3;
					rhs[ 2 ] -= baumgarte3;
				}
			}

			MatN tmpJWJt = J_W_Jt;

			VecN lambdaN = LCP_GaussSeidel( tmpJWJt, rhs );
			//			float lambdaN = ( rhs - baumgarte ) / J_W_Jt;

			VecN force = JacobianTranspose * lambdaN;

			forceInternalA[ 0 ] = force[ 0 ];
			forceInternalA[ 1 ] = force[ 1 ];
			forceInternalA[ 2 ] = force[ 2 ];

			torqueInternalA[ 0 ] = force[ 3 ];
			torqueInternalA[ 1 ] = force[ 4 ];
			torqueInternalA[ 2 ] = force[ 5 ];

			forceInternalB[ 0 ] = force[ 6 ];
			forceInternalB[ 1 ] = force[ 7 ];
			forceInternalB[ 2 ] = force[ 8 ];

			torqueInternalB[ 0 ] = force[ 9 ];
			torqueInternalB[ 1 ] = force[ 10];
			torqueInternalB[ 2 ] = force[ 11];

			if ( !forceInternalA.IsValid() ) {
				printf( "oh boy\n" );
			}
			if ( !forceInternalB.IsValid() ) {
				printf( "oh boy\n" );
			}
		}
	}

	// 	printf( "%.3f %.3f %.3f,    %.3f %.3f %.3f,    %.3f %.3f %.3f,    %.3f %.3f %.3f\n",
	// 		forceInternalA[ 0 ],	forceInternalA[ 1 ],	forceInternalA[ 2 ],
	// 		torqueInternalA[ 0 ],	torqueInternalA[ 1 ],	torqueInternalA[ 2 ],
	// 		forceInternalB[ 0 ],	forceInternalB[ 1 ],	forceInternalB[ 2 ],
	// 		torqueInternalB[ 0 ],	torqueInternalB[ 1 ],	torqueInternalB[ 2 ] );
	const Quat quatA = m_bodyA->m_orientation;
	const Quat quatB = m_bodyB->m_orientation;
	Quat relativeQuat = quatA * quatB.Inverse();
	printf( "%.3f %.3f %.3f %.3f     %.3f %.3f %.3f %.3f     %.3f %.3f %.3f %.3f\n",
		quatA.w, quatA.x, quatA.y, quatA.z,
		quatB.w, quatB.x, quatB.y, quatB.z,
		relativeQuat.w, relativeQuat.x, relativeQuat.y, relativeQuat.z );

	Vec3 forceExternal( 0.0f, 0.0f, -10.0f );
	forceExternal.Zero();
	Vec3 finalImpulseA = ( forceInternalA + forceExternal ) * dt_sec;
	m_bodyA->ApplyImpulseLinear( finalImpulseA );
	m_bodyA->ApplyImpulseAngular( torqueInternalA * dt_sec );

	Vec3 finalImpulseB = ( forceInternalB + forceExternal ) * dt_sec;
	m_bodyB->ApplyImpulseLinear( finalImpulseB );
	m_bodyB->ApplyImpulseAngular( torqueInternalB * dt_sec );
}

/*
========================================================================================================

ConstraintHingeQuat

========================================================================================================
*/

/*
====================================================
ConstraintHingeQuat::PreSolve
====================================================
*/
void ConstraintHingeQuat::PreSolve( const float dt_sec ) {
	// Get the world space position of the hinge from A's orientation
	const Vec3 worldAnchorA = m_bodyA->BodySpaceToWorldSpace( m_anchorA );

	// Get the world space position of the hinge from B's orientation
	const Vec3 worldAnchorB = m_bodyB->BodySpaceToWorldSpace( m_anchorB );

	const Vec3 r = worldAnchorB - worldAnchorA;
	const Vec3 ra = worldAnchorA - m_bodyA->GetCenterOfMassWorldSpace();
	const Vec3 rb = worldAnchorB - m_bodyB->GetCenterOfMassWorldSpace();
	const Vec3 a = worldAnchorA;
	const Vec3 b = worldAnchorB;

	// Get the orientation information of the bodies
	const Quat q1 = m_bodyA->m_orientation;
	const Quat q2 = m_bodyB->m_orientation;
	const Quat q0_inv = q0.Inverse();
	const Quat q1_inv = q1.Inverse();

	// This axis is defined in the local space of bodyA
	Vec3 u;
	Vec3 v;
	Vec3 hingeAxis = m_axisA;
	hingeAxis.GetOrtho( u, v );

	Mat4 P;
	P.rows[ 0 ] = Vec4( 0, 0, 0, 0 );
	P.rows[ 1 ] = Vec4( 0, 1, 0, 0 );
	P.rows[ 2 ] = Vec4( 0, 0, 1, 0 );
	P.rows[ 3 ] = Vec4( 0, 0, 0, 1 );
	Mat4 P_T = P.Transpose();	// I know it's pointless to do this with our particular matrix implementations.  But I like its self commenting.

	const Mat4 MatA = P * Left( q1_inv ) * Right( q2 * q0_inv ) * P_T * -0.5f;
	const Mat4 MatB = P * Left( q1_inv ) * Right( q2 * q0_inv ) * P_T * 0.5f;

	const MatMN invMassMatrix = GetInverseMassMatrix();

	m_Jacobian.Zero();

	//
	//	The distance constraint
	//
	Vec3 J1 = ( a - b ) * 2.0f;
	m_Jacobian.rows[ 0 ][ 0 ] = J1.x;
	m_Jacobian.rows[ 0 ][ 1 ] = J1.y;
	m_Jacobian.rows[ 0 ][ 2 ] = J1.z;

	Vec3 J2 = ra.Cross( ( a - b ) * 2.0f );
	m_Jacobian.rows[ 0 ][ 3 ] = J2.x;
	m_Jacobian.rows[ 0 ][ 4 ] = J2.y;
	m_Jacobian.rows[ 0 ][ 5 ] = J2.z;

	Vec3 J3 = ( b - a ) * 2.0f;
	m_Jacobian.rows[ 0 ][ 6 ] = J3.x;
	m_Jacobian.rows[ 0 ][ 7 ] = J3.y;
	m_Jacobian.rows[ 0 ][ 8 ] = J3.z;

	Vec3 J4 = rb.Cross( ( b - a ) * 2.0f );
	m_Jacobian.rows[ 0 ][ 9 ] = J4.x;
	m_Jacobian.rows[ 0 ][ 10] = J4.y;
	m_Jacobian.rows[ 0 ][ 11] = J4.z;

	//
	// The quaternion jacobians
	//
	const int idx = 1;

	Vec4 tmp;
	{
		J1.Zero();
		m_Jacobian.rows[ 1 ][ 0 ] = J1.x;
		m_Jacobian.rows[ 1 ][ 1 ] = J1.y;
		m_Jacobian.rows[ 1 ][ 2 ] = J1.z;

		tmp = MatA * Vec4( 0, u.x, u.y, u.z );
		J2 = Vec3( tmp[ idx + 0 ], tmp[ idx + 1 ], tmp[ idx + 2 ] );
		m_Jacobian.rows[ 1 ][ 3 ] = J2.x;
		m_Jacobian.rows[ 1 ][ 4 ] = J2.y;
		m_Jacobian.rows[ 1 ][ 5 ] = J2.z;

		J3.Zero();
		m_Jacobian.rows[ 1 ][ 6 ] = J3.x;
		m_Jacobian.rows[ 1 ][ 7 ] = J3.y;
		m_Jacobian.rows[ 1 ][ 8 ] = J3.z;

		tmp = MatB * Vec4( 0, u.x, u.y, u.z );
		J4 = Vec3( tmp[ idx + 0 ], tmp[ idx + 1 ], tmp[ idx + 2 ] );
		m_Jacobian.rows[ 1 ][ 9 ] = J4.x;
		m_Jacobian.rows[ 1 ][ 10] = J4.y;
		m_Jacobian.rows[ 1 ][ 11] = J4.z;
	}
	{
		J1.Zero();
		m_Jacobian.rows[ 2 ][ 0 ] = J1.x;
		m_Jacobian.rows[ 2 ][ 1 ] = J1.y;
		m_Jacobian.rows[ 2 ][ 2 ] = J1.z;

		tmp = MatA * Vec4( 0, v.x, v.y, v.z );
		J2 = Vec3( tmp[ idx + 0 ], tmp[ idx + 1 ], tmp[ idx + 2 ] );
		m_Jacobian.rows[ 2 ][ 3 ] = J2.x;
		m_Jacobian.rows[ 2 ][ 4 ] = J2.y;
		m_Jacobian.rows[ 2 ][ 5 ] = J2.z;

		J3.Zero();
		m_Jacobian.rows[ 2 ][ 6 ] = J3.x;
		m_Jacobian.rows[ 2 ][ 7 ] = J3.y;
		m_Jacobian.rows[ 2 ][ 8 ] = J3.z;

		tmp = MatB * Vec4( 0, v.x, v.y, v.z );
		J4 = Vec3( tmp[ idx + 0 ], tmp[ idx + 1 ], tmp[ idx + 2 ] );
		m_Jacobian.rows[ 2 ][ 9 ] = J4.x;
		m_Jacobian.rows[ 2 ][ 10] = J4.y;
		m_Jacobian.rows[ 2 ][ 11] = J4.z;
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
	float C = r.Dot( r );
	C = std::max( 0.0f, C - 0.01f );
	const float Beta = 0.05f;
	m_baumgarte = ( Beta / dt_sec ) * C;
#else
	m_baumgarte = 0.0f;
#endif
}

/*
====================================================
ConstraintHingeQuat::Solve
====================================================
*/
void ConstraintHingeQuat::Solve() {
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
}

/*
====================================================
ConstraintHingeQuat::PostSolve
====================================================
*/
void ConstraintHingeQuat::PostSolve() {
	// Limit the warm starting to reasonable limits
	for ( int i = 0; i < m_cachedLambda.N; i++ ) {
		if ( m_cachedLambda[ i ] * 0.0f != m_cachedLambda[ i ] * 0.0f ) {
			m_cachedLambda[ i ] = 0.0f;
		}
		const float limit = 1e5f;
		if ( m_cachedLambda[ i ] > limit ) {
			m_cachedLambda[ i ] = limit;
		}
		if ( m_cachedLambda[ i ] < -limit ) {
			m_cachedLambda[ i ] = -limit;
		}
	}
}

/*
========================================================================================================

ConstraintHingeQuatLimited

========================================================================================================
*/

/*
====================================================
ConstraintHingeQuatLimited::PreSolve
====================================================
*/
void ConstraintHingeQuatLimited::PreSolve( const float dt_sec ) {
	// Get the world space position of the hinge from A's orientation
	const Vec3 worldAnchorA = m_bodyA->BodySpaceToWorldSpace( m_anchorA );

	// Get the world space position of the hinge from B's orientation
	const Vec3 worldAnchorB = m_bodyB->BodySpaceToWorldSpace( m_anchorB );

	const Vec3 r = worldAnchorB - worldAnchorA;
	const Vec3 ra = worldAnchorA - m_bodyA->GetCenterOfMassWorldSpace();
	const Vec3 rb = worldAnchorB - m_bodyB->GetCenterOfMassWorldSpace();
	const Vec3 a = worldAnchorA;
	const Vec3 b = worldAnchorB;

	// Get the orientation information of the bodies
	const Quat q1 = m_bodyA->m_orientation;
	const Quat q2 = m_bodyB->m_orientation;
	const Quat q0_inv = m_q0.Inverse();
	const Quat q1_inv = q1.Inverse();

	// This axis is defined in the local space of bodyA
	Vec3 u;
	Vec3 v;
	Vec3 hingeAxis = m_axisA;
	hingeAxis.GetOrtho( u, v );

	Mat4 P;
	P.rows[ 0 ] = Vec4( 0, 0, 0, 0 );
	P.rows[ 1 ] = Vec4( 0, 1, 0, 0 );
	P.rows[ 2 ] = Vec4( 0, 0, 1, 0 );
	P.rows[ 3 ] = Vec4( 0, 0, 0, 1 );
	Mat4 P_T = P.Transpose();	// I know it's pointless to do this with our particular matrix implementations.  But I like its self commenting.

	const Mat4 MatA = P * Left( q1_inv ) * Right( q2 * q0_inv ) * P_T * -0.5f;
	const Mat4 MatB = P * Left( q1_inv ) * Right( q2 * q0_inv ) * P_T * 0.5f;

	const float pi = acosf( -1.0f );
	const Quat qr = q1_inv * q2;
	const float angle = 2.0f * atanf( qr.xyz().GetMagnitude() / qr.w ) * 180.0f / pi;

	const Quat qrr = qr * q0_inv;
	const float angle2 = 2.0f * atanf( qrr.xyz().GetMagnitude() / qrr.w ) * 180.0f / pi;
	const float angle3 = 2.0f * asinf( qrr.xyz().Dot( hingeAxis ) ) * 180.0f / pi;
	const float angle4 = 2.0f * atanf( qrr.xyz().Dot( hingeAxis ) / qrr.w ) * 180.0f / pi;
	const float relativeAngle = angle3;

	// Check if there's an angle violation
	m_isAngleViolated = false;
	if ( relativeAngle > 45.0f ) {
		m_isAngleViolated = true;
	}
	if ( relativeAngle < -45.0f ) {
		m_isAngleViolated = true;
	}
	m_relativeAngle = relativeAngle;

// 	printf( "   Angle: %f", angle );
// 	printf( "   Angle: %f", angle2 );
// 	printf( "   Angle: %f", angle3 );
// 	printf( "   Angle: %f", angle4 );
// 	printf( "\n" );

	//
	// First row is the primary distance constraint that holds the anchor points together
	//
	m_Jacobian.Zero();

	Vec3 J1 = ( a - b ) * 2.0f;
	m_Jacobian.rows[ 0 ][ 0 ] = J1.x;
	m_Jacobian.rows[ 0 ][ 1 ] = J1.y;
	m_Jacobian.rows[ 0 ][ 2 ] = J1.z;

	Vec3 J2 = ra.Cross( ( a - b ) * 2.0f );
	m_Jacobian.rows[ 0 ][ 3 ] = J2.x;
	m_Jacobian.rows[ 0 ][ 4 ] = J2.y;
	m_Jacobian.rows[ 0 ][ 5 ] = J2.z;

	Vec3 J3 = ( b - a ) * 2.0f;
	m_Jacobian.rows[ 0 ][ 6 ] = J3.x;
	m_Jacobian.rows[ 0 ][ 7 ] = J3.y;
	m_Jacobian.rows[ 0 ][ 8 ] = J3.z;

	Vec3 J4 = rb.Cross( ( b - a ) * 2.0f );
	m_Jacobian.rows[ 0 ][ 9 ] = J4.x;
	m_Jacobian.rows[ 0 ][ 10] = J4.y;
	m_Jacobian.rows[ 0 ][ 11] = J4.z;

	// The quaternion jacobians
	const int idx = 1;

	Vec4 tmp;
	{
		J1.Zero();
		m_Jacobian.rows[ 1 ][ 0 ] = J1.x;
		m_Jacobian.rows[ 1 ][ 1 ] = J1.y;
		m_Jacobian.rows[ 1 ][ 2 ] = J1.z;

		tmp = MatA * Vec4( 0, u.x, u.y, u.z );
		J2 = Vec3( tmp[ idx + 0 ], tmp[ idx + 1 ], tmp[ idx + 2 ] );
		m_Jacobian.rows[ 1 ][ 3 ] = J2.x;
		m_Jacobian.rows[ 1 ][ 4 ] = J2.y;
		m_Jacobian.rows[ 1 ][ 5 ] = J2.z;

		J3.Zero();
		m_Jacobian.rows[ 1 ][ 6 ] = J3.x;
		m_Jacobian.rows[ 1 ][ 7 ] = J3.y;
		m_Jacobian.rows[ 1 ][ 8 ] = J3.z;

		tmp = MatB * Vec4( 0, u.x, u.y, u.z );
		J4 = Vec3( tmp[ idx + 0 ], tmp[ idx + 1 ], tmp[ idx + 2 ] );
		m_Jacobian.rows[ 1 ][ 9 ] = J4.x;
		m_Jacobian.rows[ 1 ][ 10] = J4.y;
		m_Jacobian.rows[ 1 ][ 11] = J4.z;
	}
	{
		J1.Zero();
		m_Jacobian.rows[ 2 ][ 0 ] = J1.x;
		m_Jacobian.rows[ 2 ][ 1 ] = J1.y;
		m_Jacobian.rows[ 2 ][ 2 ] = J1.z;

		tmp = MatA * Vec4( 0, v.x, v.y, v.z );
		J2 = Vec3( tmp[ idx + 0 ], tmp[ idx + 1 ], tmp[ idx + 2 ] );
		m_Jacobian.rows[ 2 ][ 3 ] = J2.x;
		m_Jacobian.rows[ 2 ][ 4 ] = J2.y;
		m_Jacobian.rows[ 2 ][ 5 ] = J2.z;

		J3.Zero();
		m_Jacobian.rows[ 2 ][ 6 ] = J3.x;
		m_Jacobian.rows[ 2 ][ 7 ] = J3.y;
		m_Jacobian.rows[ 2 ][ 8 ] = J3.z;

		tmp = MatB * Vec4( 0, v.x, v.y, v.z );
		J4 = Vec3( tmp[ idx + 0 ], tmp[ idx + 1 ], tmp[ idx + 2 ] );
		m_Jacobian.rows[ 2 ][ 9 ] = J4.x;
		m_Jacobian.rows[ 2 ][ 10] = J4.y;
		m_Jacobian.rows[ 2 ][ 11] = J4.z;
	}
	if ( m_isAngleViolated ) {
		J1.Zero();
		m_Jacobian.rows[ 3 ][ 0 ] = J1.x;
		m_Jacobian.rows[ 3 ][ 1 ] = J1.y;
		m_Jacobian.rows[ 3 ][ 2 ] = J1.z;

		tmp = MatA * Vec4( 0, hingeAxis.x, hingeAxis.y, hingeAxis.z );
		J2 = Vec3( tmp[ idx + 0 ], tmp[ idx + 1 ], tmp[ idx + 2 ] );
		m_Jacobian.rows[ 3 ][ 3 ] = J2.x;
		m_Jacobian.rows[ 3 ][ 4 ] = J2.y;
		m_Jacobian.rows[ 3 ][ 5 ] = J2.z;

		J3.Zero();
		m_Jacobian.rows[ 3 ][ 6 ] = J3.x;
		m_Jacobian.rows[ 3 ][ 7 ] = J3.y;
		m_Jacobian.rows[ 3 ][ 8 ] = J3.z;

		tmp = MatB * Vec4( 0, hingeAxis.x, hingeAxis.y, hingeAxis.z );
		J4 = Vec3( tmp[ idx + 0 ], tmp[ idx + 1 ], tmp[ idx + 2 ] );
		m_Jacobian.rows[ 3 ][ 9 ] = J4.x;
		m_Jacobian.rows[ 3 ][ 10] = J4.y;
		m_Jacobian.rows[ 3 ][ 11] = J4.z;
	}


#define WARM_STARTING_LIMITED
#if defined( WARM_STARTING_LIMITED )
	//
	// Apply warm starting from last frame
	//
	const VecN impulses = m_Jacobian.Transpose() * m_cachedLambda;
	ApplyImpulses( impulses );
#endif

#define BAUMGARTE_STABILIZATION_LIMITED
#if defined( BAUMGARTE_STABILIZATION_LIMITED )
	//
	//	Calculate the baumgarte stabilization
	//
	float C = r.Dot( r );
	C = std::max( 0.0f, C - 0.01f );
	const float Beta = 0.05f;
	m_baumgarte = ( Beta / dt_sec ) * C;
#else
	m_baumgarte = 0.0f;
#endif
}

/*
====================================================
ConstraintHingeQuatLimited::Solve
====================================================
*/
void ConstraintHingeQuatLimited::Solve() {
	const MatMN JacobianTranspose = m_Jacobian.Transpose();

	// Build the system of equations
	const VecN q_dt = GetVelocities();
	const MatMN invMassMatrix = GetInverseMassMatrix();
	const MatMN J_W_Jt = m_Jacobian * invMassMatrix * JacobianTranspose;
	VecN rhs = ( m_Jacobian * q_dt * -1.0f );
	rhs[ 0 ] -= m_baumgarte;

	// Solve for the Lagrange multipliers
	VecN lambdaN = LCP_GaussSeidel( J_W_Jt, rhs );

	// Clamp the torque from the angle constraint.
	// We need to make sure it's a restorative torque.
	if ( m_isAngleViolated ) {
		if ( m_relativeAngle > 0.0f ) {
			lambdaN[ 3 ] = std::min( 0.0f, lambdaN[ 3 ] );
		}
		if ( m_relativeAngle < 0.0f ) {
			lambdaN[ 3 ] = std::max( 0.0f, lambdaN[ 3 ] );
		}
	}

	// Apply the impulses
	const VecN impulses = JacobianTranspose * lambdaN;
	ApplyImpulses( impulses );

	// Accumulate the impulses for warm starting
	m_cachedLambda += lambdaN;
}

/*
====================================================
ConstraintHingeQuatLimited::PostSolve
====================================================
*/
void ConstraintHingeQuatLimited::PostSolve() {
	// Limit the warm starting to reasonable limits
	for ( int i = 0; i < m_cachedLambda.N; i++ ) {
		if ( i > 0 ) {
			m_cachedLambda[ i ] = 0.0f;
		}

		if ( m_cachedLambda[ i ] * 0.0f != m_cachedLambda[ i ] * 0.0f ) {
			m_cachedLambda[ i ] = 0.0f;
		}
		const float limit = 20.0f;
		if ( m_cachedLambda[ i ] > limit ) {
			m_cachedLambda[ i ] = limit;
		}
		if ( m_cachedLambda[ i ] < -limit ) {
			m_cachedLambda[ i ] = -limit;
		}
	}
}
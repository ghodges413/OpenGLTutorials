//
//  ConstraintDistance.cpp
//
#include "Physics/Constraints/ConstraintDistance.h"
#include "Physics/Body.h"
#include "Math/lcp.h"

#pragma optimize( "", off )

/*
========================================================================================================

ConstraintDistance

========================================================================================================
*/

/*
====================================================
ConstraintDistance::Solve
====================================================
*/
void ConstraintDistance::Solve() {
	const float dt_sec = 1.0f / 60.0f;
	const Vec3 forceExternal = Vec3( 0, 0, -10 );// * mass;
	const float radius = 1.0f;

	const Vec3 r = m_bodyB->m_position - m_bodyA->m_position;	// The vector pointing from the constraint/origin to the particle position;

	// C = ( b - a ) * ( b - a ) - l^2 = 0
	// dC/dt = d( b - a )/dt * ( b - a ) + ( b - a ) * d( b - a )/dt
	// = ( vb - va ) * ( b - a ) + ( b - a ) * ( vb - va )
	// = vb * b - vb * a - va * b + va * a + b * vb - b * va - a * vb + a * va
	// = 2 * vb * b - 2 * vb * a - 2 * va * b + 2 * va * a
	// => J = ( ( - 2 b + 2 a ), ( 2 b - 2 a ) )

	// C = b * b - b * a - a * b + a * a
	// = b * b - 2 * a * b + a * a

	Vec3 forceInternalA;
	Vec3 forceInternalB;
	{
		MatMN invMassMatrix( 6, 6 );
		{
			invMassMatrix.Zero();
			invMassMatrix.rows[ 0 ][ 0 ] = m_bodyA->m_invMass;
			invMassMatrix.rows[ 1 ][ 1 ] = m_bodyA->m_invMass;
			invMassMatrix.rows[ 2 ][ 2 ] = m_bodyA->m_invMass;

			invMassMatrix.rows[ 3 ][ 3 ] = m_bodyB->m_invMass;
			invMassMatrix.rows[ 4 ][ 4 ] = m_bodyB->m_invMass;
			invMassMatrix.rows[ 5 ][ 5 ] = m_bodyB->m_invMass;
		}

		// q_dt2 = W * Q = invMassMatrix * Force
		// C = x^2 + y^2 + z^2 - r^2 = 0
		// C_dx = 2 * x
		// C_dy = 2 * y
		// C_dz = 2 * z
		// C( x, y , z )
		MatMN Jacobian( 1, 6 );
		{
			Jacobian.Zero();
			Jacobian.rows[ 0 ][ 0 ] = 2.0f * ( m_bodyA->m_position.x - m_bodyB->m_position.x );
			Jacobian.rows[ 0 ][ 1 ] = 2.0f * ( m_bodyA->m_position.y - m_bodyB->m_position.y );
			Jacobian.rows[ 0 ][ 2 ] = 2.0f * ( m_bodyA->m_position.z - m_bodyB->m_position.z );

			Jacobian.rows[ 0 ][ 3 ] = 2.0f * ( m_bodyB->m_position.x - m_bodyA->m_position.x );
			Jacobian.rows[ 0 ][ 4 ] = 2.0f * ( m_bodyB->m_position.y - m_bodyA->m_position.y );
			Jacobian.rows[ 0 ][ 5 ] = 2.0f * ( m_bodyB->m_position.z - m_bodyA->m_position.z );
		}
		MatMN JacobianTranspose = Jacobian.Transpose();

		// Time derivative of the Jacobian
		MatMN J_dt( 1, 6 );
		{
			J_dt.Zero();
			J_dt.rows[ 0 ][ 0 ] = 2.0f * ( m_bodyA->m_linearVelocity.x - m_bodyB->m_linearVelocity.x );
			J_dt.rows[ 0 ][ 1 ] = 2.0f * ( m_bodyA->m_linearVelocity.y - m_bodyB->m_linearVelocity.y );
			J_dt.rows[ 0 ][ 2 ] = 2.0f * ( m_bodyA->m_linearVelocity.z - m_bodyB->m_linearVelocity.z );

			J_dt.rows[ 0 ][ 3 ] = 2.0f * ( m_bodyB->m_linearVelocity.x - m_bodyA->m_linearVelocity.x );
			J_dt.rows[ 0 ][ 4 ] = 2.0f * ( m_bodyB->m_linearVelocity.y - m_bodyA->m_linearVelocity.y );
			J_dt.rows[ 0 ][ 5 ] = 2.0f * ( m_bodyB->m_linearVelocity.z - m_bodyA->m_linearVelocity.z );
		}

		VecN q_dt( 6 );
		{
			q_dt[ 0 ] = m_bodyA->m_linearVelocity.x;
			q_dt[ 1 ] = m_bodyA->m_linearVelocity.y;
			q_dt[ 2 ] = m_bodyA->m_linearVelocity.z;

			q_dt[ 3 ] = m_bodyB->m_linearVelocity.x;
			q_dt[ 4 ] = m_bodyB->m_linearVelocity.y;
			q_dt[ 5 ] = m_bodyB->m_linearVelocity.z;
		}

		VecN Q_ext( 6 );
		{
			// forces on body a
			Q_ext[ 0 ] = forceExternal.x;
			Q_ext[ 1 ] = forceExternal.y;
			Q_ext[ 2 ] = forceExternal.z;

			// forces on body b
			Q_ext[ 3 ] = forceExternal.x;
			Q_ext[ 4 ] = forceExternal.y;
			Q_ext[ 5 ] = forceExternal.z;
		}


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
			VecN rhs = ( J_dt * q_dt * -1.0f ) - Jacobian * invMassMatrix * Q_ext;

			// 			float k_s = 0.5f;
			// 			float k_d = 0.5f;
			// 			float C = r.DotProduct( r ) - radius * radius;
			// 			float C_dt = Jacobian.DotProduct( q_dt );
			// 			float baumgarte = k_s * C + k_d * C_dt;
			VecN k_s( 1 );
			k_s[ 0 ] = 0.5f;
			float k_d = 0.5f;
			float C = r.Dot( r ) - radius * radius;
			VecN C_dt = Jacobian * q_dt;
			VecN baumgarte = k_s * C + C_dt * k_d;
			rhs -= baumgarte;

			MatN tmpJWJt = J_W_Jt;

			VecN lambdaN = LCP_GaussSeidel( tmpJWJt, rhs );
			//			float lambdaN = ( rhs - baumgarte ) / J_W_Jt;

			VecN force = JacobianTranspose * lambdaN;
			forceInternalA[ 0 ] = force[ 0 ];
			forceInternalA[ 1 ] = force[ 1 ];
			forceInternalA[ 2 ] = force[ 2 ];

			forceInternalB[ 0 ] = force[ 3 ];
			forceInternalB[ 1 ] = force[ 4 ];
			forceInternalB[ 2 ] = force[ 5 ];
		}
	}

	Vec3 finalImpulseA = ( forceInternalA + forceExternal ) * dt_sec;
	m_bodyA->ApplyImpulse( m_bodyA->m_position, finalImpulseA );

	Vec3 finalImpulseB = ( forceInternalB + forceExternal ) * dt_sec;
	m_bodyB->ApplyImpulse( m_bodyB->m_position, finalImpulseB );
}


/*
========================================================================================================

ConstraintDistance2

========================================================================================================
*/

/*
====================================================
ConstraintDistance2::Solve
====================================================
*/
void ConstraintDistance2::Solve() {
	const float dt_sec = 1.0f / 60.0f;
	if ( dt_sec <= 0.0f ) {
		return;
	}

	const Vec3 forceExternal = Vec3( 0, 0, -10 );// * mass;
	const float radius = 1.0f;

	// The vector pointing from the constraint/origin to the particle position;
	const Vec3 r = m_bodyB->m_position - m_bodyA->m_position;

	// C = ( b - a ) * ( b - a ) - l^2 = 0
	// dC/dt = d( b - a )/dt * ( b - a ) + ( b - a ) * d( b - a )/dt
	// = ( vb - va ) * ( b - a ) + ( b - a ) * ( vb - va )
	// = vb * b - vb * a - va * b + va * a + b * vb - b * va - a * vb + a * va
	// = 2 * vb * b - 2 * vb * a - 2 * va * b + 2 * va * a
	// => J = ( ( - 2 b + 2 a ), ( 2 b - 2 a ) )

	// C = b * b - b * a - a * b + a * a
	// = b * b - 2 * a * b + a * a

	Vec3 forceInternalA( 0.0f );
	Vec3 torqueInternalA( 0.0f );
	Vec3 forceInternalB( 0.0f );
	Vec3 torqueInternalB( 0.0f );
	{
		MatMN invMassMatrix( 6, 6 );
		{
			invMassMatrix.Zero();
			invMassMatrix.rows[ 0 ][ 0 ] = m_bodyA->m_invMass;
			invMassMatrix.rows[ 1 ][ 1 ] = m_bodyA->m_invMass;
			invMassMatrix.rows[ 2 ][ 2 ] = m_bodyA->m_invMass;

			invMassMatrix.rows[ 3 ][ 3 ] = m_bodyB->m_invMass;
			invMassMatrix.rows[ 4 ][ 4 ] = m_bodyB->m_invMass;
			invMassMatrix.rows[ 5 ][ 5 ] = m_bodyB->m_invMass;
		}

		// q_dt2 = W * Q = invMassMatrix * Force
		// C = x^2 + y^2 + z^2 - r^2 = 0
		// C_dx = 2 * x
		// C_dy = 2 * y
		// C_dz = 2 * z
		// C( x, y , z )
		MatMN Jacobian( 1, 6 );
		{
			Jacobian.Zero();
			Jacobian.rows[ 0 ][ 0 ] = 2.0f * ( m_bodyA->m_position.x - m_bodyB->m_position.x );
			Jacobian.rows[ 0 ][ 1 ] = 2.0f * ( m_bodyA->m_position.y - m_bodyB->m_position.y );
			Jacobian.rows[ 0 ][ 2 ] = 2.0f * ( m_bodyA->m_position.z - m_bodyB->m_position.z );

			Jacobian.rows[ 0 ][ 3 ] = 2.0f * ( m_bodyB->m_position.x - m_bodyA->m_position.x );
			Jacobian.rows[ 0 ][ 4 ] = 2.0f * ( m_bodyB->m_position.y - m_bodyA->m_position.y );
			Jacobian.rows[ 0 ][ 5 ] = 2.0f * ( m_bodyB->m_position.z - m_bodyA->m_position.z );
		}
		MatMN JacobianTranspose = Jacobian.Transpose();

		// Time derivative of the Jacobian
		MatMN J_dt( 1, 6 );
		{
			J_dt.Zero();
			J_dt.rows[ 0 ][ 0 ] = 2.0f * ( m_bodyA->m_linearVelocity.x - m_bodyB->m_linearVelocity.x );
			J_dt.rows[ 0 ][ 1 ] = 2.0f * ( m_bodyA->m_linearVelocity.y - m_bodyB->m_linearVelocity.y );
			J_dt.rows[ 0 ][ 2 ] = 2.0f * ( m_bodyA->m_linearVelocity.z - m_bodyB->m_linearVelocity.z );

			J_dt.rows[ 0 ][ 3 ] = 2.0f * ( m_bodyB->m_linearVelocity.x - m_bodyA->m_linearVelocity.x );
			J_dt.rows[ 0 ][ 4 ] = 2.0f * ( m_bodyB->m_linearVelocity.y - m_bodyA->m_linearVelocity.y );
			J_dt.rows[ 0 ][ 5 ] = 2.0f * ( m_bodyB->m_linearVelocity.z - m_bodyA->m_linearVelocity.z );
		}

		VecN q_dt( 6 );
		{
			q_dt[ 0 ] = m_bodyA->m_linearVelocity.x;
			q_dt[ 1 ] = m_bodyA->m_linearVelocity.y;
			q_dt[ 2 ] = m_bodyA->m_linearVelocity.z;

			q_dt[ 3 ] = m_bodyB->m_linearVelocity.x;
			q_dt[ 4 ] = m_bodyB->m_linearVelocity.y;
			q_dt[ 5 ] = m_bodyB->m_linearVelocity.z;
		}

		VecN Q_ext( 6 );
		{
			// forces on body a
			Q_ext[ 0 ] = forceExternal.x;
			Q_ext[ 1 ] = forceExternal.y;
			Q_ext[ 2 ] = forceExternal.z;

			// forces on body b
			Q_ext[ 3 ] = forceExternal.x;
			Q_ext[ 4 ] = forceExternal.y;
			Q_ext[ 5 ] = forceExternal.z;
		}


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
			VecN rhs = ( J_dt * q_dt * -1.0f ) - Jacobian * invMassMatrix * Q_ext;

			// 			float k_s = 0.5f;
			// 			float k_d = 0.5f;
			// 			float C = r.DotProduct( r ) - radius * radius;
			// 			float C_dt = Jacobian.DotProduct( q_dt );
			// 			float baumgarte = k_s * C + k_d * C_dt;
			VecN k_s( 1 );
			k_s[ 0 ] = 0.5f;
			float k_d = 0.5f;
			float C = r.Dot( r ) - radius * radius;
			VecN C_dt = Jacobian * q_dt;
			VecN baumgarte = k_s * C + C_dt * k_d;
			rhs -= baumgarte;

			MatN tmpJWJt = J_W_Jt;

			VecN lambdaN = LCP_GaussSeidel( tmpJWJt, rhs );
			//			float lambdaN = ( rhs - baumgarte ) / J_W_Jt;

			VecN force = JacobianTranspose * lambdaN;
			forceInternalA[ 0 ] = force[ 0 ];
			forceInternalA[ 1 ] = force[ 1 ];
			forceInternalA[ 2 ] = force[ 2 ];

			forceInternalB[ 0 ] = force[ 3 ];
			forceInternalB[ 1 ] = force[ 4 ];
			forceInternalB[ 2 ] = force[ 5 ];
		}
	}

	printf( "%.2f %.2f %.2f,    %.2f %.2f %.2f,    %.2f %.2f %.2f,    %.2f %.2f %.2f\n",
		forceInternalA[ 0 ],	forceInternalA[ 1 ],	forceInternalA[ 2 ],
		torqueInternalA[ 0 ],	torqueInternalA[ 1 ],	torqueInternalA[ 2 ],
		forceInternalB[ 0 ],	forceInternalB[ 1 ],	forceInternalB[ 2 ],
		torqueInternalB[ 0 ],	torqueInternalB[ 1 ],	torqueInternalB[ 2 ] );

	Vec3 finalImpulseA = ( forceInternalA + forceExternal ) * dt_sec;
	m_bodyA->ApplyImpulse( m_bodyA->m_position, finalImpulseA );

	Vec3 finalImpulseB = ( forceInternalB + forceExternal ) * dt_sec;
	m_bodyB->ApplyImpulse( m_bodyB->m_position, finalImpulseB );
}

/*
========================================================================================================

ConstraintDistanceRigidBody

========================================================================================================
*/

/*
====================================================
ConstraintDistanceRigidBody::Solve
====================================================
*/
void ConstraintDistanceRigidBody::Solve() {
	const float dt_sec = 1.0f / 60.0f;
	if ( dt_sec <= 0.0f ) {
		return;
	}

	const Vec3 forceExternal = Vec3( 0, 0, -10 );// * mass;
	const float radius = 1.0f;

	// Get the world space position of the hinge from A's orientation
	const Vec3 worldAnchorA = m_bodyA->m_position;// m_bodyA->BodySpaceToWorldSpace( m_anchorA );

	// Get the world space position of the hinge from B's orientation
	const Vec3 worldAnchorB = m_bodyB->BodySpaceToWorldSpace( m_anchorB );

	const Vec3 r = worldAnchorB - worldAnchorA;
	//const Vec3 ra = worldAnchorA - m_bodyA->GetCenterOfMassWorldSpace();
	const Vec3 rb = worldAnchorB - m_bodyB->GetCenterOfMassWorldSpace();
	const Vec3 a = worldAnchorA;
	const Vec3 b = worldAnchorB;
	const Vec3 va = m_bodyA->m_linearVelocity;
	const Vec3 vb = m_bodyB->m_linearVelocity;
	const Vec3 wa = m_bodyA->m_angularVelocity;
	const Vec3 wb = m_bodyB->m_angularVelocity;

	Vec3 forceInternalA( 0.0f );
	Vec3 torqueInternalA( 0.0f );
	Vec3 forceInternalB( 0.0f );
	Vec3 torqueInternalB( 0.0f );
	{
		MatMN invMassMatrix( 6, 6 );
		{
			invMassMatrix.Zero();
			invMassMatrix.rows[ 0 ][ 0 ] = m_bodyB->m_invMass;
			invMassMatrix.rows[ 1 ][ 1 ] = m_bodyB->m_invMass;
			invMassMatrix.rows[ 2 ][ 2 ] = m_bodyB->m_invMass;

			Mat3 invInertiaTensorB = m_bodyB->m_shape->InertiaTensor().Inverse() * m_bodyB->m_invMass;
			for ( int i = 0; i < 3; i++ ) {
				invMassMatrix.rows[ 3 + i ][ 3 + 0 ] = invInertiaTensorB.rows[ i ][ 0 ];
				invMassMatrix.rows[ 3 + i ][ 3 + 1 ] = invInertiaTensorB.rows[ i ][ 1 ];
				invMassMatrix.rows[ 3 + i ][ 3 + 2 ] = invInertiaTensorB.rows[ i ][ 2 ];
			}
		}

		MatMN Jacobian( 1, 6 );
		{
			Jacobian.Zero();

			Vec3 J1 = ( b - a ) * 2.0f;
			Jacobian.rows[ 0 ][ 0 ] = J1.x;
			Jacobian.rows[ 0 ][ 1 ] = J1.y;
			Jacobian.rows[ 0 ][ 2 ] = J1.z;

			Vec3 J2 = rb.Cross( ( b - a ) * 2.0f );
			Jacobian.rows[ 0 ][ 3 ] = J2.x;
			Jacobian.rows[ 0 ][ 4 ] = J2.y;
			Jacobian.rows[ 0 ][ 5 ] = J2.z;
		}
		MatMN JacobianTranspose = Jacobian.Transpose();

		// Time derivative of the Jacobian
		MatMN J_dt( 1, 6 );
		{
			J_dt.Zero();
			
			Vec3 J1 = ( vb + wb.Cross( rb ) ) * 2.0f;
			J_dt.rows[ 0 ][ 0 ] = J1.x;
			J_dt.rows[ 0 ][ 1 ] = J1.y;
			J_dt.rows[ 0 ][ 2 ] = J1.z;

			Vec3 J2 = wb.Cross( rb ).Cross( ( b - a ) * 2.0f ) + rb.Cross( J1 );
			J_dt.rows[ 0 ][ 3 ] = J2.x;
			J_dt.rows[ 0 ][ 4 ] = J2.y;
			J_dt.rows[ 0 ][ 5 ] = J2.z;
		}

		VecN q_dt( 6 );
		{
			q_dt[ 0 ] = vb.x;
			q_dt[ 1 ] = vb.y;
			q_dt[ 2 ] = vb.z;

			q_dt[ 3 ] = wb.x;
			q_dt[ 4 ] = wb.y;
			q_dt[ 5 ] = wb.z;
		}

		VecN Q_ext( 6 );
		{
			Q_ext.Zero();

			// forces on body b
			Q_ext[ 0 ] = forceExternal.x;
			Q_ext[ 1 ] = forceExternal.y;
			Q_ext[ 2 ] = forceExternal.z;

			// torques on body b (maybe this is where we go wrong?  We don't properly apply the torque?)
// 			Vec3 torque = rb.Cross( forceExternal );	// torque from gravity
// 			//Vec3 torque = forceExternal.Cross( rb );
// 			Q_ext[ 3 ] = torque.x;
// 			Q_ext[ 4 ] = torque.y;
// 			Q_ext[ 5 ] = torque.z;
		}


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
			VecN rhs = ( J_dt * q_dt * -1.0f ) - Jacobian * invMassMatrix * Q_ext;

			// 			float k_s = 0.5f;
			// 			float k_d = 0.5f;
			// 			float C = r.DotProduct( r ) - radius * radius;
			// 			float C_dt = Jacobian.DotProduct( q_dt );
			// 			float baumgarte = k_s * C + k_d * C_dt;
			VecN k_s( 1 );
			k_s[ 0 ] = 0.5f;
			float k_d = 0.5f;
			float C = r.Dot( r );
			VecN C_dt = Jacobian * q_dt;
			VecN baumgarte = k_s * C + C_dt * k_d;
			rhs -= baumgarte;

			MatN tmpJWJt = J_W_Jt;

			VecN lambdaN = LCP_GaussSeidel( tmpJWJt, rhs );
			//			float lambdaN = ( rhs - baumgarte ) / J_W_Jt;

			VecN force = JacobianTranspose * lambdaN;
			forceInternalB[ 0 ] = force[ 0 ];
			forceInternalB[ 1 ] = force[ 1 ];
			forceInternalB[ 2 ] = force[ 2 ];

			torqueInternalB[ 0 ] = force[ 3 ];
			torqueInternalB[ 1 ] = force[ 4 ];
			torqueInternalB[ 2 ] = force[ 5 ];
		}
	}

	printf( "%.3f %.3f %.3f,    %.3f %.3f %.3f,    %.3f %.3f %.3f,    %.3f %.3f %.3f\n",
		forceInternalA[ 0 ],	forceInternalA[ 1 ],	forceInternalA[ 2 ],
		torqueInternalA[ 0 ],	torqueInternalA[ 1 ],	torqueInternalA[ 2 ],
		forceInternalB[ 0 ],	forceInternalB[ 1 ],	forceInternalB[ 2 ],
		torqueInternalB[ 0 ],	torqueInternalB[ 1 ],	torqueInternalB[ 2 ] );

// 	Vec3 finalImpulseA = ( forceInternalA + forceExternal ) * dt_sec;
// 	m_bodyA->ApplyImpulse( m_bodyA->m_position, finalImpulseA );

	Vec3 finalImpulseB = ( forceInternalB + forceExternal ) * dt_sec;
	//m_bodyB->ApplyImpulse( m_bodyB->m_position, finalImpulseB );
	//m_bodyB->ApplyImpulse( worldAnchorB, finalImpulseB );
	m_bodyB->ApplyImpulseLinear( finalImpulseB );
	m_bodyB->ApplyImpulseAngular( torqueInternalB * dt_sec );
}


/*
========================================================================================================

ConstraintDistanceRigidBody2

========================================================================================================
*/

/*
====================================================
ConstraintDistanceRigidBody2::PreSolve
====================================================
*/
void ConstraintDistanceRigidBody2::PreSolve( const float dt_sec ) {
	// Get the world space position of the hinge from A's orientation
	const Vec3 worldAnchorA = m_bodyA->BodySpaceToWorldSpace( m_anchorA );

	// Get the world space position of the hinge from B's orientation
	const Vec3 worldAnchorB = m_bodyB->BodySpaceToWorldSpace( m_anchorB );

	const Vec3 r = worldAnchorB - worldAnchorA;
	const Vec3 ra = worldAnchorA - m_bodyA->GetCenterOfMassWorldSpace();
	const Vec3 rb = worldAnchorB - m_bodyB->GetCenterOfMassWorldSpace();
	const Vec3 a = worldAnchorA;
	const Vec3 b = worldAnchorB;

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
ConstraintDistanceRigidBody2::Solve
====================================================
*/
void ConstraintDistanceRigidBody2::Solve() {
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
ConstraintDistanceRigidBody2::PostSolve
====================================================
*/
void ConstraintDistanceRigidBody2::PostSolve() {
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
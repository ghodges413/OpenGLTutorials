//
//  Constraints.cpp
//
#include "Physics/Constraints.h"
#include "Physics/Body.h"
#include "Math/lcp.h"

/*
========================================================================================================

ConstraintHinge

========================================================================================================
*/

// Take the incoming info and derive the constraint force
Vec3 DistanceConstraint( Vec3 origin, Vec3 & pos, Vec3 & velocity, float mass, float radius, Vec3 forceExternal ) {
	Vec3 r = pos - origin;	// The vector pointing from the constraint/origin to the particle position

	const float U = forceExternal.GetLengthSqr() * r.z;
	const float T = 0.5f * mass * velocity.Dot( velocity );
	const float E = T + U;
	//printf( "%.2f %.2f %.2f\n", U, T, E );

	// Ensure the position and velocity are valid
	{
		// Position constraint
		r.Normalize();
		r *= radius;

		pos = origin + r;

		// Velocity constraint
		// make sure the velocity is perpendicular to the position
		float t = r.Dot( velocity );
		velocity -= r * t;

		float hackFactor = 0.999f;
		velocity *= hackFactor;
	}

	// C = r * r - radius * radius = 0
	// C_dt = v * r = 0
	// C_dt2 = a * r + v * v = 0
	float v2 = velocity.Dot( velocity );
	float lambda = -1.0f * ( mass * v2 + r.Dot( forceExternal ) ) / r.Dot( r );
	Vec3 force = r * lambda;
	return force;
}

// Take the incoming info and derive the constraint force
Vec3 DistanceConstraint2( Vec3 origin, Vec3 & pos, Vec3 & velocity, float mass, float radius, Vec3 forceExternal ) {
	Vec3 r = pos - origin;	// The vector pointing from the constraint/origin to the particle position

	const float U = forceExternal.GetLengthSqr() * r.z;
	const float T = 0.5f * mass * velocity.Dot( velocity );
	const float E = T + U;
	//printf( "%.2f %.2f %.2f\n", U, T, E );

#if 0
	// Ensure the position and velocity are valid
	{
		// Position constraint
		r.Normalize();
		r *= radius;

		pos = origin + r;

		// Velocity constraint
		// make sure the velocity is perpendicular to the position
		float t = r.DotProduct( velocity );
		velocity -= r * t;

		float hackFactor = 0.999f;
		velocity *= hackFactor;
	}
#endif

	// C = r * r - radius * radius = 0
	// C_dt = v * r = 0
	// C_dt2 = a * r + v * v = 0

	VecN forceInternalN;
	{
		MatN massMatrix( 3 );
		massMatrix.Identity();
		massMatrix *= mass;

		MatN invMassMatrix( 3 );
		invMassMatrix.Identity();
		invMassMatrix *= 1.0f / mass;

		// q_dt2 = W * Q = invMassMatrix * Force
		// C = x^2 + y^2 + z^2 - r^2 = 0
		// C_dx = 2 * x
		// C_dy = 2 * y
		// C_dz = 2 * z
		// C( x, y , z )
		MatN Jacobian( 3 );
		Jacobian.Zero();
		Jacobian.rows[ 0 ][ 0 ] = 2.0f * r.x;
		Jacobian.rows[ 1 ][ 1 ] = 2.0f * r.y;
		Jacobian.rows[ 2 ][ 2 ] = 2.0f * r.z;
		//Vec3 Jacobian = Vec3( 2.0f * r.x, 2.0f * r.y, 2.0f * r.z );

		MatN JacobianTranspose( Jacobian );
		JacobianTranspose.Transpose();

		// Time derivative of the Jacobian
		MatN J_dt( 3 );
		J_dt.Zero();
		J_dt.rows[ 0 ][ 0 ] = 2.0f * velocity.x;
		J_dt.rows[ 1 ][ 1 ] = 2.0f * velocity.y;
		J_dt.rows[ 2 ][ 2 ] = 2.0f * velocity.z;

		VecN q_dt( 3 );
		q_dt[ 0 ] = velocity.x;
		q_dt[ 1 ] = velocity.y;
		q_dt[ 2 ] = velocity.z;

		VecN Q_ext( 3 );
		Q_ext[ 0 ] = forceExternal.x;
		Q_ext[ 1 ] = forceExternal.y;
		Q_ext[ 2 ] = forceExternal.z;


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
			MatN J_W_Jt( 3 );
			J_W_Jt = Jacobian * invMassMatrix * JacobianTranspose;

			VecN rhs( 3 );
			rhs = ( J_dt * q_dt * -1.0f ) - Jacobian * invMassMatrix * Q_ext;

			VecN lambdaN = LCP_GaussSeidel( J_W_Jt, rhs );

			forceInternalN = JacobianTranspose * lambdaN;
		}
	}

	Vec3 forceInternal3;
	{
		Mat3 massMatrix;
		massMatrix.Identity();
		massMatrix *= mass;

		Mat3 invMassMatrix;
		invMassMatrix.Identity();
		invMassMatrix *= 1.0f / mass;

		// q_dt2 = W * Q = invMassMatrix * Force
		// C = x^2 + y^2 + z^2 - r^2 = 0
		// C_dx = 2 * x
		// C_dy = 2 * y
		// C_dz = 2 * z
		// C( x, y , z )
		Mat3 Jacobian;
		Jacobian.Zero();
		Jacobian.rows[ 0 ][ 0 ] = 2.0f * r.x;
		Jacobian.rows[ 1 ][ 1 ] = 2.0f * r.y;
		Jacobian.rows[ 2 ][ 2 ] = 2.0f * r.z;
		//Vec3 Jacobian = Vec3( 2.0f * r.x, 2.0f * r.y, 2.0f * r.z );

		Mat3 JacobianTranspose( Jacobian );
		JacobianTranspose.Transpose();

		// Time derivative of the Jacobian
		Mat3 J_dt;
		J_dt.Zero();
		J_dt.rows[ 0 ][ 0 ] = 2.0f * velocity.x;
		J_dt.rows[ 1 ][ 1 ] = 2.0f * velocity.y;
		J_dt.rows[ 2 ][ 2 ] = 2.0f * velocity.z;

		Vec3 q_dt;
		q_dt[ 0 ] = velocity.x;
		q_dt[ 1 ] = velocity.y;
		q_dt[ 2 ] = velocity.z;

		Vec3 Q_ext;
		Q_ext[ 0 ] = forceExternal.x;
		Q_ext[ 1 ] = forceExternal.y;
		Q_ext[ 2 ] = forceExternal.z;


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
			Mat3 J_W_Jt = Jacobian * invMassMatrix * JacobianTranspose;
			Vec3 rhs = ( J_dt * q_dt * -1.0f );// - Jacobian * invMassMatrix * Q_ext;

			Vec3 lambdaN = LCP_GaussSeidel( J_W_Jt, rhs );

			forceInternal3 = JacobianTranspose * lambdaN;
		}
	}

	Vec3 forceInternal3b;
	{
		Mat3 massMatrix;
		massMatrix.Identity();
		massMatrix *= mass;

		Mat3 invMassMatrix;
		invMassMatrix.Identity();
		invMassMatrix *= 1.0f / mass;

		// q_dt2 = W * Q = invMassMatrix * Force
		// C = x^2 + y^2 + z^2 - r^2 = 0
		// C_dx = 2 * x
		// C_dy = 2 * y
		// C_dz = 2 * z
		// C( x, y , z )
// 		Mat3 Jacobian;
// 		Jacobian.Zero();
// 		Jacobian.rows[ 0 ][ 0 ] = 2.0f * r.x;
// 		Jacobian.rows[ 1 ][ 1 ] = 2.0f * r.y;
// 		Jacobian.rows[ 2 ][ 2 ] = 2.0f * r.z;
		Vec3 Jacobian = Vec3( 2.0f * r.x, 2.0f * r.y, 2.0f * r.z );

// 		Mat3 JacobianTranspose( Jacobian );
// 		JacobianTranspose.Transpose();

		// Time derivative of the Jacobian
// 		Mat3 J_dt;
// 		J_dt.Zero();
// 		J_dt.rows[ 0 ][ 0 ] = 2.0f * velocity.x;
// 		J_dt.rows[ 1 ][ 1 ] = 2.0f * velocity.y;
// 		J_dt.rows[ 2 ][ 2 ] = 2.0f * velocity.z;
		Vec3 J_dt = Vec3( 2.0f * velocity.x, 2.0f * velocity.y, 2.0f * velocity.z );

		Vec3 q_dt;
		q_dt[ 0 ] = velocity.x;
		q_dt[ 1 ] = velocity.y;
		q_dt[ 2 ] = velocity.z;

		Vec3 Q_ext;
		Q_ext[ 0 ] = forceExternal.x;
		Q_ext[ 1 ] = forceExternal.y;
		Q_ext[ 2 ] = forceExternal.z;


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
			Vec3 lambda0( 0.0f );

			float J_W_Jt = Jacobian.Dot( invMassMatrix * Jacobian );
			float rhs = ( J_dt.Dot( q_dt ) * -1.0f ) - Jacobian.Dot( invMassMatrix * Q_ext );

			float k_s = 0.5f;
			float k_d = 0.5f;
			float C = r.Dot( r ) - radius * radius;
			float C_dt = Jacobian.Dot( q_dt );
			float baumgarte = k_s * C + k_d * C_dt;

			const int maxIterations = 10;
			//Vec3 lambdaN = LCP_GaussSeidel( lambda0, J_W_Jt, rhs, maxIterations );
			float lambdaN = ( rhs - baumgarte ) / J_W_Jt;

			forceInternal3b = Jacobian * lambdaN;
		}
	}

	Vec3 forceInternalMN;
	{
		MatMN massMatrix( 3, 3 );
		massMatrix.Zero();
		massMatrix.rows[ 0 ][ 0 ] = mass;
		massMatrix.rows[ 1 ][ 1 ] = mass;
		massMatrix.rows[ 2 ][ 2 ] = mass;

		MatMN invMassMatrix( 3, 3 );
		invMassMatrix.Zero();
		invMassMatrix.rows[ 0 ][ 0 ] = 1.0f / mass;
		invMassMatrix.rows[ 1 ][ 1 ] = 1.0f / mass;
		invMassMatrix.rows[ 2 ][ 2 ] = 1.0f / mass;

		// q_dt2 = W * Q = invMassMatrix * Force
		// C = x^2 + y^2 + z^2 - r^2 = 0
		// C_dx = 2 * x
		// C_dy = 2 * y
		// C_dz = 2 * z
		// C( x, y , z )
		MatMN Jacobian( 1, 3 );
		Jacobian.Zero();
		Jacobian.rows[ 0 ][ 0 ] = 2.0f * r.x;
		Jacobian.rows[ 0 ][ 1 ] = 2.0f * r.y;
		Jacobian.rows[ 0 ][ 2 ] = 2.0f * r.z;
		MatMN JacobianTranspose = Jacobian.Transpose();

		// Time derivative of the Jacobian
		MatMN J_dt( 1, 3 );
		J_dt.Zero();
		J_dt.rows[ 0 ][ 0 ] = 2.0f * velocity.x;
		J_dt.rows[ 0 ][ 1 ] = 2.0f * velocity.y;
		J_dt.rows[ 0 ][ 2 ] = 2.0f * velocity.z;

		VecN q_dt( 3 );
		q_dt[ 0 ] = velocity.x;
		q_dt[ 1 ] = velocity.y;
		q_dt[ 2 ] = velocity.z;

		VecN Q_ext( 3 );
		Q_ext[ 0 ] = forceExternal.x;
		Q_ext[ 1 ] = forceExternal.y;
		Q_ext[ 2 ] = forceExternal.z;


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
			forceInternalMN[ 0 ] = force[ 0 ];
			forceInternalMN[ 1 ] = force[ 1 ];
			forceInternalMN[ 2 ] = force[ 2 ];
		}
	}

	float v2 = velocity.Dot( velocity );
	float lambda = -1.0f * ( mass * v2 + r.Dot( forceExternal ) ) / r.Dot( r );
	Vec3 force = r * lambda;
// 	printf( "%.2f %.2f %.2f,    %.2f %.2f %.2f,    %.2f %.2f %.2f,    %.2f %.2f %.2f,    %.2f %.2f %.2f\n",
// 		force[ 0 ], force[ 1 ], force[ 2 ],
// 		forceInternalN[ 0 ], forceInternalN[ 1 ], forceInternalN[ 2 ],
// 		forceInternal3[ 0 ], forceInternal3[ 1 ], forceInternal3[ 2 ],
// 		forceInternal3b[ 0 ], forceInternal3b[ 1 ], forceInternal3b[ 2 ],
// 		forceInternalMN[ 0 ], forceInternalMN[ 1 ], forceInternalMN[ 2 ] );
	//return force;
	return forceInternalMN;
}


// Take the incoming info and derive the constraint force
Vec3 DistanceConstraint3( Vec3 origin, Body * body, float radius, Vec3 forceExternal ) {
	Vec3 r = body->m_position - origin;	// The vector pointing from the constraint/origin to the particle position
	const float mass = body->m_invMass;
	const Vec3 velocity = body->m_linearVelocity;
	const Vec3 pos = body->m_position;
	Quat & orient = body->m_orientation;

	const float U = forceExternal.GetLengthSqr() * r.z;
	const float T = 0.5f * mass * velocity.Dot( velocity );
	const float E = T + U;
	//printf( "%.2f %.2f %.2f\n", U, T, E );

// 	Vec3 r = orient.RotatePoint( Vec3( 0, 0, radius ) );
// 	Vec3 d = origin - r;

	// C = d * d = 0
	// = ( o - r ) * ( o - r )
	// = o * ( o - r ) - r * ( o - r )
	// dC/dt = do/dt * ( o - r ) + o * ( do/dt - dr/dt ) - dr/dt * ( do/dt - dr/dt ), do/dt = 0
	// = o * ( - dr/dt ) - dr/dt * ( - dr/dt )
	// = - o * dr/dt + dr/dt * dr/dt
	// = -o * ( v + ( w x r ) ) + ( v + ( w x r ) ) * ( v + ( w x r ) )
	// = -o * v + o * ( w x r ) + v * v + v * ( w x r ) + ( w x r ) * v + ( w x r ) * ( w x r )

	// a * ( b x c ) = c * ( a x b ) = b * ( c x a )
	// r * ( w x r ) = r * ( r x w ) = w * ( r x r ) = 0

#if 0
	// Ensure the position and velocity are valid
	{
		// Position constraint
		r.Normalize();
		r *= radius;

		pos = origin + r;

		// Velocity constraint
		// make sure the velocity is perpendicular to the position
		float t = r.DotProduct( velocity );
		velocity -= r * t;

		float hackFactor = 0.999f;
		velocity *= hackFactor;
	}
#endif

	// Ensure the orientation is correct
	{
		// Also maybe figure out the torque required?

		// Get the "up vector"
		Vec3 up = origin - pos;
		up.Normalize();

		// Figure out what orientation would make a Vec3( 0, 0, 1 ) move to this orientation
		Vec3 cross = Vec3( 0, 0, 1 ).Cross( up );
		float angle = cross.GetMagnitude();
		cross.Normalize();
		Quat newQuat = Quat( cross, angle );

		// set the body orientation to the appropriate orientaint-yum
		orient = newQuat;

		// Okay, we have the "essence" of our orientation constraint working here
		// This means we should be able to figure out the analytical description of
		// this in the form of C = 0.
		// The "up" direction of the body and the normalized vector from body position
		// to joint location must dot product 1.

// 		Vec3 wr = ( pos - origin ).Cross( velocity );
// 		angularVelocity = wr;
// 
// 		Vec3 aplhar = ( pos - origin ).Cross( Vec3( 0, 0, -10 ) );

		// So let's just do it in the vector form right now
		// the constraint it up.dot( r ) = 1
		// => C = r * up - 1 = 0
		// Now, can we figure out how to describe this in quaternion form?
		// dumbass, our answer was right there
		// q1 = qr
		// => C = q1 - qr = 0
		// C_dt = 0.5 * q1 * w1 - 0.5 * qr * wr = 0
		// w1 is the angular velocity of the body around its center of mass
		// wr is the angular velocity of the cm about the joint
		// q1 * w1 = qr * wr
		// w1 = q1_inv * qr * wr
		//
		// C_dt2 = 0.25 * q1 * w1 * w1 + 0.5 * q1 * alpha1 - 0.25 * qr * wr * wr - 0.5 * qr * alphar = 0
		// => 0.5 * q1 * w1 * w1 + q1 * alpha1 - 0.5 * qr * wr * wr - qr * alphar = 0
		// => q1 * alpha1 = 0.5 * qr * wr * wr + qr * alphar - 0.5 * q1 * w1 * w1
		// => alpha1 = q1_inv * ( 0.5 * qr * wr * wr + qr * alphar - 0.5 * q1 * w1 * w1 )
		// Torque = I * alpha


		// Note: wr = r x v
		// Note: alphar = r x a
	}

	// C = r * r - radius * radius = 0
	// C_dt = dr/dt * r + r * dr/dt
	// = ( v + w x r ) * r + r * ( v + w x r )
	// = 2 * r * ( v + w x r )
	// = 2 * r * v + 2 * r * ( w x r )
	// C_dt = v * r = 0
	// C_dt2 = a * r + v * v = 0

	Vec3 forceInternalMN;
	{
		MatMN invMassMatrix( 3, 3 );
		invMassMatrix.Zero();
		invMassMatrix.rows[ 0 ][ 0 ] = 1.0f / mass;
		invMassMatrix.rows[ 1 ][ 1 ] = 1.0f / mass;
		invMassMatrix.rows[ 2 ][ 2 ] = 1.0f / mass;

		// q_dt2 = W * Q = invMassMatrix * Force
		// C = x^2 + y^2 + z^2 - r^2 = 0
		// C_dx = 2 * x
		// C_dy = 2 * y
		// C_dz = 2 * z
		// C( x, y , z )
		MatMN Jacobian( 1, 3 );
		Jacobian.Zero();
		Jacobian.rows[ 0 ][ 0 ] = 2.0f * r.x;
		Jacobian.rows[ 0 ][ 1 ] = 2.0f * r.y;
		Jacobian.rows[ 0 ][ 2 ] = 2.0f * r.z;
		MatMN JacobianTranspose = Jacobian.Transpose();

		// Time derivative of the Jacobian
		MatMN J_dt( 1, 3 );
		J_dt.Zero();
		J_dt.rows[ 0 ][ 0 ] = 2.0f * velocity.x;
		J_dt.rows[ 0 ][ 1 ] = 2.0f * velocity.y;
		J_dt.rows[ 0 ][ 2 ] = 2.0f * velocity.z;

		VecN q_dt( 3 );
		q_dt[ 0 ] = velocity.x;
		q_dt[ 1 ] = velocity.y;
		q_dt[ 2 ] = velocity.z;

		VecN Q_ext( 3 );
		Q_ext[ 0 ] = forceExternal.x;
		Q_ext[ 1 ] = forceExternal.y;
		Q_ext[ 2 ] = forceExternal.z;


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
			forceInternalMN[ 0 ] = force[ 0 ];
			forceInternalMN[ 1 ] = force[ 1 ];
			forceInternalMN[ 2 ] = force[ 2 ];
		}
	}

	float v2 = velocity.Dot( velocity );
	float lambda = -1.0f * ( mass * v2 + r.Dot( forceExternal ) ) / r.Dot( r );
	Vec3 force = r * lambda;
	//printf( "%.2f %.2f %.2f\n", force[ 0 ], force[ 1 ], force[ 2 ] );
	//return force;
	return forceInternalMN;
}

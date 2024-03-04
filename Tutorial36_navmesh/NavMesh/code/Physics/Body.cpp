//
//  Body.cpp
//
#include "Physics/Body.h"
#define MAX_ANGULAR_SPEED 30.0f

const bodyID_t bodyID_invalid;

/*
====================================================
Body::Body
====================================================
*/
Body::Body() {
	Reset();
}

/*
====================================================
Body::Reset
====================================================
*/
void Body::Reset() {
	m_position = Vec3( 0.0f );
	m_orientation = Quat( 0.0f, 0.0f, 0.0f, 1.0f );
	m_linearVelocity = Vec3( 0.0f );
	m_angularVelocity = Vec3( 0.0f );
	m_invMass = 0.0f;
	m_elasticity = 0.0f;
	m_friction = 0.0f;
	m_shape = NULL;

	m_enableRotation = true;
	m_enableGravity = true;
	m_bodyContents = BC_GENERIC;
	m_collidesWith = BC_ALL;
	m_isUsed = false;
	m_owner = NULL;
	m_callbacks = NULL;
	m_callbacks2 = NULL;
}

/*
====================================================
Body::GetCenterOfMassWorldSpace
====================================================
*/
Vec3 Body::GetCenterOfMassWorldSpace() const {
	const Vec3 centerOfMass = m_shape->GetCenterOfMass();
	const Vec3 pos = m_position + m_orientation.RotatePoint( centerOfMass );
	return pos;
}

/*
====================================================
Body::GetCenterOfMassModelSpace
====================================================
*/
Vec3 Body::GetCenterOfMassModelSpace() const {
	const Vec3 centerOfMass = m_shape->GetCenterOfMass();
	return centerOfMass;
}

/*
====================================================
Body::WorldSpaceToBodySpace
====================================================
*/
Vec3 Body::WorldSpaceToBodySpace( const Vec3 & worldPt ) const {
	Vec3 tmp			= worldPt - GetCenterOfMassWorldSpace();
	Quat inverseOrient	= m_orientation.Inverse();
	Vec3 bodySpace		= inverseOrient.RotatePoint( tmp );
	return bodySpace;
}

/*
====================================================
Body::BodySpaceToWorldSpace
====================================================
*/
Vec3 Body::BodySpaceToWorldSpace( const Vec3 & worldPt ) const {
	Vec3 worldSpace = GetCenterOfMassWorldSpace() + m_orientation.RotatePoint( worldPt );
	return worldSpace;
}

/*
====================================================
Body::GetInverseInertiaTensorBodySpace
====================================================
*/
Mat3 Body::GetInverseInertiaTensorBodySpace() const {
	Mat3 inertiaTensor		= m_shape->InertiaTensor();
	Mat3 invInertiaTensor	= inertiaTensor.Inverse() * m_invMass;
	return invInertiaTensor;
}

/*
====================================================
Body::GetInverseInertiaTensorWorldSpace
====================================================
*/
Mat3 Body::GetInverseInertiaTensorWorldSpace() const {
	Mat3 inertiaTensor		= m_shape->InertiaTensor();
	Mat3 invInertiaTensor	= inertiaTensor.Inverse() * m_invMass;
	Mat3 orient				= m_orientation.ToMat3();
	invInertiaTensor		= orient * invInertiaTensor * orient.Transpose();
	return invInertiaTensor;
}

/*
====================================================
Body::ApplyImpulse
====================================================
*/
void Body::ApplyImpulse( const Vec3 & impulsePoint, const Vec3 & impulse ) {
	if ( 0.0f == m_invMass ) {
		return;
	}

	// impulsePoint is the world space location of the application of the impulse
	// impulse is the world space direction and magnitude of the impulse
	ApplyImpulseLinear( impulse );

	Vec3 position = GetCenterOfMassWorldSpace();	// applying impulses must produce torques through the center of mass
	Vec3 r = impulsePoint - position;
	Vec3 dL = r.Cross( impulse );	// this is in world space
	ApplyImpulseAngular( dL );
}

/*
====================================================
Body::ApplyImpulseLinear
====================================================
*/
void Body::ApplyImpulseLinear( const Vec3 & impulse ) {
	if ( 0.0f == m_invMass ) {
		return;
	}

	// p = mv
	// dp = m dv = J
	// => dv = J / m
	m_linearVelocity += impulse * m_invMass;
}

/*
====================================================
Body::ApplyImpulseAngular
====================================================
*/
void Body::ApplyImpulseAngular( const Vec3 & impulse ) {
	if ( 0.0f == m_invMass || !m_enableRotation ) {
		return;
	}

	// L = I w = r x p
	// dL = I dw = r x J 
	// => dw = I^-1 * ( r x J )
	Mat3 orientation				= m_orientation.ToMat3();
	Mat3 transposeOrient			= orientation.Transpose();
	Mat3 inertiaTensor				= m_shape->InertiaTensor();
	Mat3 modelSpaceInvInertiaTensor	= inertiaTensor.Inverse();

	Vec3 dw = orientation * modelSpaceInvInertiaTensor * transposeOrient * impulse * m_invMass;

	m_angularVelocity += dw;
	if ( m_angularVelocity.GetLengthSqr() > MAX_ANGULAR_SPEED * MAX_ANGULAR_SPEED ) {
		m_angularVelocity.Normalize();
		m_angularVelocity *= MAX_ANGULAR_SPEED;
	}
}

/*
====================================================
Body::Update
====================================================
*/
void Body::Update( const float dt_sec ) {
	m_position += m_linearVelocity * dt_sec;

	if ( m_enableRotation ) {
		// okay, we have an angular velocity around the center of mass, this needs to be
		// converted somehow to relative to model position.  This way we can properly update
		// the orientation of the model.
		Vec3 positionCM = GetCenterOfMassWorldSpace();
		Vec3 cmToPos = m_position - positionCM;

		// Total Torque is equal to external applied torques + internal torque (precession)
		// T = T_external + omega x I * omega
		// T_external = 0 because it was applied in the collision response function
		// T = Ia = w x I * w
		// a = I^-1 ( w x I * w )
		Mat3 orientation = m_orientation.ToMat3();
		Mat3 inertiaTensor = orientation * m_shape->InertiaTensor() * orientation.Transpose();
		Vec3 alpha = inertiaTensor.Inverse() * ( m_angularVelocity.Cross( inertiaTensor * m_angularVelocity ) );
		m_angularVelocity += alpha * dt_sec;

		// Update orientation
		Vec3 dAngle = m_angularVelocity * dt_sec;
		Quat dq = Quat( dAngle, dAngle.GetMagnitude() );
		m_orientation = dq * m_orientation;
		m_orientation.Normalize();

		// Now get the new model position
		m_position = positionCM + dq.RotatePoint( cmToPos );
	}
}
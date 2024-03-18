//
//	Body.h
//
#pragma once
#include "Math/Vector.h"
#include "Math/Quat.h"
#include "Math/Matrix.h"
#include "Math/Bounds.h"
#include "Physics/Shapes.h"
#include "Physics/Contact.h"

class Body;

/*
====================================================
bodyID_t
====================================================
*/
struct bodyID_t {
public:
	bodyID_t() : id( -1 ), body( NULL ) {}
	int id;
	Body * body;
};

extern const bodyID_t bodyID_invalid;


/*
====================================================
bodyContents_t
====================================================
*/
enum bodyContents_t {
	BC_NONE			= 0,
	BC_GENERIC		= 1 << 0,	// Catch all type for things like ground
	BC_PLAYER		= 1 << 1,
	BC_ENEMY		= 1 << 2,
	BC_PROJECTILE	= 1 << 3,
	BC_BLOOD		= 1 << 4,

	BC_ALL = BC_GENERIC | BC_PLAYER | BC_ENEMY | BC_PROJECTILE | BC_BLOOD,
};

/*
====================================================
PhysicsCallbacks_t
====================================================
*/
struct PhysicsCallbacks_t {
//typedef void OnContactFunction( bodyID_t a, bodyID_t b, contact_t contact );
//OnContactFunction * OnContact;

	PhysicsCallbacks_t() {
		m_owner = NULL;

		OnContact = NULL;
		OnFilter = NULL;
	}
	void (*OnContact)( void * owner, const contact_t & contact );
	bool (*OnFilter)( void * owner, const Body * bodyA, const Body * bodyB );

	void * m_owner;
};

/*
====================================================
PhysicsCallbacks2
====================================================
*/
class PhysicsCallbacks2 {
public:
	virtual void OnContact( const contact_t & contact ) {}
	virtual bool OnFilter( const Body * bodyA, const Body * bodyB ) {}
};

/*
====================================================
Body
====================================================
*/
class Body {
public:
	Body();

	Vec3		m_position;
	Quat		m_orientation;		// orientation is relative to the model position
	Vec3		m_linearVelocity;
	Vec3		m_angularVelocity;	// this needs to be the angular velocity through the center of mass

	float		m_invMass;
	float		m_elasticity;
	float		m_friction;
	Shape *		m_shape;

	bool		m_enableRotation;
	bool		m_enableGravity;

	unsigned int m_bodyContents;	// bit flag of what this body contains
	unsigned int m_collidesWith;	// big flag of what this body collides with

	void *					m_owner;		// pointer to the owner entity
	PhysicsCallbacks_t *	m_callbacks;	// pointer to function callbacks
	PhysicsCallbacks2 *		m_callbacks2;	// pointer to function callbacks

	Vec3 GetCenterOfMassWorldSpace() const;	// returns the center of mass in world space
	Vec3 GetCenterOfMassModelSpace() const;	// returns the center of mass in model space

	Vec3 WorldSpaceToBodySpace( const Vec3 & pt ) const;
	Vec3 BodySpaceToWorldSpace( const Vec3 & pt ) const;

	Mat3 GetInverseInertiaTensorBodySpace() const;
	Mat3 GetInverseInertiaTensorWorldSpace() const;

	void ApplyImpulse( const Vec3 & impulsePoint, const Vec3 & impulse );
	void ApplyImpulseLinear( const Vec3 & impulse );
	void ApplyImpulseAngular( const Vec3 & impulse );

	void Update( const float dt_sec );

	float GetKineticEnergyLinear() const;
	float GetKineticEnergyAngular() const;

	bool IsUsed() const { return m_isUsed; }

	Bounds GetBounds() const;
	Bounds GetBounds( float dt_sec ) const;

private:
	bool m_isUsed;
	void Reset();

	friend class PhysicsWorld;
};

/*
====================================================
Body::GetKineticEnergyLinear
====================================================
*/
inline float Body::GetKineticEnergyLinear() const {
	if ( 0.0f == m_invMass ) {
		return 0.0f;
	}

	float energy = m_linearVelocity.Dot( m_linearVelocity ) / ( 2.0f * m_invMass );
	return energy;
}

/*
====================================================
Body::GetKineticEnergyAngular
====================================================
*/
inline float Body::GetKineticEnergyAngular() const {
	if ( 0.0f == m_invMass ) {
		return 0.0f;
	}

	Mat3 orientation = m_orientation.ToMat3();
	Mat3 inertiaTensor = orientation * m_shape->InertiaTensor() * orientation.Transpose();
	float energy = m_angularVelocity.Dot( inertiaTensor * m_angularVelocity ) / ( 2.0f * m_invMass );
	return energy;
}

/*
====================================================
Body::GetBounds
====================================================
*/
inline Bounds Body::GetBounds() const {
	return m_shape->GetBounds( m_position, m_orientation );
}

/*
====================================================
Body::GetBounds
====================================================
*/
inline Bounds Body::GetBounds( float dt_sec ) const {
	Bounds bounds = m_shape->GetBounds( m_position, m_orientation );

	// Expand the bounds by the linear velocity
	bounds.Expand( bounds.mins + m_linearVelocity * dt_sec );
	bounds.Expand( bounds.maxs + m_linearVelocity * dt_sec );

	return bounds;
}
//
//	Contact.h
//
#pragma once
#include "Math/Vector.h"

class Body;

/*
====================================================
Contact
====================================================
*/
struct contact_t {
	contact_t() : separationDistance( 0.0f ), timeOfImpact( -1.0f ), bodyA( NULL ), bodyB( NULL ) {}

	Vec3 ptOnA_WorldSpace;
	Vec3 ptOnB_WorldSpace;
	Vec3 ptOnA_LocalSpace;
	Vec3 ptOnB_LocalSpace;

	Vec3 normal;				// In World Space coordinates
	float separationDistance;	// positive when non-penetrating, negative when penitrode
	float timeOfImpact;

	Body * bodyA;
	Body * bodyB;

	contact_t( const contact_t & rhs ) {
		ptOnA_WorldSpace = rhs.ptOnA_WorldSpace;
		ptOnB_WorldSpace = rhs.ptOnB_WorldSpace;
		ptOnA_LocalSpace = rhs.ptOnA_LocalSpace;
		ptOnB_LocalSpace = rhs.ptOnB_LocalSpace;

		normal = rhs.normal;
		separationDistance = rhs.separationDistance;
		timeOfImpact = rhs.timeOfImpact;

		bodyA = rhs.bodyA;
		bodyB = rhs.bodyB;
	}

	const contact_t & operator = ( const contact_t & rhs ) {
		ptOnA_WorldSpace = rhs.ptOnA_WorldSpace;
		ptOnB_WorldSpace = rhs.ptOnB_WorldSpace;
		ptOnA_LocalSpace = rhs.ptOnA_LocalSpace;
		ptOnB_LocalSpace = rhs.ptOnB_LocalSpace;

		normal = rhs.normal;
		separationDistance = rhs.separationDistance;
		timeOfImpact = rhs.timeOfImpact;

		bodyA = rhs.bodyA;
		bodyB = rhs.bodyB;
		return *this;
	}
};

int CompareContacts( const void * p1, const void * p2 );
void ResolveContact( contact_t & contact );
//
//  PhysicsWorld.cpp
//
#include "Physics/PhysicsWorld.h"
#include "Physics/Intersections.h"
#include "Physics/BroadPhase.h"

PhysicsWorld * g_physicsWorld = NULL;

/*
====================================================
PhysicsWorld::PhysicsWorld
====================================================
*/
PhysicsWorld::PhysicsWorld() {
	Reset();	
}
PhysicsWorld::~PhysicsWorld() {
	Reset();	
}

/*
====================================================
PhysicsWorld::PhysicsWorld
====================================================
*/
void PhysicsWorld::Reset() {
	// Delete constraints
// 	for ( int i = 0; i < m_constraints.size(); i++ ) {
// 		delete m_constraints[ i ];
// 	}
	m_constraints.clear();

	// Initialize the free pool linked list
	m_freeNodes = &m_bodyPoolNodes[ 0 ];
	for ( int i = 0; i < m_maxBodies; i++ ) {
		m_bodyPool[ i ].Reset();

		m_bodyPoolNodes[ i ].bodyID = i;
		m_bodyPoolNodes[ i ].m_next = NULL;

		int nextIdx = i + 1;
		if ( nextIdx < m_maxBodies ) {
			m_bodyPoolNodes[ i ].m_next = &m_bodyPoolNodes[ nextIdx ];
		}
	}

	// Initialize the used node linked list
	m_usedNodes = NULL;
	m_numUsedBodies = 0;
}

/*
====================================================
PhysicsWorld::AllocateBody
====================================================
*/
bodyID_t PhysicsWorld::AllocateBody( Body body ) {
	if ( NULL == m_freeNodes ) {
		printf( "WARNING: Cannot allocate another body, max bodies has been reached\n" );
		return bodyID_invalid;
	}

	// Increment the number of used bodies
	++m_numUsedBodies;

	// Unlink the first free node
	BodyPoolNode_t * node = m_freeNodes;
	m_freeNodes = m_freeNodes->m_next;

	// Link this node into the front of the used list
	node->m_next = m_usedNodes;
	m_usedNodes = node;

	bodyID_t bodyid;
	bodyid.id = node->bodyID;
	bodyid.body = GetBody( bodyid.id );
	*bodyid.body = body;
	bodyid.body->m_isUsed = true;
	return bodyid;
}

/*
====================================================
PhysicsWorld::GetBody
====================================================
*/
Body * PhysicsWorld::GetBody( const int bodyID ) {
	if ( bodyID < 0 || bodyID >= m_maxBodies ) {
		printf( "WARNING: Attempting to get body with invalid bodyID %i\n", bodyID );
		return NULL;
	}

	return &m_bodyPool[ bodyID ];
}

/*
====================================================
PhysicsWorld::GetBody
====================================================
*/
const Body * PhysicsWorld::GetBody( const int bodyID ) const {
	if ( bodyID < 0 || bodyID >= m_maxBodies ) {
		printf( "WARNING: Attempting to get body with invalid bodyID %i\n", bodyID );
		return NULL;
	}

	return &m_bodyPool[ bodyID ];
}

/*
====================================================
PhysicsWorld::GetAllocatedBodyIDs
====================================================
*/
void PhysicsWorld::GetAllocatedBodyIDs( std::vector< int > & bodyIds ) const {
	bodyIds.clear();

	BodyPoolNode_t * node = m_usedNodes;
	while ( NULL != node ) {
		bodyIds.push_back( node->bodyID );
		node = node->m_next;
	}
}

/*
====================================================
PhysicsWorld::FreeBody
====================================================
*/
void PhysicsWorld::FreeBody( const int bodyID ) {
	if ( bodyID < 0 || bodyID >= m_maxBodies ) {
		printf( "WARNING: Attempting to free body with invalid bodyID %i\n", bodyID );
		return;
	}

	if ( NULL == m_usedNodes ) {
		printf( "WARNING: Attempting to double free bodyID %i\n", bodyID );
		return;
	}

	BodyPoolNode_t * prev = NULL;
	BodyPoolNode_t * node = m_usedNodes;

	// Flag the body as unused so that any manifolds will clear themselves
	m_bodyPool[ bodyID ].Reset();

	// If the head is the node to be removed, then we need to move the head
	if ( bodyID == m_usedNodes->bodyID ) {
		m_usedNodes = node->m_next;

		// Link node into the front of the free list
		node->m_next = m_freeNodes;
		m_freeNodes = node;

		// Decrement the number of used bodies
		--m_numUsedBodies;
		return;
	}

	// Move to the next node, since the head has already been checked
	prev = node;
	node = node->m_next;

	while ( NULL != node ) {
		if ( bodyID == node->bodyID ) {
			// Unlink from the used list
			prev->m_next = node->m_next;

			// Link node into the front of the free list
			node->m_next = m_freeNodes;
			m_freeNodes = node;

			// Decrement the number of used bodies
			--m_numUsedBodies;
			break;
		}

		prev = node;
		node = node->m_next;
	}

//#define VALIDATE_BODIES	// uncomment to validate bodies on free
#if defined( VALIDATE_BODIES )
	prev = NULL;
	node = m_usedNodes;
	while ( NULL != node ) {
		int id = node->bodyID;
		Body * body = &m_bodyPool[ id ];
		if ( NULL == body->m_shape ) {
			printf( "ruh roh\n" );
			assert( 0 );
		}

		prev = node;
		node = node->m_next;
	}
#endif
}

/*
====================================================
PhysicsWorld::RegisterConstraint
====================================================
*/
void PhysicsWorld::RegisterConstraint( Constraint * constraint ) {
	m_constraints.push_back( constraint );
}

/*
====================================================
PhysicsWorld::UnRegisterConstraint
====================================================
*/
void PhysicsWorld::UnRegisterConstraint( Constraint * constraint ) {
	for ( int i = 0; i < m_constraints.size(); i++ ) {
		if ( constraint == m_constraints[ i ] ) {
			m_constraints.erase( m_constraints.begin() + i );
			break;
		}
	}
}

/*
====================================================
PhysicsWorld::UpdateBodies
====================================================
*/
void PhysicsWorld::UpdateBodies( const float dt_sec ) {
	BodyPoolNode_t * node = m_usedNodes;

	while ( NULL != node ) {
		m_bodyPool[ node->bodyID ].Update( dt_sec );
		node = node->m_next;
	}
}

/*
====================================================
PhysicsWorld::ApplyGravity
====================================================
*/
void PhysicsWorld::ApplyGravity( const float dt_sec ) {
	BodyPoolNode_t * node = m_usedNodes;

	while ( NULL != node ) {
		Body * body = &m_bodyPool[ node->bodyID ];

		float mass = 1.0f / body->m_invMass;
		Vec3 impulseGravity = Vec3( 0, 0, -10 ) * mass * dt_sec;
		if ( body->m_enableGravity ) {
			body->ApplyImpulseLinear( impulseGravity );
		}

		node = node->m_next;
	}
}

/*
====================================================
PhysicsWorld::FilterPair
====================================================
*/
bool PhysicsWorld::FilterPair( Body * bodyA, Body * bodyB ) {
	// Function callbacks for OnContact
	if ( NULL != bodyA->m_callbacks && NULL != bodyA->m_callbacks->OnFilter ) {
		if ( bodyA->m_callbacks->OnFilter( bodyA->m_callbacks->m_owner, bodyA, bodyB ) ) {
			return true;
		}
	}
	if ( NULL != bodyB->m_callbacks && NULL != bodyB->m_callbacks->OnFilter ) {
		if ( bodyB->m_callbacks->OnFilter( bodyB->m_callbacks->m_owner, bodyA, bodyB ) ) {
			return true;
		}
	}


	// Skip body pairs with infinite mass
	if ( 0.0f == bodyA->m_invMass && 0.0f == bodyB->m_invMass ) {
		return true;
	}

	// Filter bodies that aren't flagged to collide with each other
	if ( 0 == ( bodyA->m_bodyContents & bodyB->m_collidesWith ) ) {
		return true;
	}
	if ( 0 == ( bodyB->m_bodyContents & bodyA->m_collidesWith ) ) {
		return true;
	}

	// Do not filter these bodies
	return false;
}

/*
====================================================
PhysicsWorld::RemoveExpiredContactsAndConstraints
====================================================
*/
void PhysicsWorld::RemoveExpiredContactsAndConstraints() {
	// Clear any expired manifolds
	m_manifolds.RemoveExpired();

	// Clear any expired constraints
	for ( int i = 0; i < m_constraints.size(); i++ ) {
		const Constraint * constraint = m_constraints[ i ];

		bool doRemove = false;
		if ( NULL != constraint->m_bodyA && !constraint->m_bodyA->m_isUsed ) {
			doRemove = true;
		}
		if ( NULL != constraint->m_bodyB && !constraint->m_bodyB->m_isUsed ) {
			doRemove = true;
		}

		if ( doRemove ) {
			delete m_constraints[ i ];
			m_constraints.erase( m_constraints.begin() + i );
			i--;
		}
	}
}

/*
====================================================
PhysicsWorld::StepSimulation
====================================================
*/
void PhysicsWorld::StepSimulation( const float dt_sec ) {
	RemoveExpiredContactsAndConstraints();

	//
	//	Apply Gravity to bodies
	//
	ApplyGravity( dt_sec );

	//
	// Broadphase (build potential collision pairs)
	//
	std::vector< collisionPair_t > collisionPairs;
	BroadPhase( m_bodyPool, m_usedNodes, collisionPairs, dt_sec );

	//
	//	NarrowPhase (perform actual collision detection)
	//
	int numContacts = 0;
	//contact_t * contacts = (contact_t *)alloca( sizeof( contact_t ) * collisionPairs.size() );
	contact_t * contacts = (contact_t *)malloc( sizeof( contact_t ) * collisionPairs.size() );
	for ( int i = 0; i < collisionPairs.size(); i++ ) {
		const collisionPair_t & pair = collisionPairs[ i ];
		Body * bodyA = &m_bodyPool[ pair.a ];
		Body * bodyB = &m_bodyPool[ pair.b ];

		// Skip bodies that should be filtered from collision
		if ( FilterPair( bodyA, bodyB ) ) {
			continue;
		}

		// Check for intersection
		contact_t contact;
		if ( Intersect( bodyA, bodyB, dt_sec, contact ) ) {
			if ( 0.0f == contact.timeOfImpact ) {
				// Static contact
				m_manifolds.AddContact( contact );
			} else {
				// Ballistic contact
				contacts[ numContacts ] = contact;
				numContacts++;
			}
			
			// Function callbacks for OnContact
			if ( NULL != bodyA->m_callbacks && NULL != bodyA->m_callbacks->OnContact ) {
				bodyA->m_callbacks->OnContact( bodyA->m_callbacks->m_owner, contact );
			}
			if ( NULL != bodyB->m_callbacks && NULL != bodyB->m_callbacks->OnContact ) {
				bodyB->m_callbacks->OnContact( bodyB->m_callbacks->m_owner, contact );
			}
		}
	}

	//
	//	Apply forces
	//

	//
	//	Solve Constraints
	//
	for ( int i = 0; i < m_constraints.size(); i++ ) {
		m_constraints[ i ]->PreSolve( dt_sec );
	}
	m_manifolds.PreSolve( dt_sec );

	const int maxIters = 5;
	for ( int iters = 0; iters < maxIters; iters++ ) {
		for ( int i = 0; i < m_constraints.size(); i++ ) {
			m_constraints[ i ]->Solve();
		}
		m_manifolds.Solve();
	}

	for ( int i = 0; i < m_constraints.size(); i++ ) {
		m_constraints[ i ]->PostSolve();
	}
	m_manifolds.PostSolve();

	//
	// Apply ballistic impulses
	//

	// Sort the times of impact from first to last
	if ( numContacts > 1 ) {
		qsort( contacts, numContacts, sizeof( contact_t ), CompareContacts );
	}

	float accumulatedTime = 0.0f;
	for ( int i = 0; i < numContacts; i++ ) {
		contact_t & contact = contacts[ i ];
		const float dt = contact.timeOfImpact - accumulatedTime;

		// Position update
		UpdateBodies( dt );

		ResolveContact( contact );
		accumulatedTime += dt;
	}

	//
	// Update the positions for the rest of this frame's time
	//
	const float timeRemaining = dt_sec - accumulatedTime;
	if ( timeRemaining > 0.0f ) {
		UpdateBodies( timeRemaining );
	}

	free( contacts );
}
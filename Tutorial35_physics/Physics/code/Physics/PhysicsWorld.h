//
//	PhysicsWorld.h
//
#pragma once
#include "Physics/Shapes.h"
#include "Physics/Body.h"
#include "Physics/Contact.h"
#include "Physics/Constraints.h"
#include "Physics/Manifold.h"
#include "Physics/BroadPhase.h"

struct BodyPoolNode_t {
	BodyPoolNode_t * m_next;
	int bodyID;
};

/*
====================================================
PhysicsWorld
====================================================
*/
class PhysicsWorld {
public:
	PhysicsWorld();
	~PhysicsWorld();
	void Reset();

	bodyID_t AllocateBody( Body body );
	void FreeBody( const int bodyID );
	int MaxBodies() const { return m_maxBodies; }

	void RegisterConstraint( Constraint * constraint );
	void UnRegisterConstraint( Constraint * constraint );

	void StepSimulation( const float dt_sec );

	void GetAllocatedBodyIDs( std::vector< int > & bodyIds ) const;	// Used for debug drawing
	const Body * GetBody( const int bodyID ) const;

private:
	Body * GetBody( const int bodyID );
	void UpdateBodies( const float dt_sec );
	void ApplyGravity( const float dt_sec );
	bool FilterPair( Body * bodyA, Body * bodyB );
	void RemoveExpiredContactsAndConstraints();

private:
	static const int m_maxBodies = 1024;
	Body m_bodyPool[ m_maxBodies ];
	BodyPoolNode_t	m_bodyPoolNodes[ m_maxBodies ];

	BodyPoolNode_t * m_freeNodes;
	BodyPoolNode_t * m_usedNodes;

	int m_numUsedBodies;

	std::vector< Constraint * >	m_constraints;
	ManifoldCollector			m_manifolds;

	friend class BVH;
	friend class LBVH;
	friend class BoundingVolumeHierarchy;
};

extern PhysicsWorld * g_physicsWorld;
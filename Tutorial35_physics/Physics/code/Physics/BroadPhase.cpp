//
//  BroadPhase.cpp
//
#include "Physics/BroadPhase.h"
#include "Physics/Body.h"
#include "Physics/PhysicsWorld.h"
#include "Physics/BVH.h"
#include "JobSystem/JobSystem.h"

/*
========================================================================================================

BroadPhase

========================================================================================================
*/

/*
====================================================
CompareSAP
====================================================
*/
struct psuedoBody_t {
	psuedoBody_t() : id( -1 ), value( -1.0f ), valueInt( 0xFFFFFFFF ), ismin( false ) {}
	int id;
	float value;
	unsigned int valueInt;
	bool ismin;
};
int CompareSAP( const void * a, const void * b ) {
	const psuedoBody_t * ea = (const psuedoBody_t *)a;
	const psuedoBody_t * eb = (const psuedoBody_t *)b;

	if ( ea->value < eb->value ) {
		return -1;
	}
	return 1;
}

/*
====================================================
SortBodiesBounds
====================================================
*/
void SortBodiesBounds( const Body * bodies, const int num, psuedoBody_t * sortedArray, const float dt_sec ) {
	Vec3 axis = Vec3( 1, 1, 1 );
	axis.Normalize();

	for ( int i = 0; i < num; i++ ) {
		const Body & body = bodies[ i ];
		Bounds bounds = body.GetBounds( dt_sec );

		// Expand the bounds by a tiny epsilon
		const float epsilon = 0.01f;
		bounds.Expand( bounds.mins + Vec3(-1,-1,-1 ) * epsilon );
		bounds.Expand( bounds.maxs + Vec3( 1, 1, 1 ) * epsilon );

		sortedArray[ i * 2 + 0 ].id = i;
		sortedArray[ i * 2 + 0 ].value = axis.Dot( bounds.mins );
		sortedArray[ i * 2 + 0 ].ismin = true;

		sortedArray[ i * 2 + 1 ].id = i;
		sortedArray[ i * 2 + 1 ].value = axis.Dot( bounds.maxs );
		sortedArray[ i * 2 + 1 ].ismin = false;
	}

	qsort( sortedArray, num * 2, sizeof( psuedoBody_t ), CompareSAP );
}

void SortBodiesBounds( const Body * bodies, const int * bodyIDs, const int num, psuedoBody_t * sortedArray, const float dt_sec ) {
	Vec3 axis = Vec3( 1, 1, 1 );
	axis.Normalize();

	for ( int i = 0; i < num; i++ ) {
		const int bodyID = bodyIDs[ i ];
		const Body & body = bodies[ bodyID ];
		Bounds bounds = body.GetBounds( dt_sec );

		// Expand the bounds by a tiny epsilon
		const float epsilon = 0.01f;
		bounds.Expand( bounds.mins + Vec3(-1,-1,-1 ) * epsilon );
		bounds.Expand( bounds.maxs + Vec3( 1, 1, 1 ) * epsilon );

		sortedArray[ i * 2 + 0 ].id = bodyID;
		sortedArray[ i * 2 + 0 ].value = axis.Dot( bounds.mins );
		sortedArray[ i * 2 + 0 ].ismin = true;

		sortedArray[ i * 2 + 1 ].id = bodyID;
		sortedArray[ i * 2 + 1 ].value = axis.Dot( bounds.maxs );
		sortedArray[ i * 2 + 1 ].ismin = false;
	}

	qsort( sortedArray, num * 2, sizeof( psuedoBody_t ), CompareSAP );
}

/*
====================================================
BuildPairs
====================================================
*/
void BuildPairs( std::vector< collisionPair_t > & collisionPairs, const psuedoBody_t * sortedBodies, const int num ) {
	collisionPairs.clear();

	// Now that the bodies are sorted, build the collision pairs
	for ( int i = 0; i < num * 2; i++ ) {
		const psuedoBody_t & a = sortedBodies[ i ];
		if ( !a.ismin ) {
			continue;
		}

		collisionPair_t pair;
		pair.a = a.id;		

		for ( int j = i + 1; j < num * 2; j++ ) {
			const psuedoBody_t & b = sortedBodies[ j ];
			// if we've hit the end of the a element, then we're done creating pairs with a
			if ( b.id == a.id ) {
				break;
			}

			if ( !b.ismin ) {
				continue;
			}

			pair.b = b.id;
			collisionPairs.push_back( pair );
		}		
	}
}


/*
====================================================
SweepAndPrune
====================================================
*/
void SweepAndPrune( const Body * bodies, const int num, std::vector< collisionPair_t > & finalPairs, const float dt_sec ) {
	psuedoBody_t * sortedBodies = (psuedoBody_t *)alloca( sizeof( psuedoBody_t ) * num * 2 );

	SortBodiesBounds( bodies, num, sortedBodies, dt_sec );
	BuildPairs( finalPairs, sortedBodies, num );
}

/*
====================================================
BroadPhase
====================================================
*/
void BroadPhase( const Body * bodies, const int num, std::vector< collisionPair_t > & finalPairs, const float dt_sec ) {
	finalPairs.clear();
	SweepAndPrune( bodies, num, finalPairs, dt_sec );
}


/*
====================================================
SweepAndPrune
====================================================
*/
void SweepAndPrune( const Body * bodies, const int * bodyIDs, const int num, std::vector< collisionPair_t > & finalPairs, const float dt_sec ) {
	psuedoBody_t * sortedBodies = (psuedoBody_t *)alloca( sizeof( psuedoBody_t ) * num * 2 );

	SortBodiesBounds( bodies, bodyIDs, num, sortedBodies, dt_sec );
	BuildPairs( finalPairs, sortedBodies, num );
}






/*
====================================================
BroadPhase_LBVH
====================================================
*/
void BroadPhase_LBVH( const Body * bodies, const BodyPoolNode_t * nodes, std::vector< collisionPair_t > & finalPairs, const float dt_sec ) {
	LBVH lbvh;
	lbvh.Build( g_physicsWorld, dt_sec );

	std::vector< int > colliders;
	const BodyPoolNode_t * node = nodes;
	while ( NULL != node ) {
		colliders.clear();
		Bounds bounds = bodies[ node->bodyID ].GetBounds( dt_sec );
		lbvh.GetCollisions( bounds, node->bodyID, colliders );

		for ( int i = 0; i < colliders.size(); i++ ) {
			collisionPair_t pair;
			pair.a = node->bodyID;
			pair.b = colliders[ i ];
			
			// Add this pair only if it's unique
			if ( std::find( finalPairs.begin(), finalPairs.end(), pair ) == finalPairs.end() ) {
				finalPairs.push_back( pair );
			}
		}

		node = node->m_next;
	}
}




struct BroadPhaseData_t {
	int bodyId;
	Bounds bounds;
	const BoundingVolumeHierarchy * bvh;

	std::vector< int > colliders;
};

/*
====================================================
Job_BroadPhase
====================================================
*/
void Job_BroadPhase( Job_t * job, void * data ) {
	BroadPhaseData_t * bpData = (BroadPhaseData_t *)job->m_data;

	bpData->bvh->GetCollisions( bpData->bounds, bpData->bodyId, bpData->colliders );
}
void Job_BroadPhaseElements( Job_t * job, void * data ) {
	BroadPhaseData_t * bpData = (BroadPhaseData_t *)job->m_data;

	for ( int i = 0; i < job->m_numElements; i++ ) {
		BroadPhaseData_t * data = &bpData[ i ];
		data->bvh->GetCollisions( data->bounds, data->bodyId, data->colliders );
	}
}

/*
====================================================
BroadPhase_BVH
====================================================
*/
void BroadPhase_BVH( const Body * bodies, const BodyPoolNode_t * nodes, std::vector< collisionPair_t > & finalPairs, const float dt_sec ) {
	BoundingVolumeHierarchy bvh;
	bvh.Build( g_physicsWorld, dt_sec );

//#define RUN_IN_PARALLEL
#if defined( RUN_IN_PARALLEL )
	// Build the data for jobs
	const BodyPoolNode_t * node = nodes;
	std::vector< BroadPhaseData_t > datas;
	while ( NULL != node ) {
		BroadPhaseData_t data;
		data.bvh = &bvh;
		data.bounds = bodies[ node->bodyID ].GetBounds( dt_sec );
		data.bodyId = node->bodyID;
		datas.push_back( data );

		node = node->m_next;
	}

	// Add Jobs and wait for all of them to finish
#if 0
	for ( int i = 0; i < datas.size(); i++ ) {
		g_jobSystem->AddJoby( Job_BroadPhase, &datas[ i ] );
	}
#else
	g_jobSystem->ParallelFor( Job_BroadPhaseElements, datas.data(), sizeof( BroadPhaseData_t ), datas.size() );
#endif
	g_jobSystem->Wait( NULL );

	// Collect the unique pairs
	finalPairs.clear();
	for ( int i = 0; i < datas.size(); i++ ) {
		const BroadPhaseData_t & data = datas[ i ];

		for ( int i = 0; i < data.colliders.size(); i++ ) {
			collisionPair_t pair;
			pair.a = data.bodyId;
			pair.b = data.colliders[ i ];
			
			// Add this pair only if it's unique
			if ( std::find( finalPairs.begin(), finalPairs.end(), pair ) == finalPairs.end() ) {
				finalPairs.push_back( pair );
			}
		}
	}

#else
	std::vector< int > colliders;
	const BodyPoolNode_t * node = nodes;
	while ( NULL != node ) {
		colliders.clear();
		Bounds bounds = bodies[ node->bodyID ].GetBounds( dt_sec );
		bvh.GetCollisions( bounds, node->bodyID, colliders );

		for ( int i = 0; i < colliders.size(); i++ ) {
			collisionPair_t pair;
			pair.a = node->bodyID;
			pair.b = colliders[ i ];
			
			// Add this pair only if it's unique
			if ( std::find( finalPairs.begin(), finalPairs.end(), pair ) == finalPairs.end() ) {
				finalPairs.push_back( pair );
			}
		}

		node = node->m_next;
	}
#endif
}

/*
====================================================
BroadPhase
====================================================
*/
void BroadPhase( const Body * bodies, const BodyPoolNode_t * nodes, std::vector< collisionPair_t > & finalPairs, const float dt_sec ) {
#define USE_BVH
//#define USE_LBVH
#if defined( USE_BVH )
	BroadPhase_BVH( bodies, nodes, finalPairs, dt_sec );
#elif defined( USE_LBVH )
	BroadPhase_LBVH( bodies, nodes, finalPairs, dt_sec );
#else
	int numBodies = 0;
	const BodyPoolNode_t * node = nodes;
	while ( NULL != node ) {
		node = node->m_next;
		numBodies++;
	}

	int * bodyIDs = (int *)alloca( sizeof( int ) * numBodies );

	node = nodes;
	numBodies = 0;
	while ( NULL != node ) {
		bodyIDs[ numBodies ] = node->bodyID;
		node = node->m_next;
		numBodies++;
	}

	SweepAndPrune( bodies, bodyIDs, numBodies, finalPairs, dt_sec );
#endif
}
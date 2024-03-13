//
//  BehaviorTree.cpp
//
#include "NavMesh/PathFinding.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeFile.h"
#include "Entities/Player.h"
#include "Entities/Spider.h"
#include <math.h>
#include <algorithm>
#include <stdlib.h>
#include <assert.h>
#include <vector>

extern Player g_player;
extern Spider g_spider;

static btNode_t * s_root = NULL;

static std::vector< btNode_t * > s_nodeStack; // The stack of currently active nodes (so that we don't have to parse the tree each frame, we can just call the top of the stack)

// There's a lot to do to improve the behavior tree.
// A blackboard could be used for communication between nodes.
// A stack of nodes could be used to optimize ticks (we wouldn't 
// have to parse the whole tree each frame to run a currently running node).
// OOP could clearly be used to create different nodes.


btState_t SpiderRoot( btNode_t * thisNode ) {
	// This is a selector node
	// 1. WalkToPlayer
	// if it fails then
	// 2. Idle
	btState_t state;
	if ( g_spider.m_health < 10 ) {
		state = thisNode->children[ 1 ].action( thisNode->children + 1 );
	} else {
		state = thisNode->children[ 0 ].action( thisNode->children + 0 );
	}

	if ( BT_RUNNING == state ) {
		return BT_RUNNING;
	} else {
		thisNode->children[ 2 ].action( thisNode->children + 1 );
	}
	return BT_RUNNING;
}

btState_t Idle( btNode_t * thisNode ) {
	g_spider.m_state = AI_IDLE;
	return BT_RUNNING;
}

btState_t WalkToPlayer( btNode_t * thisNode ) {
	// Get the player's position
	// Use the navmesh to find the shortest path to the player
	// Grab the position in the navmesh to the next node
	// Animate towards there
	if ( BT_SUCCESS == thisNode->children[ 0 ].action( thisNode->children + 0 ) ) {
		thisNode->data = thisNode->children[ 0 ].data;
		thisNode->children[ 1 ].data = thisNode->data;
		btState_t state = thisNode->children[ 1 ].action( thisNode->children + 1 );
		if ( BT_FAILURE == state ) {
			return BT_FAILURE;
		}
		if ( BT_SUCCESS == state ) {
			return BT_SUCCESS;
		}
		if ( BT_RUNNING == state ) {
			return BT_RUNNING;
		}
	}
	return BT_FAILURE;
}

btState_t EatFood( btNode_t * thisNode ) {
	if ( BT_SUCCESS == thisNode->children[ 0 ].action( thisNode->children + 0 ) ) {
		thisNode->data = thisNode->children[ 0 ].data;
		thisNode->children[ 1 ].data = thisNode->data;
		btState_t state = thisNode->children[ 1 ].action( thisNode->children + 1 );
		if ( BT_FAILURE == state ) {
			return BT_FAILURE;
		}
		if ( BT_SUCCESS == state ) {
			state = thisNode->children[ 2 ].action( thisNode->children + 2 );
			return state;
		}
		if ( BT_RUNNING == state ) {
			return BT_RUNNING;
		}
	}
	return BT_FAILURE;
}

btState_t Sequence( btNode_t * thisNode ) {
	for ( int i = 0; i < thisNode->numChildren; i++ ) {
		btNode_t * child = thisNode->children + i;
		if ( BT_FAILURE == child->action( child ) ) {
			return BT_FAILURE;
		}
	}

	return BT_SUCCESS;
}

btState_t Selector( btNode_t * thisNode ) {
	for ( int i = 0; i < thisNode->numChildren; i++ ) {
		btNode_t * child = thisNode->children + i;
		if ( BT_SUCCESS == child->action( child ) ) {
			return BT_SUCCESS;
		}
	}

	return BT_FAILURE;
}

btState_t GetPlayerPos( btNode_t * thisNode ) {
	thisNode->data = g_player.GetPosition() - Vec3( 0, 0, 0.5f );
	return BT_SUCCESS;
}

btState_t GetFoodPos( btNode_t * thisNode ) {
	thisNode->data = Vec3( 0, 0, 0 );
	return BT_SUCCESS;
}

btState_t Eat( btNode_t * thisNode ) {
	g_spider.m_health = 100;
	return BT_SUCCESS;
}

extern std::vector< Vec3 > g_pathPts;
extern std::vector< navEdge_t > g_navEdges;

btState_t WalkToPosition( btNode_t * thisNode ) {
	Vec3 start = g_spider.GetPosition() + Vec3( 0, 0, 0.1f );
	Vec3 end = thisNode->data + Vec3( 0, 0, 0.1f );
	Vec3 delta = end - start;
	if ( delta.GetMagnitude() < 1.0f ) {
		return BT_SUCCESS;
	}

	// Use the navmesh to determine the target pos
	Vec3 targetPos = start;
	std::vector< Vec3 > path;
	std::vector< navEdge_t > edges;
	if ( !PathFind( start, end, path, edges ) ) {
		return BT_FAILURE;
	}

	g_pathPts = path;
	g_navEdges = edges;

	targetPos = end;
	if ( edges.size() > 0 ) {
		navEdge_t edge = edges[ 0 ];
		targetPos = ( edge.a + edge.b ) * 0.5f;

		delta = targetPos - start;
		if ( delta.GetMagnitude() < 0.05f ) {
			targetPos = end;
			if ( g_pathPts.size() > 1 ) {
				edge = edges[ 1 ];
				targetPos = ( edge.a + edge.b ) * 0.5f;
			}
		}
	}

	// For debug purposes, show the target positions
	g_pathPts.clear();
	for ( int i = 0; i < g_navEdges.size(); i++ ) {
		navEdge_t edge = g_navEdges[ i ];
		g_pathPts.push_back( ( edge.a + edge.b ) * 0.5f );
	}

	g_spider.m_state = AI_WALK;
	g_spider.mLookDir = targetPos - start;
	g_spider.mLookDir.Normalize();

	return BT_RUNNING;
}

/*
================================================================

BehaviorTree

================================================================
*/

void BuildWalkToPosition( btNode_t * node ) {
	node->action = WalkToPosition;
	node->state = BT_FAILURE;
	node->numChildren = 0;
	node->children = NULL;
}

void BuildGetPlayerPos( btNode_t * node ) {
	node->action = GetPlayerPos;
	node->state = BT_FAILURE;
	node->numChildren = 0;
	node->children = NULL;
}

void BuildWalkToPlayer( btNode_t * node ) {
	node->action = WalkToPlayer;
	node->state = BT_FAILURE;
	node->numChildren = 2;
	node->children = (btNode_t *)malloc( sizeof( btNode_t ) * 2 );

	BuildGetPlayerPos( node->children + 0 );
	BuildWalkToPosition( node->children + 1 );
}

void BuildIdleNode( btNode_t * node ) {
	node->action = Idle;
	node->state = BT_FAILURE;
	node->numChildren = 0;
	node->children = NULL;
}

/*
================================
BuildBehaviorTree
================================
*/
void BuildBehaviorTree() {
	// We will build our basic behavior tree here
// 	s_root = (btNode_t *)malloc( sizeof( btNode_t ) );
// 	s_root->action = SpiderRoot;
// 	s_root->state = BT_RUNNING;
// 	s_root->numChildren = 2;
// 	s_root->children = (btNode_t *)malloc( sizeof( btNode_t ) * 2 );
// 
// 	BuildWalkToPlayer( s_root->children + 0 );
// 	BuildIdleNode( s_root->children + 1 );

	s_root = ReadBehaviorTreeFile( "data/BehaviorTrees/spider.btree" );
}

void DeleteTree_r( btNode_t * node ) {
	if ( NULL == node ) {
		return;
	}

	for ( int i = 0; i < node->numChildren; i++ ) {
		DeleteTree_r( node->children + i );
	}

	free( node );
}

void CleanUpTree() {
	DeleteTree_r( s_root );
}

/*
================================
TickTree
================================
*/
void TickTree() {
	s_root->action( s_root );

// 	int topIdx = s_nodeStack.size() - 1;
// 	if ( topIdx < 0 ) {
// 		return;
// 	}
// 
// 	btNode_t * topNode = s_nodeStack[ topIdx ];
// 	if ( NULL == topNode->action ) {
// 		return;
// 	}
// 
// 	btState_t state = topNode->action( topNode );
// 	if ( BT_RUNNING != state ) {
// 		// If the top node is no longer running, then pop it
// 		s_nodeStack.erase( s_nodeStack.begin() + topIdx );
// 	}
}



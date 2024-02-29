//
//  BVH.cpp
//
#include "Physics/BVH.h"
#include "Physics/Body.h"
#include "Physics/PhysicsWorld.h"
#include "Math/Random.h"
#include "Math/Morton.h"
#include <stack>

#pragma optimize( "", off )

int TestRadixSort();

bool DoesContainPoint( Vec2 mins, Vec2 maxs, Vec2 pt ) {
	if ( pt.x < mins.x || pt.y < mins.y ) {
		return false;
	}
	if ( pt.x > maxs.x || pt.y > maxs.y ) {
		return false;
	}
	return true;
}

Vec3 PointOnPlane( Vec3 a, Vec3 b, Vec3 pt, int axis ) {
	float aa = a[ axis ];
	float bb = b[ axis ];
	float ptpt = pt[ axis ];


	float dir = bb - aa;
	float t = ( ptpt - aa ) / dir;

	Vec3 point = a + ( b - a ) * t;
	return point;
}

bool IntersectBoundsLine( const Bounds & bounds, const Vec3 a, const Vec3 b ) {
	Vec3 ray = b - a;

	// XY plane first
// 	for ( int i = 0; i < 3; i++ ) {
// 		Vec3 point = PointOnPlane( a, b, bounds.mins, 0 );
// 		Vec2 mins = Vec2( bounds.mins[ 1 ], bounds.mins[ 2 ] );
// 		Vec2 maxs = Vec2( bounds.maxs[ 1 ], bounds.maxs[ 2 ] );
// 		if ( DoesContainPoint( mins, maxs, Vec2( pt[ 1 ], pt[ 2 ] ) ) ) {
// 			return true;
// 		}
// 	}
	return false;
}

/*
========================================================================================================

Bounding Volume Hierarchy

========================================================================================================
*/

struct BVHNode {
	Bounds bounds;
	int objectIndex;
	int parentIndex;
	int child1;
	int child2;
	bool isLeaf;
};

struct BVHTree {
	BVHNode * nodes;
	int nodeCount;
	int rootIndex;
};

bool TreeRayCast( BVHTree tree, Vec3 p1, Vec3 p2 );

/*
====================================================
TreeRayCast
====================================================
*/
bool TreeRayCast( BVHTree tree, Vec3 p1, Vec3 p2 ) {
#if 0
	//Stack< int > stack;
	std::stack< int > stack;	
	
	//Push( stack, tree.rootIndex );
	stack.push( tree.rootIndex );

	//while ( IsEmpty( stack ) == false ) {
	while ( !stack.empty() ) {
		//int index = Pop( stack );
		int index = stack.top();
		stack.pop();

		if ( !IntersectBoundsLine( tree.nodes[ index ].bounds, p1, p2 ) ) {
			continue;
		}


		if ( nodes[ index ].isLeaf ) {
			int objectIndex = tree.nodes[ index ].objectIndex;
// 			if ( RayCast( objects[ objectIndex ], p1, p2 ) ) {
// 				return true;
// 			}
		} else {
// 			Push( stack, tree.nodes[ index ].child1 );
// 			Push( stack, tree.nodes[ index ].child2 );
			stack.push( tree.nodes[ index ].child1 );
			stack.push( tree.nodes[ index ].child2 );
		}
	}
#endif
	return false;
}

/*
====================================================
InsertLeaf
====================================================
*/
void InsertLeaf( BVHTree tree, int objectIndex, Bounds box ) {
#if 0
	int leafIndex = AllocateLeafNode( tree, objectIndex, box );
	if ( tree.nodeCount == 0 ) {
		tree.rootIndex = leafIndex;
		return;
	}
	
	// Stage 1: find the best sibling for the new leaf
	int bestSibling = 0;
	for ( int i = 0; i < m_nodeCount; ++i ) {
		bestSibling = PickBest( bestSibling, i );
	}

	// Stage 2: create a new parent
	int oldParent = tree.nodes[ sibling ].parentIndex;
	int newParent = AllocateInternalNode( tree );
	tree.nodes[ newParent ].parentIndex = oldParent;
	tree.nodes[ newParent ].box = Union( box, tree.nodes[ sibling ].box );
	if ( oldParent != nullIndex ) {
		// The sibling was not the root
		if ( tree.nodes[ oldParent ].child1 == sibling ) {
			tree.nodes[ oldParent ].child1 = newParent;
		} else {
			tree.nodes[ oldParent ].child2 = newParent;
		}
		tree.nodes[ newParent ].child1 = sibling;
		tree.nodes[ newParent ].child2 = leafIndex;
		tree.nodes[ sibling ].parentIndex = newParent;
		tree.nodes[ leafIndex ].parentIndex = newParent;
	} else {
		// The sibling was the root
		tree.nodes[ newParent ].child1 = sibling;
		tree.nodes[ newParent ].child2 = leafIndex;
		tree.nodes[ sibling ].parentIndex = newParent;
		tree.nodes[ leafIndex ].parentIndex = newParent;
		tree.rootIndex = newParent;
	}


	// Stage 3a: walk back up the tree refitting AABBs and applying rotations
	// Stage 3: walk back up the tree refitting AABBs
	int index = tree.nodes[ leafIndex ].parentIndex;
	while ( index != nullIndex ) {
		int child1 = tree.nodes[ index ].child1;
		int child2 = tree.nodes[ index ].child2;
		tree.nodes[ index ].box = Union( tree.nodes[ child1 ].box, tree.nodes[ child2 ].box );
		Rotate( index );
		index = tree.nodes[ index ].parentIndex;
	}
#endif
}

/*
====================================================
ComputeCostSAH
====================================================
*/
float ComputeCostSAH( BVHTree tree ) {
	float cost = 0.0f;
	for ( int i = 0; i < tree.nodeCount; ++i ) {
		cost += tree.nodes[ i ].bounds.SurfaceArea();
	}
	return cost;
}

/*
====================================================
ComputeCostSAH2
====================================================
*/
float ComputeCostSAH2( BVHTree tree ) {
	float cost = 0.0f;
	for ( int i = 0; i < tree.nodeCount; ++i ) {
		if ( tree.nodes[ i ].isLeaf == false ) {
			cost += tree.nodes[ i ].bounds.SurfaceArea();
		}
	}
	return cost;
}






float BVH::node_t::Cost() {
	return m_bounds.SurfaceArea();
}

void BVH::node_t::Reset() {
	m_bounds.Clear();
	m_parent = -1;
	m_left = -1;
	m_right = -1;
	m_bodyId = -1;
}

void BVH::Build() {
	m_nodes[ 0 ].Reset();

// 	BodyPoolNode_t * bodyNode = g_physicsWorld->m_usedNodes;
// 	while ( NULL != bodyNode ) {
// 
// 	}
}



















int BoundsCompareX( const void * a, const void * b ) {
	BoundingVolumeHierarchy::bodyBounds_t * ah = (BoundingVolumeHierarchy::bodyBounds_t *)a;
	BoundingVolumeHierarchy::bodyBounds_t * bh = (BoundingVolumeHierarchy::bodyBounds_t *)b;

//	if ( ( boundsA.m_min.x - boundsB.m_min.x ) < 0.0f ) {
	if ( ah->m_bounds.mins.x < bh->m_bounds.mins.x ) {
		return -1;
	}

	return 1;
}
int BoundsCompareY( const void * a, const void * b ) {
	BoundingVolumeHierarchy::bodyBounds_t * ah = (BoundingVolumeHierarchy::bodyBounds_t *)a;
	BoundingVolumeHierarchy::bodyBounds_t * bh = (BoundingVolumeHierarchy::bodyBounds_t *)b;

//	if ( ( boundsA.m_min.y - boundsB.m_min.y ) < 0.0f ) {
	if ( ah->m_bounds.mins.y < bh->m_bounds.mins.y ) {
		return -1;
	}

	return 1;
}
int BoundsCompareZ( const void * a, const void * b ) {
	BoundingVolumeHierarchy::bodyBounds_t * ah = (BoundingVolumeHierarchy::bodyBounds_t *)a;
	BoundingVolumeHierarchy::bodyBounds_t * bh = (BoundingVolumeHierarchy::bodyBounds_t *)b;

//	if ( ( boundsA.m_min.z - boundsB.m_min.z ) < 0.0f ) {
	if ( ah->m_bounds.mins.z < bh->m_bounds.mins.z ) {
		return -1;
	}

	return 1;
}

/*
====================================================
BoundingVolumeHierarchy::node_t::node_t
====================================================
*/
BoundingVolumeHierarchy::node_t::node_t( bodyBounds_t * bodyIds, int numBodies ) {
	assert( numBodies > 0 );
	if ( 0 == numBodies ) {
		printf( "WARNING: this shouldn't happen.  Badly built BVH!\n" );
		return;
	}

	Reset();

	if ( 1 == numBodies ) {
		m_bodyId = bodyIds[ 0 ].m_bodyId;
		m_bounds = bodyIds[ 0 ].m_bounds;

		if ( m_bodyId > 3000 || m_bodyId < -1 ) {
			volatile static int counter = 0;
			counter++;
			counter++;
		}
		return;
	}

	const int axis = int( 3.0f * Random::Get() );
	switch ( axis ) {
		default:
		case 0: { qsort( bodyIds, numBodies, sizeof( BoundingVolumeHierarchy::bodyBounds_t ), BoundsCompareX ); } break;
		case 1: { qsort( bodyIds, numBodies, sizeof( BoundingVolumeHierarchy::bodyBounds_t ), BoundsCompareY ); } break;
		case 2: { qsort( bodyIds, numBodies, sizeof( BoundingVolumeHierarchy::bodyBounds_t ), BoundsCompareZ ); } break;
	};

	m_left = new node_t( bodyIds, numBodies / 2 );
	m_right = new node_t( bodyIds + numBodies / 2, numBodies - numBodies / 2 );

	m_bounds = m_left->m_bounds;
	m_bounds.Expand( m_right->m_bounds );

	if ( m_bodyId > 30 || m_bodyId < -1 ) {
		volatile static int counter = 0;
		counter++;
	}

	assert( m_bounds.IsValid() );
	if ( !m_bounds.IsValid() ) {
		printf( "WARNING: this shouldn't happen.  Bounds of a BVH node is invalid!\n" );
	}
}

/*
====================================================
BoundingVolumeHierarchy::node_t::GetCollisions_r
====================================================
*/
void BoundingVolumeHierarchy::node_t::GetCollisions_r( const Bounds & bounds, const int skipId, std::vector< int > & bodyIds ) const {
	if ( !m_bounds.DoesIntersect( bounds ) ) {
		return;
	}

	if ( NULL != m_left ) {
		m_left->GetCollisions_r( bounds, skipId, bodyIds );
	}
	if ( NULL != m_right ) {
		m_right->GetCollisions_r( bounds, skipId, bodyIds );
	}

	if ( -1 != m_bodyId && skipId != m_bodyId ) {
		bodyIds.push_back( m_bodyId );
	}
}

/*
====================================================
BoundingVolumeHierarchy::Build
====================================================
*/
void BoundingVolumeHierarchy::Build( const PhysicsWorld * world, const float dt_sec ) {
	//
	//	Build a list of used BodyId's
	//
	int numUsedBodies = world->m_numUsedBodies;
	bodyBounds_t * bodies = (bodyBounds_t *)alloca( sizeof( bodyBounds_t ) * numUsedBodies );
	{
		int i = 0;
		const BodyPoolNode_t * node = world->m_usedNodes;
		while ( NULL != node ) {
			bodies[ i ].m_bodyId = node->bodyID;
			bodies[ i ].m_bounds = world->GetBody( node->bodyID )->GetBounds( dt_sec );

			// Expand the bounds by a tiny epsilon
			const float epsilon = 0.01f;
			bodies[ i ].m_bounds.Expand( bodies[ i ].m_bounds.mins + Vec3(-1,-1,-1 ) * epsilon );
			bodies[ i ].m_bounds.Expand( bodies[ i ].m_bounds.maxs + Vec3( 1, 1, 1 ) * epsilon );

			node = node->m_next;
			i++;
		}
		assert( numUsedBodies == i );
		numUsedBodies = i;
	}

	//
	//	Build the hierarchy
	//
	m_nodes = new node_t( bodies, numUsedBodies );

//	Print_r( m_nodes, 0 );
}

/*
====================================================
BoundingVolumeHierarchy::Print_r
====================================================
*/
void BoundingVolumeHierarchy::Print_r( const node_t * node, int level ) const {
	if ( NULL == node ) {
		return;
	}

	Print_r( node->m_left, level + 1 );
	Print_r( node->m_right, level + 1 );

	printf( "Level: %i   BodyId: %i\n", level, node->m_bodyId );
}

/*
====================================================
BoundingVolumeHierarchy::GetCollisions
====================================================
*/
void BoundingVolumeHierarchy::GetCollisions( const Bounds & bounds, const int skipId, std::vector< int > & bodyIds ) const {
	m_nodes->GetCollisions_r( bounds, skipId, bodyIds );
}















/*
========================================================================================================

LBVH

========================================================================================================
*/

/*
====================================================
LBVH::node_t::GetCollisions_r
====================================================
*/
void LBVH::node_t::GetCollisions_r( const Bounds & bounds, const int skipId, std::vector< int > & bodyIds, int recurssion ) const {
	if ( recurssion > 30 ) {
		// TODO: There's a cycle in our tree sometimes.  Fix this.
		return;
	}
	if ( !m_bounds.DoesIntersect( bounds ) ) {
		return;
	}

	if ( NULL != m_left ) {
		m_left->GetCollisions_r( bounds, skipId, bodyIds, recurssion + 1 );
	}
	if ( NULL != m_right ) {
		m_right->GetCollisions_r( bounds, skipId, bodyIds, recurssion + 1 );
	}

	if ( -1 != m_bodyId && skipId != m_bodyId ) {
		bodyIds.push_back( m_bodyId );
	}
}

/*
====================================================
LBVH::GetCollisions
====================================================
*/
void LBVH::GetCollisions( const Bounds & bounds, const int skipId, std::vector< int > & bodyIds ) const {
	m_internalNodes->GetCollisions_r( bounds, skipId, bodyIds, 0 );
}

/*
====================================================
CompareLeafNodes
====================================================
*/
int CompareLeafNodes( const void * p1, const void * p2 ) {
	LBVH::node_t a = *(LBVH::node_t*)p1;
	LBVH::node_t b = *(LBVH::node_t*)p2;

	if ( a.m_key < b.m_key ) {
		return -1;
	}

	return 1;
}

void LBVH::BuildParentBounds_r( node_t * node ) {
	if ( NULL == node->m_parent ) {
		return;
	}

	node->m_parent->m_bounds.Expand( node->m_bounds );
	BuildParentBounds_r( node->m_parent );
}

/*
====================================================
LBVH::Build
====================================================
*/
void LBVH::Build( const PhysicsWorld * world, const float dt_sec ) {
	TestRadixSort();

	//
	//	Build a list of used BodyId's and calculate the world bounds
	//
	Bounds worldBounds;
	int numUsedBodies = world->m_numUsedBodies;
	bodyBounds_t * bodies = (bodyBounds_t *)alloca( sizeof( bodyBounds_t ) * numUsedBodies );
	{
		int i = 0;
		const BodyPoolNode_t * node = world->m_usedNodes;
		while ( NULL != node ) {
			bodies[ i ].m_bodyId = node->bodyID;
			bodies[ i ].m_bounds = world->GetBody( node->bodyID )->GetBounds( dt_sec );

			// Expand the bounds by a tiny epsilon
			const float epsilon = 0.01f;
			bodies[ i ].m_bounds.Expand( bodies[ i ].m_bounds.mins + Vec3(-1,-1,-1 ) * epsilon );
			bodies[ i ].m_bounds.Expand( bodies[ i ].m_bounds.maxs + Vec3( 1, 1, 1 ) * epsilon );
			
			Vec3 center = bodies[ i ].m_bounds.Center();
			worldBounds.Expand( center );

			node = node->m_next;
			i++;
		}
		assert( numUsedBodies == i );
		numUsedBodies = i;
	}

	Vec3 worldDimensions;
	worldDimensions.x = worldBounds.WidthX();
	worldDimensions.y = worldBounds.WidthY();
	worldDimensions.z = worldBounds.WidthZ();

	//
	//	Generate the Morton order keys ( this could be in parallel )
	//
	for ( int i = 0; i < numUsedBodies; i++ ) {
		m_leafNodes[ i ].Reset();
		m_leafNodes[ i ].m_bodyId = bodies[ i ].m_bodyId;
		m_leafNodes[ i ].m_bounds = bodies[ i ].m_bounds;

		// Get the center of this leaf node (in the [0,1] range)
		Vec3 center = m_leafNodes[ i ].m_bounds.Center();
		Vec3 r = center - worldBounds.mins;
		r.x /= worldDimensions.x;
		r.y /= worldDimensions.y;
		r.z /= worldDimensions.z;

		unsigned int mortonKey = Morton::MortonOrder3D( r );
		m_leafNodes[ i ].m_key = mortonKey;
	}

	//
	//	Sort the leaf nodes ( radix sort can be done in parallel )
	//
	qsort( m_leafNodes, numUsedBodies, sizeof( node_t ), CompareLeafNodes );

	//
	//	Build the hierarchy ( this can run in parallel )
	//
//	printf( "\n" );
	int counter = 0;
	for ( int idx = 0; idx < numUsedBodies - 1; idx++ ) {
#if 0
		int3_t result = AltBuild( m_leafNodes, numUsedBodies, idx );
		int first = result.x;
		int last = result.y;
		int split = result.z;
#else
        // Find out which range of objects the node corresponds to.
        // (This is where the magic happens!)
        int2_t range = DetermineRange( m_leafNodes, numUsedBodies, idx );
        int first = range.x;
        int last = range.y;

        // Determine where to split the range.
        int split = FindSplit( m_leafNodes, numUsedBodies, first, last );

		// Select childA
        node_t * childA;
        if ( split == first ) {
            childA = &m_leafNodes[ split ];
//			printf( "%i submitting leaf: %i\n", counter, first );
			counter++;
		} else {
            childA = &m_internalNodes[ split ];
		}

        // Select childB
        node_t * childB;
        if ( ( split + 1 ) == last ) {
            childB = &m_leafNodes[ split + 1 ];
//			printf( "%i submitting leaf: %i\n", counter, last );
			counter++;
		} else {
            childB = &m_internalNodes[ split + 1 ];
		}

        // Record parent-child relationships.
        m_internalNodes[ idx ].m_left = childA;
        m_internalNodes[ idx ].m_right = childB;
        childA->m_parent = &m_internalNodes[ idx ];
        childB->m_parent = &m_internalNodes[ idx ];
#endif
    }

	for ( int i = 0; i < numUsedBodies; i++ ) {
		BuildParentBounds_r( &m_leafNodes[ i ] );
	}

//	free( bodies );
//	Print_r( m_internalNodes, 0 );
}

/*
====================================================
LBVH::Print_r
====================================================
*/
void LBVH::Print_r( const node_t * node, int level ) const {
	if ( NULL == node ) {
		return;
	}

	Print_r( node->m_left, level + 1 );
	Print_r( node->m_right, level + 1 );

	printf( "Level: %i   BodyId: %i\n", level, node->m_bodyId );
}

/*
====================================================
LBVH::CountLeadingZeros
====================================================
*/
int LBVH::CountLeadingZeros( const unsigned int x ) {
	if ( 0 == x ) {
		return 32;
	}

	int numLeadingZeros = 0;
	unsigned int testValue = 1 << 31;
	while ( 0 == ( x & testValue ) ) {
		numLeadingZeros++;
		testValue = ( testValue >> 1 );
	}

	return numLeadingZeros;
}

/*
====================================================
LBVH::Delta
====================================================
*/
int LBVH::Delta( node_t * leafNodes, int num, int i, int j ) {
	int delta = -1;
	if ( j >= 0 && j < num ) {
		unsigned int firstCode = leafNodes[ i ].m_key;
		unsigned int lastCode = leafNodes[ j ].m_key;
		delta = CountLeadingZeros( firstCode ^ lastCode );
	}
	return delta;
}

/*
====================================================
LBVH::DetermineRange
====================================================
*/
LBVH::int2_t LBVH::DetermineRange( node_t * leafNodes, int num, int idx ) {
	// Determine direction of the range (+1 or -1)
	int deltaNeg = Delta( leafNodes, num, idx, idx - 1 );
	int deltaPos = Delta( leafNodes, num, idx, idx + 1 );
	int direction = ( ( deltaPos - deltaNeg ) > 0 ) ? 1 : -1;

	// Compute upper bound for the length of the range
	int deltaMin = Delta( leafNodes, num, idx, idx - direction );
	int lmax = 2;
	while ( Delta( leafNodes, num, idx, idx + lmax * direction ) > deltaMin ) {
		lmax = lmax * 2;
	}

	// Find the other end using binary search
	int l = 0;
	for ( int t = ( lmax >> 1 ); t > 0; t >>= 1 ) {
		int delta = Delta( leafNodes, num, idx, idx + ( l + t ) * direction );
		if ( delta > deltaMin ) {
			l = l + t;
		}
	}
	int j = idx + l * direction;

	int2_t range;
	range.x = std::min( idx, j );
	range.y = std::max( idx, j );
	return range;
}

/*
====================================================
LBVH::FindSplit
====================================================
*/
int LBVH::FindSplit( node_t * leafNodes, int num, int first, int last ) {
	// Identical Morton codes => split the range in the middle.
    unsigned int firstCode = leafNodes[ first ].m_key;
    unsigned int lastCode = leafNodes[ last ].m_key;

    if ( firstCode == lastCode ) {
        return ( first + last ) >> 1;
	}

    // Calculate the number of highest bits that are the same
    // for all objects, using the count-leading-zeros intrinsic.
    //int commonPrefix = __clz( firstCode ^ lastCode );	// wtf is this intrinsic?
	int commonPrefix = CountLeadingZeros( firstCode ^ lastCode );

    // Use binary search to find where the next bit differs.
    // Specifically, we are looking for the highest object that
    // shares more than commonPrefix bits with the first one.

    int split = first; // initial guess
    int step = last - first;

    do {
        step = ( step + 1 ) >> 1;		// exponential decrease
        int newSplit = split + step;	// proposed new position

        if ( newSplit < last ) {
            unsigned int splitCode = leafNodes[ newSplit ].m_key;
            //int splitPrefix = __clz( firstCode ^ splitCode );
			int splitPrefix = CountLeadingZeros( firstCode ^ splitCode );

            if ( splitPrefix > commonPrefix ) {
                split = newSplit;		// accept proposal
			}
        }
    }
    while ( step > 1 );

    return split;
}

/*
====================================================
LBVH::AltBuild
This one has a bug in it
====================================================
*/
LBVH::int3_t LBVH::AltBuild( node_t * leafNodes, int num, int idx ) {
	// Determine direction of the range (+1 or -1)
	int deltaNeg = Delta( leafNodes, num, idx, idx - 1 );
	int deltaPos = Delta( leafNodes, num, idx, idx + 1 );
	int direction = ( ( deltaPos - deltaNeg ) > 0 ) ? 1 : -1;

	// Compute upper bound for the length of the range
	int deltaMin = Delta( leafNodes, num, idx, idx - direction );
	int lmax = 2;
	while ( Delta( leafNodes, num, idx, idx + lmax * direction ) > deltaMin ) {
		lmax = lmax * 2;
	}

	// Find the other end using binary search
	int l = 0;
	for ( int t = ( lmax >> 1 ); t > 0; t >>= 1 ) {
		int delta = Delta( leafNodes, num, idx, idx + ( l + t ) * direction );
		if ( delta > deltaMin ) {
			l = l + t;
		}
	}
	int j = idx + l * direction;

	// Find the split position using binary search
	int deltaNode = Delta( leafNodes, num, idx, j );
	int s = 0;
	for ( int t = ( l >> 1 ); t > 0; t >>= 1 ) {
		int delta = Delta( leafNodes, num, idx, idx + ( s + t ) * direction );
		if ( delta > deltaNode ) {
			s = s + t;
		}
	}

	const int gamma = idx + s * direction + std::min( direction, 0 );

	int3_t result;
	result.x = std::min( idx, j );
	result.y = std::max( idx, j );
	result.z = gamma;

	if ( gamma == std::min( idx, j ) ) {
		m_internalNodes[ idx ].m_left = &leafNodes[ gamma ];
		printf( "submitting leaf: %i\n", gamma );
	} else {
		m_internalNodes[ idx ].m_left = &m_internalNodes[ gamma ];
	}

	if ( ( gamma + 1 ) == std::max( idx, j ) ) {
		m_internalNodes[ idx ].m_right = &leafNodes[ gamma + 1 ];
		printf( "submitting leaf: %i\n", gamma + 1 );
	} else {
		m_internalNodes[ idx ].m_right = &m_internalNodes[ gamma + 1 ];
	}

	m_internalNodes[ idx ].m_left->m_parent = &m_internalNodes[ idx ];
    m_internalNodes[ idx ].m_right->m_parent = &m_internalNodes[ idx ];

	return result;
}













/*
====================================================
RadixSortBase10
====================================================
*/
void RadixSortBase10( int * input, int num ) {
	int * output = (int *)alloca( sizeof( int ) * num );
	for ( int i = 0; i < num; i++ ) {
		output[ i ] = 0;
	}

	static const int base = 10;

	int digitsToDo = 3;
	for ( int d = 0; d < digitsToDo; d++ ) {
		// Get the counts table
		int counts[ base ] = { 0 };
		for ( int i = 0; i < num; i++ ) {
			int value = input[ i ];
			for ( int shift = 0; shift < d; shift++ ) {
				value /= base; // shift the digits over by d
			}
			value %= base;

			counts[ value ]++;
		}

		// Build the prefix sum table
		int sum = 0;
		int prefixSum[ base ];
		for ( int i = 0; i < base; i++ ) {
			sum += counts[ i ];
			prefixSum[ i ] = sum;
		}

		for ( int i = num - 1; i >= 0; i-- ) {
			int value = input[ i ];
			for ( int shift = 0; shift < d; shift++ ) {
				value /= base; // shift the digits over by d
			}
			value %= base;

			int slot = prefixSum[ value ] - 1;
			prefixSum[ value ]--;
			output[ slot ] = input[ i ];
		}

		// Copy output back to input
		for ( int i = 0; i < num; i++ ) {
			input[ i ] = output[ i ];
		}
	}
}

/*
====================================================
RadixSortBase256
====================================================
*/
void RadixSortBase256( int * input, int num ) {
	int * output = (int *)alloca( sizeof( int ) * num );
	for ( int i = 0; i < num; i++ ) {
		output[ i ] = 0;
	}

	static const int bits = 8;
	static const int base = 1 << bits;//256;
	static const int modulus = base - 1;//0xff;

	int digitsToDo = 32 / bits;//4;
	for ( int d = 0; d < digitsToDo; d++ ) {
		// Get the counts table
		int counts[ base ] = { 0 };
		for ( int i = 0; i < num; i++ ) {	// easily calculated in parallel with atomic increment ( or each thread is assigned a counter index and each thread loops over all values and increments that single counter [probably worse performance but these old articles are from before atomic incrementers on gpus existed] )
			int value = input[ i ];
			value >>= bits * d;	// shift the digits over by d
			value = value & modulus;

			counts[ value ]++;
		}

		// Build the prefix sum table
		int sum = 0;
		int prefixSum[ base ];
		for ( int i = 0; i < base; i++ ) {
			sum += counts[ i ];
			prefixSum[ i ] = sum;
		}

		for ( int i = num - 1; i >= 0; i-- ) {
			int value = input[ i ];
			value >>= bits * d;	// shift the digits over by d
			value = value & modulus;

			int slot = prefixSum[ value ] - 1;
			prefixSum[ value ]--;
			output[ slot ] = input[ i ];
		}

		// Copy output back to input
		for ( int i = 0; i < num; i++ ) {
			input[ i ] = output[ i ];
		}
	}
}

void RadixSortBitPsuedoParallel( int * input, int num ) {
	int * output = (int *)alloca( sizeof( int ) * num );
	int * eTable = (int *)alloca( sizeof( int ) * num );
	for ( int i = 0; i < num; i++ ) {
		output[ i ] = 0;
		eTable[ i ] = 0;
	}

	static const int bits = 1;
	static const int base = 1 << bits;
	static const int modulus = base - 1;

	int digitsToDo = 32 / bits;
	for ( int digitIdx = 0; digitIdx < digitsToDo; digitIdx++ ) {
		// Get the counts table
		for ( int i = 0; i < num; i++ ) {	// easily calculated in parallel with atomic increment ( or each thread is assigned a counter index and each thread loops over all values and increments that single counter [probably worse performance but these old articles are from before atomic incrementers on gpus existed] )
			int value = input[ i ];
			value >>= bits * digitIdx;	// shift the digits over by d
			value = value & modulus;

			eTable[ i ] = ( ( ~value ) & 1 ); // If value is 1, then set eTable entry 0... If value is 0, then set eTable entry 1
		}

		// This is the parallel version (this would still have to run serially on a gpu though... so weird, but hey)
		int totalFalses = 0;
		int * falses = (int *)alloca( sizeof( int ) * num );
		for ( int i = 0; i < num; i++ ) {
			falses[ i ] = totalFalses;
			totalFalses += eTable[ i ];			
		}

		// This chunk can run in parallel
		int * dIndices = (int *)alloca( sizeof( int ) * num );
		for ( int i = 0; i < num; i++ ) {
			int f = falses[ i ];
			int t = i - f + totalFalses;
			int b = ( ( ~eTable[ i ] ) & 1 );
			dIndices[ i ] = ( b > 0 ) ? t : f;
		}

		// This can now be done in parallel
		for ( int i = 0; i < num; i++ ) {
			int d = dIndices[ i ];
			output[ d ] = input[ i ];
		}

		// Copy output back to input
		for ( int i = 0; i < num; i++ ) {
			input[ i ] = output[ i ];
		}
	}
}


void PairSort( int & a, int & b ) {
	if ( a > b ) {
		int tmp = a;
		a = b;
		b = tmp;
	}
}

void Transpose( int * data, const int N1, const int N2 ) {
	const int N = N1 * N2;
	int * tmp = (int *)alloca( N * sizeof( int ) );

	for ( int i = 0; i < N; i++ ) {
		tmp[ i ] = data[ i ];
	}

	for ( int n1 = 0; n1 < N1; n1++ ) {
		for ( int n2 = 0; n2 < N2; n2++ ) {
			data[ n2 + N2 * n1 ] = tmp[ n1 + N1 * n2 ];
		}
	}
}


void PrintData( int * data, int N1, int N2 ) {
	printf( "------------------------------\n" );
	for ( int j = 0; j < N2; j++ ) {
		for ( int i = 0; i < N1; i++ ) {
			int idx = N1 * j + i;
			printf( "%i   ", data[ idx ] );
		}
		printf( "\n" );
	}
	printf( "------------------------------\n" );
}


int CompareIntegers( const void * p1, const void * p2 ) {
	int a = *(int*)p1;
	int b = *(int*)p2;

	if ( a < b ) {
		return -1;
	}

	if ( a == b ) {
		return 0;
	}

	return 1;
}

//qsort( contacts, numContacts, sizeof( contact_t ), CompareContacts );

void TransposeSortPowerOf2( int * data, const int num ) {
	int * indices = (int *)alloca( num * sizeof( int ) );
	for ( int i = 0; i < num; i++ ) {
		indices[ i ] = i;
	}

	int numRecurssions = 0;
	int tmp = num >> 1;
	while ( tmp > 0 ) {
		numRecurssions++;
		tmp >>= 1;
	}
/*
	for ( int doitAgain = 0; doitAgain < 1; doitAgain++ ) {
		for ( int r = 0; r < numRecurssions; r++ ) {
			int stride = 1 << r;

			for ( int i = 0; i < num / 2; i++ ) {
				int idxA = indices[ 2 * i + 0 ];
				int idxB = indices[ 2 * i + 1 ];

				PairSort( data[ idxA ], data[ idxB ] );
			}

			for ( int i = 0; i < num / 2; i++ ) {
				int idxA = indices[ 2 * i + 0 ];
				int idxB = indices[ 2 * i + 1 ];

				idxA = ( idxA + 1 ) % num;
				idxB = ( idxB + 1 ) % num;

				PairSort( data[ idxA ], data[ idxB ] );
			}

			//Transpose( indices, num / 2, 2 );
			Transpose( indices, 2, num / 2 );
		}

// 		PrintData( data, numRecurssions, numRecurssions );
// 		Transpose( data, numRecurssions, numRecurssions );
// 		PrintData( data, numRecurssions, numRecurssions );
// 
// 		for ( int r = 0; r < numRecurssions; r++ ) {
// 			int stride = 1 << r;
// 
// 			for ( int i = 0; i < num / 2; i++ ) {
// 				int idxA = indices[ 2 * i + 0 ];
// 				int idxB = indices[ 2 * i + 1 ];
// 
// 				PairSort( data[ idxA ], data[ idxB ] );
// 			}
// 
// 			//Transpose( indices, num / 2, 2 );
// 			Transpose( indices, 2, num / 2 );
// 		}
	}
*/

	for ( int i = 0; i < numRecurssions; i++ ) {
		qsort( data + i * numRecurssions, numRecurssions, sizeof( int ), CompareIntegers );
	}

	Transpose( data, numRecurssions, numRecurssions );

	for ( int i = 0; i < numRecurssions; i++ ) {
		qsort( data + i * numRecurssions, numRecurssions, sizeof( int ), CompareIntegers );
	}
	


	PrintData( data, numRecurssions, numRecurssions );


	printf( "boob" );
	printf( "boob" );
	printf( "boob\n" );
}

void SimpleBitonic( int * data, int num ) {
	int toggler = 0;

	for ( int iter = 0; iter < num; iter++ ) {
		for ( int tid = 0; tid < num / 2; tid++ ) {
			int idxA = 2 * tid + 0 + ( toggler & 1 );
			int idxB = 2 * tid + 1 + ( toggler & 1 );

			if ( idxB >= num ) {
				continue;
			}

			if ( data[ idxA ] > data[ idxB ] ) {
				std::swap( data[ idxA ], data[ idxB ] );
			}
		}

		toggler++;
	}
}


void SimpleOddEvenMerge( int * data, int num ) {
	int toggler = 1;

	for ( int iter = 0; iter < num; iter++ ) {
		//
		//	Perform a step of odd/even
		//
		for ( int tid = 0; tid < num / 2; tid++ ) {
			int idxA = 2 * tid + 0 + ( toggler & 1 );
			int idxB = 2 * tid + 1 + ( toggler & 1 );

			if ( idxB >= num ) {
				continue;
			}

			if ( data[ idxA ] > data[ idxB ] ) {
				std::swap( data[ idxA ], data[ idxB ] );
			}
		}

		//
		//	Merge the odd/even pairs
		//
		for ( int offset = 0; offset < num / 4; offset++ ) {
			qsort( data + offset * 4, 4, sizeof( int ), CompareIntegers );
		}

		toggler++;
		toggler = 1;
	}
}


void SimpleOddEvenQuickSort( int * data, int num ) {
	int toggler = 1;
	int groupSize = 32;//8;
	int numThreads = num / groupSize;
	int maxIters = ( ( num / groupSize ) + 1 );
	maxIters = 32 * 2;

	for ( int iter = 0; iter < maxIters; iter++ ) {
		//
		//	Merge the odd/even pairs
		//
		for ( int tid = 0; tid < numThreads; tid++ ) {
			int idx = tid * groupSize + ( toggler & 1 ) * groupSize / 2;
			if ( idx + groupSize <= num ) {
				qsort( data + idx, groupSize, sizeof( int ), CompareIntegers );
			}
		}

		toggler++;
	}
}




/*
====================================================
PrintData
====================================================
*/
static void PrintData( int * data, int num ) {
	for ( int i = 0; i < num; i++ ) {
		printf( " %i", data[ i ] );
	}
	printf( "\n" );
}

/*
====================================================
TestRadixSort
====================================================
*/
int TestRadixSort() {
#if 0
	int data[ 16 ];
	data[ 0 ] = 3;
	data[ 1 ] = 9;
	data[ 2 ] = 1;
	data[ 3 ] = 7;
	data[ 4 ] = 4;
	data[ 5 ] = 8;
	data[ 6 ] = 10;
	data[ 7 ] = 2;
	data[ 8 ] = 8;
	data[ 9 ] = 0;
	data[ 10 ] = 3;
	data[ 11 ] = 6;
	data[ 12 ] = 13;
	data[ 13 ] = 2;
	data[ 14 ] = 15;
	data[ 15 ] = 16;//-1;

	printf( "\n" );
	printf( "Radix Sort---------------------------------------------------------\n" );
	printf( "data = " );
	PrintData( data, 16 );

//	CountingSort( data, 13, 20 );

#if 1
	data[ 0 ] = 277;
	data[ 1 ] = 806;
	data[ 2 ] = 681;
	data[ 3 ] = 462;
	data[ 4 ] = 787;
	data[ 5 ] = 163983648;
	data[ 6 ] = 284;
	data[ 7 ] = 166;
	data[ 8 ] = 905;
	data[ 9 ] = 518;
	data[ 10 ] = 263;
	data[ 11 ] = 395;
	data[ 12 ] = 988;
	data[ 13 ] = 307;
	data[ 14 ] = 779;
	data[ 15 ] = 721;
#endif
	//RadixSortBase10( data, 16 );
//	RadixSortBase256( data, 16 );
//	RadixSortBitPsuedoParallel( data, 16 );

	{
		static const int num = 1 << 4;
		int data2[ num ];
		for ( int i = 0; i < num; i++ ) {
			data2[ i ] = int( Random::Get() * 1000.0f );
		}
		TransposeSortPowerOf2( data2, num );
	}

	printf( "sorted = " );
	PrintData( data, 16 );
	printf( "---------------------------------------------------------\n" );


	
	{
		static const int num = 1 << 10;
		int data2[ num ];
		for ( int i = 0; i < num; i++ ) {
			data2[ i ] = int( Random::Get() * 100.0f );
		}
		printf( "\n" );
		printf( "SimpleBitonic Sort---------------------------------------------------------\n" );
		printf( "data = " );
		PrintData( data2, num );

		//SimpleBitonic( data2, num );
		//SimpleOddEvenMerge( data2, num );
		SimpleOddEvenQuickSort( data2, num );

		printf( "\n\nsorted = " );
		PrintData( data2, num );
		printf( "---------------------------------------------------------\n" );
	}
#endif
	return 0;
}
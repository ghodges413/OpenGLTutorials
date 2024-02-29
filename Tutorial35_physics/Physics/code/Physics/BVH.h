//
//	BVH.h
//
#pragma once
#include "Math/Bounds.h"
#include "Physics/PhysicsWorld.h"


class BVH {
public:
	BVH() {}

	struct node_t {
		float Cost();
		void Reset();

		Bounds m_bounds;
		int m_parent;	// -1 if base node that starts the tree
		int m_left;		// -1 if no child (happens when this is a leaf)
		int m_right;	// -1 if no child (happens when this is a leaf)
		int m_bodyId;	// -1 if no body (happens when there's children)
	};

	node_t m_nodes[ PhysicsWorld::m_maxBodies * 2 ];	// Max physics bodies times 2

	void Build();
};

/*
========================================================================================================

BoundingVolumeHierarchy

========================================================================================================
*/

class BoundingVolumeHierarchy {
public:
	BoundingVolumeHierarchy() : m_nodes( NULL ) {}
	~BoundingVolumeHierarchy() {
		delete m_nodes;
		m_nodes = NULL;
	}

	void Build( const PhysicsWorld * world, const float dt_sec );

	void GetCollisions( const Bounds & bounds, const int skipId, std::vector< int > & bodyIds ) const;

public:
	struct bodyBounds_t {
		int m_bodyId;
		Bounds m_bounds;
	};
	struct node_t {
		node_t( bodyBounds_t * bodyIds, int numBodies );
		~node_t() {
			delete m_left;
			delete m_right;
			Reset();
		}
		void Reset() {
			//m_parent = NULL;
			m_left = NULL;
			m_right = NULL;
			m_bodyId = -1;
			m_bounds.Clear();
		}

		//node_t * m_parent;	// -1 if base node that starts the tree ( we don't need this unless we start doing tree rotations )
		node_t * m_left;		// -1 if no child (happens when this is a leaf)
		node_t * m_right;	// -1 if no child (happens when this is a leaf)

		int m_bodyId;	// -1 if no body
		Bounds m_bounds;

		void GetCollisions_r( const Bounds & bounds, const int skipId, std::vector< int > & bodyIds ) const;
	};

	node_t * m_nodes;

private:
	void Print_r( const node_t * node, int level ) const;
};


/*
========================================================================================================

LBVH - Linear Bounding Volume Hierarchy

This LBVH is based upon the paper "Maximizing Parallelism in the Construction of BVHs, oCtrees, and K-d Trees" by Karras
His blog post "Thinking in Parallel Part 3, tree construction on the gpu" is also useful.
https://developer.nvidia.com/blog/thinking-parallel-part-iii-tree-construction-gpu/

========================================================================================================
*/

class LBVH {
public:
	LBVH() {}
	~LBVH() {}

	void Build( const PhysicsWorld * world, const float dt_sec );

	void GetCollisions( const Bounds & bounds, const int skipId, std::vector< int > & bodyIds ) const;

public:
	struct bodyBounds_t {
		int m_bodyId;
		Bounds m_bounds;
	};
	struct node_t {
		node_t() { Reset(); }
		~node_t() {
			Reset();
		}
		void Reset() {
			m_parent = NULL;
			m_left = NULL;
			m_right = NULL;
			m_key = 0;
			m_bodyId = -1;
			m_bounds.Clear();
		}

		node_t * m_parent;	// -1 if base node that starts the tree ( we don't need this unless we start doing tree rotations )
		node_t *  m_left;		// -1 if no child (happens when this is a leaf)
		node_t *  m_right;	// -1 if no child (happens when this is a leaf)
		unsigned int m_key;

		int m_bodyId;	// -1 if no body
		Bounds m_bounds;

		void GetCollisions_r( const Bounds & bounds, const int skipId, std::vector< int > & bodyIds, int recurssion ) const;
	};

	void BuildParentBounds_r( node_t * node );

	node_t m_leafNodes[ PhysicsWorld::m_maxBodies ];
	node_t m_internalNodes[ PhysicsWorld::m_maxBodies ];

private:
	struct int2_t {
		int x;
		int y;
	};
	struct int3_t {
		int x;
		int y;
		int z;
	};
	static int Delta( node_t * leafNodes, int num, int i, int j );
	static int CountLeadingZeros( const unsigned int x );
	static int2_t DetermineRange( node_t * leafNodes, int numUsedBodies, int idx );
	static int FindSplit( node_t * leafNodes, int num, int first, int last );
	int3_t AltBuild( node_t * leafNodes, int num, int idx );

	void Print_r( const node_t * node, int level ) const;
};
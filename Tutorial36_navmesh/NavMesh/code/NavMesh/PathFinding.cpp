//
//  PathFinding.cpp
//
#include "NavMesh/PathFinding.h"
#include "Math/Matrix.h"
#include "Graphics/Mesh.h"

extern MatN g_navAdjacencyMatrix;

extern std::vector< vert_t > s_navmeshVerts;
extern std::vector< int > s_navmeshIndices;
extern std::vector< Bounds > s_navmeshBounds;


Vec3 GetNodePos( int idx ) {
	int idx0 = s_navmeshIndices[ 3 * idx + 0 ];
	int idx1 = s_navmeshIndices[ 3 * idx + 1 ];
	int idx2 = s_navmeshIndices[ 3 * idx + 2 ];

	Vec3 a = s_navmeshVerts[ idx0 ].pos;
	Vec3 b = s_navmeshVerts[ idx1 ].pos;
	Vec3 c = s_navmeshVerts[ idx2 ].pos;

	return ( a + b + c ) / 3.0f;
}


int GetNodeIndex( Vec3 pt ) {
	for ( int i = 0; i < s_navmeshBounds.size(); i++ ) {
		const Bounds & bounds = s_navmeshBounds[ i ];
		if ( !bounds.DoesIntersect( pt ) ) {
			continue;
		}

		int idx0 = s_navmeshIndices[ 3 * i + 0 ];
		int idx1 = s_navmeshIndices[ 3 * i + 1 ];
		int idx2 = s_navmeshIndices[ 3 * i + 2 ];

		const Vec3 & a = s_navmeshVerts[ idx0 ].pos;
		const Vec3 & b = s_navmeshVerts[ idx1 ].pos;
		const Vec3 & c = s_navmeshVerts[ idx2 ].pos;

		Vec3 norm = ( b - a ).Cross( c - a );
		norm.Normalize();

		Vec3 ap = pt - a;
		Vec3 p = pt - norm * ap.Dot( norm );
		
		float areaC = ( a - p ).Cross( b - p ).Dot( norm );
		float areaA = ( b - p ).Cross( c - p ).Dot( norm );
		float areaB = ( c - p ).Cross( a - p ).Dot( norm );
		if ( areaA < 0 || areaB < 0 || areaC < 0 ) {
			continue;
		}

		return i;
	}
	return -1;
}


int GetNodeNeighborIndices( const Vec3 & pt, int * tris ) {
	int idx = GetNodeIndex( pt );
	if ( -1 == idx ) {
		return 0;
	}

	int num = 0;
	for ( int i = 0; i < g_navAdjacencyMatrix.numDimensions; i++ ) {
		float val = g_navAdjacencyMatrix.rows[ idx ][ i ];
		if ( val > 0 ) {
			tris[ num ] = i;
			++num;
		}
		if ( 3 == num ) {
			break;
		}
	}

	return num;
}



/*
================================================================

PathFind

This function is pretty simple.  It uses the A* algorithm to find the path
from the start position to the end position in the navmesh.

A* is a path finding algorithm that is based on a combination of
Dijkstra's algorithm and Greedy Best First Search

It begins with the starting node.  It looks at its neighbors and
assigns each one a score, called an "f-score".  The f-score is
combination of how much "distance" has been traversed to reach the
node (g-score) and the heuristic "distance" from that node to the
target node.  It puts these neighbor nodes onto the "open" list,
and the "parent" node or "calling" node is put on the "closed" list.
Each neighbor node records the id of the node that put it on the
open list; we will call this the parent node id.  Then the next
node in the open list with the lowest f-score is called, and the
process is repeated until we reach the target node.

We then use the closed list to walk backwards, using the recorded
parent id's, to get the straightest path back to the source.

================================================================
*/

struct astarNode_t {
	int idx;	// index into the navmesh node's
	int daddyId;	// Who is your daddy and what does he do?
	// TODO: Add the edge id that connects this node to the parent.
	//		We will be able to use that to get the actual NPC/AI to walk to the edge without cutting corners
	Vec3 pos;
	float g_score;
	float f_score;
};

std::vector< astarNode_t > g_openList;		// Node's that are open for consideration
std::vector< astarNode_t > g_closedList;	// Node's that have already been explored
std::vector< astarNode_t > g_finalPath;		// The sorted list of nodes from start to end
std::vector< Vec3 > g_finalPathPoints;		// For now, used to debug

/*
================================
SkipNode
================================
*/
bool SkipNode( const std::vector< astarNode_t > & list, const int idx, const float f_factor, int & replaceListIdx ) {
	// Check if this node already exists, and whether we skip or replace
	for ( int i = 0; i < list.size(); i++ ) {
		const astarNode_t & node = list[ i ];
		if ( node.idx == idx ) {
			if ( f_factor < node.f_score ) {
				replaceListIdx = i;
			}
			return true;
		}
	}
	return false;
}

/*
================================
PathFind
================================
*/
Vec3 PathFind( const Vec3 & start, const Vec3 & end ) {
	g_openList.clear();
	g_closedList.clear();
	g_finalPath.clear();
	g_finalPathPoints.clear();

	// Check that the end point is in the navmesh
	const int endIdx = GetNodeIndex( end );
	if ( -1 == endIdx ) {
		return start;
	}

	// Check that the start point is in the navmesh
	const int startIdx = GetNodeIndex( start );
	if ( -1 == startIdx ) {
		return start;
	}
	
	// If start and end are in the same node, then we're good to go
	if ( startIdx == endIdx ) {
		return end;
	}

	Vec3 startPos = GetNodePos( startIdx );
	Vec3 endPos = GetNodePos( endIdx );

	astarNode_t startNode;
	startNode.idx = startIdx;
	startNode.pos = startPos;
	startNode.g_score = 0;
	startNode.f_score = 0; // The start node doesn't need a heuristic applied to it
	startNode.daddyId = -1;
	g_openList.push_back( startNode );

	while ( g_openList.size() > 0 ) {
		// Process the node with the lowest f-score
		int bestIdx = 0;
		float bestF = g_openList[ 0 ].f_score;
		for ( int i = 1; i < g_openList.size(); i++ ) {
			if ( g_openList[ i ].f_score < bestF ) {
				bestF = g_openList[ i ].f_score;
				bestIdx = i;
			}
		}
		astarNode_t currentNode = g_openList[ bestIdx ];	// also known as 'q'
		
		// Check if this node is the end node.
		// If it is, then put it on the closed list and terminate the loop
		if ( currentNode.idx == endIdx ) {
			g_closedList.push_back( currentNode );
			break;
		}
		
		// Get the neighboring node indices for the starting position
		int nodes[ 3 ];
		int num = GetNodeNeighborIndices( currentNode.pos, nodes );
		if ( 0 == num ) {
			return start;
		}

		// Add the current node to the closed list and remove from the open list
		g_closedList.push_back( currentNode );
		g_openList.erase( g_openList.begin() + bestIdx );

		// Calculating neighboring heuristics and add to the open list
		for ( int i = 0; i < num; i++ ) {
			astarNode_t neighorNode;
			neighorNode.idx = nodes[ i ];
			neighorNode.daddyId = currentNode.idx;

			neighorNode.pos = GetNodePos( nodes[ i ] );

			// Calculate the Euclidean heuristic and g_score
			float heuristic = ( endPos - neighorNode.pos ).GetMagnitude();
			float g_score = ( currentNode.pos - neighorNode.pos ).GetMagnitude();

			neighorNode.g_score = currentNode.g_score + g_score;
			neighorNode.f_score = neighorNode.g_score + heuristic;

			// If this node already exists, with a higher f-score, then skip it.
			// If this node already exists, with a lower f-score, then replace it.
			// Skip the replacing for right now
			// TODO: Implement the replacement
			int replaceListIdx = -1;
			if ( SkipNode( g_openList, neighorNode.idx, neighorNode.f_score, replaceListIdx ) ) {
				continue;
			}
			if ( SkipNode( g_closedList, neighorNode.idx, neighorNode.f_score, replaceListIdx ) ) {
				continue;
			}

			g_openList.push_back( neighorNode );
		}
	}

	// If the closed list is empty, then don't bother to path anywhere
	if ( 0 == g_closedList.size() ) {
		return start;
	}

	// If the closed list does not contain the end node, then don't bother to path anywhere
	astarNode_t currentNode = g_closedList[ g_closedList.size() - 1 ];
	if ( currentNode.idx != endIdx ) {
		return start;
	}

	// Using the parent id's, loop from the end to the beginning, this is the final path (in reverse order)
	g_finalPath.push_back( currentNode );
	while ( currentNode.idx != startIdx ) {
		const int parent = currentNode.daddyId;

		for ( int i = 0; i < g_closedList.size(); i++ ) {
			if ( parent == g_closedList[ i ].idx ) {
				currentNode = g_closedList[ i ];
				g_finalPath.push_back( currentNode );
				break;
			}
		}
	}

	for ( int i = g_finalPath.size() - 1; i >= 0; i-- ) {
		g_finalPathPoints.push_back( g_finalPath[ i ].pos );
	}

	int nextTile = g_finalPath.size() - 2;
	Vec3 nextPos = g_finalPath[ nextTile ].pos;
	return nextPos;
}

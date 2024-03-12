//
//  BehaviorTreeFile.cpp
//
#include "BehaviorTree/BehaviorTreeFile.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Miscellaneous/Fileio.h"
#include <stdlib.h>
#include <assert.h>
#include <vector>

struct nodeToken_t {
	int parent;
	char name[ 64 ];
	int numChildren;
};

static std::vector< nodeToken_t > s_nodes;

char * RemoveLeadingWhiteSpace( char * buff, int max ) {
	for ( int i = 0; i < max; i++ ) {
		if ( '\t' != buff[ i ] && ' ' != buff[ i ] ) {
			return buff + i;
		}
	}

	return buff;
}

bool HasChildren( char * buff, int max ) {
	for ( int i = 0; i < max; i++ ) {
		if ( '{' == buff[ i ] ) {
			return true;
		}
	}
	return false;
}

void ReadNode_r( FILE * fp, int parent ) {
	if ( feof( fp ) ) {
		return;
	}

	char buff[ 512 ] = { 0 };

	nodeToken_t node;
	node.parent = parent;
	node.numChildren = 0;

	// Read whole line
	fgets( buff, sizeof( buff ), fp );
	buff[ 511 ] = '\0';

	char * reader = buff;
	reader = RemoveLeadingWhiteSpace( reader, 512 );
	if ( '}' == reader[ 0 ] ) {
		// This is the end of the children
		return;
	}

	// Attempt to read node name
	if ( sscanf( reader, "\"%s\"", node.name ) != 1 ) {
		return;
	}

	// Hack, check for trailing non-characters
	for ( int i = 0; i < 64; i++ ) {
		if ( node.name[ i ] == '\"' ) {
			node.name[ i ] = '\0';
			break;
		}
	}
	
	// Awesome, got the name
	const int nodeId = s_nodes.size();
	s_nodes.push_back( node );

	// Check for children
	bool hasChildren = HasChildren( buff, 512 );
	if ( !hasChildren ) {
		return;
	}

	// Read children
	while ( !feof( fp ) ) {
		// Get the current file position
		fpos_t pos;
		fgetpos( fp, &pos );

		// Read whole line
		fgets( buff, sizeof( buff ), fp );
		buff[ 511 ] = '\0';

		char * reader = buff;
		reader = RemoveLeadingWhiteSpace( reader, 512 );

		// This is the end of the children
		if ( '}' == reader[ 0 ] ) {
			return;
		}

		// Rewind to the previous position
		// Otherwise we will skip a line
		fsetpos( fp, &pos );

		// Read more nodes
		ReadNode_r( fp, nodeId );
	}
}

action_t * GetActionFromName( const char * name ) {
	if ( 0 == strcmp( name, "SpiderRoot" ) ) {
		return SpiderRoot;
	}
	if ( 0 == strcmp( name, "Idle" ) ) {
		return Idle;
	}
	if ( 0 == strcmp( name, "WalkToPlayer" ) ) {
		return WalkToPlayer;
	}
	if ( 0 == strcmp( name, "Sequence" ) ) {
		return Sequence;
	}
	if ( 0 == strcmp( name, "Selector" ) ) {
		return Selector;
	}
	if ( 0 == strcmp( name, "GetPlayerPos" ) ) {
		return GetPlayerPos;
	}
	if ( 0 == strcmp( name, "WalkToPosition" ) ) {
		return WalkToPosition;
	}

	assert( false );
	return NULL;	// This shouldn't happen
}

void BuildTree_r( btNode_t * parent, const int parentIdx ) {
	int childStart = parentIdx + 1;

	// We will build our basic behavior tree here
	for ( int i = 0; i < parent->numChildren; i++ ) {
		btNode_t * child = parent->children + i;

		// Find a child node of the parent in the list of parsed nodes
		int childIdx = -1;
		for ( int j = childStart; j < s_nodes.size(); j++ ) {
			if ( s_nodes[ j ].parent == parentIdx ) {
				childStart = j + 1;
				childIdx = j;
				break;
			}
		}
		const nodeToken_t & node = s_nodes[ childIdx ];

		// Build the child
		strcpy( child->name, node.name );
		child->action = GetActionFromName( node.name );
		child->state = BT_READY;
		child->numChildren = node.numChildren;
		child->children = NULL;
		if ( node.numChildren > 0 ) {
			child->children = (btNode_t *)malloc( sizeof( btNode_t ) * node.numChildren );
		}

		// Build the children of this node
		for ( int j = 0; j < node.numChildren; j++ ) {
			BuildTree_r( child, childIdx );
		}
	}
}

/*
================================
ReadBehaviorTreeFile

Reads the behavior tree file, and when successful returns the root node of the behavior tree.
If it fails to read/parse, then it returns null.
================================
*/
btNode_t * ReadBehaviorTreeFile( const char * fileName ) {
	printf( "Loading model: %s\n", fileName );
	char buff[ 512 ] = { 0 };

	char fullPath[ 2048 ];
	RelativePathToFullPath( fileName, fullPath );	
	
	FILE * fp = fopen( fullPath, "rb" );
	if ( !fp ) {
		fprintf( stderr, "Error: couldn't open \"%s\"!\n", fullPath );
		return false;
    }
	
	while ( !feof( fp ) ) {
		ReadNode_r( fp, -1 );
    }
	fclose( fp );

	// Calculate the number of children for each node
	for ( int i = 0; i < s_nodes.size(); i++ ) {
		nodeToken_t & node = s_nodes[ i ];

		for ( int j = i + 1; j < s_nodes.size(); j++ ) {
			if ( i == s_nodes[ j ].parent ) {
				++node.numChildren;
			}
		}
	}

	if ( 0 == s_nodes.size() ) {
		return NULL;
	}

	btNode_t fakeRoot;
	fakeRoot.numChildren = 1;
	fakeRoot.children = (btNode_t *)malloc( sizeof( btNode_t ) );
	BuildTree_r( &fakeRoot, -1 );
	return fakeRoot.children;
}

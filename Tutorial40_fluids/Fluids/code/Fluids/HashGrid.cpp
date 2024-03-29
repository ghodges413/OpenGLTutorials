//
//	HashGrid.cpp
//
#include "Graphics/ShaderStorageBuffer.h"
#include "Graphics/Graphics.h"
#include "Graphics/ShaderManager.h"
#include "Fluids/HashGrid.h"
#include "Fluids/FluidSim.h"
#include "Math/Bounds.h"
#include <vector>
#include <array>
#include <stdio.h>
#include <string>

#define GRID_SPACING PARTICLE_RADIUS

#define RESOLUTION_X 20
#define RESOLUTION_Y 40
#define RESOLUTION_Z 40

#define NUM_CELLS ( RESOLUTION_X * RESOLUTION_Y * RESOLUTION_Z )

struct vec3i {
	int x;
	int y;
	int z;
};

/*
====================================
GetGridPos
====================================
*/
vec3i GetGridPos( Vec3 pos ) {
	vec3i v;
	v.x = pos.x / GRID_SPACING;
	v.y = pos.y / GRID_SPACING;
	v.z = pos.z / GRID_SPACING;
	return v;
}

/*
====================================
GetHashKeyFromBucketIndex
====================================
*/
int GetHashKeyFromBucketIndex( vec3i idx ) {
	vec3i idx2 = idx;

	idx2.x = idx.x % RESOLUTION_X;
	idx2.y = idx.y % RESOLUTION_Y;
	idx2.z = idx.z % RESOLUTION_Z;

	if ( idx2.x < 0 ) {
		idx2.x += RESOLUTION_X;
	}
	if ( idx2.y < 0 ) {
		idx2.y += RESOLUTION_Y;
	}
	if ( idx2.z < 0 ) {
		idx2.z += RESOLUTION_Z;
	}

	int key = idx2.x + RESOLUTION_X * ( idx2.y + RESOLUTION_Y * idx2.z );
	return key;
}

/*
====================================
GetHashKeyFromPosition
====================================
*/
int GetHashKeyFromPosition( Vec3 pos ) {
	vec3i idx = GetGridPos( pos );
	return GetHashKeyFromBucketIndex( idx );
}


/*
========================================================================

Bucket Grid Cells

This hash grid is an infinite set of cells that are mapped to a finite number of cells.
Particles are sorted into buckets for quick look up of neighbor particles.

========================================================================
*/

std::array< std::vector< int >, RESOLUTION_X * RESOLUTION_Y * RESOLUTION_Z > g_buckets;
std::array< std::vector< int >, MAX_PARTICLES > g_neighbors;

/*
====================================
BuildGrid
====================================
*/
void BuildGrid( fluid_t * points, const int num ) {
	for ( int i = 0; i < num; i++ ) {
		int key = GetHashKeyFromPosition( points[ i ].pos );
		g_buckets[ key ].push_back( i );
	}
}

/*
====================================
GetNearbyKeys
====================================
*/
void GetNearbyKeys( Vec3 pos, int * nearbyKeys ) {
	vec3i idx = GetGridPos( pos );
	vec3i nearbyBucketIndices[ 3 * 3 * 3 ];

	for ( int z = 0; z < 3; z++ ) {
		for ( int y = 0; y < 3; y++ ) {
			for ( int x = 0; x < 3; x++ ) {
				int i = x + 3 * y + ( 3 * 3 * z );
				nearbyBucketIndices[ i ] = idx;
				nearbyBucketIndices[ i ].x += x - 1;
				nearbyBucketIndices[ i ].y += y - 1;
				nearbyBucketIndices[ i ].z += z - 1;
			}
		}
	}

	for ( int i = 0; i < 27; i++ ) {
		nearbyKeys[ i ] = GetHashKeyFromBucketIndex( nearbyBucketIndices[ i ] );
	}
}

/*
====================================
GetNeighborIds
====================================
*/
void GetNeighborIds( std::vector< int > & ids, const Vec3 & pt ) {
	int keys[ 27 ] = { -1 };
	GetNearbyKeys( pt, keys );

	ids.clear();
	for ( int i = 0; i < 27; i++ ) {
		const int key = keys[ i ];
		const std::vector< int > & bucket = g_buckets[ key ];

		for ( int j = 0; j < bucket.size(); j++ ) {
			ids.push_back( bucket[ j ] );
		}
	}
}

/*
====================================
UpdateGrid
====================================
*/
void UpdateGrid( fluid_t * points, const int num ) {
	for ( int i = 0; i < g_buckets.size(); i++ ) {
		g_buckets[ i ].clear();
		g_neighbors[ i ].clear();
	}

	// Record the bucket location for each particle
	for ( int i = 0; i < num; i++ ) {
		int key = GetHashKeyFromPosition( points[ i ].pos );
		g_buckets[ key ].push_back( i );
	}

	// For each particle, build a neighbor list
	for ( int i = 0; i < num; i++ ) {
		GetNeighborIds( g_neighbors[ i ], points[ i ].pos );
	}
}

/*
====================================
GetNeighborIds
====================================
*/
void GetNeighborIds( std::vector< int > & ids, const int idx ) {
	ids = g_neighbors[ idx ];
}



/*
========================================================================

GPU Friendly Grid Cells

This version does not use buckets.  And therefore does not require dynamic memory allocation.
Instead the sorted array, sorts the grid cells by key.  And the start indices array tells us
where the particles in each cell begin.

========================================================================
*/

struct elem_t {
	int idx;	// particle index
	int key;	// cell key
};

// GPU Friendly version
std::array< elem_t, MAX_PARTICLES > g_sorted;
std::array< int, NUM_CELLS > g_startIndices;	// Given a key, lookup the start index into the sorted array... this gives the first element in the sorted array that is sitting in this cell

/*
====================================
CompareElement
====================================
*/
int CompareElement( const void * a, const void * b ) {
	const elem_t * ea = (const elem_t *)a;
	const elem_t * eb = (const elem_t *)b;

	if ( ea->key < eb->key ) {
		return -1;
	}
	if ( ea->key > eb->key ) {
		return 1;
	}
	return 0;
}

/*
====================================
UpdateCells
====================================
*/
void UpdateCells( fluid_t * points ) {
	for ( int i = 0; i < MAX_PARTICLES; i++ ) {
		elem_t elem;
		elem.idx = i;
		elem.key = GetHashKeyFromPosition( points[ i ].pos );
		g_sorted[ i ] = elem;
	}

	// Sort the sorted array
	qsort( g_sorted.data(), MAX_PARTICLES, sizeof( elem_t ), CompareElement );

	// Build the start indices array
	int start = 0;
	for ( int i = 0; i < NUM_CELLS; i++ ) {
		g_startIndices[ i ] = -1;

		for ( int j = start; j < MAX_PARTICLES; j++ ) {
			const elem_t & elem = g_sorted[ j ];

			if ( elem.key > i ) {
				start = j;
				break;
			}

			if ( i == elem.key ) {
				g_startIndices[ i ] = j;
				start = j + 1;
				break;
			}
		}
	}
}

/*
====================================
GetNeighborIds_Cells
====================================
*/
void GetNeighborIds_Cells( std::vector< int > & ids, const int idx ) {
	Vec3 pt = g_particles[ idx ].pos;

	int keys[ 27 ] = { -1 };
	GetNearbyKeys( pt, keys );

	ids.clear();
	for ( int i = 0; i < 27; i++ ) {
		const int key = keys[ i ];
		const int start = g_startIndices[ key ];
		if ( -1 == start ) {
			continue;
		}

		for ( int j = start; j < MAX_PARTICLES; j++ ) {
			if ( key != g_sorted[ j ].key ) {
				break;
			}

			ids.push_back( g_sorted[ j ].idx );
		}
	}
}

//
//  TerrainPool.cpp
//
#include "Heightmap.h"
#include "TerrainPool.h"
#include "TerrainTile.h"
#include "TerrainFile.h"
#include "TerrainStreamer.h"
#include "Terrain.h"
#include <math.h>
#include <algorithm>
#include <stdlib.h>
#include <assert.h>

extern int GetMaxDepth();

/*
================================
GetTerrainletsPerTile
================================
*/
int GetTerrainletsPerTile() {
	int maxDepth = GetMaxDepth();

	// Calculate the number of terrainlets in a tile
	// 1 + 4 + 16 + 32 + 64...
	int numTerrainlets = 0;
	for ( int i = 0; i <= maxDepth; i++ ) {
		numTerrainlets += ( 1 << ( i * 2 ) );
	}

	return numTerrainlets;
}

#define ISLAND_POOL_SIZE 128	// We only ever use 50-70ish islands in practice

// struct poolEntry_t {
// 	terrainlet_t terra;
// 	int mruCounter;
// 	bool wasUsedLastFrame;
// 	int mapId;
// 
// 	bool isLoaded;
// };
poolEntry_t s_islandPool[ ISLAND_POOL_SIZE ];

static int * s_islandMap = NULL;	// The lookup table that maps from island id to island pool

TerrainTileFile s_files[ NUM_TILES ];

poolEntry_t * GetPoolEntry( int id ) {
	return &s_islandPool[ id ];
}

/*
================================
GetTerrainTileFile
================================
*/
TerrainTileFile * GetTerrainTileFile( int tileX, int tileY ) {
	int tileId = tileX + tileY * TILES_WIDE;
	if ( tileId > NUM_TILES || tileId < 0 ) {
		return NULL;
	}

	return &s_files[ tileId ];
}

/*
================================
InitIslandPool
================================
*/
void InitIslandPool() {
	if ( s_islandMap ) {
		return;
	}

	int numIslandsPerTile = GetTerrainletsPerTile();
	int size = sizeof( int ) * numIslandsPerTile * NUM_TILES;
	s_islandMap = (int *)malloc( size );
	memset( s_islandMap, 0, size );
	for ( int i = 0; i < numIslandsPerTile * NUM_TILES; i++ ) {
		s_islandMap[ i ] = -1;
	}

	// Initialize the pool entries
	for ( int i = 0; i < ISLAND_POOL_SIZE; i++ ) {
		poolEntry_t & entry = s_islandPool[ i ];
		CreateTerrainlet( &entry.terra );
		entry.mruCounter = -1;
		entry.mapId = -1;
		entry.wasUsedLastFrame = false;
	}

	// Load tile files
	for ( int y = 0; y < TILES_WIDE; y++ ) {
		for ( int x = 0; x < TILES_WIDE; x++ ) {
			int tileId = x + y * TILES_WIDE;
			if ( !s_files[ tileId ].OpenFile( x, y ) ) {
				WriteTerrainFile( x, y );
				s_files[ tileId ].OpenFile( x, y );
			}
		}
	}
}

/*
================================
CleanupIslandPool
================================
*/
void CleanupIslandPool() {
	free( s_islandMap );
	s_islandMap = NULL;
}

/*
================================
GetIslandID
================================
*/
int GetIslandID( int depth, int x, int y ) {
	int numTerrainlets = 0;
	int numTilesWide = 1 << depth;
	for ( int i = 0; i < depth; i++ ) {
		numTerrainlets += ( 1 << ( i * 2 ) );
	}

	int islandId = numTerrainlets + x + y * numTilesWide;
	return islandId;
}

static int s_numStreamed = 0;


void TouchIsland( int tileX, int tileY, int depth, int x, int y ) {
	int tileId = tileX + tileY * TILES_WIDE;
	int islandId = GetIslandID( depth, x, y );

	int numIslandsPerTile = GetTerrainletsPerTile();
	int mapId = numIslandsPerTile * tileId + islandId;

	// Check if this island has already been streamed
	int idx = s_islandMap[ mapId ];
	if ( idx >= 0 ) {
		s_islandPool[ idx ].wasUsedLastFrame = true;
	}
}

#define STREAM_ASYNC

/*
================================
RequestIsland
================================
*/
int RequestIsland( int tileX, int tileY, int depth, int x, int y ) {
	int tileId = tileX + tileY * TILES_WIDE;
	int islandId = GetIslandID( depth, x, y );

	int numIslandsPerTile = GetTerrainletsPerTile();
	int mapId = numIslandsPerTile * tileId + islandId;

	// Check if this island has already been streamed
	int idx = s_islandMap[ mapId ];
	if ( idx >= 0 ) {
		s_islandPool[ idx ].wasUsedLastFrame = true;
	}

	if ( idx < 0 ) {
		// Find an empty pool entry and load from file the island
		for ( int i = 0; i < ISLAND_POOL_SIZE; i++ ) {
			poolEntry_t & entry = s_islandPool[ i ];
			if ( entry.mruCounter < 0 ) {
				// First, erase its old location in the map
				if ( entry.mapId >= 0 ) {
					s_islandMap[ entry.mapId ] = -1;
				}

				// Set the entry to the new location in the map
				entry.mruCounter = 1;
				entry.mapId = mapId;
				entry.wasUsedLastFrame = true;
				entry.isLoaded = false;
				idx = i;

				s_islandMap[ mapId ] = idx;
				break;
			}
		}

		if ( idx >= 0 ) {
#if defined( STREAM_ASYNC )
			streamerCmd_t cmd;
			cmd.tileX = tileX;
			cmd.tileY = tileY;
			cmd.depth = depth;
			cmd.x = x;
			cmd.y = y;
			cmd.poolId = idx;
			cmd.mapId = mapId;
			PushStreamerCmd( cmd );
#else
			poolEntry_t & entry = s_islandPool[ idx ];
 			s_files[ tileId ].ReadTerrainlet( x, y, depth, entry.terra.verts );
			entry.terra.vbo.Update( entry.terra.verts );
			entry.isLoaded = true;
#endif
			s_numStreamed++;
		}
	}

	return idx;
}

/*
================================
GetIsland
================================
*/
terrainlet_t * GetIsland( int id ) {
	if ( id < 0 ) {
		return NULL;
	}
	poolEntry_t & entry = s_islandPool[ id ];
	if ( !entry.isLoaded ) {
		return NULL;
	}

	return &entry.terra;
}

static int s_framesWithoutStreaming = 0;

/*
================================
UpdateIslandPool
================================
*/
void UpdateIslandPool() {
	int numIslandsPerTile = GetTerrainletsPerTile();
	int numUsed = 0;

	// Loop through the pool and determine what was and wasn't used,
	for ( int i = 0; i < ISLAND_POOL_SIZE; i++ ) {
		poolEntry_t & entry = s_islandPool[ i ];
		if ( entry.wasUsedLastFrame ) {
			numUsed++;
			entry.mruCounter++;
		} else {
			entry.mruCounter = -1;
		}
		entry.wasUsedLastFrame = false;
	}

	if ( s_numStreamed > 0 ) {
		printf( "Num Islands Used | Streamed | Frames without streaming:  %i  %i  %i\n", numUsed, s_numStreamed, s_framesWithoutStreaming );
		s_framesWithoutStreaming = 0;
		s_numStreamed = 0;
	} else {
		s_framesWithoutStreaming++;
	}
}



//
//  TerrainPool.h
//
#pragma once
#include "../Vector.h"
#include "TerrainTile.h"


struct terrainVert_t;
struct terrainlet_t;
class TerrainTileFile;
struct poolEntry_t;

struct poolEntry_t {
	terrainlet_t terra;
	int mruCounter;
	bool wasUsedLastFrame;
	int mapId;

	bool isLoaded;
};

void InitIslandPool();
void CleanupIslandPool();

poolEntry_t * GetPoolEntry( int id );

void TouchIsland( int tileX, int tileY, int depth, int x, int y );

int RequestIsland( int tileX, int tileY, int depth, int x, int y );
terrainlet_t * GetIsland( int id );

void UpdateIslandPool();

TerrainTileFile * GetTerrainTileFile( int tileX, int tileY );


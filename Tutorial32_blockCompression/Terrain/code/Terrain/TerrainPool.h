//
//  TerrainPool.h
//
#pragma once
#include "Math/Vector.h"
#include "Terrain/TerrainTile.h"


struct terrainVert_t;
struct terrainIsland_t;
class TerrainTileFile;
struct poolEntry_t;

struct poolEntry_t {
	terrainIsland_t terra;
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
terrainIsland_t * GetIsland( int id );

void UpdateIslandPool();

TerrainTileFile * GetTerrainTileFile( int tileX, int tileY );


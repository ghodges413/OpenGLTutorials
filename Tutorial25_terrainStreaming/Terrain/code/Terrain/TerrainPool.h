//
//  TerrainPool.h
//
#pragma once
#include "../Vector.h"


struct terrainVert_t;
struct terrainlet_t;
class TerrainTileFile;


void InitIslandPool();
void CleanupIslandPool();

int RequestIsland( int tileX, int tileY, int depth, int x, int y );
terrainlet_t * GetIsland( int id );

void UpdateIslandPool();

TerrainTileFile * GetTerrainTileFile( int tileX, int tileY );


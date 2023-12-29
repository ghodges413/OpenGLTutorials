//
//  Terrain.h
//
#pragma once
#include "TerrainTile.h"
#include "TerrainPool.h"

#define TILES_WIDE 5
#define NUM_TILES ( TILES_WIDE * TILES_WIDE )
#define MAX_RENDER_NODES 256	// On a 5x5 tile grid, the max nodes used were 70, so this should be more than enough of a buffer

/*
================================
Terrain
================================
*/
class Terrain {
public:
	Terrain() {}
	~Terrain() { CleanupIslandPool(); }

	void Terraform();
	void Update( Vec3d pos );
	void Draw();
	void DrawDebug( class Shader * shader );
	Vec3d GetSurfacePos( Vec3d pos ) const;

private:
	void StitchTerrainSeams();
	int SampleLod( Vec3d pt );

private:
	TerrainTile m_tiles[ NUM_TILES ];	// Each tile is 2km x 2km, so 5 x 5 tiles will make 10km x 10km

	terrainNode_t * m_renderNodes[ MAX_RENDER_NODES ];
	int m_numRenderNodes;	// the number of render nodes this frame
};

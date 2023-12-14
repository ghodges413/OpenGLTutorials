//
//  Terrain.h
//
#pragma once
#include "TerrainTile.h"

#define TILES_WIDE 2
#define NUM_TILES ( TILES_WIDE * TILES_WIDE )
#define MAX_RENDER_NODES 256	// On a 3x3 tile grid, the max nodes used were 57, so this should be more than enough of a buffer

/*
================================
Terrain
================================
*/
class Terrain {
public:
	Terrain() {}
	~Terrain() {}

	void Terraform();
	void Draw( Vec3d pos );
	void DrawDebug( Vec3d pos, class Shader * shader );
	Vec3d GetSurfacePos( Vec3d pos ) const;

private:
	void StitchTerrainSeams();
	int SampleLod( Vec3d pt );

private:
	TerrainTile m_tiles[ NUM_TILES ];	// Each tile is 2km x 2km, so 5 x 5 tiles will make 10km x 10km

	terrainNode_t * m_renderNodes[ MAX_RENDER_NODES ];
	int m_numRenderNodes;	// the number of render nodes this frame
};

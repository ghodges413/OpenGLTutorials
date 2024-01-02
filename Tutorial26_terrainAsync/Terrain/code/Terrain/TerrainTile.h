//
//  TerrainTile.h
//
#pragma once
#include "TerrainVert.h"
#include "../Bounds.h"
#include "../VertexArrayObject.h"
#include "../VertexBufferObject.h"

#define VERTS_PER_METER 2.0f
#define METERS_PER_VERT ( 1.0f / VERTS_PER_METER )

#define TERRAINLET_SIZE ( 128 + 1 )		// 64m with 2 verts/meter = 64 * 2 = 128
#define TILE_SIZE ( 4096 + 1 )			// 2km with 2 verts/meter = 2048 * 2 = 4096

// The whole terrain is 10km x 10km
// A sector is only 64m x 64m (highest detail is 2 verts/meter... meaning 128 verts x 128 verts = 16,384 total verts per sector)
// The whole terrain is divided into 2km x 2km tiles
// The tiles are quad trees, and the sectors are the leafs of the trees
// The root node of each quad tree is always resident

// Every terrainlet is 128 verts x 128 verts
struct terrainlet_t {	// bad name, think of something better (terrainlet_t?)
	VertexBufferObject vbo;
	VertexArrayObject vao;

	terrainVert_t verts[ TERRAINLET_SIZE * TERRAINLET_SIZE ];

	static VertexBufferObject ibo;	// this could be static and used across all of them
	static unsigned short indices[ TERRAINLET_SIZE * TERRAINLET_SIZE * 6 ];	// this could be static and used across all of them
};
void CreateTerrainlet( terrainlet_t * terra );

struct terrainNode_t {	// One of the nodes in the quad tree
	terrainNode_t( Bounds * boundsData, int tileX, int tileY, int x, int y, int depth );
	~terrainNode_t() {
		delete nodes[ 0 ]; nodes[ 0 ] = NULL;
		delete nodes[ 1 ]; nodes[ 1 ] = NULL;
		delete nodes[ 2 ]; nodes[ 2 ] = NULL;
		delete nodes[ 3 ]; nodes[ 3 ] = NULL;
	}

	int terraID;
	bool forceUpdate;

	terrainNode_t * nodes[ 4 ];
	Bounds bounds;

	int tileX;
	int tileY;
	int depth;	// we need to know our depth for stitching with neighbors
	int x;
	int y;
	int neighborLODs[ 4 ];		// px, nx, py, ny
	int neighborLODsPrev[ 4 ];	// previous frames neighbor lods (so that we only have to update vbo's when something changes)

	bool doRender;
};

/*
================================
TerrainTile
// This is the 2km x 2km quad tree
================================
*/
class TerrainTile {
public:
	TerrainTile() { m_root = NULL; m_tileX = 0; m_tileY = 0; }
	~TerrainTile() { delete m_root; m_root = NULL; }

	void Build( int tileX, int tileY );
	Vec3d GetSurfacePos( Vec3d pos ) const;

public:
	// 4096 = 2^12
	// 128 = 2^7
	// therefore numLods in a tile is 12 - 7 = 5
	//static const int numlods = 5;

	terrainNode_t * m_root;
	float m_heightmap[ TILE_SIZE * TILE_SIZE ];
	Bounds m_bounds;

	int m_tileX;
	int m_tileY;
};
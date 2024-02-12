//
//  Terrain.cpp
//
#include "Terrain/Terrain.h"
#include "Terrain/TerrainFile.h"
#include "Terrain/TerrainPool.h"
#include "Terrain/TerrainStreamer.h"
#include "Terrain/Heightmap.h"
#include "Graphics/Graphics.h"
#include "Graphics/Shader.h"
#include <math.h>
#include <algorithm>
#include <stdlib.h>
#include <assert.h>

extern int GetMaxDepth();
extern void DrawTerraID( int id );

/*
================================
Terrain::Terraform
Reads the height map file and builds the tiles
================================
*/
void Terrain::Terraform() {
	InitStreamer();

	InitIslandPool();

	for ( int y = 0; y < TILES_WIDE; y++ ) {
		for ( int x = 0; x < TILES_WIDE; x++ ) {
			int idx = x + y * TILES_WIDE;
			m_tiles[ idx ].Build( x, y );
		}
	}

	m_bounds.Clear();
	for ( int i = 0; i < NUM_TILES; i++ ) {
		m_bounds.AddPoint( m_tiles[ i ].m_bounds.max );
		m_bounds.AddPoint( m_tiles[ i ].m_bounds.min );
	}
}

/*
================================
DetermineDrawList_r
================================
*/
void DetermineDrawList_r( terrainNode_t * node, int tileX, int tileY, int depth, int maxDepth, Vec3d pos, Frustum view, int boundsWidth, terrainNode_t ** list, int & numNodes, bool & allLoaded ) {
	if ( NULL == node ) {
		return;
	}

	// view frustum culling
	if ( !view.IntersectBox( node->bounds ) ) {
		return;
	}

	node->doRender = false;
	node->depth = depth;

	if ( depth == maxDepth ) {
		node->doRender = true;
		node->terraID = RequestIsland( tileX, tileY, node->depth, node->x, node->y );
		if ( NULL == GetIsland( node->terraID ) ) {
			node->forceUpdate = true;
			allLoaded = false;
		}
		list[ numNodes ] = node;
		numNodes++;
		return;
	}

	assert( node->bounds.IsInitialized() );

	const Bounds bounds( pos - Vec3d( boundsWidth ), pos + Vec3d( boundsWidth ) );
	if ( node->bounds.IntersectBounds2D( bounds ) ) {
		DetermineDrawList_r( node->nodes[ 0 ], tileX, tileY, depth + 1, maxDepth, pos, view, boundsWidth >> 1, list, numNodes, allLoaded );
		DetermineDrawList_r( node->nodes[ 1 ], tileX, tileY, depth + 1, maxDepth, pos, view, boundsWidth >> 1, list, numNodes, allLoaded );
		DetermineDrawList_r( node->nodes[ 2 ], tileX, tileY, depth + 1, maxDepth, pos, view, boundsWidth >> 1, list, numNodes, allLoaded );
		DetermineDrawList_r( node->nodes[ 3 ], tileX, tileY, depth + 1, maxDepth, pos, view, boundsWidth >> 1, list, numNodes, allLoaded );
		return;
	}

	// Anything within 64m should be lod 0
	// 64m should be lod 1
	// 128m should be lod 2
	// 256m should be lod 3
	// 512m should be lod 4
	if ( numNodes >= MAX_RENDER_NODES ) {
		assert( numNodes < MAX_RENDER_NODES );
		return; // this should never happen
	}

	// If it didn't intersect with the tighter bounds, then render
	node->doRender = true;
	node->terraID = RequestIsland( tileX, tileY, node->depth, node->x, node->y );
	if ( NULL == GetIsland( node->terraID ) ) {
		node->forceUpdate = true;
		allLoaded = false;
	}
	list[ numNodes ] = node;
	numNodes++;
}

/*
================================
UpdateVBO
================================
*/
static vert_t g_vertsTmp[ TERRAIN_ISLAND_SIZE * TERRAIN_ISLAND_SIZE ];
void UpdateVBO( terrainNode_t * node ) {
	const int depth = node->depth;

	// There's no need to update the vbo if these lods are the same as last frame's
	if ( !node->forceUpdate
		&& node->neighborLODsPrev[ 0 ] == node->neighborLODs[ 0 ]
		&& node->neighborLODsPrev[ 1 ] == node->neighborLODs[ 1 ]
		&& node->neighborLODsPrev[ 2 ] == node->neighborLODs[ 2 ]
		&& node->neighborLODsPrev[ 3 ] == node->neighborLODs[ 3 ] ) {
		return;
	}

	node->neighborLODsPrev[ 0 ] = node->neighborLODs[ 0 ];
	node->neighborLODsPrev[ 1 ] = node->neighborLODs[ 1 ];
	node->neighborLODsPrev[ 2 ] = node->neighborLODs[ 2 ];
	node->neighborLODsPrev[ 3 ] = node->neighborLODs[ 3 ];

	// If this node is courser than its neighbors, there's no need to update
// 	if ( depth < node->neighborLODs[ 0 ]
// 		&& depth < node->neighborLODs[ 1 ]
// 		&& depth < node->neighborLODs[ 2 ]
// 		&& depth < node->neighborLODs[ 3 ] ) {
// 		return;
// 	}

	// Use the lods/depths of the neighbors to collapse edges appropriately
	terrainIsland_t * terra = GetIsland( node->terraID );
	if ( NULL == terra ) {
		return;
	}

	memcpy( g_vertsTmp, terra->verts, sizeof( vert_t ) * TERRAIN_ISLAND_SIZE * TERRAIN_ISLAND_SIZE );

	// Collapse px side (if our depth is deeper than the neighbors)
	const int dpx = node->neighborLODs[ 0 ];
	if ( depth > dpx && dpx >= 0 ) {
		int delta = depth - dpx;
		int skipVerts = 1 << delta;
		for ( int y = 0; y < TERRAIN_ISLAND_SIZE - skipVerts; y += skipVerts ) {
			int x = TERRAIN_ISLAND_SIZE - 1;
			int idx = x + y * TERRAIN_ISLAND_SIZE;

			for ( int y2 = 1; y2 < skipVerts; y2++ ) {
				int idx2 = x + ( y + y2 ) * TERRAIN_ISLAND_SIZE;
				g_vertsTmp[ idx2 ] = g_vertsTmp[ idx ];
			}
		}
	}

	// Collapse nx side (if our depth is deeper than the neighbors)
	const int dnx = node->neighborLODs[ 1 ];
	if ( depth > dnx && dnx >= 0 ) {
		int delta = depth - dnx;
		int skipVerts = 1 << delta;
		for ( int y = 0; y < TERRAIN_ISLAND_SIZE - skipVerts; y += skipVerts ) {
			int x = 0;
			int idx = x + y * TERRAIN_ISLAND_SIZE;

			for ( int y2 = 1; y2 < skipVerts; y2++ ) {
				int idx2 = x + ( y + y2 ) * TERRAIN_ISLAND_SIZE;
				g_vertsTmp[ idx2 ] = g_vertsTmp[ idx ];
			}
		}
	}

	// Collapse py side (if our depth is deeper than the neighbors)
	const int dpy = node->neighborLODs[ 2 ];
	if ( depth > dpy && dpy >= 0 ) {
		int delta = depth - dpy;
		int skipVerts = 1 << delta;
		for ( int x = 0; x < TERRAIN_ISLAND_SIZE - skipVerts; x += skipVerts ) {
			int y = TERRAIN_ISLAND_SIZE - 1;
			int idx = x + y * TERRAIN_ISLAND_SIZE;

			for ( int x2 = 1; x2 < skipVerts; x2++ ) {
				int idx2 = ( x + x2 ) + y * TERRAIN_ISLAND_SIZE;
				g_vertsTmp[ idx2 ] = g_vertsTmp[ idx ];
			}
		}
	}

	// Collapse ny side (if our depth is deeper than the neighbors)
	const int dny = node->neighborLODs[ 3 ];
	if ( depth > dny && dny >= 0 ) {
		int delta = depth - dny;
		int skipVerts = 1 << delta;
		for ( int x = 0; x < TERRAIN_ISLAND_SIZE - skipVerts; x += skipVerts ) {
			int y = 0;
			int idx = x + y * TERRAIN_ISLAND_SIZE;

			for ( int x2 = 1; x2 < skipVerts; x2++ ) {
				int idx2 = ( x + x2 ) + y * TERRAIN_ISLAND_SIZE;
				g_vertsTmp[ idx2 ] = g_vertsTmp[ idx ];
			}
		}
	}

	terra->vbo.Update( g_vertsTmp );
	node->forceUpdate = false;
}

/*
================================
Terrain::SampleLod
================================
*/
int Terrain::SampleLod( Vec3d pt ) {
	for ( int i = 0; i < m_numRenderNodes; i++ ) {
		const terrainNode_t * node = m_renderNodes[ i ];
		assert( node->bounds.IsInitialized() );

		if ( node->bounds.HasPoint2D( pt ) ) {
			return node->depth;
		}
	}

	return -1;
}

/*
================================
Terrain::StitchTerrainSeams
================================
*/
void Terrain::StitchTerrainSeams() {
	int maxDepth = GetMaxDepth();

	// Update neighbor lods for the render nodes
	for ( int i = 0; i < m_numRenderNodes; i++ ) {
		terrainNode_t * node = m_renderNodes[ i ];
		assert( node->bounds.IsInitialized() );

		float delta = 2.0f;
		Vec3d center = ( node->bounds.min + node->bounds.max ) * 0.5f;
		Vec3d px = Vec3d( node->bounds.max.x + delta, center.y, center.z );
		Vec3d nx = Vec3d( node->bounds.min.x - delta, center.y, center.z );
		Vec3d py = Vec3d( center.x, node->bounds.max.y + delta, center.z );
		Vec3d ny = Vec3d( center.x, node->bounds.min.y - delta, center.z );

		node->neighborLODs[ 0 ] = SampleLod( px );
		node->neighborLODs[ 1 ] = SampleLod( nx );
		node->neighborLODs[ 2 ] = SampleLod( ny );
		node->neighborLODs[ 3 ] = SampleLod( py );

		UpdateVBO( node );
	}
}

/*
================================
Terrain::Update
================================
*/
void Terrain::Update( Vec3d pos ) {
	UpdateIslandPool();

	int maxDepth = GetMaxDepth();
	int width = 256;

	m_numRenderNodes = 0;

	// Determine the nodes that are to be rendered
	for ( int y = 0; y < TILES_WIDE; y++ ) {
		for ( int x = 0; x < TILES_WIDE; x++ ) {
			int tileId = x + y * TILES_WIDE;
			TerrainTile * tile = &m_tiles[ tileId ];
//			DetermineDrawList_r( tile->m_root, x, y, 0, maxDepth, pos, width, m_renderNodes, m_numRenderNodes );
		}
	}

	// Stitch the terrainIsland seams by determining the lods of the neighbor terrainIslands
	StitchTerrainSeams();
}

terrainNode_t * s_renderNodesTmp[ MAX_RENDER_NODES ];

/*
================================
Terrain::Update
================================
*/
void Terrain::Update( Vec3d pos, Frustum view ) {
	UpdateIslandPool();

	int maxDepth = GetMaxDepth();
	//int width = 256;
	//int width = 512;
	int width = 1024;

	bool allLoaded = true;
	int numNodes = 0;
	//m_numRenderNodes = 0;

	// Determine the nodes that are to be rendered
	for ( int y = 0; y < TILES_WIDE; y++ ) {
		for ( int x = 0; x < TILES_WIDE; x++ ) {
			int tileId = x + y * TILES_WIDE;
			TerrainTile * tile = &m_tiles[ tileId ];
			DetermineDrawList_r( tile->m_root, x, y, 0, maxDepth, pos, view, width, s_renderNodesTmp, numNodes, allLoaded );
			//DetermineDrawList_r( tile->m_root, x, y, 0, maxDepth, pos, view, width, m_renderNodes, m_numRenderNodes, allLoaded );
		}
	}

	// If everything is loaded and available, then copy the new list over for rendering.
	// Otherwise render the previous render list.  This prevents popping when waiting for islands to 
	if ( allLoaded ) {
		m_numRenderNodes = numNodes;
		for ( int i = 0; i < m_numRenderNodes; i++ ) {
			m_renderNodes[ i ] = s_renderNodesTmp[ i ];
		}
	} else {
		// Touch the old ones to make sure they don't get unloaded
		for ( int i = 0; i < m_numRenderNodes; i++ ) {
			terrainNode_t * node = m_renderNodes[ i ];
			TouchIsland( node->tileX, node->tileY, node->depth, node->x, node->y );
		}
	}

	// Stitch the terrainIsland seams by determining the lods of the neighbor terrainIslands
	StitchTerrainSeams();
}

/*
================================
Terrain::Draw
================================
*/
void Terrain::Draw() {
	// Draw the nodes
	for ( int i = 0; i < m_numRenderNodes; i++ ) {
		terrainNode_t * node = m_renderNodes[ i ];
		DrawTerraID( node->terraID );
	}
}

/*
================================
Terrain::DrawDebug
================================
*/
void Terrain::DrawDebug( Shader * shader ) {
	int maxDepth = GetMaxDepth();

	// Draw the nodes
	for ( int i = 0; i < m_numRenderNodes; i++ ) {
		terrainNode_t * node = m_renderNodes[ i ];

		int lod = maxDepth - node->depth;
		shader->SetUniform1i( "lod", 1, &lod );

		DrawTerraID( node->terraID );
	}
}

/*
================================
Terrain::GetSurfacePos
Samples the height map and returns the height
================================
*/
Vec3d Terrain::GetSurfacePos( Vec3d pos ) const {
	for ( int i = 0; i < NUM_TILES; i++ ) {
		if ( m_tiles[ i ].m_bounds.HasPoint2D( pos ) ) {
			return m_tiles[ i ].GetSurfacePos( pos );
		}
	}
	return pos;
}

/*
================================
Terrain::SampleHeightMap
Samples the height map and returns the height
================================
*/
float Terrain::SampleHeightMap( Vec2d st ) const {
	// convert st from [0,1) to [-bounds, bounds)
	st.y = 1.0f - st.y;

	float width = m_bounds.max.x - m_bounds.min.x;
	float height = m_bounds.max.y - m_bounds.min.y;

	Vec3d pos;
	pos.x = st.x * width + m_bounds.min.x;
	pos.y = st.y * height + m_bounds.min.y;
	pos.z = 0;

	pos = GetSurfacePos( pos );
	return pos.z;
}

/*
================================
Terrain::SampleColorMap
================================
*/
Vec4d Terrain::SampleColorMap( Vec2d st ) const {
	// convert st from [0,1) to [-bounds, bounds)
	st.y = 1.0f - st.y;

	float width = m_bounds.max.x - m_bounds.min.x;
	float height = m_bounds.max.y - m_bounds.min.y;

	Vec3d pos;
	pos.x = st.x * width + m_bounds.min.x;
	pos.y = st.y * height + m_bounds.min.y;
	pos.z = 0;

	for ( int i = 0; i < NUM_TILES; i++ ) {
		if ( m_tiles[ i ].m_bounds.HasPoint2D( pos ) ) {
			return m_tiles[ i ].GetColorMap( pos );
		}
	}

	return Vec4d( 0.0f );
}

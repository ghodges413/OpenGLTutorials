//
//  Terrain.cpp
//
#include "Terrain.h"
#include "TerrainFile.h"
#include "Heightmap.h"
#include "../Graphics.h"
#include "../Shader.h"
#include <math.h>
#include <algorithm>
#include <stdlib.h>
#include <assert.h>

extern int GetMaxDepth();
extern void DrawTerrainlet( terrainlet_t & terra );

/*
================================
Terrain::Terraform
Reads the height map file and builds the tiles
================================
*/
void Terrain::Terraform() {
	for ( int y = 0; y < TILES_WIDE; y++ ) {
		for ( int x = 0; x < TILES_WIDE; x++ ) {
			int idx = x + y * TILES_WIDE;
			m_tiles[ idx ].Build( x, y );
		}
	}
}

/*
================================
DetermineDrawList_r
================================
*/
void DetermineDrawList_r( terrainNode_t * node, int depth, int maxDepth, Vec3d pos, int boundsWidth, terrainNode_t ** list, int & numNodes ) {
	if ( NULL == node ) {
		return;
	}
	node->doRender = false;
	node->depth = depth;

	if ( depth == maxDepth ) {
		node->doRender = true;
		list[ numNodes ] = node;
		numNodes++;
		return;
	}

	assert( node->bounds.IsInitialized() );

	const Bounds bounds( pos - Vec3d( boundsWidth ), pos + Vec3d( boundsWidth ) );
	if ( node->bounds.IntersectBounds2D( bounds ) ) {
		DetermineDrawList_r( node->nodes[ 0 ], depth + 1, maxDepth, pos, boundsWidth >> 1, list, numNodes );
		DetermineDrawList_r( node->nodes[ 1 ], depth + 1, maxDepth, pos, boundsWidth >> 1, list, numNodes );
		DetermineDrawList_r( node->nodes[ 2 ], depth + 1, maxDepth, pos, boundsWidth >> 1, list, numNodes );
		DetermineDrawList_r( node->nodes[ 3 ], depth + 1, maxDepth, pos, boundsWidth >> 1, list, numNodes );
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
	list[ numNodes ] = node;
	numNodes++;
}

/*
================================
UpdateVBO
================================
*/
static terrainVert_t g_vertsTmp[ TERRAINLET_SIZE * TERRAINLET_SIZE ];
void UpdateVBO( terrainNode_t * node ) {
	const int depth = node->depth;

	// There's no need to update the vbo if these lods are the same as last frame's
	if ( node->neighborLODsPrev[ 0 ] == node->neighborLODs[ 0 ]
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

	// TODO: Use the lods/depths of the neighbors to collapse edges appropriately
	memcpy( g_vertsTmp, node->terra.verts, sizeof( terrainVert_t ) * TERRAINLET_SIZE * TERRAINLET_SIZE );

	// Collapse px side (if our depth is deeper than the neighbors)
	const int dpx = node->neighborLODs[ 0 ];
	if ( depth > dpx && dpx >= 0 ) {
		int delta = depth - dpx;
		int skipVerts = 1 << delta;
		for ( int y = 0; y < TERRAINLET_SIZE - skipVerts; y += skipVerts ) {
			int x = TERRAINLET_SIZE - 1;
			int idx = x + y * TERRAINLET_SIZE;

			for ( int y2 = 1; y2 < skipVerts; y2++ ) {
				int idx2 = x + ( y + y2 ) * TERRAINLET_SIZE;
				g_vertsTmp[ idx2 ] = g_vertsTmp[ idx ];
			}
		}
	}

	// Collapse nx side (if our depth is deeper than the neighbors)
	const int dnx = node->neighborLODs[ 1 ];
	if ( depth > dnx && dnx >= 0 ) {
		int delta = depth - dnx;
		int skipVerts = 1 << delta;
		for ( int y = 0; y < TERRAINLET_SIZE - skipVerts; y += skipVerts ) {
			int x = 0;
			int idx = x + y * TERRAINLET_SIZE;

			for ( int y2 = 1; y2 < skipVerts; y2++ ) {
				int idx2 = x + ( y + y2 ) * TERRAINLET_SIZE;
				g_vertsTmp[ idx2 ] = g_vertsTmp[ idx ];
			}
		}
	}

	// Collapse py side (if our depth is deeper than the neighbors)
	const int dpy = node->neighborLODs[ 2 ];
	if ( depth > dpy && dpy >= 0 ) {
		int delta = depth - dpy;
		int skipVerts = 1 << delta;
		for ( int x = 0; x < TERRAINLET_SIZE - skipVerts; x += skipVerts ) {
			int y = TERRAINLET_SIZE - 1;
			int idx = x + y * TERRAINLET_SIZE;

			for ( int x2 = 1; x2 < skipVerts; x2++ ) {
				int idx2 = ( x + x2 ) + y * TERRAINLET_SIZE;
				g_vertsTmp[ idx2 ] = g_vertsTmp[ idx ];
			}
		}
	}

	// Collapse ny side (if our depth is deeper than the neighbors)
	const int dny = node->neighborLODs[ 3 ];
	if ( depth > dny && dny >= 0 ) {
		int delta = depth - dny;
		int skipVerts = 1 << delta;
		for ( int x = 0; x < TERRAINLET_SIZE - skipVerts; x += skipVerts ) {
			int y = 0;
			int idx = x + y * TERRAINLET_SIZE;

			for ( int x2 = 1; x2 < skipVerts; x2++ ) {
				int idx2 = ( x + x2 ) + y * TERRAINLET_SIZE;
				g_vertsTmp[ idx2 ] = g_vertsTmp[ idx ];
			}
		}
	}

	node->terra.vbo.Update( g_vertsTmp );
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
Terrain::Draw
================================
*/
void Terrain::Draw( Vec3d pos ) {
	int maxDepth = GetMaxDepth();
	int width = 256;

	m_numRenderNodes = 0;

	// Determine the nodes that are to be rendered
	for ( int i = 0; i < NUM_TILES; i++ ) {
		TerrainTile * tile = &m_tiles[ i ];
		DetermineDrawList_r( tile->m_root, 0, maxDepth, pos, width, m_renderNodes, m_numRenderNodes );
	}

	// Stitch the terrainlet seams by determining the lods of the neighbor terrainlets
	StitchTerrainSeams();

	// Draw the nodes
	for ( int i = 0; i < m_numRenderNodes; i++ ) {
		terrainNode_t * node = m_renderNodes[ i ];
		DrawTerrainlet( node->terra );
	}
}

/*
================================
Terrain::DrawDebug
================================
*/
void Terrain::DrawDebug( Vec3d pos, Shader * shader ) {
	int maxDepth = GetMaxDepth();

	// Draw the nodes
	for ( int i = 0; i < m_numRenderNodes; i++ ) {
		terrainNode_t * node = m_renderNodes[ i ];

		int lod = maxDepth - node->depth;
		shader->SetUniform1i( "lod", 1, &lod );

		DrawTerrainlet( node->terra );
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

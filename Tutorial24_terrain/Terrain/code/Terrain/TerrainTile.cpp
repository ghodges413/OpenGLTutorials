//
//  Terrain.cpp
//
#include "Terrain.h"
#include "TerrainFile.h"
#include "Heightmap.h"
#include "../Graphics.h"
#include "../Shader.h"
#include "../Fileio.h"

#include <math.h>
#include <algorithm>
#include <stdlib.h>
#include <assert.h>

extern float frac( float value );

/*
================================
GetMaxDepth
================================
*/
int GetMaxDepth() {
	int tileSize = TILE_SIZE - 1;
	int terrainlet = TERRAINLET_SIZE - 1;
	int maxDepth = 0;
	while ( terrainlet != tileSize ) {
		tileSize >>= 1;
		maxDepth++;
	}

	return maxDepth;
}

/*
================================
DrawTerrainlet
================================
*/
void DrawTerrainlet( terrainlet_t & terra ) {
	// Bind the VAO
    terra.vao.Bind();
    
    // Draw
	const int num = TERRAINLET_SIZE * TERRAINLET_SIZE * 6;
	glDrawElements( GL_TRIANGLES, num, GL_UNSIGNED_SHORT, 0 );

	// Unbind the VAO
	terra.vao.UnBind();
}

/*
================================
CreateTerrainlet
================================
*/
void CreateTerrainlet( terrainlet_t & terra, terrainVert_t * data ) {
	for ( int i = 0; i < TERRAINLET_SIZE * TERRAINLET_SIZE; i++ ) {
		terra.verts[ i ] = data[ i ];
	}

	int idx = 0;
	for ( int y = 0; y < TERRAINLET_SIZE - 1; y++ ) {
		for ( int x = 0; x < TERRAINLET_SIZE - 1; x++ ) {
			int idxA = ( x + 0 ) + ( y + 0 ) * TERRAINLET_SIZE;
			int idxB = ( x + 0 ) + ( y + 1 ) * TERRAINLET_SIZE;
			int idxC = ( x + 1 ) + ( y + 1 ) * TERRAINLET_SIZE;
			int idxD = ( x + 1 ) + ( y + 0 ) * TERRAINLET_SIZE;

			terra.indices[ idx + 0 ] = (unsigned short)idxA;
			terra.indices[ idx + 1 ] = (unsigned short)idxD;
			terra.indices[ idx + 2 ] = (unsigned short)idxB;

			terra.indices[ idx + 3 ] = (unsigned short)idxC;
			terra.indices[ idx + 4 ] = (unsigned short)idxB;
			terra.indices[ idx + 5 ] = (unsigned short)idxD;

			idx += 6;
		}
	}

	// Create the VBO
	const int stride = sizeof( terrainVert_t );

	terra.vbo.Generate( GL_ARRAY_BUFFER, TERRAINLET_SIZE * TERRAINLET_SIZE * stride, terra.verts, GL_DYNAMIC_DRAW );
	terra.ibo.Generate( GL_ELEMENT_ARRAY_BUFFER, TERRAINLET_SIZE * TERRAINLET_SIZE * 6 * sizeof( unsigned short ), terra.indices, GL_STATIC_DRAW );
    
	//
	//  Generate and Fill the VAO
	//
	terra.vao.Generate();
	terra.vao.Bind();
    
	terra.vbo.Bind();
	terra.ibo.Bind();

	unsigned long offset = 0;
	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset );
	offset += sizeof( Vec3d );
    
	glEnableVertexAttribArray( 1 );
	glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset );
	offset += sizeof( Vec3d );
    
	glEnableVertexAttribArray( 2 );
	glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset );
	offset += sizeof( Vec3d );
    
	glEnableVertexAttribArray( 3 );
	glVertexAttribPointer( 3, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset );
	offset += sizeof( Vec3d );

	terra.vao.UnBind();
	terra.vbo.UnBind();
	terra.ibo.UnBind();
}

/*
================================
terrainNode_t::terrainNode_t
================================
*/
terrainNode_t::terrainNode_t( terrainVert_t * data, int widthData ) {
	nodes[ 0 ] = NULL;
	nodes[ 1 ] = NULL;
	nodes[ 2 ] = NULL;
	nodes[ 3 ] = NULL;
	for ( int i = 0; i < 4; i++ ) {
		neighborLODs[ i ] = -1;
		neighborLODsPrev[ i ] = -1;
	}
	doRender = false;

	if ( TERRAINLET_SIZE == widthData ) {
		CreateTerrainlet( terra, data );
		nodes[ 0 ] = NULL;
		nodes[ 1 ] = NULL;
		nodes[ 2 ] = NULL;
		nodes[ 3 ] = NULL;
		return;
	}

	// Cut up the high detail into quadrants and pass them down to the children
	const int widthTmp = ( widthData >> 1 ) + 1;
	terrainVert_t * tmp = (terrainVert_t *)malloc( sizeof( terrainVert_t ) * widthTmp * widthTmp );
	assert( tmp );
	if ( NULL == tmp ) {
		return;
	}
#if 1
	for ( int y = 0; y < widthTmp; y++ ) {
		for ( int x = 0; x < widthTmp; x++ ) {
			int idx = x + y * widthData;
			int idx2 = x + y * widthTmp;
			assert( idx < widthData * widthData );
			tmp[ idx2 ] = data[ idx ];
		}
	}
	nodes[ 0 ] = new terrainNode_t( tmp, widthTmp );
	
	for ( int y = 0; y < widthTmp; y++ ) {
		for ( int x = 0; x < widthTmp; x++ ) {
			int idx = ( x + widthTmp - 1 ) + y * widthData;
			int idx2 = x + y * widthTmp;
			assert( idx < widthData * widthData );
			tmp[ idx2 ] = data[ idx ];
		}
	}
	nodes[ 1 ] = new terrainNode_t( tmp, widthTmp );

	for ( int y = 0; y < widthTmp; y++ ) {
		for ( int x = 0; x < widthTmp; x++ ) {
			int idx = x + ( y + widthTmp - 1 ) * widthData;
			int idx2 = x + y * widthTmp;
			assert( idx < widthData * widthData );
			tmp[ idx2 ] = data[ idx ];
		}
	}
	nodes[ 2 ] = new terrainNode_t( tmp, widthTmp );

	for ( int y = 0; y < widthTmp; y++ ) {
		for ( int x = 0; x < widthTmp; x++ ) {
			int idx = ( x + widthTmp - 1 ) + ( y + widthTmp - 1 ) * widthData;
			int idx2 = x + y * widthTmp;
			assert( idx < widthData * widthData );
			tmp[ idx2 ] = data[ idx ];
		}
	}
	nodes[ 3 ] = new terrainNode_t( tmp, widthTmp );
#endif
	//
	// Now mip down the data and create an appropriate lod for this
	//
	bounds.Clear();
	terrainVert_t * finality = (terrainVert_t *)malloc( sizeof( terrainVert_t ) * TERRAINLET_SIZE * TERRAINLET_SIZE );
	assert( finality );
	if ( NULL == finality ) {
		return;
	}

	for ( int y = 0; y < TERRAINLET_SIZE; y++ ) {
		for ( int x = 0; x < TERRAINLET_SIZE; x++ ) {
			int idx = x + y * TERRAINLET_SIZE;

			int ratio = ( widthData - 1 ) / ( TERRAINLET_SIZE - 1 );
			int x2 = x * ratio;
			int y2 = y * ratio;
			if ( x2 >= widthData ) {
				x2 = widthData - 1;
			}
			if ( y2 >= widthData ) {
				y2 = widthData - 1;
			}
			int idx2 = x2 + y2 * widthData;
			assert( idx2 < widthData * widthData );
			finality[ idx ] = data[ idx2 ];

			bounds.AddPoint( finality[ idx ].pos );
		}
	}
	if ( !bounds.IsInitialized() ) {
		printf( "bounds invalid!\n" );
	}
	assert( bounds.IsInitialized() );

	CreateTerrainlet( terra, finality );

	free( tmp );
	free( finality );
}

/*
================================
AssertNodeBounds
================================
*/
void AssertNodeBounds( terrainNode_t * node ) {
	if ( NULL == node ) {
		return;
	}

	if ( !node->bounds.IsInitialized() ) {
		printf( "bounds invalid!\n" );
		for ( int i = 0; i < TERRAINLET_SIZE * TERRAINLET_SIZE; i++ ) {
			Vec3d xyz = node->terra.verts[ i ].pos;
			node->bounds.AddPoint( xyz );
		}
	}
	assert( node->bounds.IsInitialized() );

	AssertNodeBounds( node->nodes[ 0 ] );
	AssertNodeBounds( node->nodes[ 1 ] );
	AssertNodeBounds( node->nodes[ 2 ] );
	AssertNodeBounds( node->nodes[ 3 ] );
}

/*
================================
TerrainTile::Build
================================
*/
void TerrainTile::Build( terrainVert_t * data ) {
	m_root = new terrainNode_t( data, TILE_SIZE );

	AssertNodeBounds( m_root );
}

/*
================================
TerrainTile::Build
================================
*/
void TerrainTile::Build( int tileX, int tileY ) {
	m_tileX = tileX;
	m_tileY = tileY;

	Vec4d * data = ReadHeightmapFile( TILE_SIZE, tileX, tileY );
	if ( NULL == data ) {
		CreateTerrainHeightmap( TILE_SIZE, tileX, tileY );
		data = ReadHeightmapFile( TILE_SIZE, tileX, tileY );
		assert( data );
		if ( NULL == data ) {
			return;
		}
	}

	// Convert the heightmap data to terrainverts
	unsigned long size = sizeof( terrainVert_t ) * TILE_SIZE * TILE_SIZE;
	terrainVert_t * verts = (terrainVert_t *)malloc( size );
	if ( verts == NULL ) {
		printf( "wtf!?\n" );
	}

	m_bounds.Clear();
	for ( int y = 0; y < TILE_SIZE; y++ ) {
		for ( int x = 0; x < TILE_SIZE; x++ ) {
			int idx = x + y * TILE_SIZE;
			verts[ idx ] = HeightmapToVert( data[ idx ], x, y, tileX, tileY );

			m_bounds.AddPoint( verts[ idx ].pos );

			m_heightmap[ idx ] = verts[ idx ].pos.z;
		}
	}

	Build( verts );
	free( verts );
	free( data );
	printf( "Done building terrain\n" );
}

/*
================================
TerrainTile::GetSurfacePos
================================
*/
Vec3d TerrainTile::GetSurfacePos( Vec3d pos ) const {
	if ( !m_bounds.HasPoint2D( pos ) ) {
		return pos;
	}

	// Get the position relative to the lower left
	Vec3d pos2 = pos - m_bounds.min;

	// Scale from world space dimensions to [0,1]
	Vec3d dim = m_bounds.max - m_bounds.min;
	pos2.x /= dim.x;
	pos2.y /= dim.y;
	pos2.y = 1.0f - pos2.y;

	// Scale from [0,1] to [0,TILE_SIZE-1]
	pos2.x *= TILE_SIZE - 1;
	pos2.y *= TILE_SIZE - 1;

	int x = (int)pos2.x;
	int y = (int)pos2.y;

	if ( x < 0 ) {
		x = 0;
	}
	if ( y < 0 ) {
		y = 0;
	}
	if ( x >= TILE_SIZE - 1 ) {
		x = TILE_SIZE - 2;
	}
	if ( y >= TILE_SIZE - 1 ) {
		y = TILE_SIZE - 2;
	}

	int idx00 = x + y * TILE_SIZE;
	int idx01 = x + ( y + 1 ) * TILE_SIZE;
	int idx10 = ( x + 1 ) + y * TILE_SIZE;
	int idx11 = ( x + 1 ) + ( y + 1 ) * TILE_SIZE;

	const float * heightmap = m_heightmap;
	float z00 = heightmap[ idx00 ];
	float z01 = heightmap[ idx01 ];
	float z10 = heightmap[ idx10 ];
	float z11 = heightmap[ idx11 ];
	float tx = frac( pos2.x );
	float ty = frac( pos2.y );
	float zx0 = z00 * ( 1.0f - tx ) + z10 * tx;
	float zx1 = z01 * ( 1.0f - tx ) + z11 * tx;
	pos.z = zx0 * ( 1.0f - ty ) + zx1 * ty;
	return pos;
}
//
//  Terrain.cpp
//
#include "Terrain/Terrain.h"
#include "Terrain/TerrainFile.h"
#include "Terrain/TerrainPool.h"
#include "Terrain/Heightmap.h"
#include "Graphics/Graphics.h"
#include "Graphics/Shader.h"
#include "Miscellaneous/Fileio.h"

#include <math.h>
#include <algorithm>
#include <stdlib.h>
#include <assert.h>

VertexBufferObject terrainIsland_t::ibo;
unsigned short terrainIsland_t::indices[ TERRAIN_ISLAND_SIZE * TERRAIN_ISLAND_SIZE * 6 ];

extern float frac( float value );

/*
================================
GetMaxDepth
================================
*/
int GetMaxDepth() {
	int tileSize = TILE_SIZE - 1;
	int terrainIsland = TERRAIN_ISLAND_SIZE - 1;
	int maxDepth = 0;
	while ( terrainIsland != tileSize ) {
		tileSize >>= 1;
		maxDepth++;
	}

	return maxDepth;
}

/*
================================
DrawterrainIsland
================================
*/
void DrawterrainIsland( terrainIsland_t * terra ) {
	if ( NULL == terra ) {
		return;
	}

	// Bind the VAO
    terra->vao.Bind();
    
    // Draw
	const int num = TERRAIN_ISLAND_SIZE * TERRAIN_ISLAND_SIZE * 6;
	glDrawElements( GL_TRIANGLES, num, GL_UNSIGNED_SHORT, 0 );

	// Unbind the VAO
	terra->vao.UnBind();
}
void DrawTerraID( int id ) {
	DrawterrainIsland( GetIsland( id ) );
}


/*
================================
CreateterrainIsland
================================
*/
void CreateterrainIsland( terrainIsland_t * terra ) {
	// Create the index buffer (if it doesn't already exist)
	if ( !terra->ibo.IsValid() ) {
		int idx = 0;
		for ( int y = 0; y < TERRAIN_ISLAND_SIZE - 1; y++ ) {
			for ( int x = 0; x < TERRAIN_ISLAND_SIZE - 1; x++ ) {
				int idxA = ( x + 0 ) + ( y + 0 ) * TERRAIN_ISLAND_SIZE;
				int idxB = ( x + 0 ) + ( y + 1 ) * TERRAIN_ISLAND_SIZE;
				int idxC = ( x + 1 ) + ( y + 1 ) * TERRAIN_ISLAND_SIZE;
				int idxD = ( x + 1 ) + ( y + 0 ) * TERRAIN_ISLAND_SIZE;

				terra->indices[ idx + 0 ] = (unsigned short)idxA;
				terra->indices[ idx + 1 ] = (unsigned short)idxD;
				terra->indices[ idx + 2 ] = (unsigned short)idxB;

				terra->indices[ idx + 3 ] = (unsigned short)idxC;
				terra->indices[ idx + 4 ] = (unsigned short)idxB;
				terra->indices[ idx + 5 ] = (unsigned short)idxD;

				idx += 6;
			}
		}

		const int size = TERRAIN_ISLAND_SIZE * TERRAIN_ISLAND_SIZE * 6 * sizeof( unsigned short );
		terra->ibo.Generate( GL_ELEMENT_ARRAY_BUFFER, size, terra->indices, GL_STATIC_DRAW );
	}

	// Create the VBO
	const int stride = sizeof( vert_t );
	const int size = TERRAIN_ISLAND_SIZE * TERRAIN_ISLAND_SIZE * stride;
	terra->vbo.Generate( GL_ARRAY_BUFFER, size, NULL, GL_DYNAMIC_DRAW );
    
	//
	//  Generate and Fill the VAO
	//
	terra->vao.Generate();
	terra->vao.Bind();
    
	terra->vbo.Bind();
	terra->ibo.Bind();

	unsigned long offset = 0;
	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset );
	offset += sizeof( Vec3d );
    
	glEnableVertexAttribArray( 1 );
	glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, stride, (const void *)offset );
	offset += sizeof( Vec2d );
    
	glEnableVertexAttribArray( 2 );
	glVertexAttribPointer( 2, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (const void *)offset );
	offset += sizeof( unsigned char ) * 4;
    
	glEnableVertexAttribArray( 3 );
	glVertexAttribPointer( 3, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (const void *)offset );
	offset += sizeof( unsigned char ) * 4;

	glEnableVertexAttribArray( 4 );
	glVertexAttribPointer( 4, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (const void *)offset );
	offset += sizeof( unsigned char ) * 4;

	terra->vao.UnBind();
	terra->vbo.UnBind();
	terra->ibo.UnBind();
}

/*
================================
terrainNode_t::terrainNode_t
================================
*/
terrainNode_t::terrainNode_t( Bounds * boundsData, int tileX_, int tileY_, int x_, int y_, int depth_ ) {
	forceUpdate = false;

	nodes[ 0 ] = NULL;
	nodes[ 1 ] = NULL;
	nodes[ 2 ] = NULL;
	nodes[ 3 ] = NULL;
	for ( int i = 0; i < 4; i++ ) {
		neighborLODs[ i ] = -1;
		neighborLODsPrev[ i ] = -1;
	}
	doRender = false;
	depth = depth_;
	x = x_;
	y = y_;
	tileX = tileX_;
	tileY = tileY_;

	// Calculate the index into the file data for this node
	extern int GetIslandID( int depth, int x, int y );
	int idx = GetIslandID( depth_, x, y );

	extern int GetTerrainIslandsPerTile();
	int maxterrainIslands = GetTerrainIslandsPerTile();
	assert( idx < maxterrainIslands );

	bounds = boundsData[ idx ];
	terraID = -1;

	int maxDepth = GetMaxDepth();
	if ( depth_ < maxDepth ) {
		nodes[ 0 ] = new terrainNode_t( boundsData, tileX_, tileY_, 2 * x + 0, 2 * y + 0, depth_ + 1 );
		nodes[ 1 ] = new terrainNode_t( boundsData, tileX_, tileY_, 2 * x + 1, 2 * y + 0, depth_ + 1 );
		nodes[ 2 ] = new terrainNode_t( boundsData, tileX_, tileY_, 2 * x + 0, 2 * y + 1, depth_ + 1 );
		nodes[ 3 ] = new terrainNode_t( boundsData, tileX_, tileY_, 2 * x + 1, 2 * y + 1, depth_ + 1 );
	}
}

/*
================================
TerrainTile::Build
================================
*/
void TerrainTile::Build( int tileX, int tileY ) {
	m_tileX = tileX;
	m_tileY = tileY;

	m_colorMap = ReadHeightmapFile( TILE_SIZE, tileX, tileY );

	TerrainTileFile * file = GetTerrainTileFile( tileX, tileY );
	if ( NULL == file ) {
		printf( "ERROR: Unable to read terrain file\n" );
		exit( 1 );
	}

	bool result = file->ReadHeightmap( m_heightmap );
	if ( !result ) {
		printf( "ERROR: Unable to read height map\n" );
		exit( 1 );
	}

	// Read the vertex data from file
	Bounds * boundsData = NULL;
	result = file->ReadBounds( &boundsData );
	if ( !result ) {
		printf( "ERROR: Unable to read bounds\n" );
		free( boundsData );
		exit( 1 );
	}
	m_bounds = boundsData[ 0 ];
	
	m_root = new terrainNode_t( boundsData, tileX, tileY, 0, 0, 0 );
	
	free( boundsData );
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

/*
================================
TerrainTile::GetColorMap
================================
*/
Vec4d TerrainTile::GetColorMap( Vec3d pos ) const {
	if ( !m_bounds.HasPoint2D( pos ) ) {
		return Vec4d( 0 );
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

	const Vec4d * heightmap = m_colorMap;
	Vec4d z00 = heightmap[ idx00 ];
	Vec4d z01 = heightmap[ idx01 ];
	Vec4d z10 = heightmap[ idx10 ];
	Vec4d z11 = heightmap[ idx11 ];
	float tx = frac( pos2.x );
	float ty = frac( pos2.y );
	Vec4d zx0 = z00 * ( 1.0f - tx ) + z10 * tx;
	Vec4d zx1 = z01 * ( 1.0f - tx ) + z11 * tx;
	Vec4d color = zx0 * ( 1.0f - ty ) + zx1 * ty;
	return color;
}
//
//  TerrainFile.h
//
#pragma once
#include "TerrainTile.h"
//#include <direct.h>
#include <stdio.h>
#include <stdlib.h>


struct TerrainVert_t;

void WriteHeightmapFile( Vec4d * data, int width );
Vec4d * ReadHeightmapFile( int width );

void WriteHeightmapFile( Vec4d * data, int width, int tileX, int tileY );
Vec4d * ReadHeightmapFile( int width, int tileX, int tileY );

void WriteTerrainFile( int tileX, int tileY );

/*
================================
TerrainTileFile
A memory mapped file for a terrain tile.
Used for streaming terrainIsland's into memory.
================================
*/
class TerrainTileFile {
public:
	TerrainTileFile() { m_file = NULL; m_size = 0; }
	~TerrainTileFile() { fclose( m_file ); m_file = NULL; }

	bool OpenFile( int tileX, int tileY );
	bool WriteFile( TerrainTile * tile );	// Probably shouldn't actually use the tile (maybe the height map?)
	bool ReadHeightmap( float * heightmap );
	bool ReadBounds( Bounds ** buffer );
	bool ReadTerrainIsland( int x, int y, int depth, void * buffer );

private:
	FILE * m_file;
	unsigned int m_size;
};
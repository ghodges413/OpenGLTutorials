//
//  TerrainFile.h
//
#pragma once
#include "TerrainTile.h"

struct TerrainVert_t;


void WriteHeightmapFile( Vec4d * data, int width );
Vec4d * ReadHeightmapFile( int width );


void WriteHeightmapFile( Vec4d * data, int width, int tileX, int tileY );
Vec4d * ReadHeightmapFile( int width, int tileX, int tileY );

//
//  TerrainVert.h
//
#pragma once
#include "../Vector.h"


struct terrainVert_t {
	Vec3d pos;
#if 0
	Vec2d st;
	char norm[ 3 ];
	char color[ 3 ];
	char tang[ 3 ];
	char buff[ 3 ];
#else
	Vec3d norm;
	Vec3d color;
#endif
};

terrainVert_t HeightmapToVert( Vec4d sample, int x, int y, int tileX, int tileY );

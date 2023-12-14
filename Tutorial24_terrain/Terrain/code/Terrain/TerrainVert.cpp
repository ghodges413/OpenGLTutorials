//
//  TerrainVert.cpp
//
#include "TerrainVert.h"
#include "TerrainTile.h"
#include "Terrain.h"
#include <math.h>
#include <algorithm>
#include <stdlib.h>
#include <assert.h>

#define WATER_HEIGHT 0.45f

#define CLIFF_COLOR    Vec3d( 0.22f, 0.2f, 0.2f )
#define DIRT_COLOR     Vec3d( 0.6f, 0.5f, 0.4f )
#define GRASS_COLOR1   Vec3d( 0.15f, 0.3f, 0.1f )
#define GRASS_COLOR2   Vec3d( 0.4f, 0.5f, 0.2f )
#define SAND_COLOR     Vec3d( 0.8f, 0.7f, 0.6f )

extern float clampf( float v, float min, float max );
extern float smoothstep( float a, float b, float x );

/*
================================
mix
================================
*/
Vec3d mix( Vec3d a, Vec3d b, float t ) {
	return a * ( 1.0f - t ) + b * t;
}

/*
================================
CalculateColor
================================
*/
Vec3d CalculateColor( Vec3d pos, Vec3d normal, Vec4d sample ) {
	float breakup = 0.0f;
	float occlusion = 0.0f;
	float erosion = 0.0f;
	breakup = sample.w;
	occlusion = sample.y;
	erosion = sample.z;
	Vec3d diffuseColor = Vec3d( 0.5f, 0.5f, 0.5f );
            
	// Cliffs / Dirt
	diffuseColor = CLIFF_COLOR * smoothstep( 0.4, 0.52, pos.z );
	diffuseColor = mix( diffuseColor, DIRT_COLOR, smoothstep( 0.3, 0.0, occlusion + breakup * 1.0 ) );

	// Grass
	Vec3d grassMix = mix( GRASS_COLOR1, GRASS_COLOR2, smoothstep( 0.4, 0.6, pos.z - erosion * 0.05 + breakup * 0.3 ) );
	diffuseColor = mix( diffuseColor, grassMix, smoothstep( WATER_HEIGHT + 0.05, WATER_HEIGHT + 0.02, pos.z - breakup * 0.02 ) * smoothstep( 0.8, 1.0, normal.z + breakup * 0.1 ) );
            
	// Snow
	diffuseColor = mix( diffuseColor, Vec3d( 1.0f, 1.0f, 1.0f ), smoothstep( 0.53, 0.6, pos.z + breakup * 0.1 ) );

	// Sand (beach)
	diffuseColor = mix( diffuseColor, SAND_COLOR, smoothstep( WATER_HEIGHT + 0.005, WATER_HEIGHT, pos.z + breakup * 0.01 ) );

	diffuseColor *= 1.0 + breakup * 0.5;

	return diffuseColor;
}

/*
================================
CalculateNormal
================================
*/
Vec3d CalculateNormal( Vec4d sample ) {
	float z = sqrtf( 1.0f - sample.y * sample.y - sample.z * sample.z );
	Vec3d norm = Vec3d( sample.y, sample.z, z );
	return norm;
}

/*
================================
HeightmapToVert
================================
*/
terrainVert_t HeightmapToVert( Vec4d sample, int x, int y, int tileX, int tileY ) {
	terrainVert_t vert;
	vert.pos = Vec3d( x, -y, sample.x );
	
	vert.norm = CalculateNormal( sample );
	vert.color = CalculateColor( vert.pos, vert.norm, sample );

	float halfTiles = float( TILES_WIDE ) * 0.5f;
	float tileCoordX = float( tileX ) - halfTiles;
	float tileCoordY = float( tileY ) - halfTiles;
	tileCoordX *= TILE_SIZE;
	tileCoordY *= TILE_SIZE;

	// dx/dy is to handle an off by 1 error.  Otherwise there's a 1-meter gap between tiles
	float dx = -tileX;
	float dy = -tileY;

	vert.pos.x += tileCoordX + dx;
	vert.pos.y += tileCoordY + dy;
	vert.pos.y += TILE_SIZE;

	vert.pos.x *= METERS_PER_VERT;	// Convert from vert coord to meters
	vert.pos.y *= METERS_PER_VERT;	// Convert from vert coord to meters

	vert.pos.z -= WATER_HEIGHT;
	vert.pos.z *= TILE_SIZE * METERS_PER_VERT;
	return vert;
}
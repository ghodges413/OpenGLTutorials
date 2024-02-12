//
//  TerrainVert.cpp
//
#include "Terrain/TerrainVert.h"
#include "Terrain/TerrainTile.h"
#include "Terrain/Terrain.h"
#include "Math/Math.h"
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

extern float smoothstep( float a, float b, float x );

/*
================================
mix
================================
*/
Vec3d mix( Vec3d a, Vec3d b, float t ) {
	return a * ( 1.0f - t ) + b * t;
}

void CalculateMaterialWeights( Vec3d normal, Vec4d sample, float weights[ 5 ] ) {
	float altitude = sample.x;
	float breakup = sample.w;
	float occlusion = sample.y;
	float erosion = sample.z;

	weights[ 0 ] = smoothstep( 0.4f, 0.52f, altitude );	// dirt
	weights[ 1 ] = smoothstep( 0.3f, 0.0f, occlusion + breakup * 1.0f );	// cliff
	weights[ 2 ] = smoothstep( WATER_HEIGHT + 0.05f, WATER_HEIGHT + 0.02f, altitude - breakup * 0.02f ) * smoothstep( 0.8f, 1.0f, normal.z + breakup * 0.1f ); // grass
	weights[ 3 ] = smoothstep( 0.53f, 0.6f, altitude + breakup * 0.1f ); // snow
	weights[ 4 ] = smoothstep( WATER_HEIGHT + 0.005f, WATER_HEIGHT, altitude + breakup * 0.01f );	// sand
}

/*
================================
CalculateColor
================================
*/
Vec3d CalculateColor( Vec3d normal, Vec4d sample ) {
	float altitude = sample.x;
	float breakup = sample.w;
	float occlusion = sample.y;
	float erosion = sample.z;
	Vec3d diffuseColor = Vec3d( 0.5f, 0.5f, 0.5f );

	// Cliffs / Dirt
	diffuseColor = CLIFF_COLOR * smoothstep( 0.4, 0.52, altitude );
	diffuseColor = mix( diffuseColor, DIRT_COLOR, smoothstep( 0.3, 0.0, occlusion + breakup * 1.0 ) );

	// Grass
	Vec3d grassMix = mix( GRASS_COLOR1, GRASS_COLOR2, smoothstep( 0.4, 0.6, altitude - erosion * 0.05 + breakup * 0.3 ) );
	diffuseColor = mix( diffuseColor, grassMix, smoothstep( WATER_HEIGHT + 0.05, WATER_HEIGHT + 0.02, altitude - breakup * 0.02 ) * smoothstep( 0.8, 1.0, normal.z + breakup * 0.1 ) );
            
	// Snow
	diffuseColor = mix( diffuseColor, Vec3d( 1.0f, 1.0f, 1.0f ), smoothstep( 0.53, 0.6, altitude + breakup * 0.1 ) );

	// Sand (beach)
	diffuseColor = mix( diffuseColor, SAND_COLOR, smoothstep( WATER_HEIGHT + 0.005, WATER_HEIGHT, altitude + breakup * 0.01 ) );

	diffuseColor *= 1.0 + breakup * 0.5;

	diffuseColor.x = Math::Clampf( diffuseColor.x, 0.0f, 1.0f );
	diffuseColor.y = Math::Clampf( diffuseColor.y, 0.0f, 1.0f );
	diffuseColor.z = Math::Clampf( diffuseColor.z, 0.0f, 1.0f );

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
	norm.Normalize();
	return norm;
}

/*
================================
HeightmapToPos
================================
*/
Vec3d HeightmapToPos( Vec4d sample, int x, int y, int tileX, int tileY ) {
	Vec3d pos = Vec3d( x, -y, sample.x );

	float halfTiles = float( TILES_WIDE ) * 0.5f;
	float tileCoordX = float( tileX ) - halfTiles;
	float tileCoordY = float( tileY ) - halfTiles;
	tileCoordX *= TILE_SIZE;
	tileCoordY *= TILE_SIZE;

	// dx/dy is to handle an off by 1 error.  Otherwise there's a 1-meter gap between tiles
	float dx = -tileX;
	float dy = -tileY;

	pos.x += tileCoordX + dx;
	pos.y += tileCoordY + dy;
	pos.y += TILE_SIZE;

	pos.x *= METERS_PER_VERT;	// Convert from vert coord to meters
	pos.y *= METERS_PER_VERT;	// Convert from vert coord to meters

	pos.z -= WATER_HEIGHT;
	pos.z *= TILE_SIZE * METERS_PER_VERT;
	return pos;
}

/*
================================
HeightmapToST
================================
*/
Vec2d HeightmapToST( int x, int y, int tileX, int tileY ) {
	Vec2d st = Vec2d( x, y );

	// Calculate the offset for the xy of this tile
	float tileOffsetX = TILE_SIZE * tileX;
	float tileOffsetY = TILE_SIZE * ( ( TILES_WIDE - 1 ) - tileY );

	st.x += tileOffsetX;
	st.y += tileOffsetY;
#if 1
	// Scale the xy from [0, 4096 * 5] to [0, 1]
	float totalVertsWide = TILES_WIDE * TILE_SIZE;

	st.x /= totalVertsWide;
	st.y /= totalVertsWide;

	//st.y = 1.0f - st.y;

	if ( st.x < 0.0f || st.x > 1.0f ) {
		printf( "Oh no!  ST out of range!:  %f %f\n", st.x, st.y );
		st.x = Math::Clampf( st.x, 0.0f, 1.0f );
	}
	if ( st.y < 0.0f || st.y > 1.0f ) {
		printf( "Oh no!  ST out of range!:  %f %f\n", st.x, st.y );
		st.y = Math::Clampf( st.y, 0.0f, 1.0f );
	}

	//printf( "ST:  %0.2f  %0.2f\n", st.x, st.y );
#endif
	return st;
}

/*
================================
HeightmapToVert
================================
*/
vert_t HeightmapToVert( Vec4d sample, int x, int y, int tileX, int tileY ) {
	vert_t vert;

	vert.pos = HeightmapToPos( sample, x, y, tileX, tileY );
	vert.st = HeightmapToST( x, y, tileX, tileY );
	
	Vec3d norm = CalculateNormal( sample );
	Vec3dToByte4_n11( vert.norm, norm );

	// TODO: Calculate the tangent vector
	vert.tang[ 0 ] = 0;
	vert.tang[ 1 ] = 0;
	vert.tang[ 2 ] = 0;
	vert.tang[ 3 ] = 0;
	
	Vec3d color = CalculateColor( norm, sample );
	Vec3dToByte4_01( vert.buff, color );

	return vert;
}

void CalculateMaterialWeights( Vec4d sample, float weights[ 5 ] ) {
	Vec3d norm = CalculateNormal( sample );
	CalculateMaterialWeights( norm, sample, weights );
}
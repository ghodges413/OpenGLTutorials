//
//  Map.cpp
//
#include "BSP/Map.h"
#include "BSP/Brush.h"
#include "BSP/BSP.h"
#include "Math/MatrixOps.h"
#include "Math/Matrix.h"
#include "Math/Bounds.h"
#include "Graphics/Graphics.h"
#include "Graphics/TextureManager.h"
#include "Graphics/ShaderManager.h"
#include "Graphics/Targa.h"
#include "Miscellaneous/Fileio.h"
#include <math.h>
#include <algorithm>
#include <stdlib.h>
#include <assert.h>
#include <vector>

std::vector< vert_t > g_quakeVerts;
std::vector< int > g_quakeIndices;

static VertexBufferObject g_quakeVBO;
static VertexArrayObject g_quakeVAO;
static VertexBufferObject g_quakeIBO;

static std::vector< brush_t > s_brushes;

static bsp_t * s_bsp = NULL;

/*
================================================================

Map


================================================================
*/

void UnloadMap() {
	DeleteBSP_r( s_bsp );
}

/*
================================
LoadMap
================================
*/
bool LoadMap() {
	unsigned int size = 0;
	unsigned char * data = NULL;

	//const char * fileName = "../../common/maps/211491Level.map";
	//const char * fileName = "../../common/maps/infernalislands.map";
	//const char * fileName = "../../common/maps/handuo_stepwell.map";
	//const char * fileName = "../../common/maps/devotion.map";
	//const char * fileName = "../../common/maps/rgfire.map";
	//const char * fileName = "../../common/maps/rgcastle.map";
	const char * fileName = "../../common/maps/testmap.map";

	char fullPath[ 2048 ];
	RelativePathToFullPath( fileName, fullPath );

	FILE * fp = fopen( fullPath, "rb" );
	if ( !fp ) {
		return false;
	}

	char buff[ 1024 ] = { 0 };
	while ( !feof( fp ) ) {
		// Read whole line
		fgets( buff, sizeof( buff ), fp );

		int brushid = 0;
		if ( sscanf( buff, "// brush %i", &brushid ) ) {
			// Read whole line
			fgets( buff, sizeof( buff ), fp );	// '{' line

			Vec3d a;
			Vec3d b;
			Vec3d c;
			char strBuff[ 64 ] = { 0 };
			Vec2d stOffset;
			float stRotation = 0;
			Vec2d stScale;

			brush_t brush;
			brush.numPlanes = 0;

			while ( brush.numPlanes < s_maxPlanes - 1 ) {
				// Read whole line
				fgets( buff, sizeof( buff ), fp );

				int numRead = sscanf(
					buff, "( %f %f %f ) ( %f %f %f ) ( %f %f %f ) %s %f %f %f %f %f",
					&a.x, &a.y, &a.z,
					&b.x, &b.y, &b.z,
					&c.x, &c.y, &c.z,
					&strBuff,
					&stOffset.x, &stOffset.y,
					&stRotation,
					&stScale.x, &stScale.y
				);

				if ( 0 == numRead ) {
					// '}' line
					break;
				}

				// Quake brushes are stored in clockwise order.
				// This is because their normals point towards the inside.
				// This is probably due to some older papers where a convex
				// set was considered the intersection of half spaces, and
				// the positive side of the half space was considered "full"
				// while the negative side was considered "empty".

				// We, however, do not use this convention.  This is probably due
				// to my physics point of view.  Where the normals of an object's
				// surface points outward.
				plane_t plane;
				plane.pts[ 0 ] = b;
				plane.pts[ 1 ] = a;
				plane.pts[ 2 ] = c;
				plane.normal = CalculateNormal( b, a, c );

				brush.planes[ brush.numPlanes ] = plane;
				brush.numPlanes++;
			}

			s_brushes.push_back( brush );
		}
	}
	fclose( fp );

	// Now that we have all the brushes, we wish to build their geometry
	for ( int i = 0; i < s_brushes.size(); i++ ) {
		brush_t & brush = s_brushes[ i ];
		BuildBrush( brush );

		if ( ( i % 100 ) == 0 ) {
			printf( "%i of %i brushes\n", i, s_brushes.size() );
		}
	}

	s_bsp = BuildBSP( s_brushes.data(), s_brushes.size() );

	int totalWindings = 0;
	for ( int i = 0; i < s_brushes.size(); i++ ) {
		const brush_t & brush = s_brushes[ i ];

		for ( int w = 0; w < brush.numPlanes; w++ ) {
			const winding_t & winding = brush.windings[ w ];
			WindingToVerts( winding, g_quakeVerts, g_quakeIndices );
		}
	}

	int totalWindings2 = 0;
	g_quakeVerts.clear();
	g_quakeIndices.clear();
	BuildBSPModel_r( s_bsp, g_quakeVerts, g_quakeIndices, totalWindings2 );

	printf( "Brush Windings: %i   BSP Windings: %i\n", totalWindings, totalWindings2 );

	const int sizeIBO = g_quakeIndices.size() * sizeof( unsigned int );
	g_quakeIBO.Generate( GL_ELEMENT_ARRAY_BUFFER, sizeIBO, &g_quakeIndices[ 0 ], GL_STATIC_DRAW );

	const int stride = sizeof( vert_t );
	const int sizeVBO = g_quakeVerts.size() * stride;
	g_quakeVBO.Generate( GL_ARRAY_BUFFER, sizeVBO, &g_quakeVerts[ 0 ], GL_DYNAMIC_DRAW );

	g_quakeVAO.Generate();
	g_quakeVAO.Bind();
    
	g_quakeVBO.Bind();
	g_quakeIBO.Bind();

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

	g_quakeVAO.UnBind();
	g_quakeVBO.UnBind();
	g_quakeIBO.UnBind();

	// That should be it here
	volatile static int counter = 0;
	counter++;
	counter++;
	return true;
}

/*
================================
DrawMap
================================
*/
void DrawMap() {
	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

	// Bind the VAO
    g_quakeVAO.Bind();
    
    // Draw
	const int num = g_quakeIndices.size();
	glDrawElements( GL_TRIANGLES, num, GL_UNSIGNED_INT, 0 );

	// Unbind the VAO
	g_quakeVAO.UnBind();

	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
}

void DrawMap( Shader * shader, Vec3d camPos ) {
	DrawBSP_r( s_bsp, shader );
	DrawSplitPlaneBSP_r( s_bsp, shader, camPos, 0 );
}
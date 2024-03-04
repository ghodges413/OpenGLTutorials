//
//  BSP.h
//
#pragma once
#include "Math/Vector.h"
#include "Graphics/Mesh.h"
#include "Graphics/RenderSurface.h"
#include "Graphics/Shader.h"
#include "Miscellaneous/Types.h"
#include "BSP/Brush.h"
#include <vector>

struct bsp_t {
	plane_t plane;	// the splitting plane
	std::vector< winding_t > windings;

	bsp_t * front;
	bsp_t * back;
};

bsp_t * BuildBSP( const brush_t * brush, int numBrushes );
void DeleteBSP_r( bsp_t * node );

void WindingToVerts( const winding_t & winding, std::vector< vert_t > & verts, std::vector< int > & indices );
void BuildBSPModel_r( bsp_t * node, std::vector< vert_t > & verts, std::vector< int > & indices, int & totalWindings );

void DrawBSP_r( bsp_t * node, Shader * shader );
void DrawSplitPlaneBSP_r( bsp_t * node, Shader * shader, Vec3 camPos, int depth );
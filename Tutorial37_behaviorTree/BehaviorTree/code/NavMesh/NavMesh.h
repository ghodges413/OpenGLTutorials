//
//  NavMesh.h
//
#pragma once
#include "Math/Vector.h"
#include "Graphics/Mesh.h"
#include "Graphics/RenderSurface.h"
#include "Graphics/Shader.h"
#include "Miscellaneous/Types.h"
#include "BSP/Brush.h"

struct navEdge_t {
	Vec3 a;
	Vec3 b;
};

void LoadNavMesh();
void DrawNavMesh();
void DrawNavMeshEdges( Shader * shader );
void DrawNavMeshNode( Vec3 pt, Shader * shader );
void DrawNavMeshNeighborTriangle( Vec3 pt, Shader * shader );



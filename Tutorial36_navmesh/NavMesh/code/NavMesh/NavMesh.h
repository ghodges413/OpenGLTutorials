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

void BuildNavMesh();
void DrawNavMesh();
void DrawNavMeshTriangle( Vec3 pt, Shader * shader );
void DrawNavMeshNeighborTriangle( Vec3 pt, Shader * shader );



//
//  Map.h
//
#pragma once
#include "Math/Vector.h"
#include "Graphics/Mesh.h"
#include "Graphics/RenderSurface.h"
#include "Graphics/Shader.h"
#include "Miscellaneous/Types.h"
#include "BSP/Brush.h"

bool LoadMap();
void DrawMap();

extern std::vector< brush_t > g_brushes;

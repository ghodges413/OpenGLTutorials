//
//  Map.h
//
#pragma once
#include "Math/Vector.h"
#include "Graphics/Mesh.h"
#include "Graphics/RenderSurface.h"
#include "Graphics/Shader.h"
#include "Miscellaneous/Types.h"

bool LoadMap();
void DrawMap();
void DrawMap( Shader * shader, Vec3d camPos );
void UnloadMap();
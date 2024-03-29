//
//	HashGrid.h
//
#pragma once
#include "Miscellaneous/Array.h"
#include "Math/Quat.h"
#include "Math/Matrix.h"
#include "Fluids/FluidSim.h"
#include <vector>

void BuildGrid( fluid_t * points, const int num );
void UpdateGrid( fluid_t * points, const int num );
void GetNeighborIds( std::vector< int > & ids, const Vec3 & pt );
void GetNeighborIds( std::vector< int > & ids, const int idx );

void UpdateCells( fluid_t * points );
void GetNeighborIds_Cells( std::vector< int > & ids, const int idx );


void InitGPUHashGrid();
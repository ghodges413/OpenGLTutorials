//
//  PathFinding.h
//
#pragma once
#include "NavMesh/NavMesh.h"
#include <vector>

Vec3 PathFind( const Vec3 & start, const Vec3 & end );
bool PathFind( const Vec3 & start, const Vec3 & end, std::vector< Vec3 > & path, std::vector< navEdge_t > & edges );

bool IsPositionInNavMesh( const Vec3 & pos );
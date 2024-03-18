//
//  epa.h
//
#pragma once
#include "Vector.h"
#include "Quat.h"
#include "../Physics/Body.h"
#include "gjk.h"

float EPA_Expand( const Body * bodyA, const Body * bodyB, const float bias, const point_t simplexPoints[ 4 ], Vec3 & ptOnA, Vec3 & ptOnB );
//
//	Quat.cpp
//
#include "Math/Quat.h"
#include "Math/Math.h"

/*
 ================================
 Quat::Quat
 ================================
 */

// Quat::Quat( const Vec3 & n, const float angleDegrees ) {
//     const float halfAngleDegrees = 0.5f * angleDegrees;
// 	const float halfAngleRadians = halfAngleDegrees * Math::PI / 180.0f;
// 
// 	w = cosf( halfAngleRadians );
// 	
// 	const float halfSine = sinf( halfAngleRadians );
// 	x = n.x * halfSine;
// 	y = n.y * halfSine;
// 	z = n.z * halfSine;
// }
/*
 *  Quat.cpp
 *
 */
#include "Quat.h"

#ifndef MY_PI
#define MY_PI 3.14159265f
#endif

/*
 ================================
 Quat::Quat
 ================================
 */
Quat::Quat( const Vec3d & n, const float angleDegrees ) {
    const float halfAngleDegrees = 0.5f * angleDegrees;
	const float halfAngleRadians = halfAngleDegrees * MY_PI / 180.0f;

	w = cosf( halfAngleRadians );
	
	const float halfSine = sinf( halfAngleRadians );
	x = n.x * halfSine;
	y = n.y * halfSine;
	z = n.z * halfSine;
}
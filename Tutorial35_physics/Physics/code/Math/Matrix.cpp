//
//	Matrix.cpp
//
#pragma once
#include "Vector.h"
#include "Quat.h"

void Mat4::Orient( const Vec3 & pos, const Quat & quat ) {
	Vec3 fwd = quat.RotatePoint( Vec3( 1, 0, 0 ) );
	Vec3 up = quat.RotatePoint( Vec3( 0, 0, 1 ) );

	Orient( pos, fwd, up );
}
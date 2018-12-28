/*
 *  Vector.cpp
 *
 */
#include "Vector.h"

/*
 ================================
 operator*
 ================================
 */
Vec2d operator*( const int& lhs, const Vec2d& rhs ) {
	Vec2d tmp;
	tmp.x = lhs * rhs.x;
	tmp.y = lhs * rhs.y;
	return tmp; 
}

/*
 ================================
 operator*
 ================================
 */
Vec2d operator*( const float& lhs, const Vec2d& rhs ) { 
	Vec2d tmp;
	tmp.x = lhs * rhs.x;
	tmp.y = lhs * rhs.y;
	return tmp; 
}

/*
 ================================
 operator*
 ================================
 */
Vec3d operator*( const int& lhs, const Vec3d& rhs ) {
	Vec3d tmp;
	tmp.x = lhs * rhs.x;
	tmp.y = lhs * rhs.y;
	tmp.z = lhs * rhs.z;
	return tmp; 
}

/*
 ================================
 operator*
 ================================
 */
Vec3d operator*( const float& lhs, const Vec3d& rhs ) { 
	Vec3d tmp;
	tmp.x = lhs * rhs.x;
	tmp.y = lhs * rhs.y;
	tmp.z = lhs * rhs.z;
	return tmp; 
}

/*
 ================================
 operator*
 ================================
 */
Vec4d operator*( const int& lhs, const Vec4d& rhs ) {
	Vec4d tmp;
	tmp.x = lhs * rhs.x;
	tmp.y = lhs * rhs.y;
	tmp.z = lhs * rhs.z;
    tmp.w = lhs * rhs.w;
	return tmp; 
}

/*
 ================================
 operator*
 ================================
 */
Vec4d operator*( const float& lhs, const Vec4d& rhs ) { 
	Vec4d tmp;
	tmp.x = lhs * rhs.x;
	tmp.y = lhs * rhs.y;
	tmp.z = lhs * rhs.z;
    tmp.w = lhs * rhs.w;
	return tmp; 
}


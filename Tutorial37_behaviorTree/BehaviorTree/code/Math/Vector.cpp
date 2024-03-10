/*
 *  Vector.cpp
 *
 */
#include "Math/Vector.h"

/*
 ================================
 operator*
 ================================
 */
Vec2 operator*( const int& lhs, const Vec2& rhs ) {
	Vec2 tmp;
	tmp.x = lhs * rhs.x;
	tmp.y = lhs * rhs.y;
	return tmp; 
}

/*
 ================================
 operator*
 ================================
 */
Vec2 operator*( const float& lhs, const Vec2& rhs ) { 
	Vec2 tmp;
	tmp.x = lhs * rhs.x;
	tmp.y = lhs * rhs.y;
	return tmp; 
}

/*
 ================================
 operator*
 ================================
 */
Vec3 operator*( const int& lhs, const Vec3& rhs ) {
	Vec3 tmp;
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
Vec3 operator*( const float& lhs, const Vec3& rhs ) { 
	Vec3 tmp;
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
Vec4 operator*( const int& lhs, const Vec4& rhs ) {
	Vec4 tmp;
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
Vec4 operator*( const float& lhs, const Vec4& rhs ) { 
	Vec4 tmp;
	tmp.x = lhs * rhs.x;
	tmp.y = lhs * rhs.y;
	tmp.z = lhs * rhs.z;
    tmp.w = lhs * rhs.w;
	return tmp; 
}


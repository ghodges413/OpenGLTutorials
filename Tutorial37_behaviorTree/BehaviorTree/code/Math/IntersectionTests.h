//
//	IntersectionTests.h
//
#pragma once
#include "Math/Vector.h"

extern bool RayTriangleIntersectionTest( const Vec3& rpos, const Vec3& rdir, 
										const Vec3& v0, const Vec3& v1, const Vec3& v2 );
extern bool RayTriangleIntersectionTestBackFaceCull( const Vec3& rpos, const Vec3& rdir, 
													const Vec3& v0, const Vec3& v1, const Vec3& v2 );

extern int triBoxOverlap( const float boxcenter[3], const float boxhalfsize[3], const float triverts[3][3]);



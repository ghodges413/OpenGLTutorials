//
//	IntersectionTests.h
//
#pragma once
#include "Math/Vector.h"

extern bool RayTriangleIntersectionTest( const Vec3d& rpos, const Vec3d& rdir, 
										const Vec3d& v0, const Vec3d& v1, const Vec3d& v2 );
extern bool RayTriangleIntersectionTestBackFaceCull( const Vec3d& rpos, const Vec3d& rdir, 
													const Vec3d& v0, const Vec3d& v1, const Vec3d& v2 );

extern int triBoxOverlap( const float boxcenter[3], const float boxhalfsize[3], const float triverts[3][3]);



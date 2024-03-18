//
//  Frustum.h
//
#pragma once
#include "Math/Plane.h"
#include "Math/Bounds.h"
#include "Math/Matrix.h"

/*
================================
Frustum
================================
*/
class Frustum {
public:
	Frustum();
	Frustum( const Frustum & rhs );
    Frustum( const float & near,
                  const float & far,
                  const float & fovy_degrees,
                  const float & screenWidth,
                  const float & screenHeight,
                  const Vec3 & camPos,
                  const Vec3 & camUp,
                  const Vec3 & camLook );
    Frustum( const float * mat );
    Frustum( const float * proj, const float * view );
	Frustum & operator=( const Frustum & rhs );
	~Frustum();
    
    void Build(   const float & near,
                  const float & far,
                  const float & fovy_degrees,
                  const float & screenWidth,
                  const float & screenHeight,
                  const Vec3 & camPos,
                  const Vec3 & camUp,
                  const Vec3 & camLook );
    void MoveNearPlane( const float & near, const Vec3 & camPos, const Vec3 & camLook );
    void Build( const float * mat );
    void Build( const float * proj, const float * view );
	void Build( const Mat4 & mat );
    
    //bool IsSphereInFrustum( const Sphere & sphere ) const;
    bool IsBoxInFrustum( const Bounds & box ) const;
	bool IntersectBox( const Bounds & box ) const;
	//bool IntersectSphere( const Sphere & sphere ) const;
    
private:
	static Vec4 ApplyInverseProject( const Mat4 & matProjInv, const Vec4 & pt );
	static Vec3 CalculateSideNormal( const Vec4 & sideNear,
										const Vec4 & sideFar,
										const Vec4 & ptNear );
    
public:
    Plane   m_planes[ 6 ];
    bool    m_isInitialized;

    Vec3   m_corners[ 8 ];
    Bounds  m_bounds;
};
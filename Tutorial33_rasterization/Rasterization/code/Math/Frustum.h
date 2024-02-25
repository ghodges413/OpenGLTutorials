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
                  const Vec3d & camPos,
                  const Vec3d & camUp,
                  const Vec3d & camLook );
    Frustum( const float * mat );
    Frustum( const float * proj, const float * view );
	Frustum & operator=( const Frustum & rhs );
	~Frustum();
    
    void Build(   const float & near,
                  const float & far,
                  const float & fovy_degrees,
                  const float & screenWidth,
                  const float & screenHeight,
                  const Vec3d & camPos,
                  const Vec3d & camUp,
                  const Vec3d & camLook );
    void MoveNearPlane( const float & near, const Vec3d & camPos, const Vec3d & camLook );
    void Build( const float * mat );
    void Build( const float * proj, const float * view );
	void Build( const Matrix & mat );
    
    //bool IsSphereInFrustum( const Sphere & sphere ) const;
    bool IsBoxInFrustum( const Bounds & box ) const;
	bool IntersectBox( const Bounds & box ) const;
	//bool IntersectSphere( const Sphere & sphere ) const;
    
private:
	static Vec4d ApplyInverseProject( const Matrix & matProjInv, const Vec4d & pt );
	static Vec3d CalculateSideNormal( const Vec4d & sideNear,
										const Vec4d & sideFar,
										const Vec4d & ptNear );
    
public:
    Plane   m_planes[ 6 ];
    bool    m_isInitialized;

    Vec3d   m_corners[ 8 ];
    Bounds  m_bounds;
};
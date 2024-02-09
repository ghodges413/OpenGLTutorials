//
//  Sphere.h
//
#pragma once
#include "Math/Vector.h"

// Forward declarator
template < typename T > class Array;

/*
 ===============================
 Sphere
 ===============================
 */
class Sphere {
public:
    Sphere();
    Sphere( const Vec3d & center, const float radius );
    Sphere( const float x, const float y, const float z, const float r );
    Sphere( const Sphere & rhs );
    Sphere( const Vec3d * pts, const int num_pts );
    const Sphere & operator = ( const Sphere & rhs );
    ~Sphere() {}
    
    void Inflate( const Vec3d & pt );
    void Inflate( const Vec3d * pts, const int& num_pts );
    
    void TightestFit( const Vec3d * pts, const int& num_pts );
    void GeometricFit( const Vec3d * pts, const int& num_pts );
    
	static void BuildHalfSphereGeo( Array< Vec3d > & out, const int & numSteps );
    static void BuildGeometry( Array< Vec3d > & out, const int & numSteps );
    static Sphere Expand( const Vec3d & poleA, const Vec3d & poleB, const Vec3d & ext );

	bool IsPointInSphere( const Vec3d & point ) const;
	bool IntersectSphere( const Sphere & rhs ) const;
    
public:
	Vec3d mCenter;
	float   mRadius;
	bool    mInitialized;
};

/*
 ===============================
 hbIntersectRaySphere
 always t1 <= t2
 if t1 < 0 && t2 > 0 ray is inside
 if t1 < 0 && t2 < 0 sphere is behind ray origin
 recover the 3D position with p = rayStart + t * rayDir
 ===============================
 */
inline bool hbIntersectRaySphere( const Vec3d & rayStart, const Vec3d & rayDir, const Sphere & sphere, float & t1, float & t2 ) {
    // Note:    If we force the rayDir to be normalized,
    //          then we can get an optimization where
    //          a = 1, b = m.
    //          Which would decrease the number of operations
    const Vec3d m = sphere.mCenter - rayStart;
    const float a   = rayDir.DotProduct( rayDir );
    const float b   = m.DotProduct( rayDir );
    const float c   = m.DotProduct( m ) - sphere.mRadius * sphere.mRadius;
    
    const float delta = b * b - a * c;
    const float invA = 1.0f / a;
    
    if ( delta < 0 ) {
        // no real solutions exist
        return false;
    }
    
    const float deltaRoot = sqrtf( delta );
    t1 = invA * ( b - deltaRoot );
    t2 = invA * ( b + deltaRoot );

    return true;
}
/*
 *  PointLight.cpp
 *
 */
#include "PointLight.h"
#include <math.h>
#include <assert.h>

/*
 ================================================================
 
 PointLight
 
 ================================================================
 */

const float PointLight::sIntensityMinimum = 0.1f;
Mesh PointLight::sSphereMesh;

/*
 ================================
 PointLight::PointLight
 ================================
 */
PointLight::PointLight() :
mPosition( 0 ),
mMaxInvSqrDist( 0.00001f ),
mColor( 1 ),
mScale( 0.0f ) {
	SetIntensity( 2000 );

	if ( 0 == sSphereMesh.NumVerts() ) {
		sSphereMesh.Load( "data/Meshes/sphere.obj" );
	}
}

/*
 ================================
 PointLight::PointLight
 ================================
 */
PointLight::PointLight( const PointLight & rhs ) :
mPosition( rhs.mPosition ),
mMaxInvSqrDist( rhs.mMaxInvSqrDist ),
mMaxDist( rhs.mMaxDist ),
mScale( rhs.mScale ),
mColor( rhs.mColor ),
mIntensity( rhs.mIntensity ) {
}

/*
 ================================
 PointLight::PointLight
 ================================
 */
const PointLight & PointLight::operator=( const PointLight & rhs ) {
    mPosition       = rhs.mPosition;
    mMaxInvSqrDist  = rhs.mMaxInvSqrDist;
	mMaxDist		= rhs.mMaxDist;
	mScale			= rhs.mScale;
    mColor          = rhs.mColor;
    mIntensity      = rhs.mIntensity;
    
    return *this;
}

/*
 ================================
 PointLight::SetIntensity
 ================================
 */
void PointLight::SetIntensity( const float & intensity ) {
	assert( intensity > 0.0f );

	// I( r ) = I / r^2
	mIntensity = intensity;
	const float maxSqrDist = mIntensity / sIntensityMinimum;
	mMaxDist = sqrtf( maxSqrDist );

	mScale = mMaxDist * 1.05f;
}

/*
 ================================
 PointLight::Draw
 ================================
 */
void PointLight::Draw() const {
	sSphereMesh.Draw();
}

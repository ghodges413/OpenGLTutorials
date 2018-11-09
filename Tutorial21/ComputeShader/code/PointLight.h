/*
 *  PointLight.h
 *
 */
#pragma once
#include "Vector.h"
#include "Array.h"
#include "Mesh.h"

/*
 ================================
 PointLight
 ================================
 */
class PointLight {
	friend class hbLightProbe;
public:
    PointLight();
    PointLight( const PointLight & rhs );
    const PointLight & operator=( const PointLight & rhs );
    
    ~PointLight() {}
    
	void SetIntensity( const float & intensity );
	float GetIntensity() const { return mIntensity; }

	float GetScale() const { return mScale; }

	void Draw() const;

public:
    Vec3d mPosition;
	Vec3d mColor;

	float	mMaxDist;
	float mScale;	// The scale used to size the unit S2 sphere
	float   mIntensity;
private:
    float   mMaxInvSqrDist;

	// The intensity minimum is used as a cutoff of light for pointlights.
	// This way we don't have to draw the theoretically infinite drawing
	// volume for a point source, like what would actually happen in the
	// real world.
	static const float	sIntensityMinimum;

	static Mesh sSphereMesh;
};

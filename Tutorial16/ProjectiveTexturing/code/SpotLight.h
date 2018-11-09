/*
 *  SpotLight.h
 *
 */
#pragma once
#include "Vector.h"
#include "Mesh.h"
#include "RenderSurface.h"
#include "Texture.h"
#include "Shader.h"

/*
 ================================
 SpotLight
 ================================
 */
class SpotLight {
public:
	SpotLight();
	~SpotLight() {}
    
	void Initialize();
	void SetHalfAngle( const float radians );
    
	void ClearShadowMap();
	void Update( Mesh & mesh, const float * matModelToWorld );

	void DebugDraw();
	
public:
    Vec3d mPosition;
    Vec3d mDirection;
    Vec3d mUp;
    Vec3d mColor;
    float   mMaxDistance;
	float	mNearPlane;
    float   mIntensity;
    
    RenderSurface mShadowSurface;
	Texture mTextureProjective;
	
    float   mMatProj[ 16 ];
    float   mMatView[ 16 ];
    float   mMatViewProj[ 16 ];
	float	mMatBias[ 16 ];

	float	mCosHalfAngle;

private:
	float   mMaxHalfAngle;  // in radians
	float	mPhysicalFalloffConstant;

	Shader mShaderDepthOnly;
	Shader mShaderDebug;
};

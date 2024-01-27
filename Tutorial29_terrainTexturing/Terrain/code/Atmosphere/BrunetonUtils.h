//
//  BrunetonUtils.h
//
#pragma once
#ifndef __BrunetonUtils__H_
#define __BrunetonUtils__H_

#include "Math/Vector.h"

struct BrunetonData_t {
	float radiusGround;
	float radiusTop;
	int numSamplesExtinction;
	Vec2d dimTransmission;
	Vec3d betaRayleighExtinction;
	Vec3d betaMieExtinction;
	float scaleHeightRayleigh;
	float scaleHeightMie;
	Vec3d betaRayleighScatter;
	Vec3d betaMieScatter;

	Vec2d dimIrradiance;
	float numSamplesIrradiance;

	Vec4d dimScatter;
	int numSamplesScatter;

	int numSamplesScatterSpherical;

	float averageGroundReflectence;
	float mieG;
};

float GetRadiusExtinction( const int coord, const int dim, const float radiusTop, const float radiusGround );
float GetAngleExtinction( const int coord, const int dim );
Vec4d Transmittance( const BrunetonData_t & data, const float radius, const float cosAngle, const Vec4d * sampler );
Vec4d Transmittance( const BrunetonData_t & data, Vec3d pos, Vec3d view, const Vec4d * sampler );

float GetRadiusIrradiance( const int coord, const int dim, const float radiusTop, const float radiusGround );
float GetAngleIrradiance( const int coord, const int dim );
Vec4d Irradiance( const BrunetonData_t & data, const float radius, const float cosAngle, const Vec4d * sampler );

bool DoesCollideGround( const const Vec3d & pt, const Vec3d & ray, const float ground );
float IntersectGroundTop( const Vec3d & pt, const Vec3d & ray, const BrunetonData_t & data );


float ScatterPhaseFunctionRayleigh( const float cosTheta );
float ScatterPhaseFunctionMie( const float cosTheta, const float mieG );


float GetRadius( const int coord, const int dim, const float radiusTop, const float radiusGround );
float GetCosAngleViewSun( const int coord, const int dim );
float GetCosAngleSun( const int coord, const int dim );
float GetCosAngleView( const int coord, const int dim, const float radius, const float radiusTop, const float radiusGround );
float ClampCosAngleViewSun( const float cosAngleViewSun, const float cosAngleView, const float cosAngleSun );
Vec4d GetCoords4D( const BrunetonData_t & data, const float radius, const float cosAngleView, const float cosAngleSun, const float cosAngleViewSun );
Vec4d SampleScatter( const BrunetonData_t & data, const float radius, const float cosAngleView, const float cosAngleSun, const float cosAngleViewSun, const Vec4d * table );


Vec4d SampleTexture1D( const Vec4d * texture, float s, const int dimX );
Vec4d SampleTexture1DLinear( const Vec4d * texture, float s, const int dimX );
Vec4d SampleTexture2D( const Vec4d * texture, float s, float t, const int dimX, const int dimY );
Vec4d SampleTexture2DLinear( const Vec4d * texture, float s, float t, const int dimX, const int dimY );
Vec4d SampleTexture3D( const Vec4d * texture, float s, float t, float r, const int dimX, const int dimY, const int dimZ );
Vec4d SampleTexture3DLinear( const Vec4d * texture, float s, float t, float r, const int dimX, const int dimY, const int dimZ );
Vec4d SampleTexture4D( const Vec4d * texture, float s, float t, float r, float q, const int dimX, const int dimY, const int dimZ, const int dimW );
Vec4d SampleTexture4DLinear( const Vec4d * texture, float s, float t, float r, float q, const int dimX, const int dimY, const int dimZ, const int dimW );

#endif /* defined(__BrunetonUtils__H_) */

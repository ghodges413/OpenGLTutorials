//
//  BuildBruneton.h
//
#pragma once
#ifndef __BuildBruneton__H_
#define __BuildBruneton__H_

#include "Math/Vector.h"


struct atmosphereBuildData_t {
	float radiusGround;	// the radius of the planet surface [km]
	float radiusTop;		// the radius of the top of the atmosphere [km]
	float scaleHeightRayleigh;	// [km]
	float scaleHeightMie;		// [km]
	float mieG;			// Used in the scattering phase function
	Vec3d betaRayleighScatter;
	Vec3d betaRayleighExtinction;
	Vec3d betaMieScatter;
	Vec3d betaMieExtinction;
	Vec3d sunLightIntensity;
	Vec3d indicesOfRefraction;
	double molecularDensity_NsR; // molecular density Rayleigh ( particles / meter^3 )
	double molecularDensity_NsM; // particle density Mie ( particles / meter^3 )
	int numSamples;
	int textureResX;	// Height
	int textureResY;	// Sun Angle
	int textureResZ;	// View Angle
};


extern class Texture * g_transmittanceTexture;
extern class Texture * g_groundIrradianceTexture;
extern class Texture * g_inscatterTexture;


/*
 =====================================
 Build Atmosphere
 =====================================
 */
void BuildAtmosphereBruneton( const atmosphereBuildData_t & atmosData );

#endif /* defined(__BuildBruneton__H_) */

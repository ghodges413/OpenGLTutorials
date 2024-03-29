//
//	FluidSim.h
//
#pragma once
#include "Miscellaneous/Array.h"
#include "Math/Quat.h"
#include "Math/Matrix.h"

#define FLUID_SIM_CPU // toggle to switch between cpu and gpu sims

#define SIDE_SIZE 24
#if defined( FLUID_SIM_CPU )
#define MAX_PARTICLES ( SIDE_SIZE * SIDE_SIZE )

#define PARTICLE_MASS 1
#define PARTICLE_RADIUS 0.25f

#else
#define MAX_PARTICLES ( SIDE_SIZE * SIDE_SIZE * SIDE_SIZE )

#define PARTICLE_MASS 1
#define PARTICLE_RADIUS 1.25f
#endif

#define FS_PI 3.1415f


struct fluid_t {
	Vec3 pos;
	float pad0;

	Vec3 vel;
	float pressure;

	Vec3 posOld;
	float density;
};

extern fluid_t g_particles[ MAX_PARTICLES ];

void ResetFluidSim();
void FluidSimStep( float dt );
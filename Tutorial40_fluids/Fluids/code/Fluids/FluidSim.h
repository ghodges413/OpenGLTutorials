//
//	FluidSim.h
//
#pragma once
#include "Miscellaneous/Array.h"
#include "Math/Quat.h"
#include "Math/Matrix.h"

#define SIDE_SIZE 24
//#define MAX_PARTICLES ( SIDE_SIZE * SIDE_SIZE )
#define MAX_PARTICLES ( SIDE_SIZE * SIDE_SIZE * SIDE_SIZE )
#define PARTICLE_MASS 1
#define PARTICLE_RADIUS 0.25f

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




#define FS_PI 3.1415f
#define SPEED_OF_SOUND 1.0f
#define VISCOSITY_COEFFICIENT 0.001f
#define TARGET_DENSITY ( g_targetDensity )
#define PRESSURE_MULTIPLIER 0.25f
#define PRESSURE_GAMMA 1.0f

#define NEGATIVE_PRESSURE_SCALE 1.0f
#define VISCOSITY_STRENGTH 0.185f
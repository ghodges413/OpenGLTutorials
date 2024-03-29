//
//	HashGrid.cpp
//
#include "Graphics/ShaderStorageBuffer.h"
#include "Graphics/Graphics.h"
#include "Graphics/ShaderManager.h"
#include "Fluids/HashGrid.h"
#include "Fluids/FluidSim.h"
#include "Math/Bounds.h"
#include <vector>
#include <array>
#include <stdio.h>
#include <string>

#if 0
#define SPEED_OF_SOUND 1.0f
#define VISCOSITY_COEFFICIENT 0.001f
#define TARGET_DENSITY ( g_targetDensity * 0.01f )
#define PRESSURE_MULTIPLIER 1.0f
#define PRESSURE_GAMMA 1.0f

#define NEGATIVE_PRESSURE_SCALE 0.0f
#define VISCOSITY_STRENGTH 0.185f
#else
#define SPEED_OF_SOUND 0.5f//100.0f
#define VISCOSITY_COEFFICIENT 0.001f
#define TARGET_DENSITY ( g_targetDensity * 0.1f )
#define PRESSURE_MULTIPLIER 0.25f
#define PRESSURE_GAMMA 1.0f

#define NEGATIVE_PRESSURE_SCALE 1.0f
#define VISCOSITY_STRENGTH 0.185f
#endif

#define RESTITUTION 0.85f

#define RESOLUTION_X 20
#define RESOLUTION_Y 40
#define RESOLUTION_Z 40

#define NUM_CELLS ( RESOLUTION_X * RESOLUTION_Y * RESOLUTION_Z )

struct vec3i {
	int x;
	int y;
	int z;
};

struct elem_t {
	int idx;	// particle index
	int key;	// cell key
};

ShaderStorageBuffer g_ssboFluidParticles;

ShaderStorageBuffer g_ssboSorted;
ShaderStorageBuffer g_ssboStartIndices;

/*
==========================
InitGPUHashGrid
==========================
*/
void InitGPUHashGrid() {
	if ( !g_ssboSorted.IsValid() ) {
		g_ssboSorted.Generate( sizeof( elem_t ), MAX_PARTICLES );
	}

	if ( !g_ssboStartIndices.IsValid() ) {
		g_ssboStartIndices.Generate( sizeof( int ), NUM_CELLS );
	}
}

/*
==========================
NextPowerOfTwo
==========================
*/
int NextPowerOfTwo( int val ) {
	unsigned int v = (unsigned int)val; // compute the next highest power of 2 of 32-bit v

	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

/*
==========================
BitonicMergeSortGPU

This function has been adapted from Sebastian Langue's "Fluid Simulation"

// From Wikipedia, a power of 2 bitonic sort:
// given an array of length n, this code sorts it in place
// all indices run from 0 to n-1
// for (k = 2; k <= n; k *= 2) // k is doubled every iteration
//     for (j = k/2; j > 0; j /= 2) // j is halved at every iteration, with truncation of fractional parts
//         for (i = 0; i < n; i++)
//             l = bitwiseXOR (i, j); // in C-like languages this is "i ^ j"
//             if (l > i)
//                 if (  (bitwiseAND (i, k) == 0) AND (arr[i] > arr[l])
//                     OR (bitwiseAND (i, k) != 0) AND (arr[i] < arr[l]) )
//                         swap the elements arr[i] and arr[l]
==========================
*/
void BitonicMergeSortGPU() {
	// 2. Sort the keys in the sorted array
	Shader * shader = g_shaderManager->GetAndUseShader( "FluidSim/SortKeys" );

	const int maxParticles = MAX_PARTICLES;

	const int numPairs = NextPowerOfTwo( MAX_PARTICLES ) / 2;
	const int numStages = (int)( logf( numPairs * 2 ) / logf( 2 ) );

	for ( int stageIndex = 0; stageIndex < numStages; stageIndex++ ) {
		for ( int stepIndex = 0; stepIndex < stageIndex + 1; stepIndex++ ) {
			const int groupWidth = 1 << ( stageIndex - stepIndex );
			const int groupHeight = 2 * groupWidth - 1;

			shader->SetUniform1i( "groupWidth", 1, &groupWidth );
			shader->SetUniform1i( "groupHeight", 1, &groupHeight );
			shader->SetUniform1i( "stepIndex", 1, &stepIndex );

			shader->SetUniform1i( "MAX_PARTICLES", 1, &maxParticles );

			shader->DispatchCompute( ( numPairs + 127 ) / 128, 1, 1 );

			// Memory barriers are used to block programs from using
			// a buffer that is being written to from a previous program
			glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );
		}
	}
}

/*
==========================
UpdateHashGridGPU
==========================
*/
void UpdateHashGridGPU() {
	Shader * shader = NULL;

	g_ssboFluidParticles.Bind( 0 );
	g_ssboSorted.Bind( 1 );
	g_ssboStartIndices.Bind( 2 );

	// 1. Update GPU Hash Cells
	// This is going to insert the gpu particles into the cells and build the hash keys
	shader = g_shaderManager->GetAndUseShader( "FluidSim/PredictAndUpdateCells" );
	{
		const int maxParticles = MAX_PARTICLES;
		const float particleRadius = PARTICLE_RADIUS;

		shader->SetUniform1i( "MAX_PARTICLES", 1, &maxParticles );
		shader->SetUniform1f( "PARTICLE_RADIUS", 1, &particleRadius );

		const int resolutionX = RESOLUTION_X;
		const int resolutionY = RESOLUTION_Y;
		const int resolutionZ = RESOLUTION_Z;

		shader->SetUniform1i( "RESOLUTION_X", 1, &resolutionX );
		shader->SetUniform1i( "RESOLUTION_Y", 1, &resolutionY );
		shader->SetUniform1i( "RESOLUTION_Z", 1, &resolutionZ );

		shader->DispatchCompute( ( maxParticles + 127 ) / 128, 1, 1 );
		glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );
	}

	// 2. Sort the keys in the sorted array
	BitonicMergeSortGPU();

	// 3. Reset the start indices array
	shader = g_shaderManager->GetAndUseShader( "FluidSim/ResetStartIndices" );
	{
		const int maxParticles = MAX_PARTICLES;
		const int numCells = NUM_CELLS;

		shader->SetUniform1i( "MAX_PARTICLES", 1, &maxParticles );
		shader->SetUniform1i( "NUM_CELLS", 1, &numCells );

		shader->DispatchCompute( ( numCells + 127 ) / 128, 1, 1 );
		glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );
	}

	// 4. Build the start indices array
	shader = g_shaderManager->GetAndUseShader( "FluidSim/BuildStartIndices" );
	{
		const int maxParticles = MAX_PARTICLES;
		const int numCells = NUM_CELLS;

		shader->SetUniform1i( "MAX_PARTICLES", 1, &maxParticles );
		shader->SetUniform1i( "NUM_CELLS", 1, &numCells );

		shader->DispatchCompute( ( maxParticles + 127 ) / 128, 1, 1 );
		glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );
	}
}

/*
==========================
FluidSimGPU
==========================
*/
void FluidSimGPU() {
	// 1. Update GPU Hash Cells
	// This is going to insert the gpu particles into the cells and build the hash keys

	// 2. Sort the keys in the sorted array

	// 3. Build the start indices array
	UpdateHashGridGPU();
	
	// 5. Calculate the fluid density
	Shader * shader = g_shaderManager->GetAndUseShader( "FluidSim/FluidSimDensity" );
	{
		const int maxParticles = MAX_PARTICLES;
		const int numCells = NUM_CELLS;
		const float radius = PARTICLE_RADIUS;

		shader->SetUniform1i( "MAX_PARTICLES", 1, &maxParticles );
		shader->SetUniform1i( "NUM_CELLS", 1, &numCells );
		shader->SetUniform1f( "PARTICLE_RADIUS", 1, &radius );

		extern float g_targetDensity;
		const float targetDensity = TARGET_DENSITY;
		const float speedOfSound = SPEED_OF_SOUND;
		const float pressureGamma = PRESSURE_GAMMA;

		shader->SetUniform1f( "TARGET_DENSITY", 1, &targetDensity );
		shader->SetUniform1f( "SPEED_OF_SOUND", 1, &speedOfSound );
		shader->SetUniform1f( "PRESSURE_GAMMA", 1, &pressureGamma );

		const int resolutionX = RESOLUTION_X;
		const int resolutionY = RESOLUTION_Y;
		const int resolutionZ = RESOLUTION_Z;

		shader->SetUniform1i( "RESOLUTION_X", 1, &resolutionX );
		shader->SetUniform1i( "RESOLUTION_Y", 1, &resolutionY );
		shader->SetUniform1i( "RESOLUTION_Z", 1, &resolutionZ );

		shader->DispatchCompute( ( maxParticles + 127 ) / 128, 1, 1 );
		glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );
	}

	// 6. Calculate pressure and viscosity and step the simulation
	shader = g_shaderManager->GetAndUseShader( "FluidSim/FluidSimSPH" );
	{
		const int maxParticles = MAX_PARTICLES;
		const int numCells = NUM_CELLS;
		const float radius = PARTICLE_RADIUS;

		extern float g_targetDensity;
		const float targetDensity = g_targetDensity;

		const float pressureMultiplier = PRESSURE_MULTIPLIER;
		const float negativePressureScale = NEGATIVE_PRESSURE_SCALE;
		//const float viscosityStrength = VISCOSITY_STRENGTH;
		const float viscosityStrength = VISCOSITY_COEFFICIENT;

		extern Bounds g_particleBounds;
		const Vec3 boundsMin = g_particleBounds.mins;
		const Vec3 boundsMax = g_particleBounds.maxs;

		const float restitution = RESTITUTION;

		shader->SetUniform1i( "MAX_PARTICLES", 1, &maxParticles );
		shader->SetUniform1i( "NUM_CELLS", 1, &numCells );
		shader->SetUniform1f( "PARTICLE_RADIUS", 1, &radius );

		shader->SetUniform1f( "TARGET_DENSITY", 1, &targetDensity );
		shader->SetUniform1f( "PRESSURE_MULTIPLIER", 1, &pressureMultiplier );
		shader->SetUniform1f( "NEGATIVE_PRESSURE_SCALE", 1, &negativePressureScale );
		shader->SetUniform1f( "VISCOSITY_STRENGTH", 1, &viscosityStrength );

		shader->SetUniform1f( "restitution", 1, &restitution );
		shader->SetUniform3f( "boundsMin", 1, boundsMin.ToPtr() );
		shader->SetUniform3f( "boundsMax", 1, boundsMax.ToPtr() );

		const int resolutionX = RESOLUTION_X;
		const int resolutionY = RESOLUTION_Y;
		const int resolutionZ = RESOLUTION_Z;

		shader->SetUniform1i( "RESOLUTION_X", 1, &resolutionX );
		shader->SetUniform1i( "RESOLUTION_Y", 1, &resolutionY );
		shader->SetUniform1i( "RESOLUTION_Z", 1, &resolutionZ );

		shader->DispatchCompute( ( maxParticles + 127 ) / 128, 1, 1 );
		glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );

		// Map the particles back to the cpu for drawing
		g_ssboFluidParticles.Bind();
		fluid_t * particles = (fluid_t *)g_ssboFluidParticles.MapBuffer( GL_MAP_READ_BIT );
		extern fluid_t g_particles[ MAX_PARTICLES ];
		for ( int i = 0; i < MAX_PARTICLES; i++ ) {
			g_particles[ i ] = particles[ i ];
		}
		g_ssboFluidParticles.UnMapBuffer();
	}
}
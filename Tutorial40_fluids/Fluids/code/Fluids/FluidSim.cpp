//
//	FluidSim.cpp
//
#include "Graphics/ShaderStorageBuffer.h"
#include "Graphics/Graphics.h"
#include "Fluids/FluidSim.h"
#include "Fluids/HashGrid.h"
#include "Math/Bounds.h"
#include "Math/Random.h"
#include <stdio.h>
#include <string>
#include <math.h>

float g_targetDensity = 1;


fluid_t g_particles[ MAX_PARTICLES ];

Bounds g_particleBounds;

ShaderStorageBuffer g_ssboFluidParticles;

typedef float kernel_ftor( Vec3 pos, Vec3 a );

/*
====================================
ResetFluidSim
====================================
*/
void ResetFluidSim() {
	g_particleBounds.Clear();

	memset( g_particles, 0, sizeof( fluid_t ) * MAX_PARTICLES );

	int idx = 0;
	for ( int z = 0; z < SIDE_SIZE; z++ ) {
		for ( int y = 0; y < SIDE_SIZE; y++ ) {
			for ( int x = 0; x < SIDE_SIZE; x++ ) 
			{
				//int x = 0;
				fluid_t & particle = g_particles[ idx ];
				particle.pos = Vec3( x, y - SIDE_SIZE / 2, z ) * 0.125f;
				particle.pos += Vec3( 0, 0, 1 );
				particle.posOld = particle.pos;

				g_particleBounds.Expand( particle.pos );
				++idx;
			}
		}
	}

	g_particleBounds.Expand( g_particleBounds.mins + Vec3( -0.1, -3, -3 ) );
	g_particleBounds.Expand( g_particleBounds.maxs + Vec3( 0.1, 3, 3 ) );

	float dy = g_particleBounds.maxs.y - g_particleBounds.mins.y;
	float dz = g_particleBounds.maxs.z - g_particleBounds.mins.z;
	float area = dy * dz;
	printf( "Fluid area: %f\n", area );

	g_targetDensity = MAX_PARTICLES / area;

	BuildGrid( g_particles, MAX_PARTICLES );


	// Initialize the ssbo for gpu particles
	if ( !g_ssboFluidParticles.IsValid() ) {
		g_ssboFluidParticles.Generate( sizeof( fluid_t ), MAX_PARTICLES );
	}

	// Copy the particles to the gpu
	g_ssboFluidParticles.Bind();
	fluid_t * mapBuffer = (fluid_t *)g_ssboFluidParticles.MapBuffer( GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT );
	for ( int i = 0; i < MAX_PARTICLES; i++ ) {
		mapBuffer[ i ] = g_particles[ i ];
	}
	g_ssboFluidParticles.UnMapBuffer();

	InitGPUHashGrid();
}

/*
====================================
Kernel
This Kernel was proposed by Muller in "Particle based fluid simulation for interactive applications" From 2003 ACM Siggraph

Smoothed Particle Hydrodynamics (originally used in astrophysics simulations)
	First introduced by J.J. Monaghan "Smoothed particle hydrodynamics"

This Kernel is the following equation:
if r <= h:
W(r) = 315 * ( 1 - r^2 / h^2 )^3 / ( 64 * pi * h^3 )
if r > h:
W(r) = 0

Here 'pos' is the position of the fluid particle, and 'a' is the sampling point
====================================
*/
float Kernel_STD( Vec3 pos, Vec3 a ) {
	const float h = PARTICLE_RADIUS;

	float r = ( a - pos ).GetMagnitude();
	if ( r >= h ) {
		return 0.0f;
	}

	float r2 = r * r;
	float h2 = h * h;
	float h3 = h * h * h;

	float constant = 315.0f / ( 64.0f * FS_PI * h3 );
	float numerator = 1.0f - r2 / h2;
	float num3 = numerator * numerator * numerator;

	float W = constant * num3;
	return W;
}

float Kernel_STD_DerivativeFirst( Vec3 pos, Vec3 a ) {
	const float h = PARTICLE_RADIUS;

	float r = ( a - pos ).GetMagnitude();
	if ( r >= h ) {
		return 0.0f;
	}

	float r2 = r * r;
	float h2 = h * h;
	float h5 = h * h * h * h * h;

	float constant = -945.0f / ( 32.0f * FS_PI * h5 );
	float numerator = 1.0f - r2 / h2;
	float num2 = numerator * numerator;

	float Wprime = constant * r * num2;
	return Wprime;
}

Vec3 Kernel_STD_Gradient( Vec3 pos, Vec3 a ) {
	float firstDerivative = Kernel_STD_DerivativeFirst( pos, a );
	
	Vec3 dirToCenter = pos - a;
	dirToCenter.Normalize();

	Vec3 grad = dirToCenter * (-firstDerivative);
	return grad;
}

/*
====================================
Kernel
The standard SPH kernel has an oscillatory first and second derivative.
This is problematic when calculating pressure.  And makes it possible for the
calculated pressure to be near zero, even though the particles are on top of each other.

To eliminate the oscillation, the spiky kernel was proposed by Mueller.

This Kernel is the following equation:
if r <= h:
W(r) = 15 * ( 1 - r / h )^3 / ( pi * h^3 )
if r > h:
W(r) = 0

Here 'pos' is the position of the fluid particle, and 'a' is the sampling point
====================================
*/
float Kernel_Spiky( float r ) {
	const float h = PARTICLE_RADIUS;

	//float r = ( a - pos ).GetMagnitude();
	if ( r >= h ) {
		return 0.0f;
	}

	float h3 = h * h * h;

	float constant = 15.0f / ( FS_PI * h3 );
	float numerator = 1.0f - r / h;
	float num3 = numerator * numerator * numerator;

	float W = constant * num3;
	return W;
}

float Kernel_Spiky_1stDerivative( float dist ) {
	const float h = PARTICLE_RADIUS;

	if ( dist >= h ) {
		return 0;
	}

	float x = 1.0f - dist / h;
	float derivative = ( -45.0f / ( FS_PI * h * h * h * h ) ) * x * x;
	return derivative;
}

Vec3 Kernel_Spiky_Gradient( float dist, Vec3 dirToCenter ) {
	return dirToCenter * ( -Kernel_Spiky_1stDerivative( dist ) );
}

float Kernel_Spiky_2ndDerivative( float dist ) {
	const float h = PARTICLE_RADIUS;

	if ( dist >= h ) {
		return 0;
	}

	float x = 1.0f - dist / h;
	return ( 90.0f / ( FS_PI * h * h * h * h * h ) ) * x;
}

// ( r^2 - x^2 )^3
float Kernel_SL_smooth( float x ) {
	const float s = PARTICLE_RADIUS;
	if ( x >= s ) {
		return 0;
	}

	// spikey
	float volume = ( FS_PI * powf( s, 4 ) ) / 6.0f;
	float kernel = ( s - x ) * ( s - x ) / volume;
	return kernel;
}

float Kernel_SL_1stDerivative( float x ) {
	const float s = PARTICLE_RADIUS;
	if ( x >= s ) {
		return 0;
	}

	// spikey
	float scale = 12.0f / ( powf( s, 4 ) * FS_PI );
	float deriv = ( s - x ) * scale;
	return deriv;
}

std::vector< int > g_neighborIds;

float Density_SL( Vec3 pt, int skip ) {
	const float mass = 1;
	float density = 0;

	g_neighborIds.clear();
	GetNeighborIds_Cells( g_neighborIds, skip );

	for ( int i = 0; i < g_neighborIds.size(); i++ ) {
		const int idx = g_neighborIds[ i ];
		if ( idx == skip ) {
			continue;
		}
		const fluid_t & particle = g_particles[ idx ];
		float dist = ( pt - particle.pos ).GetMagnitude();
		float influence = Kernel_SL_smooth( dist );
		density += mass * influence;
	}

	return density;
}

float DensityToPressure_SL( float density ) {
	// Apparently this is a common pressure function for approximating gases
	float densityError = density - TARGET_DENSITY;
	float pressure = densityError * PRESSURE_MULTIPLIER;
	if ( pressure < 0 ) {
		pressure *= NEGATIVE_PRESSURE_SCALE;
	}
	return pressure;
}

float DensityToPressureShared_SL( float density0, float density1 ) {
	float pressure0 = DensityToPressure_SL( density0 );
	float pressure1 = DensityToPressure_SL( density1 );
	return ( pressure0 + pressure1 ) * 0.5f;
}

Vec3 Kernel_SL_Gradient( Vec3 pt ) {
	const float mass = 1;
	Vec3 grad = Vec3( 0, 0, 0 );

	g_neighborIds.clear();
	GetNeighborIds( g_neighborIds, pt );

	for ( int i = 0; i < g_neighborIds.size(); i++ ) {
		const int idx = g_neighborIds[ i ];

		const fluid_t & particle = g_particles[ idx ];
		float dist = ( pt - particle.pos ).GetMagnitude();
		Vec3 dir = ( pt - particle.pos ) / dist;
		if ( dist < 0.00001f ) {
			// if the two points are the same, then just choose a random direction
			dir = Random::RandomOnSphereSurface();
		}
		float slope = Kernel_SL_1stDerivative( dist );
		float density = Density_SL( pt, -1 );
		grad -= dir * slope * mass / density;
	}

	return grad;
}

Vec3 PressureForce_SL( Vec3 pt, int skip ) {
	const float mass = 1;
	Vec3 grad = Vec3( 0, 0, 0 );

	g_neighborIds.clear();
	GetNeighborIds_Cells( g_neighborIds, skip );

	for ( int i = 0; i < g_neighborIds.size(); i++ ) {
		const int idx = g_neighborIds[ i ];
		if ( idx == skip ) {
			continue;
		}

		const fluid_t & particle = g_particles[ idx ];
		float dist = ( pt - particle.pos ).GetMagnitude();
		Vec3 dir( 0, 0, 0 );
		if ( dist > 0.00001f ) {
			dir = ( pt - particle.pos ) / dist;
		}
		float slope = Kernel_SL_1stDerivative( dist );
		float density = particle.density;
		float pressure = DensityToPressureShared_SL( density, g_particles[ skip ].density );
		if ( fabsf( density ) > 0.00001f ) {
			grad += dir * pressure * slope * mass / density;
		}
	}

	return grad;
}

Vec3 ViscosityForce_SL( int skip ) {
	Vec3 viscosityForce = Vec3( 0, 0, 0 );
	Vec3 pos = g_particles[ skip ].pos;
	Vec3 vel = g_particles[ skip ].vel;

	g_neighborIds.clear();
	GetNeighborIds_Cells( g_neighborIds, skip );

	for ( int i = 0; i < g_neighborIds.size(); i++ ) {
		const int idx = g_neighborIds[ i ];
		if ( idx == skip ) {
			continue;
		}

		float dist = ( pos - g_particles[ idx ].pos ).GetMagnitude();
		float influence = Kernel_SL_smooth( dist );
		viscosityForce += ( g_particles[ idx ].vel - vel ) * influence;
	}

	return viscosityForce * VISCOSITY_STRENGTH;
}

/*
====================================
Density

Calculates the density of the sample position, within the neighborhood of particles
====================================
*/
float Density( const int i ) {
	float sum = 0;

	g_neighborIds.clear();
	GetNeighborIds_Cells( g_neighborIds, i );

	for ( int j = 0; j < g_neighborIds.size(); j++ ) {
		const int idx = g_neighborIds[ j ];
		if ( idx == i ) {
			continue;
		}

		Vec3 xi = g_particles[ i ].pos;
		Vec3 xj = g_particles[ idx ].pos;
		float dist = ( xi - xj ).GetMagnitude();
		float weight = PARTICLE_MASS * Kernel_Spiky( dist );
		sum += weight;
	}

	return sum;
}


/*
====================================
PressureEOS
====================================
*/
float PressureEOS( float density, float targetDensity, float eosScale, float eosExponent, float negativePressureScale ) {
	float p = eosScale / eosExponent * ( powf( ( density / targetDensity ), eosExponent ) - 1.0f );
	
	if ( p < 0 ) {
		p *= negativePressureScale;
	}

	return p;
}

/*
====================================
Pressure
====================================
*/
float Pressure( const int i ) {
	float p = 0;

	float di = g_particles[ i ].density;

	float targetDensity = TARGET_DENSITY;
	float eosScale = targetDensity * SPEED_OF_SOUND * SPEED_OF_SOUND / PRESSURE_GAMMA;

	p = PressureEOS( di, targetDensity, eosScale, PRESSURE_GAMMA, 0.0f );
	return p;
}

/*
====================================
PressureForce
====================================
*/
Vec3 PressureForce( const int i ) {
	Vec3 f = Vec3( 0, 0, 0 );

	const Vec3 xi = g_particles[ i ].pos;
	const float pi = g_particles[ i ].pressure;
	const float di = g_particles[ i ].density;

	g_neighborIds.clear();
	GetNeighborIds_Cells( g_neighborIds, i );

	for ( int j = 0; j < g_neighborIds.size(); j++ ) {
		const int idx = g_neighborIds[ j ];
		if ( idx == i ) {
			continue;
		}

		Vec3 xj = g_particles[ idx ].pos;
		Vec3 dx = xj - xi;
		float dist = dx.GetMagnitude();

		Vec3 dir = ( xj - xi ) / dist;
		float pj = g_particles[ idx ].pressure;
		float dj = g_particles[ idx ].density;

		Vec3 grad = Kernel_Spiky_Gradient( dist, dir );
		if ( di > 0.001f && dj > 0.001f && grad.GetLengthSqr() > 0.001f ) {
			f -= grad * PARTICLE_MASS * PARTICLE_MASS * ( pi / ( di * di ) + pj / ( dj * dj ) );
		}
	}

	return f;
}

/*
====================================
ViscosityForce
====================================
*/
Vec3 ViscosityForce( const int i ) {
	Vec3 f = Vec3( 0, 0, 0 );

	Vec3 xi = g_particles[ i ].pos;
	Vec3 vi = g_particles[ i ].vel;

	g_neighborIds.clear();
	GetNeighborIds_Cells( g_neighborIds, i );

	for ( int j = 0; j < g_neighborIds.size(); j++ ) {
		const int idx = g_neighborIds[ j ];
		if ( idx == i ) {
			continue;
		}

		Vec3 xj = g_particles[ idx ].pos;
		Vec3 dx = xj - xi;
		float dist = dx.GetMagnitude();

		Vec3 vj = g_particles[ idx ].vel;
		float dj = g_particles[ idx ].density;
		if ( dj > 0.001f ) {
			f += ( ( vj - vi ) / dj ) * VISCOSITY_COEFFICIENT * PARTICLE_MASS * PARTICLE_MASS * Kernel_Spiky_2ndDerivative( dist );
		}
	}

	return f;
}

extern void FluidSimGPU();

void FluidSimStep( float dt ) {
	dt = 1.0f / 60.0f;// 0.1667f;	// Fix the time-step to 60fps

	FluidSimGPU();
	return;

	// Prediction + Gravity
	for ( int i = 0; i < MAX_PARTICLES; i++ ) {
		fluid_t & pt = g_particles[ i ];
		pt.posOld = pt.pos;
		pt.vel.z += -0.10f * dt;
		//pt.vel.z += -1.10f * dt;
		pt.pos = pt.pos + pt.vel * dt;
	}

#if 1
	//UpdateGrid( g_particles, MAX_PARTICLES );
	UpdateCells( g_particles );

	// Calculate densities
	for ( int i = 0; i < MAX_PARTICLES; i++ ) {
		Vec3 pos = g_particles[ i ].pos;
		g_particles[ i ].density = Density_SL( pos, i );
	}

	// Integrate
	for ( int i = 0; i < MAX_PARTICLES; i++ ) {
		// Calculate pressure force
		Vec3 pressureForce = PressureForce_SL( g_particles[ i ].pos, i );

		// Calculate viscosity force
		Vec3 viscosityForce = ViscosityForce_SL( i );

		Vec3 force = pressureForce + viscosityForce;
		Vec3 a = force / g_particles[ i ].density;
		if ( !a.IsValid() ) {
			a.Zero();
		}

		g_particles[ i ].vel += a * dt;
		//g_particles[ i ].vel = a * dt;
		g_particles[ i ].vel.x = 0;
		g_particles[ i ].pos = g_particles[ i ].posOld + g_particles[ i ].vel * dt;
	}
#else
	UpdateCells( g_particles );

	// Calculate densities
	for ( int i = 0; i < MAX_PARTICLES; i++ ) {
		g_particles[ i ].density = Density( i );
	}

	// Calculate pressure
	for ( int i = 0; i < MAX_PARTICLES; i++ ) {
		g_particles[ i ].pressure = Pressure( i );
	}

	// Integrate
	for ( int i = 0; i < MAX_PARTICLES; i++ ) {
		// Calculate pressure force
		Vec3 pressureForce = PressureForce( i );

		// Calculate viscosity force
		Vec3 viscosityForce = ViscosityForce( i );

		Vec3 force = viscosityForce + pressureForce;
		Vec3 a = force / PARTICLE_MASS;

		g_particles[ i ].vel += a * dt;
		//g_particles[ i ].vel.x = 0;
		g_particles[ i ].pos = g_particles[ i ].posOld + g_particles[ i ].vel * dt;
	}
#endif

	// Bounds check
	for ( int i = 0; i < MAX_PARTICLES; i++ ) {
		// Do a bounds check for the particle
		if ( g_particleBounds.DoesIntersect( g_particles[ i ].pos ) ) {
			continue;
		}

		Vec3 & pos = g_particles[ i ].pos;
		Vec3 & vel = g_particles[ i ].vel;

		// Handle collisions
		const float restitution = 0.059f;
		for ( int j = 0; j < 3; j++ ) {
			if ( pos[ j ] < g_particleBounds.mins[ j ] ) {
				pos[ j ] = g_particleBounds.mins[ j ];
				vel[ j ] = -restitution * vel[ j ];
			}

			if ( pos[ j ] > g_particleBounds.maxs[ j ] ) {
				pos[ j ] = g_particleBounds.maxs[ j ];
				vel[ j ] = -restitution * vel[ j ];
			}
		}
	}
}
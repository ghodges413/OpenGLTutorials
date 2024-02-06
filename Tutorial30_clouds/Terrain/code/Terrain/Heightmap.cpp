//
//  Heightmap.cpp
//
#include "Terrain/Heightmap.h"
#include "Terrain/Terrain.h"
#include "Terrain/TerrainFile.h"
#include "Graphics/Mesh.h"
#include <math.h>
#include <algorithm>
#include <stdlib.h>
#include <assert.h>

/*
================================================================
 
Heightmap

This height map is generated using Fractal Brownian Motion.
https://thebookofshaders.com/13/
https://iquilezles.org/articles/morenoise/
https://iquilezles.org/articles/warp/

This height map is adopted from shader toy code
"Terrain Erosion Noise" by Fewes
https://www.shadertoy.com/view/7ljcRW
 
================================================================
*/


float clampf( float v, float min, float max ) {
    if ( v < min ) {
        v = min;
    }
    if ( v > max ) {
        v = max;
    }
    return v;
}

//smoothstep — perform Hermite interpolation between two values
float smoothstep( float a, float b, float x ) {
    //genType t;  /* Or genDType t; */
    float t = clampf( ( x - a ) / ( b - a ), 0.0f, 1.0f );
    return t * t * ( 3.0f - 2.0f * t );
}

float dot( Vec2d a, Vec2d b ) {
    return a.Dot( b );
}

float dot( Vec3d a, Vec3d b ) {
    return a.DotProduct( b );
}

float frac( float value ) {
    float poo;
    float f = modff( value, &poo );
    return f;
}

Vec2d floor( Vec2d v ) {
    Vec2d f;
    f.x = floorf( v.x );
    f.y = floorf( v.y );
    return f;
}

Vec2d fract( Vec2d v ) {
    Vec2d f;
    f.x = frac( v.x );
    f.y = frac( v.y );
    return f;
}

/*
==========================================================================================

This shader is a fork of clayjohn's awesome "Eroded Terrain Noise":
https://www.shadertoy.com/view/MtGcWh

The original function seemed to have some biasing problems which I've attempted to fix.
I also wanted to improve the visualization to really show how great the result is.
I take no credit for the actual noise/math. I simply wanted to help showcase it.

==========================================================================================
*/

/*
==========================================================================================

Erosion parameters

==========================================================================================
*/

#define EROSION_TILES 4.0
#define EROSION_OCTAVES 5
#define EROSION_GAIN 0.5
#define EROSION_LACUNARITY 2.0

// Scale the input slope, leading to more erosion.
#define EROSION_SLOPE_STRENGTH 3.0
// Continuously modify the noise direction based on the previous fractal sample.
// This is what gives the slopes an interesting "branching" structure.
// A higher value will give you more branches.
#define EROSION_BRANCH_STRENGTH 3.0
// Maximum amount the erosion will modify the base height map
#define EROSION_STRENGTH 0.04

// Debug slider comparing the heightmap with and without erosion
#define COMPARISON_SLIDER

/*
==========================================================================================

The stuff below is not strictly related to the erosion effect

==========================================================================================
*/

#define PI 3.14159265358979f

// Base height noise parameters
#define HEIGHT_TILES 3.0f
#define HEIGHT_OCTAVES 3
#define HEIGHT_AMP 0.25f
#define HEIGHT_GAIN 0.1f
#define HEIGHT_LACUNARITY 2.0f

#define WATER_HEIGHT 0.45f

/*
================================
hash
================================
*/
Vec2d hash( Vec2d x ) {
    const Vec2d k = Vec2d( 0.3183099f, 0.3678794f );
    x.x = x.x * k.x + k.y;
    x.y = x.y * k.y + k.x;

    Vec2d k2 = k * 16.0f * fract( x.x * x.y * ( x.x + x.y ) );
    Vec2d k3 = Vec2d( -1.0f, -1.0f ) + fract( k2 ) * 2.0f;
    return k3;
}

/*
================================
noised
// from https://www.shadertoy.com/view/XdXBRH
================================
*/
Vec3d noised( Vec2d p ) {
    Vec2d i = floor( p );
    Vec2d f = fract( p );

    Vec2d f2;
    f2.x = f.x * ( f.x * 6.0f - 15.0f ) + 10.0f;
    f2.y = f.y * ( f.y * 6.0f - 15.0f ) + 10.0f;

    Vec2d f3;
    f3.x = f.x * ( f.x - 2.0f ) + 1.0f;
    f3.y = f.y * ( f.y - 2.0f ) + 1.0f;

    Vec2d u = f * f * f * f2;
    Vec2d du = f * f * f3 * 30.0f;
    
    Vec2d ga = hash( i + Vec2d( 0.0f, 0.0f ) );
    Vec2d gb = hash( i + Vec2d( 1.0f, 0.0f ) );
    Vec2d gc = hash( i + Vec2d( 0.0f, 1.0f ) );
    Vec2d gd = hash( i + Vec2d( 1.0f, 1.0f ) );
    
    float va = dot( ga, f - Vec2d( 0.0f, 0.0f ) );
    float vb = dot( gb, f - Vec2d( 1.0f, 0.0f ) );
    float vc = dot( gc, f - Vec2d( 0.0f, 1.0f ) );
    float vd = dot( gd, f - Vec2d( 1.0f, 1.0f ) );

    float x = va + u.x * ( vb - va ) + u.y * ( vc - va ) + u.x * u.y * ( va - vb - vc + vd );
    Vec2d yz = ga + u.x * ( gb - ga ) + u.y * ( gc - ga ) + u.x * u.y * ( ga - gb - gc + gd ) + du * ( Vec2d( u.y, u.x ) * ( va - vb - vc + vd ) + Vec2d( vb, vc ) - va );
    return Vec3d( x, yz.x, yz.y );
}


/*
==========================================================================================

Buffer A generates the heightmap (X), normals (YZ) and erosion mask used for coloring (W)

==========================================================================================
*/

// code adapted from https://www.shadertoy.com/view/llsGWl
// name: Gavoronoise
// author: guil
// license: Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License
// Code has been modified to return analytic derivatives and to favour 
// direction quite a bit.

/*
================================
HeightSample
================================
*/
Vec3d Erosion( Vec2d p, Vec2d dir ) {
    Vec2d ip = floor( p );
    Vec2d fp = fract( p );
    float f = 2.0f * PI;
    Vec3d va = Vec3d( 0.0f, 0.0f, 0.0f );
    float wt = 0.0f;
    for ( int i = -2; i <= 1; i++ ) {
        for ( int j = -2; j <=1 ; j++ ) {
            Vec2d o = Vec2d( i, j );
            Vec2d h = hash( ip - o ) * 0.5f;
            Vec2d pp = fp + o - h;
            float d = dot( pp, pp );
            float w = exp( -d * 2.0f );
            wt +=w;
            float mag = dot( pp, dir );
            Vec2d yz = ( pp * 0.0f + dir ) * ( -sinf( mag * f ) );
            va += Vec3d( cosf( mag * f ), yz.x, yz.y ) * w;
        }
    }
    return va / wt;
}

/*
================================
HeightSample
================================
*/
Vec2d Heightmap( Vec2d uv ) {
    Vec2d p = uv * HEIGHT_TILES;
    
    // FBM terrain
    Vec3d n = Vec3d( 0.0f, 0.0f, 0.0f );
    float nf = 1.0;
    float na = HEIGHT_AMP;
    for ( int i = 0; i < HEIGHT_OCTAVES; i++ ) {
        n += noised( p * nf ) * na * Vec3d( 1.0, nf, nf );
        na *= HEIGHT_GAIN;
        nf *= HEIGHT_LACUNARITY;
    }
    
    // [-1, 1] -> [0, 1]
    n.x = n.x * 0.5f + 0.5f;
    
    // Take the curl of the normal to get the gradient facing down the slope
    Vec2d dir = Vec2d( n.z, n.y ) * Vec2d( 1.0, -1.0 ) * EROSION_SLOPE_STRENGTH;
    
    // Now we compute another fbm type noise
    // erosion is a type of noise with a strong directionality
    // we pass in the direction based on the slope of the terrain
    // erosion also returns the slope. we add that to a running total
    // so that the direction of successive layers are based on the
    // past layers
    Vec3d h = Vec3d( 0.0f, 0.0f, 0.0f );
    
    float a = 0.5f;
    float f = 1.0f;
    
    a *= smoothstep( WATER_HEIGHT - 0.1, WATER_HEIGHT + 0.2, n.x );

    int octaves = EROSION_OCTAVES;
    for ( int i = 0; i < octaves; i++ ) {
        h += Erosion( p * EROSION_TILES * f, dir + Vec2d( h.z, h.y ) * Vec2d( 1.0f, -1.0f ) * EROSION_BRANCH_STRENGTH ) * a * Vec3d( 1.0f, f, f);
        a *= EROSION_GAIN;
        f *= EROSION_LACUNARITY;
    }
    
    return Vec2d( n.x + ( h.x - 0.5f ) * EROSION_STRENGTH, h.x );
}

/*
================================
HeightSample
================================
*/
Vec4d HeightSample( Vec2d coord, float width ) {
    Vec2d uv = coord;
    uv.x = coord.x / width;
    uv.y = coord.y / width;
    
    Vec2d h = Heightmap( uv );
    
    // Calculate the normal from neighboring samples
    Vec2d uv1 = uv + Vec2d( 1.0f, 0.0f ) * ( 1.0f / width );
    Vec2d uv2 = uv + Vec2d( 0.0f, 1.0f ) * ( 1.0f / width );
    Vec2d h1 = Heightmap( uv1 );
    Vec2d h2 = Heightmap( uv2 );
    Vec2d uva = uv1 - uv;
    Vec2d uvb = uv2 - uv;
    Vec3d v1 = Vec3d( uva.x, uva.y, ( h1.x - h.x ) );
    Vec3d v2 = Vec3d( uvb.x, uvb.y, ( h2.x - h.x ) );
    v1.Normalize();
    v2.Normalize();
    Vec3d normal = v1.Cross( v2 );
    normal = Vec3d( normal.x, normal.z, normal.y );

    return Vec4d( h.x, normal.x, normal.z, h.y );
}

/*
================================
CreateTerrainHeightmap
================================
*/
void CreateTerrainHeightmap( int width, int tileX, int tileY ) {
    Vec4d * heightmap = (Vec4d *)malloc( sizeof( Vec4d ) * width * width );
    assert( heightmap );
    if ( NULL == heightmap ) {
        return;
    }

    for ( int y = 0; y < width; y++ ) {
        for ( int x = 0; x < width; x++ ) {
            Vec2d coord;
            coord.x = x + tileX * ( width - 1 ); // The -1 is to prevent an "off by 1" error in the sampling,
            coord.y = y + tileY * ( width - 1 ); // otherwise tiles won't line up properly
            Vec4d color = HeightSample( coord, width );

            int idx = x + width * y;
            heightmap[ idx ] = color;
        }

        if ( 0 == ( y % 64 ) ) {
            printf( "Terra: %i of %i\n", y, width );
        }
    }

    WriteHeightmapFile( heightmap, width, tileX, tileY );
    free( heightmap );
}
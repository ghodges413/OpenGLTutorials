#version 440

/*
==========================================
input
==========================================
*/

in vec2 v_coords;
in vec3 v_ray;

/*
==========================================
output
==========================================
*/

layout( location = 0 ) out vec4 outColor;

/*
==========================================
uniforms
==========================================
*/

uniform float time;
uniform vec3 camPos;
uniform vec3 sunDir;

uniform sampler3D worleyNoise;

/*
==========================================
constants
==========================================
*/

#define CLOUD_ALTITUDE 800
#define BETA vec3( 0.7, 0.7, 0.9 )
#define PI 3.14159265359

/*
====================================================================================

Signed Distance Functions

https://iquilezles.org/www/articles/distfunctions/distfunctions.htm

====================================================================================
*/

float sdCeiling( vec3 p ) {
	return -p.z;
}
float sdFloor( vec3 p ) {
	return p.z;
}
float sdSmoothUnion( float d1, float d2, float k ) {
    float h = clamp( 0.5 + 0.5 * ( d2 - d1 ) / k, 0.0, 1.0 );
    return mix( d2, d1, h ) - k * h * ( 1.0 - h ); 
}
float sdSphere( vec3 p, float s ) {
    return length( p ) - s;
}
float sdSphere( vec3 p, vec3 origin, float s ) {
    p = p - origin;
    return sdSphere( p, s );
}
float sdSphereInverted( vec3 p, vec3 origin, float s ) {
    return -sdSphere( p, origin, s );
}
float sdCapsule( vec3 p, vec3 a, vec3 b, float r ) {
    vec3 pa = p - a;
    vec3 ba = b - a;
    float h = clamp( dot( pa, ba ) / dot( ba, ba ), 0.0, 1.0 );
    return length( pa - ba * h ) - r;
}
float sdPlane( vec3 p, vec3 n, float h ) {
    // n must be normalized
    return dot( p, n ) + h;
}

/*
==========================================
IntersectBox
https://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms
==========================================
*/
bool IntersectBox( vec3 rayOrigin, vec3 rayDir, vec3 mins, vec3 maxs, out float t0, out float t1 ) {
    mins = ( mins - rayOrigin ) / rayDir;
    maxs = ( maxs - rayOrigin ) / rayDir;
    vec3 a = min( mins, maxs );
    vec3 b = max( mins, maxs );
    t0 = max( max( a.x, a.y ), a.z );
    t1 = min( min( b.x, b.y ), b.z );
    return ( ( t1 > t0 ) && ( t1 > 0 ) );
}

/*
==========================
Remap
// From GPU Pro7 article on Horizon Zero Dawn clouds
// Remaps a value in the [a0,b0] range to the [a1,b1] range
==========================
*/
float Remap( float val, float a0, float b0, float a1, float b1 ) {
	float t = ( val - a0 ) / ( b0 - a0 );
	float v = a1 * ( 1.0 - t ) + b1 * t;
	return v;
}

/*
==========================================
Luminance
==========================================
*/
float Luminance( vec3 color ) {
    return ( color.r * 0.3 ) + ( color.g * 0.59 ) + ( color.b * 0.11 );
}

/*
==========================================
IsColorInsignificant
==========================================
*/
bool IsColorInsignificant( vec3 color ) {
    return Luminance( color ) < 0.01;
}

vec3 SkyColor( float z ) {
    return mix( vec3( 1, 1, 1 ), vec3( 0, 0, 1 ), abs( z ) );
}

float HenyeyGreenstein( float g, float costh ) {
    return ( 1.0 / ( 4.0 * PI ) ) * ( ( 1.0 - g * g ) / pow( 1.0 + g * g - 2.0 * g * costh, 1.5 ) );
}

float IsotropicPhaseFunction( float g, float costh ) {
    return 1.0 / ( 4.0 * PI );
}

float BeerPowder( float coeff, float depth ) {
    float powderSugarEffect = 1.0f - exp( -depth * 2.0f );
    float beersLaw = exp( -depth );
    float lightEnergy = 2.0f * beersLaw * powderSugarEffect;
    return lightEnergy;
}

float HenyeyGreenstein2( float g, float costh ) {
    return mix( HenyeyGreenstein( -g, costh ), HenyeyGreenstein( g, costh ), 0.7 );
}

float PhaseFunction( float g, float costh ) {
#if 0
    return IsotropicPhaseFunction( g, costh );
#else
    return HenyeyGreenstein2( g, costh );
#endif
}

vec3 SampleWorleyNoise( vec3 pos, float scale ) {
    vec3 coord = pos.xzy * vec3( 1.0 / 32.0, 1.0 / 32.0, 1.0 / 64.0 ) * scale;
    return texture( worleyNoise, coord ).rgb;
}

/*
==========================================
SampleCloudSDF
Low resolution cloud geometry
==========================================
*/
float SampleCloudSDF( vec3 p ) {
    float sdfSpheraA = sdSphere( p, vec3( -40, 0, -80 + CLOUD_ALTITUDE ), 60 );
    float sdfSpheraB = sdSphere( p, vec3( 40, 0, -80 + CLOUD_ALTITUDE ), 60 );
    float sdfSpheraC = sdSphere( p, vec3( -40, 0, 80 + CLOUD_ALTITUDE ), 60 );
    float sdfSpheraD = sdSphere( p, vec3( 40, 0, 80 + CLOUD_ALTITUDE ), 60 );
    float sdfValue = sdCapsule( p, vec3( 0, 0, -80 + CLOUD_ALTITUDE ), vec3( 0, 0, 80 + CLOUD_ALTITUDE ), 40 );
    sdfValue = min( sdfValue, sdfSpheraA );
    sdfValue = min( sdfValue, sdfSpheraB );
    sdfValue = min( sdfValue, sdfSpheraC );
    sdfValue = min( sdfValue, sdfSpheraD );
#if 0
    float z = CLOUD_ALTITUDE + fract( time * 0.05 ) * 200.0;
    float sdfSpheraI = sdSphere( p, vec3( 0, 0, 80 + z ), 25 );
    float sdfSpheraJ = sdSphere( p, vec3( 0, 0, 50 + z ), 25 );
    float sdfSpheraK = sdSphere( p, vec3( 0, 0, 20 + z ), 25 );
    sdfValue = min( sdfValue, sdfSpheraI );
    sdfValue = min( sdfValue, sdfSpheraJ );
    sdfValue = min( sdfValue, sdfSpheraK );
#endif
    return sdfValue;
}

float SampleCloudSDF2( vec3 p ) {
    float width = 200;

    if ( p.x > width ) {
        p.x -= width;
        p.x = fract( p.x / ( width * 2.0 ) ) * width * 2.0;
        p.x -= width;
    }
    if ( p.x < -width ) {
        p.x += width;
        p.x = -fract( -p.x / ( width * 2.0 ) ) * width * 2.0;
        p.x += width;
    }
    if ( p.y > width ) {
        p.y -= width;
        p.y = fract( p.y / ( width * 2.0 ) ) * width * 2.0;
        p.y -= width;
    }
    if ( p.y < -width ) {
        p.y += width;
        p.y = -fract( -p.y / ( width * 2.0 ) ) * width * 2.0;
        p.y += width;
    }
    float sdf = SampleCloudSDF( p );
    return sdf;
}

/*
==========================================
SampleCloudNoise
High resolution noise/detail
==========================================
*/
float SampleCloudNoise( float sdfSample, vec3 samplePos ) {
    float cloud = clamp( -sdfSample / 20.0, 0.0, 1.0 );
    if ( cloud > 0.0 ) {
        vec3 velocity = vec3( 1, 2, 3 );
        vec3 offset = velocity * time;
        vec3 worley = SampleWorleyNoise( samplePos + offset, 0.4 ) * 0.8;

        cloud = Remap( cloud, worley.x, 1.0, 0.0, 1.0 );
        cloud = clamp( cloud, 0.0, 1.0 );
    }
    return cloud * 0.5;
}

/*
==========================================
MultipleOctaveScattering
// Adapted from: https://twitter.com/FewesW/status/1364629939568451587/photo/1
// Original Paper: https://magnuswrenninge.com/wp-content/uploads/2010/03/Wrenninge-OzTheGreatAndVolumetric.pdf
==========================================
*/
vec3 MultipleOctaveScattering( float density, float cosTheta ) {
    float attenuation = 0.2;
    float contribution = 0.2;
    float phaseAttenuation = 0.5;

    float a = 1.0;
    float b = 1.0;
    float c = 1.0;
    float g = 0.85;
    const float scatteringOctaves = 4.0;

    vec3 luminance = vec3( 0.0 );

    for ( float i = 0.0; i < scatteringOctaves; i++ ) {
        float phaseFunction = PhaseFunction( 0.3 * c, cosTheta );
        vec3 beers = exp( -density * BETA * a );

        luminance += b * phaseFunction * beers;

        a *= attenuation;
        b *= contribution;
        c *= ( 1.0 - phaseAttenuation );
    }
    return luminance;
}

/*
==========================================
PathDensity
Accumulates the cloud density along the path
==========================================
*/
float PathDensity( vec3 rayPos, vec3 rayDir ) {
    int maxSteps = 12;
    float maxDistance = 50.0;
    float ds = maxDistance / maxSteps;
    float density = 0.0;
    float s = 0.0;

    // Accumulate the cloud density in the direction of the ray
    for ( int i = 0; i < maxSteps; i++ ) {
        vec3 samplePos = rayPos + rayDir * s;

        float sdfSample = SampleCloudSDF2( samplePos );
        float densitySample = SampleCloudNoise( sdfSample, samplePos );

        density += densitySample * ds;
        s += ds;
    }

    return density;
}

/*
==========================================
LightEnergy
Calculates the amount of sun light entering the sample position in the cloud
==========================================
*/
vec3 LightEnergy( vec3 samplePos, float cosTheta ) {
    float density = PathDensity( samplePos, sunDir );
    vec3 beersLaw = MultipleOctaveScattering( density, cosTheta );
    vec3 powderSugarEffect = 1.0 - exp( -density * 2.0 * BETA );
    vec3 lightEnergy = 2.0f * beersLaw * powderSugarEffect;
    return lightEnergy;
}

/*
==========================================
RayMarch
ray marches through the cloud scape
==========================================
*/
vec3 RayMarch( vec3 rayPos, vec3 rayDir ) {
    const float w = 200;
    const vec3 cloudBounds = vec3( 100 * w, 100 * w, w );
    const vec3 cloudMins = -cloudBounds + vec3( 0, 0, CLOUD_ALTITUDE );
    const vec3 cloudMaxs = cloudBounds + vec3( 0, 0, CLOUD_ALTITUDE );

    float t0;
    float t1;
    if ( !IntersectBox( rayPos, rayDir, cloudMins, cloudMaxs, t0, t1 ) ) {
        discard;
    }

    // If we're inside the box, then start marching at the ray position
    if ( t0 < 0.0 ) {
        t0 = 0.0;
    }

    vec3 sunLightColour = vec3( 1.0 );
    vec3 sunLight = sunLightColour * 50.0;
    vec3 ambient = sunLightColour * 0.1;

    float cosTheta = dot( rayDir, sunDir );

    const int maxSamples = 256;
    float minStep = ( t1 - t0 ) / maxSamples;
    minStep = max( 1.0, minStep );
    minStep = 1.0;

    //
    //  Ray march through the cloudscape
    //
    float t = t0;
    vec3 transmittance = vec3( 1 );
    vec3 scattering = vec3( 0 );
    while ( t < t1 ) {
        vec3 samplePos = rayPos + rayDir * t;
        float sdfSample = SampleCloudSDF2( samplePos );
        float dt = ( sdfSample > minStep ) ? sdfSample : minStep;
        t += dt;

        // Not in a cloud, skip expensive sampling
        if ( sdfSample >= 0.0 ) {
            continue;
        }

        // Sample the high detail cloud density from the worley noise
        float density = SampleCloudNoise( sdfSample, samplePos );
        if ( density <= 0.01 ) {
            // There's no cloud here, it's just empty sky.  Therefore there's no scattering event.
            continue;
        }

        // Calculate the light entering this sample position
        vec3 energy = LightEnergy( samplePos, cosTheta );
        vec3 luminance = ambient + sunLight * energy;

        // Calcualte the light scattered into the camera from this position
        vec3 transmission = exp( -density * BETA * dt );
        vec3 scatter = luminance * ( 1.0 - transmission );

        // Accumulate light
        scattering += transmittance * scatter;
        transmittance *= transmission;

        // Early out if the transmittance is opaque
        if ( IsColorInsignificant( transmittance ) ) {
            break;
        }
    }

    // Discard anything that didn't hit
    if ( dot( transmittance, vec3( 1 ) ) >= 3.0 ) {
        discard;
    }

    vec3 sky = SkyColor( rayDir.z );
    vec3 color = scattering + sky * transmittance;
    return color;
}

/*
==========================================
HDR
==========================================
*/
vec3 HDR( vec3 L ) {
    float exposure = 0.5;

    L = L * exposure;
	L.r = L.r < 1.413 ? pow( L.r * 0.38317, 1.0 / 2.2 ) : 1.0 - exp( -L.r );
	L.g = L.g < 1.413 ? pow( L.g * 0.38317, 1.0 / 2.2 ) : 1.0 - exp( -L.g );
	L.b = L.b < 1.413 ? pow( L.b * 0.38317, 1.0 / 2.2 ) : 1.0 - exp( -L.b );
    return L;
}

/*
==========================================
main
==========================================
*/
void main() {
    vec3 rayDir = normalize( v_ray );
	vec3 color = RayMarch( camPos, rayDir );
    color.rgb = HDR( color.rgb );

	outColor = vec4( color.rgb, 1.0 );
}
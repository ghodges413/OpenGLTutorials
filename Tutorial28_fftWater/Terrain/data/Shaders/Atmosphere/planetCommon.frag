#version 440

const float solarIntensity = 100.0;

uniform float radiusGround;
uniform float radiusTop;

uniform float mieG;
uniform float exposure;

uniform vec3 betaRayleighScatter;

uniform vec3 u_rayToCamera;
uniform vec3 u_rayToLight;

uniform sampler2D s_texturePlanet;

uniform sampler2D transmittanceSampler;
uniform sampler2D irradianceSampler;
uniform sampler3D inscatterSampler;

uniform sampler3D inscatterSampler0;
uniform sampler3D inscatterSampler1;
uniform sampler3D inscatterSampler2;
uniform sampler3D inscatterSampler3;
uniform sampler3D inscatterSampler4;
uniform sampler3D inscatterSampler5;
uniform sampler3D inscatterSampler6;
uniform sampler3D inscatterSampler7;

/*
 ===============================
 IntersectRaySphere
 always t1 <= t2
 if t1 < 0 && t2 > 0 ray is inside
 if t1 < 0 && t2 < 0 sphere is behind ray origin
 recover the 3D position with p = rayStart + t * rayDir
 ===============================
 */
bool IntersectRaySphere( in vec3 rayStart, in vec3 rayDir, in vec4 sphere, out float t1, out float t2 ) {
    // Note:    If we force the rayDir to be normalized,
    //          then we can get an optimization where
    //          a = 1, b = m.
    //          Which would decrease the number of operations
    vec3 m = sphere.xyz - rayStart;
    float a   = dot( rayDir, rayDir );
    float b   = dot( m, rayDir );
    float c   = dot( m, m ) - sphere.w * sphere.w;
    
    float delta = b * b - a * c;
    float invA = 1.0 / a;
    
    if ( delta < 0.0 ) {
        // no real solutions exist
        return false;
    }
    
    float deltaRoot = sqrt( delta );
    t1 = invA * ( b - deltaRoot );
    t2 = invA * ( b + deltaRoot );

    return true;
}

/*
 ===============================
 IntersectTop
 ===============================
 */
float IntersectTop( in vec3 pos, in vec3 ray ) {
	vec4 sphereTop = vec4( vec3( 0.0 ), radiusTop );

	float t0 = -1.0;
	float t1 = -1.0;
	IntersectRaySphere( pos, ray, sphereTop, t0, t1 );
	float tout = -1.0;
	if ( t0 > 0.0 && t1 > 0.0 ) {
		tout = min( t0, t1 );
	}
    return tout;
}

/*
 ===============================
 MoveCameraInsideAtmosphere
 ===============================
 */
vec3 MoveCameraInsideAtmosphere( in vec3 pos, in vec3 view ) {
	float r = length( pos );
	if ( r < radiusTop ) {
		return pos;
	}
	
	float t = IntersectTop( pos, view );
	if ( t <= 0.0 ) {
		return pos;
	}
	
	vec3 newPos = pos + t * view;
	return newPos;
}

/*
 ===============================
 ToneMapping
 ===============================
 */
vec3 ToneMapping( vec3 Intensity ) {
    Intensity = Intensity * exposure;
	
	Intensity = vec3( 1.0 ) - exp( -Intensity );
	
	// Gamma correction value
	const float m = 1.0 / 2.2;
	
	for ( int i = 0; i < 3; ++i ) {
		Intensity[ i ] = pow( Intensity[ i ], m );
	}
    return Intensity;
}

/*
 ===============================
 ApproximateMie
 // algorithm described in Bruneton2008 Section4 paragraph Angular Precision
 ===============================
 */
vec3 ApproximateMie( in vec4 inscatter ) {
	// Grab C* from the paper
	vec3 inscatterRayleigh = inscatter.rgb;
	
	// Grab Cm from the paper
	float inscatterMie = inscatter.a;
	
	// Calculate the Cmr / C*r (and don't divide by zero)
	float Cr = max( inscatterRayleigh.r, 1e-6 );
	float inscatterRatio = inscatterMie / Cr;
	
	// Calculate beta ratio from the paper (mie cancels itself out... so it's only rayleigh)
	vec3 betaRayleighRatio = betaRayleighScatter.r / betaRayleighScatter;
	
	// Output the final equation
	return inscatterRayleigh * inscatterRatio * betaRayleighRatio;
}

/*
 ===============================
 SampleScatterTable
 ===============================
 */
vec4 SampleScatterTable( in sampler3D table, in float radius, in float cosAngleView, in float cosAngleSun, in float cosAngleViewSun ) {
    float H = sqrt( radiusTop * radiusTop - radiusGround * radiusGround );
    float rho = sqrt( radius * radius - radiusGround * radiusGround );
	
	float rmu = radius * cosAngleView;
	float delta = rmu * rmu - radius * radius + radiusGround * radiusGround;
	
	// Coordinate lookups from paper
	float uR = rho / H;
	
	float uMu = 0.5;
	if ( rmu < 0.0 && delta > 0.0 ) {
		uMu += ( rmu + sqrt( delta ) ) / ( 2.0 * rho );
	} else {
		uMu -= ( rmu - sqrt( delta + H * H ) ) / ( 2.0 * rho + 2.0 * H );
	}
	
	float uMuS = ( 1.0 - exp( -3.0 * cosAngleSun - 0.6 ) ) / ( 1.0 - exp( -3.6 ) );
	
	const float dimNU = 8;
	float t = ( 1.0 + cosAngleViewSun ) * 0.5;
	t *= ( dimNU - 1.0 );
    float uNu = floor( t );
    t = t - uNu;
	
	vec3 coord = vec3( uMuS, uMu, uR );

	vec4 sample0;
	vec4 sample1;
	
	if ( 0.0 == uNu ) {
		sample0 = texture( inscatterSampler0, coord );
		sample1 = texture( inscatterSampler1, coord );
	} else if ( 1.0 == uNu ) {
		sample0 = texture( inscatterSampler1, coord );
		sample1 = texture( inscatterSampler2, coord );
	} else if ( 2.0 == uNu ) {
		sample0 = texture( inscatterSampler2, coord );
		sample1 = texture( inscatterSampler3, coord );
	} else if ( 3.0 == uNu ) {
		sample0 = texture( inscatterSampler3, coord );
		sample1 = texture( inscatterSampler4, coord );
	} else if ( 4.0 == uNu ) {
		sample0 = texture( inscatterSampler4, coord );
		sample1 = texture( inscatterSampler5, coord );
	} else if ( 5.0 == uNu ) {
		sample0 = texture( inscatterSampler5, coord );
		sample1 = texture( inscatterSampler6, coord );
	} else if ( 6.0 == uNu ) {
		sample0 = texture( inscatterSampler6, coord );
		sample1 = texture( inscatterSampler7, coord );
	} else {
		sample0 = texture( inscatterSampler7, coord );
		sample1 = texture( inscatterSampler0, coord );
	}
    return mix( sample0, sample1, t );
}

/*
 ===============================
 SampleScatterTable
 ===============================
 *
vec4 SampleScatterTable( in sampler3D table, in float radius, in float cosAngleView, in float cosAngleSun, in float cosAngleViewSun ) {
    float H = sqrt( radiusTop * radiusTop - radiusGround * radiusGround );
    float rho = sqrt( radius * radius - radiusGround * radiusGround );
	
	float rmu = radius * cosAngleView;
	float delta = rmu * rmu - radius * radius + radiusGround * radiusGround;
	
	// Coordinate lookups from paper
	float uR = rho / H;
	
	float uMu = 0.5;
	if ( rmu < 0.0 && delta > 0.0 ) {
		uMu += ( rmu + sqrt( delta ) ) / ( 2.0 * rho );
	} else {
		uMu -= ( rmu - sqrt( delta + H * H ) ) / ( 2.0 * rho + 2.0 * H );
	}
	
	//
	// Special handling of the uMus and uNu, since they're packed in together
	//
	
	float uMuS = ( 1.0 - exp( -3.0 * cosAngleSun - 0.6 ) ) / ( 1.0 - exp( -3.6 ) );
	
	// Clamp the uMus so that we don't accidentally lerp between the end ranges
	const float dimMUS = 32;
	const float deltaMUS = 0.5 / dimMUS;
	uMuS = clamp( uMuS, deltaMUS, 1.0 - deltaMUS );
	
	// Special handling for NU to choose the appropriate textures to blend between
	const float dimNU = 8;
	float t = ( 1.0 + cosAngleViewSun ) * 0.5;
	t *= ( dimNU - 1.0 );
    float uNu = floor( t );
    t = t - uNu;
	
	vec3 coord0 = vec3( ( uNu + uMuS ) / dimNU, uMu, uR );
	vec3 coord1 = vec3( ( uNu + uMuS + 1.0 ) / dimNU, uMu, uR );
	
	vec4 table0 = texture( table, coord0 );
	vec4 table1 = texture( table, coord1 );
    return mix( table0, table1, t );
}

/*
==========================
ScatterPhaseFunctionRayleigh
// Equation 2 from Bruneton2008
==========================
*/
float ScatterPhaseFunctionRayleigh( in float cosTheta ) {
	const float pi = acos( -1.0 );

	float phase = ( 3.0 / ( 16.0 * pi ) ) * ( 1.0 + cosTheta * cosTheta );
	return phase;
}

/*
==========================
ScatterPhaseFunctionMie
// Equation 4 from Bruneton2008
==========================
*/
float ScatterPhaseFunctionMie( in float cosTheta ) {
	const float pi = acos( -1.0 );

	float g = mieG;
	float g2 = g * g;
	
	float phase = ( 3.0 / ( 8.0 * pi ) ) * ( 1.0 - g2 ) * ( 1.0 + cosTheta * cosTheta ) / ( ( 2.0 + g2 ) * pow( (1.0 + g2 - 2.0 * g * cosTheta ), 1.5 ) );
	return phase;
}

/*
 ===============================
 Inscatter
 Lookup the inscattering light entering the position along the view ray
 ===============================
 */
vec3 Inscatter( vec3 pos, vec3 view, vec3 sun ) {
	float radius = length( pos );
	vec3 zenith = normalize( pos );
	
	float cosSun = dot( zenith, sun );
	float cosView = dot( zenith, view );
	float cosViewSun = dot( view, sun );
	
	vec4 inscatter = SampleScatterTable( inscatterSampler, radius, cosView, cosSun, cosViewSun );
	vec3 result = inscatter.rgb * ScatterPhaseFunctionRayleigh( cosViewSun );
	result += ApproximateMie( inscatter ) * ScatterPhaseFunctionMie( cosViewSun );
	
    return result;
}



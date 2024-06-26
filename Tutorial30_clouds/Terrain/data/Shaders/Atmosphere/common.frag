#version 440

const vec4 dimInScatter = vec4( 32, 128, 32, 8 );

uniform float radiusGround;
uniform float radiusTop;

// Rayleigh
uniform float scaleHeightRayleigh;
uniform vec3 betaRayleighScatter;
uniform vec3 betaRayleighExtinction;

// Mie
// DEFAULT
uniform float scaleHeightMie;
uniform vec3 betaMieScatter;
uniform vec3 betaMieExtinction;
uniform float mieG;
// CLEAR SKY
/*const float scaleHeightMie = 1.2;
const vec3 betaMieScatter = vec3(20e-3);
const vec3 betaMieExtinction = betaMieScatter / 0.9;
const float mieG = 0.76;*/
// PARTLY CLOUDY
/*const float scaleHeightMie = 3.0;
const vec3 betaMieScatter = vec3(3e-3);
const vec3 betaMieExtinction = betaMieScatter / 0.9;
const float mieG = 0.65;*/

//#define LINEAR_INSCATTER

/*
 ===============================
 GetNewRadius
 Calculates the radius of a point that is a distance, s,
 and at an angle, cosAngle, from the starting point
 ===============================
 */
float GetNewRadius( in float radius, in float s, in float cosAngle ) {
#if 1
	float dx = sqrt( 1.0 - cosAngle * cosAngle );	// get the sine of the angle
	//float dx = sin( acos( cosAngle ) );		// get the sine of the angle
	float dy = cosAngle;
	
	float x = dx * s;
	float y = dy * s + radius;
	float r = sqrt( x * x + y * y );
#else
	// Use the law of cosines to calculate the new radius (it's mathematically equivalent to using vectors)
	float r = sqrt( radius * radius + s * s + 2.0 * radius * s * cosAngle );
#endif
	return r;
}

/*
 ===============================
 GetNextAngleView
 Calculates the next view angle relative to a new point
 ===============================
 */
float GetNextAngleView( in float radius, in float s, in float radiusNext, in float cosAngleView ) {
#if 1
	float dx = sqrt( 1.0 - cosAngleView * cosAngleView );	// get the sine of the angle
	float dy = cosAngleView;
	
	float x = dx * s;
	float y = dy * s + radius;
	
	// Get a vector to the new position
	vec2 r = normalize( vec2( x, y ) );
	
	// Grab the view vector
	vec2 view = vec2( dx, dy );
	
	// Get the angle between the view and the current pos
	float cosAngle = dot( view, r );
#else
	//float cosAngle = ( radius * cosAngleView + s ) / radiusNext;
	float cosAngle = ( radius * radius + s * s - radiusNext * radiusNext ) / ( 2.0 * radius * s );
#endif
	return cosAngle;
}

/*
 ===============================
 GetViewAndSunRaysFromAngles
 ===============================
 */
void GetViewAndSunRaysFromAngles( in float cosAngleView, in float cosAngleSun, in float cosAngleViewSun, out vec3 view, out vec3 sun ) {
	float sinAngleView = sqrt( 1.0 - cosAngleView * cosAngleView );	// get the sine of the angle
	float sinAngleSun = sqrt( 1.0 - cosAngleSun * cosAngleSun );	// get the sine of the angle

	// Build a ray for the view
    view = vec3( sinAngleView, 0.0, cosAngleView );
	
	// Build a ray for the sun
	sun = vec3( sinAngleSun, 0.0, cosAngleSun );
	
	// If the view is directly up, then it doesn't matter if the sun is in the same plane... it's all symmetric.
	// In the event that the view isn't directly up, we need to calculate the 3D direction to the sun.
	if ( sinAngleView > 0.0 ) {
		// cosAngleViewSun is the dot product between the sun and view
		// cosAngleViewSun = sx * vx + sy * vy + sz + vz
		// = sx * vx + sz + vz
		// => sx * vx = cosAngleViewSun - sz * vz
		sun.x = ( cosAngleViewSun - sun.z * cosAngleView ) / view.x;
		
		// x2 + y2 + z2 = 1.0
		// => y = sqrt( 1.0 - x2 - z2 )
		float ySqr = 1.0 - sun.x * sun.x - sun.z * sun.z;
		if ( ySqr > 0.0 ) {
			sun.y = sqrt( ySqr );
		}
	}
}

/*
 ===============================
 GetNextAngleSun
 Calculates the next solar angle relative to a new point
 ===============================
 */
float GetNextAngleSun( in float radius, in float s, in float radiusNext, in float cosAngleView, in float cosAngleSun, in float cosAngleViewSun ) {
#if 1
	vec3 view;
	vec3 sun;
	GetViewAndSunRaysFromAngles( cosAngleView, cosAngleSun, cosAngleViewSun, view, sun );
	
	//
	//	Build a ray for the new position
	//
	float dx = sqrt( 1.0 - cosAngleView * cosAngleView );	// get the sine of the angle
	float dy = cosAngleView;
	
	float x = dx * s;
	float y = dy * s + radius;
	
	// Get a vector to the new position
	vec3 r = normalize( vec3( x, 0, y ) );

	// Get the new angle between the new position and the sun
	float cosAngle = dot( sun, r );
#else
	// Again, a trigonometric solution
	float cosAngle = ( radius * cosAngleSun + s * cosAngleViewSun ) / radiusNext;
#endif
	return cosAngle;
}

/*
 ===============================
 texture4D
 ===============================
 */
vec4 texture4D( in sampler3D table, in float radius, in float cosAngleView, in float cosAngleSun, in float cosAngleViewSun ) {
    float H = sqrt( radiusTop * radiusTop - radiusGround * radiusGround );
    float rho = sqrt( radius * radius - radiusGround * radiusGround );
	
#ifdef LINEAR_INSCATTER

	float uR = 0.5 / dimInScatter.x + rho / H * ( 1.0 - 1.0 / dimInScatter.x );
    float uMu = 0.5 / dimInScatter.y + ( cosAngleView + 1.0 ) / 2.0 * ( 1.0 - 1.0 / dimInScatter.y );
    float uMuS = 0.5 / dimInScatter.z + max( cosAngleSun + 0.2, 0.0 ) / 1.2 * ( 1.0 - 1.0 / dimInScatter.z );
	
#else
	
	float rmu = radius * cosAngleView;
	float delta = rmu * rmu - radius * radius + radiusGround * radiusGround;
	
	vec4 cst = vec4( -1.0, H * H, H, 0.5 + 0.5 / dimInScatter.y );
	if ( rmu < 0.0 && delta > 0.0 ) {
		cst = vec4( 1.0, 0.0, 0.0, 0.5 - 0.5 / dimInScatter.y );
	}
	
	float uR = 0.5 / dimInScatter.x + rho / H * ( 1.0 - 1.0 / dimInScatter.x );
	float uMu = cst.w + ( rmu * cst.x + sqrt( delta + cst.y ) ) / ( rho + cst.z ) * ( 0.5 - 1.0 / dimInScatter.y );
	
	// paper formula
	float uMuS = 0.5 / dimInScatter.z + max( ( 1.0 - exp( -3.0 * cosAngleSun - 0.6 ) ) / ( 1.0 - exp( -3.6 ) ), 0.0 ) * ( 1.0 - 1.0 / dimInScatter.z );
	
#endif
    
	float t = ( cosAngleViewSun + 1.0 ) / 2.0 * ( dimInScatter.w - 1.0 );
    float uNu = floor( t );
    t = t - uNu;
	
	vec3 coord0 = vec3( ( uNu + uMuS ) / dimInScatter.w, uMu, uR );
	vec3 coord1 = vec3( ( uNu + uMuS + 1.0 ) / dimInScatter.w, uMu, uR );
	
	vec4 table0 = texture( table, coord0 );
	vec4 table1 = texture( table, coord1 );
    return mix( table0, table1, t );
}

/*
 ===============================
 GetNu
 ===============================
 */
float GetNu() {
	float x = gl_FragCoord.x - 0.5;
	float nu = -1.0 + floor( x / dimInScatter.z ) / ( dimInScatter.w - 1.0 ) * 2.0;
	return nu;
}
 
/*
 ===============================
 GetPhysicalAnglesFromTextureCoords
 ===============================
 */
vec3 GetPhysicalAnglesFromTextureCoords( in float radius ) {
    float x = gl_FragCoord.x - 0.5;
    float y = gl_FragCoord.y - 0.5;
	
	float cosAngleView = 0.0;
	float cosAngleSun = 0.0;
	float cosAngleViewSun = GetNu();
	
#ifdef LINEAR_INSCATTER

	cosAngleView = -1.0 + 2.0 * y / ( dimInScatter.y - 1.0 );
    cosAngleSun = mod( x, dimInScatter.z ) / ( dimInScatter.z - 1.0 );
    cosAngleSun = -0.2 + cosAngleSun * 1.2;
	
#else
	
	vec4 dhdH;
	{
		float dmin = radiusTop - radius;
		float dmax = sqrt( radius * radius - radiusGround * radiusGround ) + sqrt( radiusTop * radiusTop - radiusGround * radiusGround );
		
		float dminp = radius - radiusGround;
		float dmaxp = sqrt( radius * radius - radiusGround * radiusGround );
		
		dhdH.x = dmin;
		dhdH.y = dmax;
		dhdH.z = dminp;
		dhdH.w = dmaxp;
	}
    
	if ( y < dimInScatter.y / 2.0 ) {
        float d = 1.0 - y / ( dimInScatter.y / 2.0 - 1.0 );
        d = min( max( dhdH.z, d * dhdH.w ), dhdH.w * 0.999 );
        cosAngleView = ( radiusGround * radiusGround - radius * radius - d * d ) / ( 2.0 * radius * d );
        cosAngleView = min( cosAngleView, -sqrt( 1.0 - ( radiusGround / radius ) * ( radiusGround / radius ) ) - 0.001 );
    } else {
        float d = ( y - dimInScatter.y / 2.0 ) / ( dimInScatter.y / 2.0 - 1.0 );
        d = min( max( dhdH.x, d * dhdH.y ), dhdH.y * 0.999 );
        cosAngleView = ( radiusTop * radiusTop - radius * radius - d * d ) / ( 2.0 * radius * d );
    }
    cosAngleSun = mod( x, dimInScatter.z ) / ( dimInScatter.z - 1.0 );
    
	// paper formula
    cosAngleSun = -( 0.6 + log( 1.0 - cosAngleSun * ( 1.0 -  exp( -3.6 ) ) ) ) / 3.0;

#endif
	
	return vec3( cosAngleView, cosAngleSun, cosAngleViewSun );
}

/*
 ===============================
 GetRadius
 fragCoord is in the range [0, width], [0, height], [0, depth]
 ===============================
 */
float GetRadius( in vec3 fragCoord ) {
	float radius = fragCoord.z / ( dimInScatter.x - 1.0 );
	radius = sqrt( radiusGround * radiusGround + radius * radius * ( radiusTop * radiusTop - radiusGround * radiusGround ) );
	if ( fragCoord.z <= 0.0 ) {
		radius += 0.01f;
	}
	if ( fragCoord.z >= dimInScatter.x - 1.0 ) {
		radius -= 0.001f;
	}
	return radius;
}

/*
 ===============================
 GetNu
 fragCoord is in the range [0, width], [0, height]
 ===============================
 */
float GetNu( in vec2 fragCoord ) {
	float x = fragCoord.x + 0.5;
	float nu = -1.0 + floor( x / dimInScatter.z ) / ( dimInScatter.w - 1.0 ) * 2.0;
	return nu;
}
 
/*
 ===============================
 GetPhysicalAnglesFromTextureCoords
 fragCoord is in the range [0, width], [0, height]
 ===============================
 */
vec3 GetPhysicalAnglesFromTextureCoords( in float radius, in vec2 fragCoord ) {
    float x = fragCoord.x + 0.5;
    float y = fragCoord.y + 0.5;
	
	float cosAngleView = 0.0;
	float cosAngleSun = 0.0;
	float cosAngleViewSun = GetNu( fragCoord );
	
#ifdef LINEAR_INSCATTER

	cosAngleView = -1.0 + 2.0 * y / ( dimInScatter.y - 1.0 );
    cosAngleSun = mod( x, dimInScatter.z ) / ( dimInScatter.z - 1.0 );
    cosAngleSun = -0.2 + cosAngleSun * 1.2;
	
#else
	
	vec4 dhdH;
	{
		float dmin = radiusTop - radius;
		float dmax = sqrt( radius * radius - radiusGround * radiusGround ) + sqrt( radiusTop * radiusTop - radiusGround * radiusGround );
		
		float dminp = radius - radiusGround;
		float dmaxp = sqrt( radius * radius - radiusGround * radiusGround );
		
		dhdH.x = dmin;
		dhdH.y = dmax;
		dhdH.z = dminp;
		dhdH.w = dmaxp;
	}
    
	if ( y < dimInScatter.y / 2.0 ) {
        float d = 1.0 - y / ( dimInScatter.y / 2.0 - 1.0 );
        d = min( max( dhdH.z, d * dhdH.w ), dhdH.w * 0.999 );
        cosAngleView = ( radiusGround * radiusGround - radius * radius - d * d ) / ( 2.0 * radius * d );
        cosAngleView = min( cosAngleView, -sqrt( 1.0 - ( radiusGround / radius ) * ( radiusGround / radius ) ) - 0.001 );
    } else {
        float d = ( y - dimInScatter.y / 2.0 ) / ( dimInScatter.y / 2.0 - 1.0 );
        d = min( max( dhdH.x, d * dhdH.y ), dhdH.y * 0.999 );
        cosAngleView = ( radiusTop * radiusTop - radius * radius - d * d ) / ( 2.0 * radius * d );
    }
    cosAngleSun = mod( x, dimInScatter.z ) / ( dimInScatter.z - 1.0 );
    
	// paper formula
    cosAngleSun = -( 0.6 + log( 1.0 - cosAngleSun * ( 1.0 -  exp( -3.6 ) ) ) ) / 3.0;

#endif
	
	return vec3( cosAngleView, cosAngleSun, cosAngleViewSun );
}

/*
 ===============================
 DoesCollideGround
 ===============================
 */
bool DoesCollideGround( in float radius, in float cosAngle, in float ground ) {
	float g2 = ground * ground;
	float r2 = radius * radius;
	
	if ( cosAngle < -sqrt( 1.0 - g2 / r2 ) ) {
		return true;
	}
	
	return false;
}

/*
 ===============================
 IntersectGroundTop
 // Intersect the ground or the top, whichever is nearest, and return the distance
 ===============================
 */
float IntersectGroundTop( in float radius, in float cosTheta ) {
	float top = radiusTop + 1.0;
	float radiusSqr = radius * radius;
	float sinThetaSqr = 1.0 - cosTheta * cosTheta;
    float dout = -radius * cosTheta + sqrt( top * top - radiusSqr * sinThetaSqr );
    float delta2 = radiusGround * radiusGround - radiusSqr * sinThetaSqr;
	
    if ( delta2 >= 0.0 ) {
        float din = -radius * cosTheta - sqrt( delta2 );
		
        if ( din >= 0.0 ) {
            dout = min( dout, din );
        }
    }
	
    return dout;	
}

// Used for widening/narrowing the range of the cosAngle
const float deltaTransmittance = 0.15;

/*
 ===============================
 ConvertTransmittanceCoordToPhysicalHeightAngle
 ===============================
 */
void ConvertTransmittanceCoordToPhysicalHeightAngle( in vec2 st, out float radius, out float cosAngle ) {
	radius = radiusGround + st.x * ( radiusTop - radiusGround );
	cosAngle = -deltaTransmittance + st.y * ( 1.0 + deltaTransmittance );
}

/*
 ===============================
 Transmittance
 // Returns the calculated transmittance for the height/angle.  Ignores ground intersections.
 ===============================
 */
vec3 Transmittance( in float radius, in float cosAngle, in sampler2D sampler ) {
	vec2 st;
	st.x = ( radius - radiusGround ) / ( radiusTop - radiusGround );
	st.y = ( cosAngle + deltaTransmittance ) / ( 1.0 + deltaTransmittance );
    return texture( sampler, st ).rgb;
}

/*
 ===============================
 TransmittanceWithShadow
 // Transmittance(=transparency) of atmosphere for infinite ray (r,mu)
 // (mu=cos(view zenith angle)), or zero if ray intersects ground
 ===============================
 */
vec3 TransmittanceWithShadow( in float radius, in float cosAngle, in sampler2D sampler ) {
    if ( cosAngle < -sqrt( 1.0 - ( radiusGround / radius ) * ( radiusGround / radius ) ) ) {
		return vec3( 0.0 );
	}

	return Transmittance( radius, cosAngle, sampler );
}

/*
 ===============================
 Transmittance
 // Transmittance(=transparency) of atmosphere between x and x0
 // assume segment x,x0 not intersecting ground
 // r=||x||, mu=cos(zenith angle of [x,x0) ray at x), v=unit direction vector of [x,x0) ray
 ===============================
 */
vec3 Transmittance( in float radius, in float cosAngle, in vec3 v, in vec3 x0, in sampler2D sampler ) {
    vec3 result;
    float r1 = length( x0 );
    float mu1 = dot( x0, v ) / radius;
    if ( cosAngle > 0.0 ) {
        result = min( Transmittance( radius, cosAngle, sampler ) / Transmittance( r1, mu1, sampler ), 1.0 );
    } else {
        result = min( Transmittance( r1, -mu1, sampler ) / Transmittance( radius, -cosAngle, sampler ), 1.0 );
    }
    return result;
}

/*
 ===============================
 Transmittance
 // Transmittance(=transparency) of atmosphere between x and x0
 // assume segment x,x0 not intersecting ground
 // d = distance between x and x0, mu=cos(zenith angle of [x,x0) ray at x)
 ===============================
 */
vec3 Transmittance( in float radius, in float cosAngle, in float d, in sampler2D sampler ) {
    vec3 result;
    float r1 = sqrt( radius * radius + d * d + 2.0 * radius * cosAngle * d );
    float mu1 = ( radius * cosAngle + d ) / r1;
    if ( cosAngle > 0.0 ) {
        result = min( Transmittance( radius, cosAngle, sampler ) / Transmittance( r1, mu1, sampler ), 1.0 );
    } else {
        result = min( Transmittance( r1, -mu1, sampler ) / Transmittance( radius, -cosAngle, sampler ), 1.0 );
    }
    return result;
}

// Used for widening/narrowing the range of the cosAngle
const float deltaIrradiance = 0.2;

/*
 ===============================
 ConvertIrradianceCoordToPhysicalHeightAngle
 ===============================
 */
void ConvertIrradianceCoordToPhysicalHeightAngle( in vec2 st, out float radius, out float cosAngle ) {
	radius = radiusGround + st.x * ( radiusTop - radiusGround );
	cosAngle = -deltaIrradiance + st.y * ( 1.0 + deltaIrradiance );
}

/*
 ===============================
 Irradiance
 ===============================
 */
vec3 Irradiance( in sampler2D sampler, in float radius, in float cosAngle ) {
	vec2 st;
	st.x = ( radius - radiusGround ) / ( radiusTop - radiusGround );
	st.y = ( cosAngle + deltaIrradiance ) / ( 1.0 + deltaIrradiance );
    
    return texture( sampler, st ).rgb;
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

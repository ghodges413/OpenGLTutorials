
#include "common.frag"


const float ISun = 100.0;

uniform vec3 c;
uniform vec3 s;
uniform mat4 projInverse;
uniform mat4 viewInverse;
uniform float exposure;

uniform sampler2D transmittanceSampler;
//uniform sampler2D reflectanceSampler;//ground reflectance texture
uniform sampler2D irradianceSampler;//precomputed skylight irradiance (E table)
uniform sampler3D inscatterSampler;//precomputed inscattered light (S table)

in vec2 v_coords;
in vec3 v_ray;

layout(location = 0) out vec4 FragColor;

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
 Inscatter
 //inscattered light along ray x+tv, when sun in direction s (=S[L])
 ===============================
 */
vec3 Inscatter( inout vec3 x, vec3 v, vec3 s, out float r, out float mu ) {
    vec3 result;
    r = length( x );
    mu = dot( x, v ) / r;
    float d = -r * mu - sqrt( r * r * ( mu * mu - 1.0 ) + radiusTop * radiusTop );
    if ( d > 0.0 ) {
		// if x in space and ray intersects atmosphere
        // move x to nearest intersection of ray with top atmosphere boundary
        x += d * v;
        mu = ( r * mu + d ) / radiusTop;
        r = radiusTop;
    }
    if ( r <= radiusTop ) { // if ray intersects atmosphere
        float nu = dot( v, s );
        float muS = dot( x, s ) / r;
        float phaseR = ScatterPhaseFunctionRayleigh( nu );
        float phaseM = ScatterPhaseFunctionMie( nu );
        vec4 inscatter = max( texture4D( inscatterSampler, r, mu, muS, nu ), 0.0 );
        result = max( inscatter.rgb * phaseR + ApproximateMie( inscatter ) * phaseM, 0.0 );
    } else { // x in space and ray looking in space
        result = vec3( 0.0 );
    }
    return result * ISun;
}

/*
 ===============================
 GroundColor
 //ground radiance at end of ray x+tv, when sun in direction s
 //attenuated bewteen ground and viewer (=R[L0]+R[L*])
 ===============================
 */
vec3 GroundColor( vec3 x, vec3 v, vec3 s, float r, float mu ) {
	const float pi = acos( -1.0 );

    vec3 result;
    float d = -r * mu - sqrt( r * r * ( mu * mu - 1.0 ) + radiusGround * radiusGround);
    if ( d > 0.0 ) { // if ray hits ground surface
        // ground reflectance at end of ray, x0
        vec3 x0 = x + d * v;
        vec3 n = normalize( x0 );
        vec2 coords = vec2( atan( n.y, n.x ), acos( n.z ) ) * vec2( 0.5, 1.0 ) / pi + vec2( 0.5, 0.0 );
        //vec4 reflectance = texture( reflectanceSampler, coords ) * vec4( 0.2, 0.2, 0.2, 1.0 );
        vec4 reflectance = vec4( 0.6, 0.4, 0.1, 1.0 ) * vec4( 0.2, 0.2, 0.2, 1.0 );

        // direct sun light (radiance) reaching x0
        float muS = dot( n, s );
        vec3 sunLight = TransmittanceWithShadow( radiusGround, muS, transmittanceSampler );

        // precomputed sky light (irradiance) (=E[L*]) at x0
        vec3 groundSkyLight = Irradiance( irradianceSampler, radiusGround, muS );

        // light reflected at x0 (=(R[L0]+R[L*])/T(x,x0))
        vec3 groundColor = reflectance.rgb * ( max( muS, 0.0 ) * sunLight + groundSkyLight ) * ISun / pi;

        // attenuation of light to the viewer, T(x,x0)
        vec3 attenuation = Transmittance( r, mu, v, x0, transmittanceSampler );

        // water specular color due to sunLight
        if ( reflectance.w > 0.0 ) {
            vec3 h = normalize( s - v );
            float fresnel = 0.02 + 0.98 * pow( 1.0 - dot( -v, h ), 5.0 );
            float waterBrdf = fresnel * pow( max( dot( h, n ), 0.0 ), 150.0 );
            groundColor += reflectance.w * max( waterBrdf, 0.0 ) * sunLight * ISun;
        }

        result = attenuation * groundColor; //=R[L0]+R[L*]
    } else { // ray looking at the sky
        result = vec3( 0.0 );
    }
    return result;
}

/*
 ===============================
 SunColor
 // direct sun light for ray x+tv, when sun in direction s (=L0)
 ===============================
 */
vec3 SunColor( vec3 x, vec3 v, vec3 s, float r, float mu ) {
	const float pi = acos( -1.0 );

    vec3 transmittance = r <= radiusTop ? TransmittanceWithShadow( r, mu, transmittanceSampler ) : vec3( 1.0 ); // T(x,xo)
    float isun = step( cos( pi / 180.0 ), dot( v, s ) ) * ISun; // Lsun
    return transmittance * isun; // Eq (9)
}

/*
 ===============================
 HDR
 ===============================
 */
vec3 HDR( vec3 L ) {
    L = L * exposure;
	L.r = L.r < 1.413 ? pow( L.r * 0.38317, 1.0 / 2.2 ) : 1.0 - exp( -L.r );
	L.g = L.g < 1.413 ? pow( L.g * 0.38317, 1.0 / 2.2 ) : 1.0 - exp( -L.g );
	L.b = L.b < 1.413 ? pow( L.b * 0.38317, 1.0 / 2.2 ) : 1.0 - exp( -L.b );
    return L;
}

/*
 ===============================
 main
 ===============================
 */
void main() {
    vec3 x = c;
    vec3 v = normalize( v_ray );
    float r, mu;
	vec3 inscatterColor = Inscatter( x, v, s, r, mu ); //S[L]-T(x,xs)S[l]|xs = S[L] for spherical ground
    vec3 groundColor = GroundColor( x, v, s, r, mu ); //R[L0]+R[L*]
    vec3 sunColor = SunColor( x, v, s, r, mu ); //L0
    FragColor = vec4( HDR( sunColor + groundColor + inscatterColor ), 1.0 ); // Eq (16)

    //FragColor += texture(inscatterSampler,vec3(v_coords,(s.x+1.0)/2.0));
    //FragColor += vec4(texture(irradianceSampler,v_coords).rgb*5.0, 1.0);
    //FragColor += texture(transmittanceSampler,v_coords);
	FragColor.a = 1.0;
    //FragColor.r = 1.0;
    //FragColor.b = 1.0;
    //FragColor.gb = sunColor.gb;
}

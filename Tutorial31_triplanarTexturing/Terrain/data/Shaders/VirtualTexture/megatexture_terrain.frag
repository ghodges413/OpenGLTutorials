// This line tells OpenGL which version of GLSL we're using
#include "../Atmosphere/common.frag"

const float ISun = 10.0;

uniform vec3 cameraPos;
uniform vec3 sunDir;
//uniform mat4 projInverse;
//uniform mat4 viewInverse;
//uniform mat4 matBias;
//uniform mat4 matViewProj;	// Shadow matrix view proj
uniform float exposure;

uniform sampler2D transmittanceSampler;
uniform sampler2D reflectanceSampler;//ground reflectance texture
uniform sampler2D irradianceSampler;//precomputed skylight irradiance (E table)
uniform sampler3D inscatterSampler;//precomputed inscattered light (S table)




in vec2 v_texCoord;
in vec3 v_color;
in vec3 v_normal;
in vec3 v_position;
in float v_distance;	// distance to camera

uniform sampler2D pageCoordTexture;
uniform sampler2D scaleBiasTexture;
uniform sampler2D physicalTexture;

uniform int virtualPagesWide;
uniform int pageWidth;
uniform int pageCount;
uniform float virtualDistance;

layout( location = 0 ) out vec4 diffuseColor;











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

vec3 InscatterGeo( inout vec3 x, vec3 v, vec3 s, float r, float mu ) {
    vec3 result;
#if 0
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
#endif
//    if ( r <= radiusTop ) { // if ray intersects atmosphere
        float nu = dot( v, s );
        float muS = dot( x, s ) / r;
        float phaseR = ScatterPhaseFunctionRayleigh( nu );
        float phaseM = ScatterPhaseFunctionMie( nu );
        vec4 inscatter = max( texture4D( inscatterSampler, r, mu, muS, nu ), 0.0 );
        result = max( inscatter.rgb * phaseR + ApproximateMie( inscatter ) * phaseM, 0.0 );
//    } else { // x in space and ray looking in space
//        result = vec3( 0.0 );
//    }
    return result * ISun;
}

/*
 ===============================
 GroundColor
 //ground radiance at end of ray x+tv, when sun in direction s
 //attenuated bewteen ground and viewer (=R[L0]+R[L*])
 ===============================
 */
 #if 1
vec3 GroundColor( vec3 x, vec3 v, vec3 s, float r, float mu ) {
	const float pi = acos( -1.0 );

    vec3 result;
    float d = -r * mu - sqrt( r * r * ( mu * mu - 1.0 ) + radiusGround * radiusGround);
    if ( d > 0.0 ) { // if ray hits ground surface
        // ground reflectance at end of ray, x0
        vec3 x0 = x + d * v;
        vec3 n = normalize( x0 );
        vec2 coords = vec2( atan( n.y, n.x ), acos( n.z ) ) * vec2( 0.5, 1.0 ) / pi + vec2( 0.5, 0.0 );
        vec4 reflectance = texture( reflectanceSampler, coords ) * vec4( 0.2, 0.2, 0.2, 1.0 );

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
#endif

/*
 ===============================
 GroundColorGeo
 //ground radiance at end of ray x+tv, when sun in direction s
 //attenuated bewteen ground and viewer (=R[L0]+R[L*])
 ===============================
 */
vec3 GroundColorGeo( vec3 x, vec3 v, vec3 s, float r, float mu, vec3 normal, float shadow ) {
	const float pi = acos( -1.0 );

	// ground reflectance at end of ray, x0
	vec3 x0 = x;// + d * v;
	vec3 n = normalize( x );
	vec2 coords = vec2( atan( n.y, n.x ), acos( n.z ) ) * vec2( 0.5, 1.0 ) / pi + vec2( 0.5, 0.0 );

	// direct sun light (radiance) reaching x0
	float muS = dot( n, s );
	float muS2 = dot( normal, s );
	vec3 sunLight = TransmittanceWithShadow( radiusGround, muS, transmittanceSampler ) * shadow;

	// precomputed sky light (irradiance) (=E[L*]) at x0
	vec3 groundSkyLight = Irradiance( irradianceSampler, radiusGround, muS );
	groundSkyLight *= ( normal.z * 0.5 + 0.5 );	// The full of hemisphere of blue light is proportional to how "up" the surface points

	// light reflected at x0 (=(R[L0]+R[L*])/T(x,x0))
	vec3 groundColor = ( max( muS2, 0.0 ) * sunLight + groundSkyLight ) * ISun / pi;

	// attenuation of light to the viewer, T(x,x0)
	vec3 attenuation = vec3( 1.0, 1.0, 1.0 );//Transmittance( r, mu, v, x0, transmittanceSampler );

	vec3 result = attenuation * groundColor; //=R[L0]+R[L*]
	
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


//vec3 DirectLight( vec3 p, vec3 s ) {
//}


/*
 ===========================
 pcf4samples()
 // 4 sample PCF
 ===========================
 *
float pcf4samples( sampler2D map, vec4 shadowCoord, vec2 mapScale, float projected_vert_depth_value ) {
    float sum = 0.0;
    float x;
    float y;
    
    for ( x = -0.5; x <= 0.5; x += 1.0 ) {
        for ( y = -0.5; y <= 0.5; y += 1.0 ) {
            float depth = offset_lookup( map, shadowCoord, mapScale, vec2( x, y ) ).r;
            sum += ( depth < projected_vert_depth_value ) ? 0.0 : 1.0;
        }
    }
    
    float shadowCoeff = sum / 4.0;
    return shadowCoeff;
}

/*
 ===========================
 GetShadowingFactor()
 ===========================
 */
float GetShadowingFactor( sampler2D shadowMap, vec4 shadowCoord, float depth ) {
	float shadowDepth = textureProj( shadowMap, shadowCoord ).r;
	
    float shadow = 1.0;
    if ( shadowDepth < depth - 0.00001 ) {
		shadow = 0.0;
    }

	// Return the shadowing factor
    return shadow;
}

//out vec4 FragColor;

/*
 ===========================
 LightGroundGeometry()
 ===========================
 */
 #if 0
void LightGroundGeometry( in vec3 normal ) {
	// Convert cameraPos and v_position from meters to km
	vec3 x = cameraPos * 0.001;
	vec3 p = v_position * 0.001;
	vec3 ray = p - x;
	vec3 x = cameraPos;
    vec3 v = normalize( ray );
	float r = radiusGround + p.z;
	float mu = 0.0;
	
	// Get the world space position of the geometry
//	vec4 position = texture( s_positions, coords ).xyzw;
//	position.w = 1.0;
//	position = viewInverse * position;
//	position.w = 1.0;
//	vec4 position = vec4( v_position.xyz, 1.0 );
	
	// Get the shadow coordinates
	//vec4 posLightSpace = matBias * matViewProj * position;
	
	///float fragDepth     = posLightSpace.z / posLightSpace.w;
	
	//float shadow = GetShadowingFactor( s_shadow, posLightSpace, fragDepth );
	//float shadow = pcf4samples( s_shadow, posLightSpace, u_texMapScale, fragDepth - 0.000005 );
	
	float shadow = 1.0;	// Not in shadow
	vec3 groundColor = GroundColorGeo( x, v, sunDir, r, mu, normal.xyz, shadow ); //R[L0]+R[L*]
	
	vec3 albedo = v_color;
	
	diffuseColor.rgb = groundColor;
	diffuseColor.rgb *= albedo.rgb;
}
#endif


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
==========================
ComputeMipLevel
==========================
*/
float ComputeMipLevel( in vec2 virtCoords ) {
	const float maxAniso = 4;
	const float maxAnisoLog2 = log2( maxAniso );
	//const float virtPagesWide = 1024;
	//const float pageWidth = 128;
	const float pageBorder = 4;
	const float virtTexelsWide = virtualPagesWide * ( pageWidth - 2 * pageBorder );

	vec2 texcoords = virtCoords.xy * virtTexelsWide;
	
	vec2 dx = dFdx( texcoords );
	vec2 dy = dFdy( texcoords );
	
	float px = dot( dx, dx );
	float py = dot( dy, dy );

	float maxLod = 0.5 * log2( max( px, py ) ); // log2(sqrt()) = 0.5*log2()
	float minLod = 0.5 * log2( min( px, py ) );

	float anisoLOD = maxLod - min( maxLod - minLod, maxAnisoLog2 );
	return anisoLOD;
}

// ======================================================
// Converts the virtual texture coordinates
//	to the appropriate physical texture coordinates
// ======================================================
vec2 PhysicalCoordinates( in vec2 virtCoord ) {
	// The pageCoordinate will give a [0, 0.125] range.
	// This is because the phyiscal texture is only 32 slots wide, 8 bits per id, so 32/256 is 0.125
	// So we need to multiply by 256/32 (or 8) to expand it to the full [0,1] range
	vec2 pageCoord = texture2D( pageCoordTexture, virtCoord ).zw * 256.0f / float( pageCount );
	vec4 scaleBias = texture2D( scaleBiasTexture, pageCoord );
	vec2 physCoord = virtCoord.xy * scaleBias.x + scaleBias.zw;
	return physCoord;
}

vec2 PhysicalCoordinates( in vec2 virtCoord, in float mip ) {
	// The pageCoordinate will give a [0, 0.125] range.
	// This is because the phyiscal texture is only 32 slots wide, 8 bits per id, so 32/256 is 0.125
	// So we need to multiply by 256/32 (or 8) to expand it to the full [0,1] range
	vec2 pageCoord = textureLod( pageCoordTexture, virtCoord, mip ).zw * 256.0f / float( pageCount );
	vec4 scaleBias = texture2D( scaleBiasTexture, pageCoord );
	vec2 physCoord = virtCoord.xy * scaleBias.x + scaleBias.zw;
	return physCoord;
}

/*
==========================
main
==========================
*/
void main() {
	diffuseColor.a = 1.0;

	// Check if this fragment is too far from the camera
	if ( v_distance > virtualDistance ) {
		diffuseColor.rgb = v_color;
		//diffuseColor.rgb = vec3( 1, 0, 1 );
	} else {
#if 1
		vec2 physCoords = PhysicalCoordinates( v_texCoord );
#else	
		float mipLevel = ComputeMipLevel( v_texCoord );
		vec2 physCoords = PhysicalCoordinates( v_texCoord, mipLevel );
#endif
		diffuseColor = texture2D( physicalTexture, physCoords );
	}
	vec3 textureColor = diffuseColor.rgb;

#if 1
	vec3 x = cameraPos * 0.001 + vec3( 0, 0, radiusGround );		// convert from meters to km
	vec3 p = v_position * 0.001 + vec3( 0, 0, radiusGround );	// convert from meters to km
	vec3 ray = p - x;
	float d = length( ray );
	vec3 v = normalize( ray );

	float r = length( x );
	float mu = dot( v, x ) / r;

	// This is the transmittance of the terrain point to the camera
	//vec3 Transmittance( in float radius, in float cosAngle, in float d, in sampler2D sampler )
	vec3 trans = Transmittance( r, mu, d, transmittanceSampler );

	// Get the transmittance of the sun to the terrain point
	float r2 = length( p );
	float mu2 = dot( sunDir, p ) / r2;
	vec3 trans2 = Transmittance( r2, mu, d, transmittanceSampler );

	vec3 color = textureColor * dot( sunDir, v_normal ) * trans2;	// TODO: Calculate how much sunlight makes it to this point via transmittance

	diffuseColor.rgb = color * trans;
	//diffuseColor.rgb = trans;
	
	float shadow = 1.0;
	diffuseColor.rgb = GroundColorGeo( x, v, sunDir, r, mu, v_normal, shadow ) * trans2 * textureColor;

	// TODO: Add in the Inscatter color
	//diffuseColor.rgb += InscatterGeo( x, v, sunDir, r, mu );
	vec3 ray2 = v_position - cameraPos;
	float d2 = length( ray2 ) * 0.001;
	diffuseColor.rgb += Inscatter( x, v, sunDir, r, mu );// * d2 * d2;

	diffuseColor.rgb = HDR( diffuseColor.rgb );
	//diffuseColor.r = 1.0;
#else
	float light = dot( normalize( vec3( 1, 1, 1 ) ), v_normal ) * 0.75f + 0.25f;
	//light = 1.0;
	diffuseColor.rgb *= light;
#endif
}

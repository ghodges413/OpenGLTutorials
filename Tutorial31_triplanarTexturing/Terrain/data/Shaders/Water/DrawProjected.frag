#version 450

in vec3	v_worldSpacPos;
in vec2 v_texCoords[ 4 ];
in vec4 v_weights;

uniform sampler2D   s_normals0;
uniform sampler2D   s_normals1;
uniform sampler2D   s_normals2;
uniform sampler2D   s_normals3;

uniform vec3 camPos;
uniform vec3 dirToSun;

out vec4 FragColor;

/*
 ==========================
 SnellsLaw
 L = vector to surface
 N = normal
 n1 = index of refraction
 n2 = index of refraction
 ==========================
 */
vec3 SnellsLaw( in vec3 L, in vec3 N, in float n1, in float n2 ) {
	float cosTheta1 = -dot( N, L );
	float r = n1 / n2;
	float cosTheta2 = sqrt( 1.0f - r * r * ( 1.0f - cosTheta1 * cosTheta1 ) );
	vec3 refract = r * L + ( r * cosTheta1 - cosTheta2 ) * N;
	return normalize( refract );
}

/*
 ==========================
 FresnelEquations
 L = vector to surface
 N = normal
 n1 = index of refraction
 n2 = index of refraction
 ==========================
 */
vec2 FresnelEquations( in vec3 L, in vec3 N, in float n1, in float n2 ) {
	float cosTheta1 = -dot( N, L );
	float r = n1 / n2;
	float cosTheta2 = sqrt( 1.0f - r * r * ( 1.0f - cosTheta1 * cosTheta1 ) );
	
	float Rs = ( n1 * cosTheta1 - n2 * cosTheta2 ) / ( n1 * cosTheta1 + n2 * cosTheta2 );
	Rs = Rs * Rs;
	
	float Rp = ( n1 * cosTheta2 - n2 * cosTheta1 ) / ( n1 * cosTheta2 + n2 * cosTheta1 );
	Rp = Rp * Rp;
	
	float R = ( Rs + Rp ) / 2.0f;
	float T = 1.0f - R;
	return vec2( R, T );
}

/*
==========================
GetSkyColor
==========================
*/
vec3 GetSkyColor( in vec3 e ) {
    e.z = max( e.z, 0.0f );
    vec3 ret;
    ret.x = pow( 1.0f - e.z, 2.0f );
    ret.y = 0.6f + ( 1.0f - e.z ) * 0.4f;
	ret.z = 1.0f - e.z;
    return ret;
}

/*
 ==========================
 main
 ==========================
 */
void main() {
	vec4 normals[ 4 ];
	normals[ 0 ] = texture( s_normals0, v_texCoords[ 0 ] ).xyzw;
	normals[ 1 ] = texture( s_normals1, v_texCoords[ 1 ] ).xyzw;
	normals[ 2 ] = texture( s_normals2, v_texCoords[ 2 ] ).xyzw;
	normals[ 3 ] = texture( s_normals3, v_texCoords[ 3 ] ).xyzw;
	vec4 normal = vec4( 0.0f );
	for ( int i = 0; i < 4; i++ ) {
		normal += normals[ i ] * v_weights[ i ];
	}
	
	
	// Get the HDR color from the floating point diffuse buffer
	float Jacobian	= max( 0.0f, 0.5f - normals[ 0 ].w );
	float distToCam2D = length( camPos.xy - v_worldSpacPos.xy );
	Jacobian *= exp( -distToCam2D * 0.075f );
	
	vec3 dirToCam = normalize( camPos - v_worldSpacPos );
	vec3 lookDir = -dirToCam;
	vec3 reflection = lookDir + 2.0f * normal.xyz;
	vec3 transmission = SnellsLaw( lookDir, normal.xyz, 1.0f, 1.33f );
	vec2 RT = FresnelEquations( lookDir, normal.xyz, 1.0f, 1.33f );
	float R = RT.x;
	float T = RT.y;
	
	vec3 skyColor = vec3( 0.0f, 0.0f, 1.0f );
	vec3 waterColor = vec3( 0.0f, 1.0f, 0.0f );
	
	reflection.z = -reflection.z;
	skyColor = GetSkyColor( reflection );
	skyColor = GetSkyColor( lookDir );
	
	vec3 seaBase = vec3( 0.3f, 0.63f, 0.73f );
	if ( true ) {
		seaBase = vec3( 0.3f, 0.73f, 0.63f );
		seaBase *= 0.5f;
	} else {
		seaBase = vec3( 0.83f, 0.15f, 0.1f );
		seaBase *= 0.5f;
	}
	
	float tuner = 1.9f;
	waterColor = pow( seaBase, vec3( max( 0.0f, -transmission.z * tuner ) ) );// * 40.0f;
	
	float brightness = 0.0f;
	if ( dirToSun.z > 0.0f ) {
		brightness = clamp( sqrt( dirToSun.z ), 0.0f, 1.0f );
	}
	
	vec3 finalColor = skyColor * R + waterColor * T + Jacobian;
	finalColor *= brightness;
    
	//FragColor.rgb    = vec3( 1.0 ) - exp( -exposure * color.rgb );
	//FragColor.rgb	= normal.rgb;
	//FragColor.rgb	= reflection.rgb;
	FragColor.rgb	= finalColor.rgb;
    FragColor.a		= 1.0f;
}

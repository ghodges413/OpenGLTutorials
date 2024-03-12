// This line tells OpenGL which version of GLSL we're using
#version 430

in vec3 v_pos;
in vec2 v_st;
in vec3 v_normal;
in vec3 v_tangent;
in vec3 v_bitangent;
in vec3 v_color;

uniform sampler2D rasterBuffer;
//uniform sampler2DArray textureArray0;

layout( location = 0 ) out vec4 diffuseColor;
#if 0
/*
====================================================

Texture Bombing techniques as outlined by Inigo Quillez
//https://iquilezles.org/articles/texturerepetition/

Other resources:
Procedural Stochastic Textures by Tiling and Blending (Thomas Deliot and Eric Heltz)
https://eheitzresearch.wordpress.com/738-2/

====================================================
*/

vec4 Hash4( vec2 p ) {
	vec4 val;
	val.x = 1.0 + dot( p, vec2( 37.0, 17.0 ) );
	val.y = 2.0 + dot( p, vec2( 11.0, 47.0 ) );
	val.z = 3.0 + dot( p, vec2( 41.0, 29.0 ) );
	val.w = 4.0 + dot( p, vec2( 23.0, 31.0 ) );
	return fract( sin( val ) * 103.0 );
}

// Technique 1
vec4 TextureNoTile( sampler2D samp, in vec2 uv ) {
    ivec2 iuv = ivec2( floor( uv ) );
    vec2 fuv = fract( uv );

    // generate per-tile transform
    vec4 ofa = Hash4( iuv + ivec2( 0, 0 ) );
    vec4 ofb = Hash4( iuv + ivec2( 1, 0 ) );
    vec4 ofc = Hash4( iuv + ivec2( 0, 1 ) );
    vec4 ofd = Hash4( iuv + ivec2( 1, 1 ) );
    
    vec2 ddx = dFdx( uv );
    vec2 ddy = dFdy( uv );

    // transform per-tile uvs
    ofa.zw = sign( ofa.zw - 0.5 );
    ofb.zw = sign( ofb.zw - 0.5 );
    ofc.zw = sign( ofc.zw - 0.5 );
    ofd.zw = sign( ofd.zw - 0.5 );
    
    // uv's, and derivatives (for correct mipmapping)
    vec2 uva = uv * ofa.zw + ofa.xy, ddxa = ddx * ofa.zw, ddya = ddy * ofa.zw;
    vec2 uvb = uv * ofb.zw + ofb.xy, ddxb = ddx * ofb.zw, ddyb = ddy * ofb.zw;
    vec2 uvc = uv * ofc.zw + ofc.xy, ddxc = ddx * ofc.zw, ddyc = ddy * ofc.zw;
    vec2 uvd = uv * ofd.zw + ofd.xy, ddxd = ddx * ofd.zw, ddyd = ddy * ofd.zw;
        
    // fetch and blend
    vec2 b = smoothstep( 0.25,0.75, fuv );
    
	vec4 mixA = mix( textureGrad( samp, uva, ddxa, ddya ), textureGrad( samp, uvb, ddxb, ddyb ), b.x );
	vec4 mixB = mix( textureGrad( samp, uvc, ddxc, ddyc ), textureGrad( samp, uvd, ddxd, ddyd ), b.x );
    return mix( mixA, mixB, b.y );
}
vec4 TextureNoTileArray( sampler2DArray samp, in vec2 uv, in float slice ) {
    ivec2 iuv = ivec2( floor( uv ) );
    vec2 fuv = fract( uv );

    // generate per-tile transform
    vec4 ofa = Hash4( iuv + ivec2( 0, 0 ) );
    vec4 ofb = Hash4( iuv + ivec2( 1, 0 ) );
    vec4 ofc = Hash4( iuv + ivec2( 0, 1 ) );
    vec4 ofd = Hash4( iuv + ivec2( 1, 1 ) );
    
    vec2 ddx = dFdx( uv );
    vec2 ddy = dFdy( uv );

    // transform per-tile uvs
    ofa.zw = sign( ofa.zw - 0.5 );
    ofb.zw = sign( ofb.zw - 0.5 );
    ofc.zw = sign( ofc.zw - 0.5 );
    ofd.zw = sign( ofd.zw - 0.5 );
    
    // uv's, and derivatives (for correct mipmapping)
    vec2 uva = uv * ofa.zw + ofa.xy, ddxa = ddx * ofa.zw, ddya = ddy * ofa.zw;
    vec2 uvb = uv * ofb.zw + ofb.xy, ddxb = ddx * ofb.zw, ddyb = ddy * ofb.zw;
    vec2 uvc = uv * ofc.zw + ofc.xy, ddxc = ddx * ofc.zw, ddyc = ddy * ofc.zw;
    vec2 uvd = uv * ofd.zw + ofd.xy, ddxd = ddx * ofd.zw, ddyd = ddy * ofd.zw;
        
    // fetch and blend
    vec2 b = smoothstep( 0.25,0.75, fuv );
    
	vec4 mixA = mix( textureGrad( samp, vec3( uva, slice ), ddxa, ddya ), textureGrad( samp, vec3( uvb, slice ), ddxb, ddyb ), b.x );
	vec4 mixB = mix( textureGrad( samp, vec3( uvc, slice ), ddxc, ddyc ), textureGrad( samp, vec3( uvd, slice ), ddxd, ddyd ), b.x );
    return mix( mixA, mixB, b.y );
}

// Technique 2
vec3 TextureNoTile( sampler2D samp, in vec2 uv, float v ) {
    vec2 p = floor( uv );
    vec2 f = fract( uv );
	
    // derivatives (for correct mipmapping)
    vec2 ddx = dFdx( uv );
    vec2 ddy = dFdy( uv );
    
	vec3 va = vec3( 0 );
	float w1 = 0.0;
    float w2 = 0.0;
    for( int j = -1; j <= 1; j++ ) {
    	for( int i = -1; i <= 1; i++ ) {
			vec2 g = vec2( float( i ), float( j ) );
			vec4 o = Hash4( p + g );
			vec2 r = g - f + o.xy;
			float d = dot( r, r );
			float w = exp( -5.0 * d );
			vec3 c = textureGrad( samp, uv + v * o.zw, ddx, ddy ).xyz;
			va += w * c;
			w1 += w;
			w2 += w*w;
		}
	}
    
    // normal averaging --> lowers contrasts
    //return va/w1;

    // contrast preserving average
    float mean = 0.3;// textureGrad( samp, uv, ddx*16.0, ddy*16.0 ).x;
    vec3 res = mean + ( va - w1 * mean ) / sqrt( w2 );
    return mix( va / w1, res, v );
}

/*
float Sum( vec3 v ) {
	return v.x + v.y + v.z;
}

// Technique 3
vec4 TextureNoTile( sampler2D samp, in vec2 uv ) {
    // sample variation pattern    
    float k = texture( iChannel1, 0.005 * x ).x; // cheap (cache friendly) lookup    
    
    // compute index    
    float index = k * 8.0;
    float i = floor( index );
    float f = fract( index );

    // offsets for the different virtual patterns    
    vec2 offa = sin( vec2( 3.0, 7.0 ) * ( i + 0.0 ) ); // can replace with any other hash    
    vec2 offb = sin( vec2( 3.0, 7.0 ) * ( i + 1.0 ) ); // can replace with any other hash    

    // compute derivatives for mip-mapping    
    vec2 dx = dFdx( x );
	vec2 dy = dFdy( x );
    
    // sample the two closest virtual patterns    
    vec3 cola = textureGrad( iChannel0, x + offa, dx, dy ).xxx;
    vec3 colb = textureGrad( iChannel0, x + offb, dx, dy ).xxx;

    // interpolate between the two virtual patterns    
    return mix( cola, colb, smoothstep( 0.2, 0.8, f - 0.1 * Sum( cola - colb ) ) );

}
*/

/*
==========================
TriplanarMapping
==========================
*/
vec4 TriplanarMapping( vec3 pos, vec3 normal, float slice, sampler2DArray tex ) {
	float scale = 100.0;
	vec4 dx = TextureNoTileArray( tex, vec2( pos.zy / scale ), slice );
	vec4 dy = TextureNoTileArray( tex, vec2( pos.xz / scale ), slice );
	vec4 dz = TextureNoTileArray( tex, vec2( pos.xy / scale ), slice );

	vec3 weights = abs( normal.xyz );
	weights = weights / ( weights.x + weights.y + weights.z );

	vec4 color = dx * weights.x + dy * weights.y + dz * weights.z;
	return color;
}
vec4 TriplanarMapping( vec3 pos, vec3 normal, sampler2D tex ) {
	float scale = 100.0;
	vec4 dx = TextureNoTile( tex, vec2( pos.zy / scale ) );
	vec4 dy = TextureNoTile( tex, vec2( pos.xz / scale ) );
	vec4 dz = TextureNoTile( tex, vec2( pos.xy / scale ) );

	vec3 weights = abs( normal.xyz );
	weights = weights / ( weights.x + weights.y + weights.z );

	vec4 color = dx * weights.x + dy * weights.y + dz * weights.z;
	return color;
}

vec4 TiledTriplanarMapping( vec3 pos, vec3 normal, float slice, sampler2DArray tex ) {
	float scale = 100.0;
	vec4 dx = texture( tex, vec3( pos.zy / scale, slice ) );
	vec4 dy = texture( tex, vec3( pos.xz / scale, slice ) );
	vec4 dz = texture( tex, vec3( pos.xy / scale, slice ) );

	vec3 weights = abs( normal.xyz );
	weights = weights / ( weights.x + weights.y + weights.z );

	vec4 color = dx * weights.x + dy * weights.y + dz * weights.z;
	return color;
}

vec3 UnpackNormal( vec3 n ) {
    n.x = ( n.x - 0.5 ) * 2.0;
    n.y = -( n.y - 0.5 ) * 2.0;  // TODO: double check if this needs to be negative or not
    return n;
}

vec3 TiledTriplanarNormalMapping( vec3 pos, vec3 normal, float slice, sampler2DArray tex ) {
	float scale = 100.0;
	vec4 dx = texture( tex, vec3( pos.zy / scale, slice ) );
	vec4 dy = texture( tex, vec3( pos.xz / scale, slice ) );
	vec4 dz = texture( tex, vec3( pos.xy / scale, slice ) );

	vec3 weights = abs( normal.xyz );
	weights = weights / ( weights.x + weights.y + weights.z );

    // GPU Gems 3 blend (https://bgolus.medium.com/normal-mapping-for-a-triplanar-shader-10bf39dca05a)
    // We also might want to look into an alternative:
    // https://mmikkelsen3d.blogspot.com/2011/07/derivative-maps.html
    // Perhaps mikktspace is better suited for triplanar mapping
    // Tangent space normal maps
    vec3 tnormalX = UnpackNormal( dx.xyz );
    vec3 tnormalY = UnpackNormal( dy.xyz );
    vec3 tnormalZ = UnpackNormal( dz.xyz );

    // Swizzle tangemt normals into world space and zero out "z"
    vec3 normalX = vec3( 0.0, tnormalX.yx );
    vec3 normalY = vec3( tnormalY.x, 0.0, tnormalY.y );
    vec3 normalZ = vec3( tnormalZ.xy, 0.0 );

    vec3 worldNormal = normalX.xyz * weights.x + normalY.xyz * weights.y + normalZ.xyz * weights.z + normal;
    return normalize( worldNormal );
}


// Normal conversion from CoCg_Y to RGB
vec4 CoCg_YtoRGB( vec4 color ) {
    float Co = color.x - ( 0.5 * 256.0 / 255.0 );
    float Cg = color.y - ( 0.5 * 256.0 / 255.0 );
    float Y = color.w;
    float R = Y + Co - Cg;
    float G = Y + Cg;
    float B = Y - Co - Cg;
    return vec4( R, G, B, 1 );
}

// Greyscaled conversion from CoCg_Y to RGB (the version we will be using)
vec4 ScaledCoCg_YtoRGB( vec4 color ) {
    float scale = ( color.z * ( 255.0 / 8.0 ) ) + 1.0;
    float Co = ( color.x - ( 0.5 * 256.0 / 255.0 ) ) / scale;
    float Cg = ( color.y - ( 0.5 * 256.0 / 255.0 ) ) / scale;
    float Y = color.w;
    float R = Y + Co - Cg;
    float G = Y + Cg;
    float B = Y - Co - Cg;
    return vec4( R, G, B, 1 );
}
#endif
/*
==========================
main
==========================
*/
void main() {
	float slice = ( v_pos.z + 50.0 ) / 50.0;

	float slice0 = clamp( floor( slice ), 0.0, 4.0 );
	float slice1 = clamp( ceil( slice ), 0.0, 4.0 );
	float t = fract( slice );
	t = smoothstep( 0.0, 1.0, t );

    vec3 normal = v_normal.xyz;
#if 1
    vec4 color = texture( rasterBuffer, v_st );
#elif 0
    vec4 color = TriplanarMapping( v_pos, v_normal, textureArray0 );
#elif 1
    // Triplanar mapping + texture bombing
	vec4 color = TriplanarMapping( v_pos, v_normal, slice0 * 2.0 + 0.0, textureArray0 );
	vec4 colorB = TriplanarMapping( v_pos, v_normal, slice1 * 2.0 + 0.0, textureArray0 );
	color = mix( color, colorB, t );
#else
    vec4 color = TiledTriplanarMapping( v_pos, v_normal, slice0 * 2.0 + 0.0, textureArray0 );
	vec4 colorB = TiledTriplanarMapping( v_pos, v_normal, slice1 * 2.0 + 0.0, textureArray0 );
	color = mix( color, colorB, t );

    vec3 normalA = TiledTriplanarNormalMapping( v_pos, v_normal, slice0 * 2.0 + 0.0, textureArray0 );
	vec3 normalB = TiledTriplanarNormalMapping( v_pos, v_normal, slice1 * 2.0 + 0.0, textureArray0 );
	normal = mix( normalA, normalB, t );
#endif

	//color.rgb *= max( dot( sunDir, normal ), 0.1 );
	diffuseColor = color;
}
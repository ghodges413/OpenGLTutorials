#version 330

in vec4        v_position;
in vec4        v_normal;
in vec4        v_tangent;
in vec2        v_texCoord;

uniform sampler2D   s_diffuse;
uniform sampler2D   s_normal;

layout( location = 0 ) out vec4 FragData[ 3 ];

/*
 ==========================
 main
 ==========================
 */
void main() {
    // store diffuse color
    FragData[ 0 ] = texture( s_diffuse, v_texCoord );

    // store position data
    FragData[ 1 ] = vec4( v_position.xyz, 1.0 );
    
    //
    // Transform the normal from tangent space to view space
    //
    vec3 norm   = normalize( v_normal.xyz );
    vec3 tang   = normalize( v_tangent.xyz );
    vec3 binorm = cross( norm, tang );
    
    vec3 normal = ( 2.0 * texture( s_normal, v_texCoord ).rgb ) - 1.0;
    normal      = normalize( normal );

    vec3 finalNorm  = normal.x * tang + normal.y * binorm + normal.z * norm;
    finalNorm       = normalize( finalNorm );
    
    // store the normal data
    FragData[ 2 ] = vec4( finalNorm, 1.0 );
    
    // store specular color
    //FragData[ 3 ] = texture( s_specular, v_texCoord );
    
    // store the tang data
    //binorm  = normalize( cross( finalNorm, tang ) );
    //tang    = normalize( cross( binorm, finalNorm ) );
    //FragData[ 4 ] = vec4( tang, 1.0 );
}


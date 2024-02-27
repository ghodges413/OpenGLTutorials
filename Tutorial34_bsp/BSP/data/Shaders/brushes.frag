// This line tells OpenGL which version of GLSL we're using
#version 430

in vec3 v_pos;
in vec2 v_st;
in vec3 v_normal;
in vec3 v_tangent;
in vec3 v_bitangent;
in vec3 v_color;

uniform sampler2DArray textureArray0;

layout( location = 0 ) out vec4 diffuseColor;

/*
==========================
main
==========================
*/
void main() {
    vec3 normal = v_normal.xyz;
    float z = 0;
    if ( normal.z > 0.5 ) {
        // floor texture
        z = 1;
    }
    if ( normal.z < -0.5 ) {
        // ceiling texture
        z = 2;
    }
    vec4 color = texture( textureArray0, vec3( v_st, z ) );
    color.rgb *= v_color.rgb;

	diffuseColor = color;
}

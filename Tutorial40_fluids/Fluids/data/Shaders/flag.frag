// This line tells OpenGL which version of GLSL we're using
#version 430

//layout( location = 0 ) in vec2 v_st;

in vec3 v_pos;
in vec2 v_st;
in vec3 v_normal;
in vec3 v_tangent;
in vec3 v_bitangent;
in vec3 v_color;

uniform sampler2D flagTexture;

layout( location = 0 ) out vec4 diffuseColor;

/*
==========================
main
==========================
*/
void main() {
    vec4 color = texture( flagTexture, v_st );
	diffuseColor = color;
}

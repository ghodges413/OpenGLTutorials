#version 330

in vec4 v_posLightSpace;

// the light's projection matrix
uniform sampler2D s_light_proj_texture;

/*
 ==========================
 main
 ==========================
 */
void main() {
    // Get the color of the projected texture (the spotlight texture)
    vec4 projectedFrag = texture2DProj( s_light_proj_texture, v_posLightSpace );
    gl_FragColor = projectedFrag;
}

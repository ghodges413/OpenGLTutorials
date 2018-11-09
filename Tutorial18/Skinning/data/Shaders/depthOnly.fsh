#version 330

out vec4 FragColor;

/*
==========================
main
==========================
*/
void main() {
	gl_FragDepth = gl_FragCoord.z;
}

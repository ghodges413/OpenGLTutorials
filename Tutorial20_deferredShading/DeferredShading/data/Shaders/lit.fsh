// This lines tells OpenGL which version of GLSL we're using
#version 330

// This defines the 2D texture
uniform sampler2D s_textureDiffuse;
uniform sampler2D s_textureNormals;
uniform vec3 rayToLight;
uniform vec3 lightColor;

// This is the incoming color that came from the vertex shader
in vec2 v_texCoord;
in vec3 v_normal;
in vec3 v_tangent;

// This is the outgoing color that'll be the final color for output
out vec4 FragColor;

/*
==========================
main
==========================
*/
void main() {
	vec3 colorDiffuse = texture( s_textureDiffuse, v_texCoord ).rgb;
	vec3 colorNormal = texture( s_textureNormals, v_texCoord ).rgb;
	
	// Get the normal from the normal map
	vec3 normal = normalize( ( colorNormal.xyz - 0.5 ) * 2.0 );
	
	vec3 norm = normalize( v_normal );
	vec3 tang = normalize( v_tangent );
	vec3 bitang = normalize( cross( norm, tang ) );
	
	// Transform the normal map's normal from tangent space to world space
	normal = normal.x * tang + normal.y * bitang + normal.z * norm;

	// Calculate the shading from the light
	float shading = dot( normal.xyz, rayToLight.xyz );
	
	// Use the texture coordinates to look up the texel value in the texture and copy out to the fragment
	FragColor.rgb = colorDiffuse.rgb * shading * lightColor;
	FragColor.a = 1.0;
	
	//FragColor.rgb = normal.xyz;
}

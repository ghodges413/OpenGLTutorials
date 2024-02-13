//
//  winmain.cpp
//
#include <string>
#include <stdio.h>

#ifdef WINDOWS
#include <GL/glew.h>
#include <GL/glut.h>
#else
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <OpenGL/glu.h>
#endif

#include "Math/Vector.h"
#include "Math/MatrixOps.h"
#include "Math/Frustum.h"

#include "Graphics/Mesh.h"
#include "Graphics/Targa.h"
#include "Graphics/Texture.h"
#include "Graphics/Shader.h"
#include "Graphics/RenderSurface.h"
#include "Graphics/ShaderManager.h"
#include "Graphics/TextureManager.h"

#include "Miscellaneous/Time.h"
#include "Miscellaneous/Input.h"
#include "Terrain/Terrain.h"
#include "VirtualTexture/VirtualTexture.h"
#include "Atmosphere/BuildBruneton.h"
#include "Water/Ocean.h"
#include "Clouds/Clouds.h"

// Global storage of the window size
// const int g_screenWidth  = 1200;
// const int g_screenHeight = 720;
int g_screenWidth  = 1920;
int g_screenHeight = 1080;

Mesh g_modelScreenSpaceFarPlane;
Mesh g_modelScreenSpaceNearPlane;

VirtualTexture g_virtualTexture;
Terrain g_terrain;

Shader g_shaderDepthOnly;
RenderSurface g_renderSurface;

Shader g_shaderTerrainTextured;
Shader g_shaderTerrainDebug;

Vec3d g_cameraPos( 0, 0, 10000 );
Vec3d g_cameraUp( 0, 0, 1 );
Vec3d g_cameraLook( 1, 0, 0 );
float g_matView[ 16 ];

// These are only used in first person mode
float g_cameraTheta = 0;
float g_cameraPhi = 3.14f * 0.5f;

bool g_noclip = false;
float g_dtSec = 0;
float g_timeTime = 0;

atmosphereBuildData_t g_atmos;
float g_sunAngle = 3.14f * 0.25f;

Texture * g_textureArray = NULL;

float g_timeMS = 0;
void Update() {
	// Calculate the timing
	static int timeLastFrame	= 0;
	const int time				= GetTimeMicroseconds();
	const float dt_us			= time - timeLastFrame;
	const float dt_ms			= dt_us * 0.001f;
	timeLastFrame				= time;
	static float runTimeSeconds = 0;
	runTimeSeconds += dt_ms * 0.001f;
	g_dtSec = dt_ms * 0.001f;
	g_timeTime += g_dtSec;

	// Update the ocean water
	g_timeMS += dt_ms;
	{
		OceanUpdateParms_t parms;
		parms.m_timeMS = g_timeMS;
		OceanUpdate( parms );
	}

	//
	//	Input Updates
	//
	{
		Vec3d right = g_cameraLook.Cross( Vec3d( 0, 0, 1 ) );
		right.Normalize();

		Vec3d dir = Vec3d( 0 );
		if ( g_keyboard.IsKeyDown( 'w' ) || g_keyboard.IsKeyDown( 'W' ) ) {
			dir += g_cameraLook;
		}
		if ( g_keyboard.IsKeyDown( 's' ) || g_keyboard.IsKeyDown( 'S' ) ) {
			dir -= g_cameraLook;
		}
		if ( g_keyboard.IsKeyDown( 'd' ) || g_keyboard.IsKeyDown( 'D' ) ) {
			dir += right;
		}
		if ( g_keyboard.IsKeyDown( 'a' ) || g_keyboard.IsKeyDown( 'A' ) ) {
			dir -= right;
		}

		if ( g_keyboard.IsKeyDown( 'e' ) || g_keyboard.IsKeyDown( 'E' ) ) {
			dir += Vec3d( 0, 0, 100 );
		}
		if ( g_keyboard.IsKeyDown( 'q' ) || g_keyboard.IsKeyDown( 'Q' ) ) {
			dir -= Vec3d( 0, 0, 100 );
		}

		float deltaSun = 0;
		if ( g_keyboard.IsKeyDown( 'o' ) || g_keyboard.IsKeyDown( 'O' ) ) {
			deltaSun += 3;
		}
		if ( g_keyboard.IsKeyDown( 'p' ) || g_keyboard.IsKeyDown( 'P' ) ) {
			deltaSun -= 3;
		}
		g_sunAngle += deltaSun * g_dtSec;

		if ( g_keyboard.WasKeyDown( 'n' ) ) {
			g_noclip = !g_noclip;
		}

		float speed = 10.0f;
		if ( g_noclip ) {
			speed = 1000.0f;
		}
		if ( g_keyboard.IsKeyDown( 'r' ) ) {
			speed *= 10.0f;
		}
		g_cameraPos += dir * g_dtSec * speed;
	}

	//
	//	Update mouse inputs
	//
	{
		Vec3d ds = g_mouse.GLPosDelta();
		float dtX = -ds.x;
		float dtY = -ds.y;

		if ( fabsf( dtX ) > 100 ) {
			dtX = 0;
		}
		if ( fabsf( dtY ) > 100 ) {
			dtY = 0;
		}

		g_cameraTheta += dtX * g_dtSec;
		g_cameraPhi += dtY * g_dtSec;
	}

	//
	//	Update camera matrices
	//
	{
		// Update the camera view matrix
		const float radius = 1000;
	#if 0
		const float angle = runTimeSeconds * 0.0613f;
		g_cameraPos	= Vec3d( radius * cosf( angle ), radius * sinf( angle ), 300 );
		Vec3d lookat = Vec3d( 0, 0, 50 );
	#elif 0
		const float angle = runTimeSeconds * 0.013f;
		g_cameraPos	= Vec3d( radius * cosf( angle ), radius * sinf( angle ), 400 );
		g_cameraPos = g_terrain.GetSurfacePos( g_cameraPos ) + Vec3d( 0, 0, 2 );
		g_cameraUp	= Vec3d( 0, 0, 1 );
 		Vec3d lookat = g_cameraPos * 0.95f;
		lookat = g_cameraPos;
		lookat.x += -sinf( angle ) * 30.0f;
		lookat.y += cosf( angle ) * 30.0f;
 		lookat = g_terrain.GetSurfacePos( lookat ) + Vec3d( 0, 0, 2 );
	#else
		const float pi = acosf( -1.0f );
		if ( g_cameraPhi > pi * 0.9f ) {
			g_cameraPhi = pi * 0.9f;
		}
		if ( g_cameraPhi < pi * 0.1f ) {
			g_cameraPhi = pi * 0.1f;
		}

	#if 0
		float theta = -g_cameraTheta;
		float phi = -g_cameraPhi;

		Vec3d spherePos = Vec3d( cosf( theta ) * sinf( phi ), sinf( theta ) * sinf( phi ), cosf( phi ) );
		g_cameraPos = spherePos * 1000.0f;
		g_cameraLook = spherePos * -1.0f;
	#else
		g_cameraLook = Vec3d( cosf( g_cameraTheta ) * sinf( g_cameraPhi ), sinf( g_cameraTheta ) * sinf( g_cameraPhi ), cosf( g_cameraPhi ) );
		if ( !g_noclip ) {
			g_cameraPos = g_terrain.GetSurfacePos( g_cameraPos ) + Vec3d( 0, 0, 2 );
		}
	// 	const float upPhi = g_cameraPhi + pi * 0.5f;
	// 	g_cameraUp = Vec3d( cosf( g_cameraTheta ) * sinf( upPhi ), sinf( g_cameraTheta ) * sinf( upPhi ), cosf( upPhi ) );
	#endif
		Vec3d right = g_cameraLook.Cross( Vec3d( 0, 0, 1 ) );
		g_cameraUp = right.Cross( g_cameraLook );
		g_cameraUp.Normalize();




		Vec3d lookat = g_cameraPos + g_cameraLook;
	#endif
		myLookAt( g_cameraPos, lookat, g_cameraUp, g_matView );
	}
}

/*
 ================================
 DrawFrame

 This is the function that is repeatedly called.
 And it is where all the magic happens.
 ================================
 */

void DrawFrame( void ) {
	Update();

	const float bias_matrix[ 16 ] = {
		0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.5f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f
	};

	const Vec3d sunDir = Vec3d( cosf( g_sunAngle ), 0.0f, sinf( g_sunAngle ) );


	// This sets the perspective projection matrix
	float matProj[ 16 ] = { 0 };
	const float fieldOfViewDegrees = 45;
	const float aspectRatio = static_cast< float >( g_screenHeight ) / static_cast< float >( g_screenWidth );
	const float nearDepth = 1;
	const float farDepth = 10000;
	myPerspective( fieldOfViewDegrees, aspectRatio, nearDepth, farDepth, matProj );

	
	Frustum viewFrustum;
	viewFrustum.Build( matProj, g_matView );

	g_terrain.Update( g_cameraPos, viewFrustum );

	//
	//	Perform Sampler Feedback for Virtual Texture
	//
#if 0
	g_virtualTexture.BeginSamplerFeedBack();
	{
		Shader * feedbackShader = g_virtualTexture.GetFeedbackShader();

		float matIdentity[ 16 ] = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		};

		// Send the transformation matrix to the shader
		feedbackShader->SetUniformMatrix4f( "matModelWorld", 1, false, matIdentity );

		// Send the projection matrix to the shader
		feedbackShader->SetUniformMatrix4f( "matProj", 1, false, matProj );
		feedbackShader->SetUniformMatrix4f( "matView", 1, false, g_matView );

		// Draw the room
		g_terrain.Draw();
	}
	g_virtualTexture.EndSamplerFeedBack();
#endif
	// Bind the off screen render surface for rendering
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, g_renderSurface.GetFBO() );

	// Enabling color writing to the frame buffer
	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );

	// Enable writing to the depth buffer
	glDepthMask( GL_TRUE );

	// Set the depth test function to less than or equal
	glDepthFunc( GL_LEQUAL );

	// Enable testing against the depth buffer
	glEnable( GL_DEPTH_TEST );

	// Disable blending
	glDisable( GL_BLEND );
	
	// Set the OpenGL viewport to be the entire size of the window
	glViewport( 0, 0, g_screenWidth, g_screenHeight );

	// Clear the frame buffer color
	// Inputs are Red Green Blue Alpha (RGBA)
	glClearColor( 0.0f, 0.0f, 1.0f, 1.0f );

	// Clear previous frame values
	glClear( GL_COLOR_BUFFER_BIT );
	glClear( GL_DEPTH_BUFFER_BIT );

	// This will turn on back face culling.  Uncomment to turn on.
	glCullFace( GL_BACK );
	glEnable( GL_CULL_FACE );
	glDisable( GL_CULL_FACE );

	//
	//	Fill the depth buffer
	//
	{
		// Set the shader program that'll be used to render the mesh
		g_shaderDepthOnly.UseProgram();
	
		float matIdentity[ 16 ] = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		};

		// Send the transformation matrix to the shader
		g_shaderDepthOnly.SetUniformMatrix4f( "matModelWorld", 1, false, matIdentity );

		// Send the projection matrix to the shader
		g_shaderDepthOnly.SetUniformMatrix4f( "matProj", 1, false, matProj );
		g_shaderDepthOnly.SetUniformMatrix4f( "matView", 1, false, g_matView );

		// Draw the terrain
		g_terrain.Draw();
	}

	//
	//	Fill depth with water
	//
	{
		float matViewProj[ 16 ] = { 0 };
		myMatrixMultiply( g_matView, matProj, matViewProj );

		OceanFillGBufferParms_t parms;
		parms.m_camPos = g_cameraPos;
		parms.m_dirToSun = sunDir;
		parms.m_matView = g_matView;
		parms.m_matProj = matProj;
		parms.m_matViewProj = matViewProj;
		OceanFillGBuffer( parms );
	}

	//
	//	Draw the atmosphere
	//
	{
		float * proj = matProj;

		Vec3d camPos2 = g_cameraPos;
		camPos2 *= 0.001f;	// Convert from meters to kilometers
		camPos2.z += g_atmos.radiusGround + 1.0f;

		float projInverse[ 16 ];
 		myMatrixInverse4x4( matProj, projInverse );

 		float viewInverse[ 16 ];
 		myMatrixInverse4x4( g_matView, viewInverse );

		Shader * shader = g_shaderManager->GetAndUseShader( "Atmosphere/earth" );
		shader->UseProgram();
		shader->SetUniform3f( "c", 1, camPos2.ToPtr() );
		shader->SetUniform3f( "s", 1, sunDir.ToPtr() );
		shader->SetUniformMatrix4f( "projInverse", 1, false, projInverse );
		shader->SetUniformMatrix4f( "viewInverse", 1, false, viewInverse );
		const float exposure = 0.5f;
		shader->SetUniform1f( "exposure", 1, &exposure );

		const float radiusGround = g_atmos.radiusGround;
		const float radiusTop = g_atmos.radiusTop;
		shader->SetUniform1f( "radiusGround", 1, &radiusGround );
		shader->SetUniform1f( "radiusTop", 1, &radiusTop );

		const Vec3d betaRayleighScatter = g_atmos.betaRayleighScatter;
		shader->SetUniform3f( "betaRayleighScatter", 1, betaRayleighScatter.ToPtr() );

		const float mieG = g_atmos.mieG;
		shader->SetUniform1f( "mieG", 1, &mieG );

		shader->SetAndBindUniformTexture( "transmittanceSampler", 0, GL_TEXTURE_2D, g_transmittanceTexture->GetName() );
		shader->SetAndBindUniformTexture( "irradianceSampler", 1, GL_TEXTURE_2D, g_groundIrradianceTexture->GetName() );
		shader->SetAndBindUniformTexture( "inscatterSampler", 2, GL_TEXTURE_3D, g_inscatterTexture->GetName() );
		g_modelScreenSpaceFarPlane.Draw();
	}

	//
	//	Draw the clouds
	//
#if 0
	{
		CloudDrawParms_t parms;
		parms.m_matProj = matProj;
		parms.m_matView = g_matView;
		parms.m_camPos = g_cameraPos;
		parms.m_dirToSun = sunDir;
		parms.m_time = g_timeTime;
		CloudDraw( parms );
	}
#endif

	//
	//	Draw the sunlit megatextured terrain
	//
#if 0
	g_virtualTexture.BeginMegaTextureDraw();
	{
		// Turn off writing to the depth buffer, but keep using it
		glDepthMask( GL_FALSE );
		Shader * shader = g_virtualTexture.GetMegatextureShader();

		float matIdentity[ 16 ] = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		};

		Vec3d camPos2 = g_cameraPos;
		camPos2 *= 0.001f;	// Convert from meters to kilometers
		camPos2.z += g_atmos.radiusGround + 1.0f;

		float projInverse[ 16 ];
 		myMatrixInverse4x4( matProj, projInverse );

 		float viewInverse[ 16 ];
 		myMatrixInverse4x4( g_matView, viewInverse );

		shader->SetUniform3f( "cameraPos", 1, camPos2.ToPtr() );
		shader->SetUniform3f( "sunDir", 1, sunDir.ToPtr() );

		const float exposure = 0.5f;
		shader->SetUniform1f( "exposure", 1, &exposure );

		const float radiusGround = g_atmos.radiusGround;
		const float radiusTop = g_atmos.radiusTop;
		shader->SetUniform1f( "radiusGround", 1, &radiusGround );
		shader->SetUniform1f( "radiusTop", 1, &radiusTop );
 
		const Vec3d betaRayleighScatter = g_atmos.betaRayleighScatter;
		shader->SetUniform3f( "betaRayleighScatter", 1, betaRayleighScatter.ToPtr() );

		const float mieG = g_atmos.mieG;
		shader->SetUniform1f( "mieG", 1, &mieG );

		shader->SetAndBindUniformTexture( "transmittanceSampler", 3, GL_TEXTURE_2D, g_transmittanceTexture->GetName() );
		shader->SetAndBindUniformTexture( "irradianceSampler", 4, GL_TEXTURE_2D, g_groundIrradianceTexture->GetName() );
		shader->SetAndBindUniformTexture( "inscatterSampler", 5, GL_TEXTURE_3D, g_inscatterTexture->GetName() );

		// Send the transformation matrix to the shader
		shader->SetUniformMatrix4f( "matModelWorld", 1, false, matIdentity );

		// Send the projection matrix to the shader
		shader->SetUniformMatrix4f( "matProj", 1, false, matProj );
		shader->SetUniformMatrix4f( "matView", 1, false, g_matView );

		// Draw the terrain
		g_terrain.Draw();

		myglGetError();
	}
#else
	//
	//	Draw triplanar textured terrain
	//
	{
		// Set the shader program that'll be used to render the mesh
		Shader * shader = g_shaderManager->GetAndUseShader( "triplanarTexturing" );
	
		float matIdentity[ 16 ] = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		};

		shader->SetUniform3f( "sunDir", 1, sunDir.ToPtr() );

		// Send the transformation matrix to the shader
		shader->SetUniformMatrix4f( "matModelWorld", 1, false, matIdentity );

		// Send the projection matrix to the shader
		shader->SetUniformMatrix4f( "matProj", 1, false, matProj );
		shader->SetUniformMatrix4f( "matView", 1, false, g_matView );

		//shader->SetAndBindUniformTexture( "texture0", 0, GL_TEXTURE_2D, g_texture.GetName() );
		//shader->SetAndBindUniformTexture( "textureArray0", 0, GL_TEXTURE_2D, g_textureArray->GetName() );
		shader->SetAndBindUniformTexture( "textureArray0", 0, GL_TEXTURE_2D_ARRAY, g_textureArray->GetName() );

		// Draw the terrain
		g_terrain.Draw();
	}
#endif

	//
	//	Draw Debug Terrain
	//
#if 0
	{
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

		// Turn off writing to the depth buffer, but keep using it
		glDepthMask( GL_FALSE );
		//glDisable( GL_DEPTH_TEST );

		g_shaderTerrainDebug.UseProgram();

		float matIdentity[ 16 ] = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		};

		// Send the transformation matrix to the shader
		g_shaderTerrainDebug.SetUniformMatrix4f( "matModelWorld", 1, false, matIdentity );

		// Send the projection matrix to the shader
		g_shaderTerrainDebug.SetUniformMatrix4f( "matProj", 1, false, matProj );
		g_shaderTerrainDebug.SetUniformMatrix4f( "matView", 1, false, g_matView );

		// Draw the terrain
		g_terrain.DrawDebug( &g_shaderTerrainDebug );

		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	}
#endif

	//
	//	Draw the ocean
	//
	{
		// concatenate these matrices
		float matViewProj[ 16 ] = { 0 };
		myMatrixMultiply( g_matView, matProj, matViewProj );

		OceanDrawParms_t parms;
		parms.m_camPos = g_cameraPos;
		parms.m_dirToSun = sunDir;
		parms.m_matView = g_matView;
		parms.m_matProj = matProj;
		parms.m_matViewProj = matViewProj;
		OceanDraw( parms );
	}

    //
    //  Copy the render buffer's color buffer to the primary framebuffer
    //
    {
        const int width         = g_screenWidth;
        const int height        = g_screenHeight;
		const unsigned int fbo  = g_renderSurface.GetFBO();
		
        glBindFramebufferEXT( GL_READ_FRAMEBUFFER_EXT, fbo );
        glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, 0 );
        glBlitFramebufferEXT(   0, 0, width, height,
                                0, 0, width, height,
                                GL_COLOR_BUFFER_BIT,
                                GL_NEAREST );
        glBindFramebufferEXT( GL_READ_FRAMEBUFFER_EXT, 0 );
        glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
        myglGetError();
    }

	//
	//	Finish and swap buffers
	//
    
	// Tell OpenGL to finish all the previous OpenGL commands before continuing
	glFinish();

	// Swap the back buffer to the front buffer (display all the OpenGL commands that just completed).
	glutSwapBuffers();
}


/*
 ================================
 reshape
 ================================
 */
void reshape( int w, int h ) {
    // TODO:    implement me
}

int ColorAverage( int a, int b, int c, int d ) {
	float f = a * a + b * b + c * c + d * d;
	f *= 0.25f;
	f = sqrtf( f );
	return (int)f;
}

void MipInPlace( unsigned char * data, int width ) {
	int target = 0;
	for ( int y = 0; y < width - 1; y += 2 ) {
		for ( int x = 0; x < width - 1; x += 2 ) {
			int source[ 4 ];
			source[ 0 ] = x + y * width;
			source[ 1 ] = source[ 0 ] + 1;
			source[ 2 ] = source[ 0 ] + width;
			source[ 3 ] = source[ 0 ] + width + 1;

#if 1
			// Block average mipmapping
			int R = data[ source[ 0 ] * 4 + 0 ] + data[ source[ 1 ] * 4 + 0 ] + data[ source[ 2 ] * 4 + 0 ] + data[ source[ 3 ] * 4 + 0 ];
			int G = data[ source[ 0 ] * 4 + 1 ] + data[ source[ 1 ] * 4 + 1 ] + data[ source[ 2 ] * 4 + 1 ] + data[ source[ 3 ] * 4 + 1 ];
			int B = data[ source[ 0 ] * 4 + 2 ] + data[ source[ 1 ] * 4 + 2 ] + data[ source[ 2 ] * 4 + 2 ] + data[ source[ 3 ] * 4 + 2 ];
			int A = data[ source[ 0 ] * 4 + 3 ] + data[ source[ 1 ] * 4 + 3 ] + data[ source[ 2 ] * 4 + 3 ] + data[ source[ 3 ] * 4 + 3 ];
#elif 0
			// Gamma corrected mipmapping
			int R = ColorAverage( data[ source[ 0 ] * 4 + 0 ], data[ source[ 1 ] * 4 + 0 ], data[ source[ 2 ] * 4 + 0 ], data[ source[ 3 ] * 4 + 0 ] ) * 4;
			int G = ColorAverage( data[ source[ 0 ] * 4 + 1 ], data[ source[ 1 ] * 4 + 1 ], data[ source[ 2 ] * 4 + 1 ], data[ source[ 3 ] * 4 + 1 ] ) * 4;
			int B = ColorAverage( data[ source[ 0 ] * 4 + 2 ], data[ source[ 1 ] * 4 + 2 ], data[ source[ 2 ] * 4 + 2 ], data[ source[ 3 ] * 4 + 2 ] ) * 4;
			int A = ColorAverage( data[ source[ 0 ] * 4 + 3 ], data[ source[ 1 ] * 4 + 3 ], data[ source[ 2 ] * 4 + 3 ], data[ source[ 3 ] * 4 + 3 ] ) * 4;
#elif 1
			// Stochastic mip-mapping
			int idx = rand() % 4;
			int R = data[ source[ idx ] * 4 + 0 ] * 4;
			int G = data[ source[ idx ] * 4 + 1 ] * 4;
			int B = data[ source[ idx ] * 4 + 2 ] * 4;
			int A = data[ source[ idx ] * 4 + 3 ] * 4;
#elif 0
			// There's other forms of mipmapping.  Luminance based (pick the brightest texel).  There's probably guassian, and a plethora of others.
#endif	

			data[ target * 4 + 0 ] = R / 4;
			data[ target * 4 + 1 ] = G / 4;
			data[ target * 4 + 2 ] = B / 4;
			data[ target * 4 + 3 ] = A / 4;

			target++;
		}
	}
}

#include "Graphics/BlockCompression.h"
void MipMapAndCompress( const Targa & image, int name, int layer ) {
	glBindTexture( GL_TEXTURE_2D_ARRAY, name );

	int width = image.GetWidth();
	int height = image.GetHeight();
	assert( width == height );
	int lod = 0;
	unsigned char * data = (unsigned char *)malloc( width * width * 4 );
	uint8 * compressed = (uint8 *)malloc( width * width * 4 );

	memcpy( data, image.DataPtr(), width * width * 4 );

	int w = width;
	while ( w >= 4 ) {
		if ( w < width ) {
			MipInPlace( data, w << 1 );
		}
		int size = w * w / 2;	// bc1 is 4-bits per texel

		int numOutBytes = 0;
		CompressImageDXT1( data, compressed, w, w, numOutBytes );
		//CompressImageDXT5( data, compressed, w, w, numOutBytes );
		size = numOutBytes;

		int offX = 0;
		int offY = 0;
		int offZ = layer;
		int depth = 1;
		glCompressedTexSubImage3D( GL_TEXTURE_2D_ARRAY, lod, offX, offY, offZ, w, w, depth, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, size, compressed );
		//glCompressedTexSubImage3D( GL_TEXTURE_2D_ARRAY, lod, offX, offY, offZ, w, w, depth, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, size, compressed );
		myglGetError();

		lod++;
		w = w >> 1;
	}
	glBindTexture( GL_TEXTURE_2D_ARRAY, 0 );
	free( data );
	free( compressed );
}

/*
 ================================
 main
 ================================
 */
int main( int argc, char ** argv ) {
	// Initialize GLUT
	glutInit( &argc, argv );

	// Tell GLUT to create a single display with Red Green Blue (RGB) color and a depth buffer.
	glutInitDisplayMode( GLUT_SINGLE | GLUT_RGB );

	// Tell GLUT the size of the desired window
	glutInitWindowSize( g_screenWidth, g_screenHeight );

	// Set the intial window position
	glutInitWindowPosition( 50, 50 );

	// Create the window
	glutCreateWindow( "OpenGL program" );

	// Initialize GLEW (this will set the proper bindings for all of our OpenGL calls).
	// GLEW is a third party library that connects our application with OpenGL drivers
	// that are installed on the current system.
	GLenum err = glewInit();

	// Check for any errors that may have occured during the initialization of GLEW
	if ( GLEW_OK != err ) {
		// Problem: glewInit failed, something is seriously wrong.
		fprintf( stderr, "Error: %s\n", glewGetErrorString( err ) );

		// Quit the program because it will not run without GLEW
		return 0;
	}

	// Print out the installed version of OpenGL on this system
	printf( "GL_VERSION:  %s\n", (const char *)glGetString( GL_VERSION ) );

	g_shaderManager = new hbShaderManager;
	g_textureManager = new TextureManager;

	g_renderSurface.CreateSurface( RS_COLOR_BUFFER | RS_DEPTH_BUFFER, g_screenWidth, g_screenHeight );

	// Load the shader program that will be used for rendering
	g_shaderDepthOnly.LoadFromFile( "data/Shaders/depthOnly.frag", "data/Shaders/depthOnly.vert", NULL, NULL, NULL, NULL );
	g_shaderTerrainDebug.LoadFromFile( "data/Shaders/terrainDebug.frag", "data/Shaders/terrainDebug.vert", NULL, NULL, NULL, NULL );
	
	// Setup the view matrix
	g_cameraPos	= Vec3d( 300, 0, 1 );
	g_cameraUp	= Vec3d( 0, 0, 1 );
	g_cameraLook	= Vec3d( -1, 0, 0 );
	myLookAt( g_cameraPos, g_cameraPos + g_cameraLook, g_cameraUp, g_matView );

	g_terrain.Terraform();

	g_virtualTexture.Init();
	g_virtualTexture.InitSamplerFeedback( g_screenWidth >> 2, g_screenHeight >> 2 );
	//g_virtualTexture.InitSamplerFeedback( g_screenWidth, g_screenHeight );

	{
		Targa g_targa0;
		Targa g_targa1;
		Targa g_targa2;
		Targa g_targa3;
		Targa g_targa4;
		g_targa0.Load( "../../common/pebbles_diffuse.tga" );
		g_targa1.Load( "../../common/grass_diffuse.tga" );
		g_targa2.Load( "../../common/dirt_diffuse.tga" );
		g_targa3.Load( "../../common/rock045_diffuse.tga" );
		g_targa4.Load( "../../common/snow001_diffuse.tga" );

#if 1
		{
			TextureOpts_t opts;
			opts.wrapS = WM_REPEAT;
			opts.wrapR = WM_REPEAT;
			opts.wrapT = WM_REPEAT;
			opts.minFilter = FM_LINEAR_MIPMAP_NEAREST;
			opts.magFilter = FM_LINEAR;
			opts.dimX = 1024;
			opts.dimY = 1024;
			opts.dimZ = 5;
			opts.type = TT_TEXTURE_2D_ARRAY;
			opts.format = FMT_BC1;
			g_textureArray = g_textureManager->GetTexture( "_textureArray", opts, NULL );
		}
		MipMapAndCompress( g_targa0, g_textureArray->GetName(), 0 );
		MipMapAndCompress( g_targa1, g_textureArray->GetName(), 1 );
		MipMapAndCompress( g_targa2, g_textureArray->GetName(), 2 );
		MipMapAndCompress( g_targa3, g_textureArray->GetName(), 3 );
		MipMapAndCompress( g_targa4, g_textureArray->GetName(), 4 );
#elif 1
		{
			TextureOpts_t opts;
			opts.wrapS = WM_REPEAT;
			opts.wrapR = WM_REPEAT;
			opts.wrapT = WM_REPEAT;
			opts.minFilter = FM_LINEAR_MIPMAP_NEAREST;
			opts.minFilter = FM_LINEAR;
			opts.magFilter = FM_LINEAR;
			opts.dimX = 1024;
			opts.dimY = 1024;
			opts.dimZ = 5;
			opts.dimZ = 1;
			opts.type = TT_TEXTURE_2D;
			opts.format = FMT_BC1;
			uint8 * data = RGBAtoBC1( g_targa0.DataPtr(), g_targa0.GetWidth(), g_targa0.GetHeight() );
			g_textureArray = g_textureManager->GetTexture( "_textureArray", opts, NULL );
			int size = 1024 * 1024 / 2;	// bc1 is 4-bits per texel
			glBindTexture( GL_TEXTURE_2D, g_textureArray->GetName() );
			glCompressedTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 1024, 1024, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, size, data );
			myglGetError();
			glBindTexture( GL_TEXTURE_2D, 0 );
		}
#else
		int layerSize = 1024 * 1024 * 4;
		unsigned char * data = (unsigned char *)malloc( layerSize * 5 );
		if ( NULL != data ) {
			memcpy( data + 0 * layerSize, g_targa0.DataPtr(), layerSize );
			memcpy( data + 1 * layerSize, g_targa1.DataPtr(), layerSize );
			memcpy( data + 2 * layerSize, g_targa2.DataPtr(), layerSize );
			memcpy( data + 3 * layerSize, g_targa3.DataPtr(), layerSize );
			memcpy( data + 4 * layerSize, g_targa4.DataPtr(), layerSize );

			TextureOpts_t opts;
			opts.wrapS = WM_REPEAT;
			opts.wrapR = WM_REPEAT;
			opts.wrapT = WM_REPEAT;
			opts.minFilter = FM_LINEAR_MIPMAP_NEAREST;
			opts.magFilter = FM_LINEAR;
			opts.dimX = 1024;
			opts.dimY = 1024;
			opts.dimZ = 5;
			opts.type = TT_TEXTURE_2D_ARRAY;
			opts.format = FMT_RGBA8;
			g_textureArray = g_textureManager->GetTexture( "_textureArray", opts, data );
			free( data );
		}
#endif
	}

	// Build an Earth like atmosphere
	g_atmos.numSamples = 256;	// Number of integration samples
	g_atmos.textureResX = 128;	// Height
	g_atmos.textureResY = 512;	// Sun Angle
	g_atmos.textureResZ = 512;	// View Angle
	g_atmos.sunLightIntensity = Vec3d( 1.0, 0.78132, 0.477507 );
	g_atmos.indicesOfRefraction = Vec3d( 1.000271287, 1.000274307, 1.000275319 );  // indices of refraction
	g_atmos.molecularDensity_NsR = 2.653e25;
	g_atmos.molecularDensity_NsM = 1.5e10;
	g_atmos.radiusGround = 6360;	// the radius of the earth [km]
	g_atmos.radiusTop = 6420;		// the radius of the top of the atmosphere [km]
	g_atmos.scaleHeightRayleigh = 7.994;	// km
	g_atmos.scaleHeightMie = 1.2;			// km
	g_atmos.betaRayleighScatter = Vec3d( 0.0058, 0.0135, 0.0331 );		//5.8*10-3,1.35*10-2,3.31*10-2 Earth
	g_atmos.betaRayleighExtinction = Vec3d( 0.0058, 0.0135, 0.0331 );
	g_atmos.betaMieScatter = Vec3d( 0.004, 0.004, 0.004 );					//4.0*10-3, 4.0*10-3, 4.0*10-3 Earth
	g_atmos.betaMieExtinction = Vec3d( 0.00444, 0.00444, 0.00444 );		// betaMieScatter / 0.9f;
	g_atmos.mieG = 0.8f;
	BuildAtmosphereBruneton( g_atmos );

	// Initialize the noise textures used by the clouds
	CloudInit();

	//
	//	Build the near/far planes (for full screen rendering)
	//
	{
		const unsigned short indices[ 6 ] = {
			0, 1, 2,
			0, 2, 3,
		};

		vert_t data[ 4 ];
		memset( data, 0, sizeof( vert_t ) * 4 );

		data[ 0 ].pos = Vec3d( -1.0f, -1.0f, 1.0f );
		data[ 0 ].st = Vec2d( 0.0f, 0.0f );
		Vec3dToByte4_n11( data[ 0 ].norm, Vec3d( 0.0f, 0.0f, -1.0f ) );
		Vec3dToByte4_n11( data[ 0 ].tang, Vec3d( 1.0f, 0.0f, 0.0f ) );

		data[ 1 ].pos = Vec3d( 1.0f, -1.0f, 1.0f );
		data[ 1 ].st = Vec2d( 1.0f, 0.0f );
		Vec3dToByte4_n11( data[ 1 ].norm, Vec3d( 0.0f, 0.0f, -1.0f ) );
		Vec3dToByte4_n11( data[ 1 ].tang, Vec3d( 1.0f, 0.0f, 0.0f ) );

		data[ 2 ].pos = Vec3d( 1.0f, 1.0f, 1.0f );
		data[ 2 ].st = Vec2d( 1.0f, 1.0f );
		Vec3dToByte4_n11( data[ 2 ].norm, Vec3d( 0.0f, 0.0f, -1.0f ) );
		Vec3dToByte4_n11( data[ 2 ].tang, Vec3d( 1.0f, 0.0f, 0.0f ) );

		data[ 3 ].pos = Vec3d( -1.0f, 1.0f, 1.0f );
		data[ 3 ].st = Vec2d( 0.0f, 1.0f );
		Vec3dToByte4_n11( data[ 3 ].norm, Vec3d( 0.0f, 0.0f, -1.0f ) );
		Vec3dToByte4_n11( data[ 3 ].tang, Vec3d( 1.0f, 0.0f, 0.0f ) );
		
		g_modelScreenSpaceFarPlane.LoadFromData( data, 4, indices, 6 );

		data[ 0 ].pos.z = -1.0f;
		data[ 1 ].pos.z = -1.0f;
		data[ 2 ].pos.z = -1.0f;
		data[ 3 ].pos.z = -1.0f;

		g_modelScreenSpaceNearPlane.LoadFromData( data, 4, indices, 6 );
	}
	
	//
	// Set GLUT function pointers here
	//
	glutReshapeFunc( reshape );
	//glutDisplayFunc( DrawFrame );
	glutKeyboardFunc( keyboard );
	glutKeyboardUpFunc( keyboardup );
	glutSpecialFunc( special );
	glutMouseFunc( mouse );
	glutMotionFunc( motion );
	glutPassiveMotionFunc( motionPassive );
	glutEntryFunc( entry );

	// Setting the idle function to point to the DrawFrame function tells GLUT to call this function in GLUT's infinite loop
	glutIdleFunc( DrawFrame );
	
	//
	//	Perform the infinite loop
	//

	// Do the infinite loop.  This starts glut's infinite loop.  It will call our draw function over and over
	glutMainLoop();

	// The code reaches here, when the application is quiting.
	delete g_shaderManager;
	delete g_textureManager;
	return 0;
}

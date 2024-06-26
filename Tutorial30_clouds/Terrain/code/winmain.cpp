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
#include "Terrain/Terrain.h"
#include "VirtualTexture/VirtualTexture.h"
#include "Atmosphere/BuildBruneton.h"
#include "Water/Ocean.h"
#include "Clouds/Clouds.h"

// Global storage of the window size
// const int g_screenWidth  = 1200;
// const int g_screenHeight = 720;
const int g_screenWidth  = 1920;
const int g_screenHeight = 1080;

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

int g_prevMouseX = 0;
int g_prevMouseY = 0;

atmosphereBuildData_t g_atmos;
float g_sunAngle = 3.14f * 0.25f;


Targa g_targa;
Texture g_texture;

/*
 ================================
 DrawFrame

 This is the function that is repeatedly called.
 And it is where all the magic happens.
 ================================
 */
float g_timeMS = 0;
void DrawFrame( void ) {
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

	const float bias_matrix[ 16 ] = {
		0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.5f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f
	};

	const Vec3d sunDir = Vec3d( cosf( g_sunAngle ), 0.0f, sinf( g_sunAngle ) );

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
	{
		CloudDrawParms_t parms;
		parms.m_matProj = matProj;
		parms.m_matView = g_matView;
		parms.m_camPos = g_cameraPos;
		parms.m_dirToSun = sunDir;
		parms.m_time = g_timeTime;
		CloudDraw( parms );
	}

	//
	//	Draw the sunlit megatextured terrain
	//
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

/*
 ================================
 keyboard
 ================================
 */
void keyboard( unsigned char key, int x, int y ) {
	Vec3d right = g_cameraLook.Cross( Vec3d( 0, 0, 1 ) );
	right.Normalize();

	Vec3d dir = Vec3d( 0 );
	if ( key == 'w' || key == 'W' ) {
		dir += g_cameraLook;
	}
	if ( key == 's' || key == 'S' ) {
		dir -= g_cameraLook;
	}
	if ( key == 'd' || key == 'D' ) {
		dir += right;
	}
	if ( key == 'a' || key == 'A' ) {
		dir -= right;
	}

	if ( key == 'e' || key == 'E' ) {
		dir += Vec3d( 0, 0, 100 );
	}
	if ( key == 'q' || key == 'Q' ) {
		dir -= Vec3d( 0, 0, 100 );
	}

	float deltaSun = 0;
	if ( key == 'o' || key == 'O' ) {
		deltaSun += 3;
	}
	if ( key == 'p' || key == 'P' ) {
		deltaSun -= 3;
	}
	g_sunAngle += deltaSun * g_dtSec;

	float speed = 10.0f;
	if ( g_noclip ) {
		speed = 10000.0f;
	}
	g_cameraPos += dir * g_dtSec * speed;
}

/*
 ================================
 keyboardup
 ================================
 */
void keyboardup( unsigned char key, int x, int y ) {
	if ( 'n' == key ) {
		g_noclip = !g_noclip;
	}
}

/*
 ================================
 special
 ================================
 */
bool ignoreRepeats = false;
void special( int key, int x, int y ) {
}

/*
 ================================
 mouse
 ================================
 */
void mouse( int button, int state, int x, int y ) {
	// Convert from windows coords (origin at top right)
	// To gl coords (origin at lower left)
	y = g_screenHeight - y;
}

/*
 ================================
 motion

 * Updates mouse position when buttons are pressed
 ================================
 */
void motion( int x, int y ) {
	// Convert from windows coords (origin at top right)
	// To gl coords (origin at lower left)
	y = g_screenHeight - y;
}

/*
 ================================
 motionPassive

 * Updates mouse position when zero buttons are pressed
 ================================
 */
void motionPassive( int x, int y ) {
	// Convert from windows coords (origin at top right)
	// To gl coords (origin at lower left)
	y = g_screenHeight - y;

	float dtX = g_prevMouseX - x;
	float dtY = g_prevMouseY - y;

	if ( fabsf( dtX ) > 100 ) {
		dtX = 0;
	}
	if ( fabsf( dtY ) > 100 ) {
		dtY = 0;
	}

	g_prevMouseX = x;
	g_prevMouseY = y;

	g_cameraTheta += dtX * g_dtSec;
	g_cameraPhi += dtY * g_dtSec;
}

/*
 ================================
 entry
 ================================
 */
void entry( int state ) {
	if ( GLUT_ENTERED == state ) {
		printf( "Mouse entered window\n" );
	} else {
		printf( "Mouse left window area\n" );
	}
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

	g_targa.Load( "../../common/Ground_Forest_002_baseColor.tga" );
	g_texture.InitWithData( g_targa.DataPtr(), g_targa.GetWidth(), g_targa.GetHeight() );

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

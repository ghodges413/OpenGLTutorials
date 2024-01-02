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

#include "Vector.h"
#include "Shader.h"
#include "MatrixOps.h"
#include "Mesh.h"
#include "RenderSurface.h"
#include "Time.h"
#include "Terrain/Terrain.h"
#include "Frustum.h"

// Global storage of the window size
// const int g_screenWidth  = 1200;
// const int g_screenHeight = 720;
const int g_screenWidth  = 1920;
const int g_screenHeight = 1080;

Terrain g_terrain;

Shader g_shaderDepthOnly;
RenderSurface g_renderSurface;

Shader g_shaderTerrain;
Shader g_shaderTerrainDebug;

Vec3d g_cameraPos;
Vec3d g_cameraUp;
Vec3d g_cameraLook;
float g_matView[ 16 ];

/*
 ================================
 DrawFrame

 This is the function that is repeatedly called.
 And it is where all the magic happens.
 ================================
 */
void DrawFrame( void ) {
	// Calculate the timing
	static int timeLastFrame	= 0;
	const int time				= GetTimeMicroseconds();
	const float dt_us			= time - timeLastFrame;
	const float dt_ms			= dt_us * 0.001f;
	timeLastFrame				= time;
	static float runTimeSeconds = 0;
	runTimeSeconds += dt_ms * 0.001f;

	// Update the camera view matrix
	const float radius = 1000;
#if 1
	const float angle = runTimeSeconds * 0.0613f;
	g_cameraPos	= Vec3d( radius * cosf( angle ), radius * sinf( angle ), 300 );
	Vec3d lookat = Vec3d( 0, 0, 50 );
#else
	const float angle = runTimeSeconds * 0.013f;
	g_cameraPos	= Vec3d( radius * cosf( angle ), radius * sinf( angle ), 400 );
	g_cameraPos = g_terrain.GetSurfacePos( g_cameraPos ) + Vec3d( 0, 0, 2 );
	g_cameraUp	= Vec3d( 0, 0, 1 );
 	Vec3d lookat = g_cameraPos * 0.95f;
	lookat = g_cameraPos;
	lookat.x += -sinf( angle ) * 30.0f;
	lookat.y += cosf( angle ) * 30.0f;
 	lookat = g_terrain.GetSurfacePos( lookat ) + Vec3d( 0, 0, 2 );
#endif
	myLookAt( g_cameraPos, lookat, g_cameraUp, g_matView );

	// This sets the perspective projection matrix
	float matProj[ 16 ] = { 0 };
	const float fieldOfViewDegrees = 45;
	const float aspectRatio = static_cast< float >( g_screenHeight ) / static_cast< float >( g_screenWidth );
	const float nearDepth = 1;
	const float farDepth = 10000;
	myPerspective( fieldOfViewDegrees, aspectRatio, nearDepth, farDepth, matProj );

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
	glClearColor( 1.0f, 0.0f, 0.0f, 1.0f );

	// Clear previous frame values
	glClear( GL_COLOR_BUFFER_BIT );
	glClear( GL_DEPTH_BUFFER_BIT );

	// This will turn on back face culling.  Uncomment to turn on.
	glCullFace( GL_BACK );
	glEnable( GL_CULL_FACE );
	glDisable( GL_CULL_FACE );

	Frustum viewFrustum;
	viewFrustum.Build( matProj, g_matView );

	g_terrain.Update( g_cameraPos, viewFrustum );

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
	//	Draw the terrain
	//
	{
		// Turn off writing to the depth buffer, but keep using it
		glDepthMask( GL_FALSE );

		g_shaderTerrain.UseProgram();

		float matIdentity[ 16 ] = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		};

		// Send the transformation matrix to the shader
		g_shaderTerrain.SetUniformMatrix4f( "matModelWorld", 1, false, matIdentity );

		// Send the projection matrix to the shader
		g_shaderTerrain.SetUniformMatrix4f( "matProj", 1, false, matProj );
		g_shaderTerrain.SetUniformMatrix4f( "matView", 1, false, g_matView );

		// Draw the terrain
		g_terrain.Draw();
	}

	//
	//	Draw Debug Terrain
	//
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

    //
    //  Copy the render buffer's color buffer to the primary frambuffer
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

	g_renderSurface.CreateSurface( RS_COLOR_BUFFER | RS_DEPTH_BUFFER, g_screenWidth, g_screenHeight );

	// Load the shader program that will be used for rendering
	g_shaderDepthOnly.LoadFromFile( "data/Shaders/depthOnly.frag", "data/Shaders/depthOnly.vert", NULL, NULL, NULL, NULL );
	g_shaderTerrain.LoadFromFile( "data/Shaders/terrain.frag", "data/Shaders/terrain.vert", NULL, NULL, NULL, NULL );
	g_shaderTerrainDebug.LoadFromFile( "data/Shaders/terrainDebug.frag", "data/Shaders/terrainDebug.vert", NULL, NULL, NULL, NULL );
	
	// Setup the view matrix
	g_cameraPos	= Vec3d( 300, 0, 1 );
	g_cameraUp	= Vec3d( 0, 0, 1 );
	g_cameraLook	= Vec3d( -1, 0, 0 );
	myLookAt( g_cameraPos, g_cameraPos + g_cameraLook, g_cameraUp, g_matView );

	g_terrain.Terraform();
	
	//
	// Set GLUT function pointers here
	//

	// Setting the idle function to point to the DrawFrame function tells GLUT to call this function in GLUT's infinite loop
	glutIdleFunc( DrawFrame );
	
	//
	//	Perform the infinite loop
	//

	// Do the infinite loop.  This starts glut's infinite loop.  It will call our draw function over and over
	glutMainLoop();

	// The code reaches here, when the application is quiting.
	return 0;
}

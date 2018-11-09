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
#include "Targa.h"
#include "Texture.h"
#include "MatrixOps.h"
#include "Mesh.h"
#include "RenderSurface.h"
#include "SpotLight.h"
#include "MD5Model.h"
#include "MD5Anim.h"
#include "Time.h"
#include "PointLight.h"

// Global storage of the window size
const int gScreenWidth  = 1200;
const int gScreenHeight = 720;

// The shader program that runs on the GPU
Shader gShaderSimple;
Shader gShaderCompute;

GLuint gTextureImage = 0;

/*
 ================================
 DrawFrame

 This is the function that is repeatedly called.
 And it is where all the magic happens.
 ================================
 */
void DrawFrame( void ) {
	// Bind the off screen render surface for rendering
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
	
	// Enabling color writing to the frame buffer
	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );

	// Enable writing to the depth buffer
	glDepthMask( GL_TRUE );

	// Set the depth test function to less than or equal
	glDepthFunc( GL_LEQUAL );

	// Enable testing against the depth buffer
	glEnable( GL_DEPTH_TEST );

	glDisable( GL_BLEND );
	
	// Set the OpenGL viewport to be the entire size of the window
	glViewport( 0, 0, gScreenWidth, gScreenHeight );

	// Clear the frame buffer color
	// Inputs are Red Green Blue Alpha (RGBA)
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

	// Clear previous frame values
	glClear( GL_COLOR_BUFFER_BIT );
	glClear( GL_DEPTH_BUFFER_BIT );

	// This will turn on back face culling.  Uncomment to turn on.
	glCullFace( GL_BACK );
	glEnable( GL_CULL_FACE );
	
	//
	//	Create texture image for the compute shader to fill
	//
	
	const int width		= 512;
	const int height	= 512;

	if ( 0 == gTextureImage ) {
		// We create a single float channel 512^2 texture
		glGenTextures( 1, &gTextureImage );

		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, gTextureImage );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, NULL );

		// Because we're also using this tex as an image (in order to write to it),
		// we bind it to an image unit as well
		glBindImageTexture( 0, gTextureImage, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F );
		myglGetError();
	}

	//
	//	Use the compute shader to fill the texture image
	//

	// glBindImageTexture binds a single level of a texture to an image unit for the purpose of reading and writing it from shaders
	glBindImageTexture( 0, gTextureImage, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F );
		
	static int frame = 0;
	++frame;

	float fFrame = frame;
	fFrame *= 0.001f;

	gShaderCompute.UseProgram();

	gShaderCompute.SetUniform1f( "roll", 1, &fFrame );		

	gShaderCompute.DispatchCompute( width / 16, height / 16, 1 );
	myglGetError();

	//
	//	Draw the compute shader generated texture image
	//

	// Positions for drawing to the lower left quarter of the screen
	Vec3d positions[ 4 ] = {
		Vec3d( -0.5f, 0.5f, 0 ),
		Vec3d( -0.5f, -0.5f, 0 ),
		Vec3d( 0.5f, -0.5f, 0 ),
		Vec3d( 0.5f, 0.5f, 0 )
	};

	Vec2d st[ 4 ] = {
		Vec2d( 0, 1 ),
		Vec2d( 0, 0 ),
		Vec2d( 1, 0 ),
		Vec2d( 1, 1 )
	};

	// Set the shader program that'll be used to render the offscreen texture
	gShaderSimple.UseProgram();

	// Update attribute values.
	gShaderSimple.SetVertexAttribPointer( "st", 2, GL_FLOAT, 0, 0, st );

	gShaderSimple.SetVertexAttribPointer( "position", 3, GL_FLOAT, 0, 0, positions );
	gShaderSimple.SetAndBindUniformTexture( "s_texture", 0, GL_TEXTURE_2D, gTextureImage );
	glDrawArrays( GL_QUADS, 0, 4 );

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
	glutInitDisplayMode( GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH );

	// Tell GLUT the size of the desired window
	glutInitWindowSize( gScreenWidth, gScreenHeight );

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

	// Load the shader program that will be used for rendering
	gShaderSimple.LoadFromFile( "data/Shaders/simple.fsh", "data/Shaders/simple.vsh", NULL, NULL, NULL, NULL );
	gShaderCompute.LoadFromFile( NULL, NULL, NULL, NULL, NULL, "data/Shaders/computeTest.csh" );

	//
	// Set GLUT function pointers here
	//

	// Setting the idle function to point to the DrawFrame function tells GLUT to call this function in GLUT's infinite loop
	glutIdleFunc( DrawFrame );
	
	//
	//	Perform the infinite loop
	//

	// Do the infinite loop.  This starts glut's inifinite loop.  It will call our draw function over and over
	glutMainLoop();

	// Clean up the texture image
	if ( 0 == gTextureImage ) {
		glDeleteTextures( 1, &gTextureImage );
		gTextureImage = 0;
	}

	// The code reaches here, when the application is quiting.
	return 0;
}

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

// Global storage of the window size
const int gScreenWidth  = 1200;
const int gScreenHeight = 720;

/*
 ================================
 DrawFrame

 This is the function that is repeatedly called.
 And it is where all the magic happens.
 ================================
 */
void DrawFrame( void ) {
	// Bind the primary frame buffer for rendering
    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );

	// Enabling color writing to the frame buffer
	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
	
	// Set the OpenGL viewport to be the entire size of the window
    const int width = gScreenWidth;
    const int height = gScreenHeight;
	glViewport( 0, 0, width, height );

	// Clear the frame buffer color
	// Inputs are Red Green Blue Alpha (RGBA)
	glClearColor( 0.0f, 1.0f, 0.0f, 1.0f );
    
	// Clear previous frame values
	glClear( GL_COLOR_BUFFER_BIT );

	//
	//	Draw a single triangle
	//

	// Set the color of the triangle
	glColor3f( 0.0f, 0.0f, 1.0f );

	// Draw a simple triangle
	glBegin( GL_TRIANGLES );
	glVertex3f( -0.5f, -0.5f, 0.0f );
	glVertex3f( 0.5f, -0.5f, 0.0f );
	glVertex3f( 0.0f, 0.5f, 0.0f );
	glEnd();
    
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

	// Tell GLUT to create a single display with Red Green Blue (RGB) color.
	glutInitDisplayMode( GLUT_SINGLE | GLUT_RGB );

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

	// The above glutMainLoop is equivalent to using the infinite loop below.
	// Later, we won't be able to use this infinite below.  The code below
	// will not respond to mouse or keyboard inputs or any other messages
	// the operating system may send to our app, but the glutMainLoop() will.
	/*
	while ( 1 ) {
		DrawFrame();
	}
	*/

	// The code reaches here, when the application is quiting.
	return 0;
}

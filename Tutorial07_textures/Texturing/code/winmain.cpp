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

// Global storage of the window size
const int gScreenWidth  = 1200;
const int gScreenHeight = 720;

// The texture that resides on the GPU
Texture gTexture;

// The shader program that runs on the GPU
Shader gShader;

// The new interlaced format of the vertices
struct vert_t {
	Vec3d position;
	Vec2d st;		// texture coordinates
};

// Declaration of the vertices
const int gNumVertices = 3;
vert_t gVertices[ gNumVertices ];

/*
 ================================
 BuildVertices
 This function is called once, at startup.  Initializes the vertices.
 ================================
 */
void BuildVertices() {
	gVertices[ 0 ].position = Vec3d( -0.5f, -0.5f, 0.0f );
	gVertices[ 0 ].st		= Vec2d( 0.0f, 0.0f );

	gVertices[ 1 ].position = Vec3d( 0.5f, -0.5f, 0.0f );
	gVertices[ 1 ].st		= Vec2d( 1.0f, 0.0f );

	gVertices[ 2 ].position = Vec3d( 0.0f, 0.5f, 0.0f );
	gVertices[ 2 ].st		= Vec2d( 0.5f, 1.0f );
}

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
	glClearColor( 1.0f, 0.0f, 0.0f, 1.0f );

	// Clear previous frame values
	glClear( GL_COLOR_BUFFER_BIT );

	//
	//	Draw a single triangle
	//

	// Set the shader program that'll be used to render the cube
	gShader.UseProgram();

	// Bind the texture to be rendered
	gShader.SetAndBindUniformTexture( "s_texture", 0, GL_TEXTURE_2D, gTexture.GetName() );
	
	// Update attribute values.
	const int stride = sizeof( vert_t );
	gShader.SetVertexAttribPointer( "position", 3, GL_FLOAT, 0, stride, gVertices[ 0 ].position.ToPtr() );
	gShader.SetVertexAttribPointer( "st", 2, GL_FLOAT, 0, stride, gVertices[ 0 ].st.ToPtr() );

	// Draw
	glDrawArrays( GL_TRIANGLES, 0, gNumVertices );

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

	// Build the vertex data
	BuildVertices();

	// Load the shader program that will be used for rendering
	gShader.LoadFromFile( "data/Shaders/textured.fsh", "data/Shaders/textured.vsh" );

	// Load the targa data from file
	Targa targa;
	targa.Load( "data/Images/smiley_face.tga", true );

	// Create a texture on the GPU and initialize it with the targa data
	gTexture.InitWithData( targa.DataPtr(), targa.GetWidth(), targa.GetHeight() );

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

	// The code reaches here, when the application is quiting.
	return 0;
}

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
const int gNumVertices = 3 * 2 * 6;	// 3 verts per triangle, 2 triangles per face, 6 faces in a cube
vert_t gVertices[ gNumVertices ];

/*
 ================================
 BuildVertices
 This function is called once, at startup.  Initializes the vertices.
 ================================
 */
void BuildVertices() {
	// -x face
	gVertices[ 0 ].position	= Vec3d( -0.5f, -0.5f, -0.5f );
	gVertices[ 0 ].st		= Vec2d( 0, 0 );
	gVertices[ 1 ].position	= Vec3d( -0.5f, -0.5f,  0.5f );
	gVertices[ 1 ].st		= Vec2d( 0, 1 );
	gVertices[ 2 ].position	= Vec3d( -0.5f,  0.5f, -0.5f );
	gVertices[ 2 ].st		= Vec2d( 1, 0 );

	gVertices[ 3 ].position = Vec3d( -0.5f,  0.5f,  0.5f );
	gVertices[ 3 ].st		= Vec2d( 1, 1 );
	gVertices[ 4 ].position = Vec3d( -0.5f,  0.5f, -0.5f );
	gVertices[ 4 ].st		= Vec2d( 1, 0 );
	gVertices[ 5 ].position = Vec3d( -0.5f, -0.5f,  0.5f );
	gVertices[ 5 ].st		= Vec2d( 0, 1 );

	// +x face
	gVertices[ 6 ].position		= Vec3d(  0.5f, -0.5f, -0.5f );
	gVertices[ 6 ].st			= Vec2d( 0, 0 );
	gVertices[ 7 ].position		= Vec3d(  0.5f,  0.5f, -0.5f );
	gVertices[ 7 ].st			= Vec2d( 1, 0 );
	gVertices[ 8 ].position		= Vec3d(  0.5f, -0.5f,  0.5f );
	gVertices[ 8 ].st			= Vec2d( 0, 1 );

	gVertices[ 9 ].position		= Vec3d(  0.5f,  0.5f,  0.5f );
	gVertices[ 9 ].st			= Vec2d( 1, 1 );
	gVertices[ 10 ].position	= Vec3d(  0.5f, -0.5f,  0.5f );
	gVertices[ 10 ].st			= Vec2d( 0, 1 );
	gVertices[ 11 ].position	= Vec3d(  0.5f,  0.5f, -0.5f );
	gVertices[ 11 ].st			= Vec2d( 1, 0 );

	// -y face
	gVertices[ 12 ].position	= Vec3d( -0.5f, -0.5f, -0.5f );
	gVertices[ 12 ].st			= Vec2d( 0, 0 );
	gVertices[ 13 ].position	= Vec3d(  0.5f, -0.5f, -0.5f );
	gVertices[ 13 ].st			= Vec2d( 1, 0 );
	gVertices[ 14 ].position	= Vec3d( -0.5f, -0.5f,  0.5f );
	gVertices[ 14 ].st			= Vec2d( 0, 1 );
	
	gVertices[ 15 ].position	= Vec3d(  0.5f, -0.5f,  0.5f );
	gVertices[ 15 ].st			= Vec2d( 1, 1 );
	gVertices[ 16 ].position	= Vec3d( -0.5f, -0.5f,  0.5f );
	gVertices[ 16 ].st			= Vec2d( 0, 1 );
	gVertices[ 17 ].position	= Vec3d(  0.5f, -0.5f, -0.5f );
	gVertices[ 17 ].st			= Vec2d( 1, 0 );

	// +y face
	gVertices[ 18 ].position	= Vec3d( -0.5f,  0.5f, -0.5f );
	gVertices[ 18 ].st			= Vec2d( 0, 0 );
	gVertices[ 19 ].position	= Vec3d( -0.5f,  0.5f,  0.5f );
	gVertices[ 19 ].st			= Vec2d( 0, 1 );
	gVertices[ 20 ].position	= Vec3d(  0.5f,  0.5f, -0.5f );
	gVertices[ 20 ].st			= Vec2d( 1, 0 );

	gVertices[ 21 ].position	= Vec3d(  0.5f,  0.5f,  0.5f );
	gVertices[ 21 ].st			= Vec2d( 1, 1 );
	gVertices[ 22 ].position	= Vec3d(  0.5f,  0.5f, -0.5f );
	gVertices[ 22 ].st			= Vec2d( 1, 0 );
	gVertices[ 23 ].position	= Vec3d( -0.5f,  0.5f,  0.5f );
	gVertices[ 23 ].st			= Vec2d( 0, 1 );

	// -z face
	gVertices[ 24 ].position	= Vec3d( -0.5f, -0.5f, -0.5f );
	gVertices[ 24 ].st			= Vec2d( 0, 0 );
	gVertices[ 25 ].position	= Vec3d( -0.5f,  0.5f, -0.5f );
	gVertices[ 25 ].st			= Vec2d( 0, 1 );
	gVertices[ 26 ].position	= Vec3d(  0.5f, -0.5f, -0.5f );
	gVertices[ 26 ].st			= Vec2d( 1, 0 );

	gVertices[ 27 ].position	= Vec3d(  0.5f,  0.5f, -0.5f );
	gVertices[ 27 ].st			= Vec2d( 1, 1 );
	gVertices[ 28 ].position	= Vec3d(  0.5f, -0.5f, -0.5f );
	gVertices[ 28 ].st			= Vec2d( 1, 0 );
	gVertices[ 29 ].position	= Vec3d( -0.5f,  0.5f, -0.5f );
	gVertices[ 29 ].st			= Vec2d( 0, 1 );

	// +z face
	gVertices[ 30 ].position	= Vec3d( -0.5f, -0.5f,  0.5f );
	gVertices[ 30 ].st			= Vec2d( 0, 0 );
	gVertices[ 31 ].position	= Vec3d(  0.5f, -0.5f,  0.5f );
	gVertices[ 31 ].st			= Vec2d( 1, 0 );
	gVertices[ 32 ].position	= Vec3d( -0.5f,  0.5f,  0.5f );
	gVertices[ 32 ].st			= Vec2d( 0, 1 );

	gVertices[ 33 ].position	= Vec3d( 0.5f,  0.5f,  0.5f );
	gVertices[ 33 ].st			= Vec2d( 1, 1 );
	gVertices[ 34 ].position	= Vec3d( -0.5f,  0.5f,  0.5f );
	gVertices[ 34 ].st			= Vec2d( 0, 1 );
	gVertices[ 35 ].position	= Vec3d(  0.5f, -0.5f,  0.5f );
	gVertices[ 35 ].st			= Vec2d( 1, 0 );
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

	// This will turn on back face culling.  Uncomment to turn on.
	glCullFace( GL_BACK );
	glEnable( GL_CULL_FACE );

	static float angle = 0;
	angle += 0.05f;

	// This sets the rotation matrix.
	float matRotate[ 16 ] = { 0 };
	myRotateY( angle, matRotate );

	// This sets the translation matrix
	float matTranslate[ 16 ] = { 0 };
	myTranslate( 0, 0, -10, matTranslate );

	// This sets the perspective projection matrix
	float matProj[ 16 ] = { 0 };
	const float fieldOfViewDegrees = 45;
	const float aspectRatio = static_cast< float >( gScreenHeight ) / static_cast< float >( gScreenWidth );
	const float nearDepth = 1;
	const float farDepth = 1000;
	myPerspective( fieldOfViewDegrees, aspectRatio, nearDepth, farDepth, matProj );

	// Concatenate the rotation and translation matrix.
	// This will make it so the cube is rotated and then translated.
	// Reversing the order of the multiply will translate the cube and then rotate it.
	float matModelView[ 16 ] = { 0 };
	myMatrixMultiply( matRotate, matTranslate, matModelView );
	//myMatrixMultiply( matTranslate, matRotate, matModelView );

	// Concatenate the model/view matrix with the perspective projection matrix
	float matFinal[ 16 ] = { 0 };
	myMatrixMultiply( matModelView, matProj, matFinal );

	//
	//	Draw a single cube
	//

	// Set the shader program that'll be used to render the cube
	gShader.UseProgram();

	// Bind the texture to be rendered
	gShader.SetAndBindUniformTexture( "s_texture", 0, GL_TEXTURE_2D, gTexture.GetName() );

	// Send the transformation matrix to the shader
	gShader.SetUniformMatrix4f( "matTransform", 1, false, matFinal );
	
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

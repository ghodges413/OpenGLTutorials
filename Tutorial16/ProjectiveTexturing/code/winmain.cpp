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

// Global storage of the window size
const int gScreenWidth  = 1200;
const int gScreenHeight = 720;

// The texture that'll be used for projective texturing
Texture gTextureProjective;

// The shader program that runs on the GPU
Shader gShader;

// A class for holding very simple static meshes
Mesh gMesh;

Vec3d gCameraPos;
Vec3d gCameraUp;
Vec3d gCameraLook;
float gMatView[ 16 ];

float gMatTextureProj[ 16 ];
float gMatTextureView[ 16 ];
float gMatTextureBias[ 16 ] = {
	0.5f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.5f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.5f, 0.0f,
	0.5f, 0.5f, 0.5f, 1.0f
};

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
	//glDisable( GL_DEPTH_TEST ); // uncomment to disable the depth test
	
	// Set the OpenGL viewport to be the entire size of the window
	glViewport( 0, 0, gScreenWidth, gScreenHeight );

	// Clear the frame buffer color
	// Inputs are Red Green Blue Alpha (RGBA)
	glClearColor( 1.0f, 0.0f, 0.0f, 1.0f );

	// Clear previous frame values
	glClear( GL_COLOR_BUFFER_BIT );
	glClear( GL_DEPTH_BUFFER_BIT );

	// This will turn on back face culling.  Uncomment to turn on.
	glCullFace( GL_BACK );
	glEnable( GL_CULL_FACE );

	static float angle = 0;
	angle += 0.01f;

	// This sets the rotation matrix.
	float matRotate[ 16 ] = { 0 };
	myRotateY( angle, matRotate );

	// This sets the translation matrix
	float matTranslate[ 16 ] = { 0 };
	myTranslate( 0, 0, 5, matTranslate );

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
	float matModelWorld[ 16 ] = { 0 };
	myMatrixMultiply( matRotate, matTranslate, matModelWorld );
	//myMatrixMultiply( matTranslate, matRotate, matModelView );

	//
	//	Draw a single mesh
	//

	// Set the shader program that'll be used to render the mesh
	gShader.UseProgram();

	// Bind the texture to be rendered
	gShader.SetAndBindUniformTexture( "s_light_proj_texture", 3, GL_TEXTURE_2D, gTextureProjective.GetName() );

	// Send the transformation matrix to the shader
	gShader.SetUniformMatrix4f( "matModelWorld", 1, false, matModelWorld );

	// Send the projection matrix to the shader
	gShader.SetUniformMatrix4f( "matProj", 1, false, matProj );
	gShader.SetUniformMatrix4f( "matView", 1, false, gMatView );

	gShader.SetUniformMatrix4f( "matLightProj", 1, false, gMatTextureProj );
	gShader.SetUniformMatrix4f( "matLightView", 1, false, gMatTextureView );
	gShader.SetUniformMatrix4f( "matLightBias", 1, false, gMatTextureBias );
	
	// Update attribute values.
	const int stride = sizeof( vert_t );
	gShader.SetVertexAttribPointer( "position", 3, GL_FLOAT, 0, stride, gMesh.mVerts[ 0 ].pos.ToPtr() );

	// Draw
	glDrawArrays( GL_TRIANGLES, 0, gMesh.mVerts.Num() );

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
	gShader.LoadFromFile( "data/Shaders/projectiveTexturing.fsh", "data/Shaders/projectiveTexturing.vsh" );

	// Load the targa data from file
	Targa targaProjective;
	targaProjective.Load( "data/Images/smiley_face.tga", true );

	// Create a texture on the GPU and initialize it with the targa data
	gTextureProjective.InitWithData( targaProjective.DataPtr(), targaProjective.GetWidth(), targaProjective.GetHeight() );

	// Load a mesh for drawing
	//gMesh.Load( "data/Meshes/companion_cube.obj" );
	gMesh.Load( "data/Meshes/torus01.obj" );
	//gMesh.Load( "data/Meshes/sphere.obj" );

	gCameraPos	= Vec3d( -10, 0, 20 );
	gCameraUp	= Vec3d( 0, 1, 0 );
	gCameraLook	= Vec3d( 1, 0, 0 );
	myLookAt( gCameraPos, Vec3d( 0, 0, 5 ), gCameraUp, gMatView );

	Vec3d mPosition = Vec3d( 0, 0, 10 );
	Vec3d mDirection = Vec3d( 0, 0, -1 );
	mDirection.Normalize();
	Vec3d mUp = Vec3d( 0, 1, 0 );

	const float zNear   = 1;
    const float zFar    = 1000;
    const float aspect  = 1.0f;
    const float fov     = 60;

    // light's projection matrix
    myPerspective( fov, aspect, zNear, zFar, gMatTextureProj );

    // light's view matrix
    myLookAt( mPosition, mPosition + mDirection, mUp, gMatTextureView );

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

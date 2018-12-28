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

// Global storage of the window size
const int gScreenWidth  = 1200;
const int gScreenHeight = 720;

// The texture that resides on the GPU
Texture gTextureDiffuse;
Texture gTextureNormals;

// The shader program that runs on the GPU
Shader gShader;
Shader gShaderColor;

Mesh gMeshPlane;
Texture gTexturePlaneDiffuse;
Texture gTexturePlaneNormals;

// The animated mesh and an animation
MD5Model gModelMD5;
MD5Anim gAnimMD5;

Vec3d gCameraPos;
Vec3d gCameraUp;
Vec3d gCameraLook;
float gMatView[ 16 ];

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
	const float radius = 300;
	gCameraPos	= Vec3d( radius * cosf( runTimeSeconds ), radius * sinf( runTimeSeconds ), 200 );
	gCameraUp	= Vec3d( 0, 0, 1 );
	myLookAt( gCameraPos, Vec3d( 0, 0, 70 ), gCameraUp, gMatView );

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

	// This sets the perspective projection matrix
	float matProj[ 16 ] = { 0 };
	const float fieldOfViewDegrees = 45;
	const float aspectRatio = static_cast< float >( gScreenHeight ) / static_cast< float >( gScreenWidth );
	const float nearDepth = 1;
	const float farDepth = 1000;
	myPerspective( fieldOfViewDegrees, aspectRatio, nearDepth, farDepth, matProj );

	//
	//	Draw a single mesh
	//

	// Set the shader program that'll be used to render the mesh
	gShader.UseProgram();

	Vec3d rayToLight = Vec3d( 1, 1, 0.25f );
	rayToLight.Normalize();
	Vec3d lightColor = Vec3d( 1, 1, 1 );

	gShader.SetUniform3f( "rayToLight", 1, rayToLight.ToPtr() );
	gShader.SetUniform3f( "lightColor", 1, lightColor.ToPtr() );

	float matIdentity[ 16 ] = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	// Send the transformation matrix to the shader
	gShader.SetUniformMatrix4f( "matModelWorld", 1, false, matIdentity );

	// Send the projection matrix to the shader
	gShader.SetUniformMatrix4f( "matProj", 1, false, matProj );
	gShader.SetUniformMatrix4f( "matView", 1, false, gMatView );

	//
	//	Draw the md5 model
	//
	{
		// Bind the texture to be rendered
		gShader.SetAndBindUniformTexture( "s_textureDiffuse", 0, GL_TEXTURE_2D, gTextureDiffuse.GetName() );
		gShader.SetAndBindUniformTexture( "s_textureNormals", 1, GL_TEXTURE_2D, gTextureNormals.GetName() );

		const int iframe = (int)( runTimeSeconds * gAnimMD5.FrameRate() );
		const md5Skeleton_t & skeleton = gAnimMD5[ iframe ];
		gModelMD5.PrepareMesh( skeleton );
		gModelMD5.UpdateVBO();
		gModelMD5.Draw();
	}

	//
	//	Draw the plane
	//
	{
		gShader.SetAndBindUniformTexture( "s_textureDiffuse", 0, GL_TEXTURE_2D, gTexturePlaneDiffuse.GetName() );
		gShader.SetAndBindUniformTexture( "s_textureNormals", 1, GL_TEXTURE_2D, gTexturePlaneNormals.GetName() );
		gMeshPlane.Draw();
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
	gShader.LoadFromFile( "data/Shaders/lit.fsh", "data/Shaders/lit.vsh" );
	gShaderColor.LoadFromFile( "data/Shaders/color.fsh", "data/Shaders/color.vsh" );

	// Load the targa data from file
	Targa targaDiffuse;
	targaDiffuse.Load( "data/Images/hellknight_d.tga", true );

	// Create a texture on the GPU and initialize it with the targa data
	gTextureDiffuse.InitWithData( targaDiffuse.DataPtr(), targaDiffuse.GetWidth(), targaDiffuse.GetHeight() );

	// Load the targa data from file
	Targa targaNormals;
	targaNormals.Load( "data/Images/hellknight_n.tga", true );

	// Create a texture on the GPU and initialize it with the targa data
	gTextureNormals.InitWithData( targaNormals.DataPtr(), targaNormals.GetWidth(), targaNormals.GetHeight() );

	// Load animated mesh for drawing
	gModelMD5.Load( "data/Meshes/hellknight.md5mesh" );
	gAnimMD5.Load( "data/Anims/hellknight_idle2.md5anim" );
	
	// Setup the view matrix
	gCameraPos	= Vec3d( 300, 0, 70 );
	gCameraUp	= Vec3d( 0, 0, 1 );
	gCameraLook	= Vec3d( -1, 0, 0 );
	myLookAt( gCameraPos, gCameraPos + gCameraLook, gCameraUp, gMatView );

	gMeshPlane.Load( "data/Meshes/plane.obj" );

	// Load the targa data from file
	Targa targaDiffusePlane;
	targaDiffusePlane.Load( "data/Images/StoneBrick_d.tga", true );

	// Create a texture on the GPU and initialize it with the targa data
	gTexturePlaneDiffuse.InitWithData( targaDiffusePlane.DataPtr(), targaDiffusePlane.GetWidth(), targaDiffusePlane.GetHeight() );

	// Load the targa data from file
	Targa targaNormalsPlane;
	targaNormalsPlane.Load( "data/Images/StoneBrick_n.tga", true );

	// Create a texture on the GPU and initialize it with the targa data
	gTexturePlaneNormals.InitWithData( targaNormalsPlane.DataPtr(), targaNormalsPlane.GetWidth(), targaNormalsPlane.GetHeight() );

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

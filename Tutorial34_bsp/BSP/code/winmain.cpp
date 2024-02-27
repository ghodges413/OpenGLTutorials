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
// #include "Terrain/Terrain.h"
// #include "VirtualTexture/VirtualTexture.h"
// #include "Atmosphere/BuildBruneton.h"
// #include "Water/Ocean.h"
// #include "Clouds/Clouds.h"
#include "BSP/Map.h"

// Global storage of the window size
int g_screenWidth  = 1200;
int g_screenHeight = 720;

Mesh g_modelScreenSpaceFarPlane;
Mesh g_modelScreenSpaceNearPlane;

RenderSurface g_renderSurface;

Vec3d g_cameraPos( 0, 0, 256 );
Vec3d g_cameraUp( 0, 0, 1 );
Vec3d g_cameraLook( 1, 0, 0 );
float g_matView[ 16 ];

// These are only used in first person mode
float g_cameraTheta = 0;
float g_cameraPhi = 3.14f * 0.5f;

bool g_noclip = true;
float g_dtSec = 0;
float g_timeTime = 0;

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
	g_timeMS += dt_ms;

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
		Vec3d ds = g_mouse.GLPosDelta() * 10.0f;
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
		const float pi = acosf( -1.0f );
		if ( g_cameraPhi > pi * 0.9f ) {
			g_cameraPhi = pi * 0.9f;
		}
		if ( g_cameraPhi < pi * 0.1f ) {
			g_cameraPhi = pi * 0.1f;
		}
		g_cameraLook = Vec3d( cosf( g_cameraTheta ) * sinf( g_cameraPhi ), sinf( g_cameraTheta ) * sinf( g_cameraPhi ), cosf( g_cameraPhi ) );

		Vec3d right = g_cameraLook.Cross( Vec3d( 0, 0, 1 ) );
		g_cameraUp = right.Cross( g_cameraLook );
		g_cameraUp.Normalize();

		Vec3d lookat = g_cameraPos + g_cameraLook;
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
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

	// Clear previous frame values
	glClear( GL_COLOR_BUFFER_BIT );
	glClear( GL_DEPTH_BUFFER_BIT );

	// This will turn on back face culling.  Uncomment to turn on.
	glCullFace( GL_BACK );
	glEnable( GL_CULL_FACE );
	glDisable( GL_CULL_FACE );

	//
	//	Draw brushes and BSP
	//
	{
		// Set the shader program that'll be used to render the mesh
		Shader * shader = g_shaderManager->GetAndUseShader( "brushes" );
	
		float matIdentity[ 16 ] = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		};

		//shader->SetUniform3f( "sunDir", 1, sunDir.ToPtr() );

		// Send the transformation matrix to the shader
		shader->SetUniformMatrix4f( "matModelWorld", 1, false, matIdentity );

		// Send the projection matrix to the shader
		shader->SetUniformMatrix4f( "matProj", 1, false, matProj );
		shader->SetUniformMatrix4f( "matView", 1, false, g_matView );

		shader->SetAndBindUniformTexture( "textureArray0", 0, GL_TEXTURE_2D_ARRAY, g_textureArray->GetName() );

		// Draw the terrain
		//DrawMap();
		DrawMap( shader, g_cameraPos );
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
void MipMapAndCompress( const Targa & image, int name, int layer, bool doCompression ) {
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

		int offX = 0;
		int offY = 0;
		int offZ = layer;
		int depth = 1;

		if ( doCompression ) {
			int size = 0;
			CompressImageDXT1( data, compressed, w, w, size );
			//CompressImageDXT5( data, compressed, w, w, numOutBytes );
		
			glCompressedTexSubImage3D( GL_TEXTURE_2D_ARRAY, lod, offX, offY, offZ, w, w, depth, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, size, compressed );
			//glCompressedTexSubImage3D( GL_TEXTURE_2D_ARRAY, lod, offX, offY, offZ, w, w, depth, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, size, compressed );
		} else {
			glTexSubImage3D( GL_TEXTURE_2D_ARRAY, lod, offX, offY, offZ, w, w, depth, GL_RGBA, GL_UNSIGNED_BYTE, data );
		}
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

	LoadMap();


	g_shaderManager = new hbShaderManager;
	g_textureManager = new TextureManager;

	g_renderSurface.CreateSurface( RS_COLOR_BUFFER | RS_DEPTH_BUFFER, g_screenWidth, g_screenHeight );
	
	// Setup the view matrix
	g_cameraPos	= Vec3d( 300, 0, 1 );
	g_cameraPos = Vec3d( 0, 0, 256 );
	g_cameraUp	= Vec3d( 0, 0, 1 );
	g_cameraLook	= Vec3d( -1, 0, 0 );
	myLookAt( g_cameraPos, g_cameraPos + g_cameraLook, g_cameraUp, g_matView );

	{
		Targa targa[ 10 ];
		targa[ 0 ].Load( "../../common/pebbles_diffuse.tga" );
		targa[ 1 ].Load( "../../common/pebbles_normal.tga" );
		targa[ 2 ].Load( "../../common/grass_diffuse.tga" );
		targa[ 3 ].Load( "../../common/grass_normal.tga" );
		targa[ 4 ].Load( "../../common/dirt_diffuse.tga" );
		targa[ 5 ].Load( "../../common/dirt_normal.tga" );
		targa[ 6 ].Load( "../../common/rock045_diffuse.tga" );
		targa[ 7 ].Load( "../../common/rock045_normal.tga" );
		targa[ 8 ].Load( "../../common/snow001_diffuse.tga" );
		targa[ 9 ].Load( "../../common/snow001_normal.tga" );

		{
			TextureOpts_t opts;
			opts.wrapS = WM_REPEAT;
			opts.wrapR = WM_REPEAT;
			opts.wrapT = WM_REPEAT;
			opts.minFilter = FM_LINEAR_MIPMAP_NEAREST;
			opts.magFilter = FM_LINEAR;
			opts.dimX = 1024;
			opts.dimY = 1024;
			opts.dimZ = 10;
			opts.type = TT_TEXTURE_2D_ARRAY;
			opts.format = FMT_BC1;
			//opts.format = FMT_RGBA8;
			g_textureArray = g_textureManager->GetTexture( "_textureArray", opts, NULL );
		}
		for ( int i = 0; i < 10; i++ ) {
			MipMapAndCompress( targa[ i ], g_textureArray->GetName(), i, true );
		}

	}

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
	UnloadMap();
	return 0;
}

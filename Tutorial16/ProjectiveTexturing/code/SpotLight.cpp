/*
 *  SpotLight.cpp
 *
 */
#include "SpotLight.h"
#include "MatrixOps.h"
#include "Graphics.h"
#include "Targa.h"
#include <math.h>

#define MY_PI 3.14159265f

/*
 ================================================================
 
 SpotLight
 
 ================================================================
 */

/*
 ================================
 SpotLight::SpotLight
 ================================
 */
SpotLight::SpotLight() :
mPosition( 0 ),
mDirection( 1, 0, 0 ),
mUp( 0, 0, 1 ),
mColor( 1, 1, 1 ),
mMaxHalfAngle( 15.0f * MY_PI / 180.0f ),
mMaxDistance( 1000 ),
mNearPlane( 1 ),
mIntensity( 600 ) {
	SetHalfAngle( mMaxHalfAngle );
	
    const float bias_matrix[ 16 ] = {
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.5f, 1.0f
	};
	memcpy( mMatBias, bias_matrix, sizeof( float ) * 16 );
}

/*
 ================================
 SpotLight::SetHalfAngle
 ================================
 */
void SpotLight::SetHalfAngle( const float radians ) {
	mMaxHalfAngle = radians;

	mCosHalfAngle = cosf( radians );
	const float denominator = 2.0f * MY_PI * ( 1.0f - mCosHalfAngle );
	mPhysicalFalloffConstant = 1.0f / denominator;
}

/*
 ================================
 SpotLight::Initialize
 ================================
 */
void SpotLight::Initialize() {
	mPosition = Vec3d( 0, 0, 10 );
	mDirection = Vec3d( 0, 0, -1 );
	mDirection.Normalize();
	mUp = Vec3d( 0, 1, 0 );

	const float zNear   = mNearPlane;
    const float zFar    = mMaxDistance;
    const float aspect  = 1.0f;
    const float fov     = mMaxHalfAngle * 2.0f * 180.0f / MY_PI;

    // light's projection matrix
    myPerspective( fov, aspect, zNear, zFar, mMatProj );

    // light's view matrix
    myLookAt( mPosition, mPosition + mDirection, mUp, mMatView );

	// Concatenate the view and projection matrix
	myMatrixMultiply( mMatView, mMatProj, mMatViewProj );

	// Create the FBO for shadow mapping
	mShadowSurface.CreateSurface( RS_DEPTH_BUFFER, 1024, 1024 );

	// Load shaders
	mShaderDepthOnly.LoadFromFile( "data/Shaders/depthOnly.fsh", "data/Shaders/depthOnly.vsh" );
	mShaderDebug.LoadFromFile( "data/Shaders/simple.fsh", "data/Shaders/simple.vsh" );

	// Load the targa data from file
	Targa targaProjective;
	targaProjective.Load( "data/Images/smiley_face.tga", true );

	// Create a texture on the GPU and initialize it with the targa data
	mTextureProjective.InitWithData( targaProjective.DataPtr(), targaProjective.GetWidth(), targaProjective.GetHeight() );
}

/*
 ================================
 SpotLight::ClearShadowMap
 ================================
 */
void SpotLight::ClearShadowMap() {
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, mShadowSurface.GetFBO() );
    glViewport( 0, 0, mShadowSurface.GetWidth(), mShadowSurface.GetHeight() );
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

/*
 ================================
 SpotLight::Update
 ================================
 */
void SpotLight::Update( Mesh & mesh, const float * matModelToWorld ) {
	glCullFace( GL_FRONT );

	// Concatenate the model to world space + world space to view space matrices
	float mvp[ 16 ] = { 0 };
	myMatrixMultiply( matModelToWorld, mMatViewProj, mvp );

	//
    // Draw the scene from the light's perspective
    //

	// get shader
	mShaderDepthOnly.UseProgram();

	// set the model view project matrix
    mShaderDepthOnly.SetUniformMatrix4f( "mvp", 1, false, mvp );

	// Update attribute values.
	const int stride = sizeof( vert_t );
	mShaderDepthOnly.SetVertexAttribPointer( "position", 3, GL_FLOAT, 0, stride, mesh.mVerts[ 0 ].pos.ToPtr() );

	// Draw
	glDrawArrays( GL_TRIANGLES, 0, mesh.mVerts.Num() );

	glCullFace( GL_BACK );
}

/*
 ================================
 SpotLight::DebugDraw
 ================================
 */
void SpotLight::DebugDraw() {
	// Positions for drawing to the lower left quarter of the screen
	Vec3d positions[ 4 ] = {
		Vec3d( -1, -0.5f, 0 ),
		Vec3d( -1, -1, 0 ),
		Vec3d( -0.5f, -1, 0 ),
		Vec3d( -0.5f, -0.5f, 0 )
	};

	// Texture coordinates
	Vec2d st[ 4 ] = {
		Vec2d( 0, 1 ),
		Vec2d( 0, 0 ),
		Vec2d( 1, 0 ),
		Vec2d( 1, 1 )
	};

	// Set the shader program that'll be used to render the offscreen texture
	mShaderDebug.UseProgram();

	// Update attribute values.
	mShaderDebug.SetVertexAttribPointer( "position", 3, GL_FLOAT, 0, 0, positions );
	mShaderDebug.SetVertexAttribPointer( "st", 2, GL_FLOAT, 0, 0, st );

	// Bind the off screen texture to be rendered
	mShaderDebug.SetAndBindUniformTexture( "s_texture", 0, GL_TEXTURE_2D, mShadowSurface.GetDepthTexture() );

	// Draw
	glDrawArrays( GL_QUADS, 0, 4 );
}


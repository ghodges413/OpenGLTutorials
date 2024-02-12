//
//  Clouds.cpp
//
#include "Clouds/Clouds.h"
#include "Clouds/BlueNoise.h"
#include "Math/Complex.h"
#include "Math/MatrixOps.h"
#include "Math/Bounds.h"
#include "Math/Random.h"
#include "Math/Math.h"
#include "Math/MatrixOps.h"
#include "Graphics/TextureManager.h"
#include "Graphics/Mesh.h"
#include "Graphics/ShaderManager.h"
#include "Miscellaneous/String.h"
#include "Miscellaneous/Types.h"

static Texture * s_worleyTexture = NULL;
static Texture * s_blueNoiseTexture = NULL;

/*
=====================================
CloudInit
=====================================
*/
void CloudInit() {
	TextureOpts_t opts;
	opts.dimX = 32;
	opts.dimY = 32;
	opts.dimZ = 64;
	opts.format = FMT_RGBA8;
	opts.magFilter = FM_LINEAR;
	opts.minFilter = FM_LINEAR;
	opts.type = TT_TEXTURE_3D;
	opts.wrapR = WM_REPEAT;
	opts.wrapS = WM_REPEAT;
	opts.wrapT = WM_REPEAT;
	s_worleyTexture = g_textureManager->GetTexture( "_worleyNoise", opts, NULL );

	//
	//	Build the Worley Noise
	//
	{
		Shader * shader = g_shaderManager->GetAndUseShader( "Clouds/WorleyGenerator" );

		glBindImageTexture( 0, s_worleyTexture->GetName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8 );

		const int workGroupSize = 8;
		shader->DispatchCompute( opts.dimX / workGroupSize, opts.dimY / workGroupSize, opts.dimZ / workGroupSize );
		glFlush();
	}
}

/*
=====================================
CloudDraw
=====================================
*/
void CloudDraw( const CloudDrawParms_t & parms ) {
	glDisable( GL_DEPTH_TEST );
	Shader * shader = g_shaderManager->GetAndUseShader( "Clouds/sdfClouds" );

	float projInverse[ 16 ];
 	myMatrixInverse4x4( parms.m_matProj.ToPtr(), projInverse );

 	float viewInverse[ 16 ];
 	myMatrixInverse4x4( parms.m_matView.ToPtr(), viewInverse );

	shader->SetUniform1f( "time", 1, &parms.m_time );
	shader->SetUniform3f( "camPos", 1, parms.m_camPos.ToPtr() );
	shader->SetUniform3f( "sunDir", 1, parms.m_dirToSun.ToPtr() );
	shader->SetUniformMatrix4f( "projInverse", 1, false, projInverse );
	shader->SetUniformMatrix4f( "viewInverse", 1, false, viewInverse );

	shader->SetAndBindUniformTexture( "worleyNoise", 0, GL_TEXTURE_3D, s_worleyTexture->GetName() );

	extern Mesh g_modelScreenSpaceFarPlane;
	extern Mesh g_modelScreenSpaceNearPlane;
	g_modelScreenSpaceNearPlane.Draw();
	glEnable( GL_DEPTH_TEST );
}

//
//  BuildBruneton.cpp
//

#include "Atmosphere/BuildBruneton.h"
#include "Math/Matrix.h"
#include "Miscellaneous/Time.h"
#include <stdio.h>
//#include "../../../Media/Textures/TextureManager.h"
#include "Atmosphere/BrunetonUtils.h"
#include "Atmosphere/BuildTransmission.h"
#include "Atmosphere/BuildIrradiance.h"
#include "Atmosphere/BuildScatter.h"
#include "Graphics/Graphics.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture.h"
#include "Graphics/ShaderManager.h"
#include "Graphics/TextureManager.h"

Vec4d * g_transmission = NULL;
Vec4d * g_deltaIrradiance = NULL;
Vec4d * g_deltaScatter = NULL;
Vec4d * g_scatterTable = NULL;
Vec4d * g_deltaLightTable = NULL;
Vec4d * g_irradianceTable = NULL;

Vec4d * g_scatterTableResorted = NULL;


Texture * g_transmittanceTexture = NULL;
Texture * g_groundIrradianceTexture = NULL;
Texture * g_inscatterTexture = NULL;
Texture * deltaGroundIrradianceTexture = NULL;
Texture * deltaScatterTexture = NULL;
Texture * deltaJTexture = NULL;

/*
 ========================================================================================================================
 
 Multi-scattering atmosphere
 Based on the paper from Bruneton 2008 "Precomputed Atmospheric Scattering"
  
 ========================================================================================================================
 */

Shader * g_shaderCopyInscatter1 = NULL;
Shader * g_shaderCopyInscatterN = NULL;
Shader * g_shaderCopyIrradiance = NULL;
Shader * g_shaderInscatter1 = NULL;
Shader * g_shaderInscatterN = NULL;
Shader * g_shaderInscatterS = NULL;
Shader * g_shaderIrradiance1 = NULL;
Shader * g_shaderIrradianceN = NULL;
Shader * g_shaderTransmittance = NULL;

void LoadShaders( hbShaderManager * shaders ) {
	g_shaderTransmittance = shaders->GetShader( "Atmosphere/Compute/transmittance" );
	g_shaderIrradiance1 = shaders->GetShader( "Atmosphere/Compute/groundIrradiance1" );
	g_shaderInscatter1 = shaders->GetShader( "Atmosphere/Compute/initInscatter" );
	g_shaderCopyInscatter1 = shaders->GetShader( "Atmosphere/Compute/copyInitInscatter" );
	g_shaderInscatterS = shaders->GetShader( "Atmosphere/Compute/inscatterS" );
	g_shaderIrradianceN = shaders->GetShader( "Atmosphere/Compute/irradianceN" );
	g_shaderInscatterN = shaders->GetShader( "Atmosphere/Compute/inscatterN" );
	g_shaderCopyIrradiance = shaders->GetShader( "Atmosphere/Compute/addDeltaIrradiance" );
	g_shaderCopyInscatterN = shaders->GetShader( "Atmosphere/Compute/addDeltaInscatterN" );
}

const int transmittanceUnit = 1;
const int groundIrradianceUnit = 2;
const int inscatterUnit = 3;
const int deltaEUnit = 4;
const int deltaScatterUnit = 5;
const int deltaJUnit = 6;


const int TRANSMITTANCE_WIDTH = 64;
const int TRANSMITTANCE_HEIGHT = 512;

const int GROUND_IRRADIANCE_WIDTH = 16;
const int GROUND_IRRADIANCE_HEIGHT = 128;

const int RES_R = 32;
const int RES_MU = 128;
const int RES_MU_S = 32;
const int RES_NU = 8;

Texture * g_inscatterTexture4D[ RES_NU ] = { NULL };

/*
 ===============================
 CreateTextures
 ===============================
 */
void CreateTextures( TextureManager * textures ) {
	const int maxSize = 4 * 4 * RES_R * RES_MU * RES_MU_S * RES_NU;
	unsigned char * nullData = new unsigned char[ maxSize ];
	memset( nullData, 0, maxSize );
	{
		TextureOpts_t opts;
		opts.type = TT_TEXTURE_2D;
		opts.magFilter = FM_LINEAR;
		opts.minFilter = FM_LINEAR;
		opts.wrapS = WM_CLAMP;
		opts.wrapT = WM_CLAMP;
		opts.format = FMT_RGBA32F;
		opts.dimX = TRANSMITTANCE_WIDTH;
		opts.dimY = TRANSMITTANCE_HEIGHT;
		unsigned char * data_ptr = NULL;
		if ( g_transmission ) {
			data_ptr = ( unsigned char * )g_transmission;
		}
		g_transmittanceTexture = textures->GetTexture( "_transmittance", opts, data_ptr );
	}

	{
		TextureOpts_t opts;
		opts.type = TT_TEXTURE_2D;
		opts.magFilter = FM_LINEAR;
		opts.minFilter = FM_LINEAR;
		opts.wrapS = WM_CLAMP;
		opts.wrapT = WM_CLAMP;
		opts.format = FMT_RGBA32F;
		opts.dimX = GROUND_IRRADIANCE_WIDTH;
		opts.dimY = GROUND_IRRADIANCE_HEIGHT;
		unsigned char * data_ptr = NULL;
		if ( g_irradianceTable ) {
			data_ptr = ( unsigned char * )g_irradianceTable;
		}
		g_groundIrradianceTexture = textures->GetTexture( "_groundIrradiance", opts, data_ptr );
	}

	{
		TextureOpts_t opts;
		opts.type = TT_TEXTURE_3D;
		opts.magFilter = FM_LINEAR;
		opts.minFilter = FM_LINEAR;
		opts.wrapS = WM_CLAMP;
		opts.wrapT = WM_CLAMP;
		opts.wrapR = WM_CLAMP;
		opts.format = FMT_RGBA32F;
		opts.dimX = RES_MU_S * RES_NU;
		opts.dimY = RES_MU;
		opts.dimZ = RES_R;
		unsigned char * data_ptr = NULL;
		if ( g_scatterTable ) {
			data_ptr = ( unsigned char * )g_scatterTable;
		}
		g_inscatterTexture = textures->GetTexture( "_inscatter", opts, data_ptr );
	}
	for ( int i = 0; i < RES_NU; ++i ) {
		TextureOpts_t opts;
		opts.type = TT_TEXTURE_3D;
		opts.magFilter = FM_LINEAR;
		opts.minFilter = FM_LINEAR;
		opts.wrapS = WM_CLAMP;
		opts.wrapT = WM_CLAMP;
		opts.wrapR = WM_CLAMP;
		opts.format = FMT_RGBA32F;
		opts.dimX = RES_MU_S;
		opts.dimY = RES_MU;
		opts.dimZ = RES_R;
		unsigned char * data_ptr = NULL;
		if ( g_scatterTableResorted ) {
			Vec4d * data4d_ptr = g_scatterTableResorted + RES_MU_S * RES_MU * RES_R * i;
			data_ptr = ( unsigned char * )( data4d_ptr );
		}

		char buffer[ 128 ];
		sprintf( buffer, "_inscatter%i", i );
		g_inscatterTexture4D[ i ] = textures->GetTexture( buffer, opts, data_ptr );
	}

	{
		TextureOpts_t opts;
		opts.type = TT_TEXTURE_2D;
		opts.magFilter = FM_LINEAR;
		opts.minFilter = FM_LINEAR;
		opts.wrapS = WM_CLAMP;
		opts.wrapT = WM_CLAMP;
		opts.format = FMT_RGBA32F;
		opts.dimX = GROUND_IRRADIANCE_WIDTH;
		opts.dimY = GROUND_IRRADIANCE_HEIGHT;
		unsigned char * data_ptr = NULL;
		if ( g_deltaIrradiance ) {
			data_ptr = ( unsigned char * )g_deltaIrradiance;
		}
		deltaGroundIrradianceTexture = textures->GetTexture( "_deltaGroundIrradiance", opts, data_ptr );
	}

	{
		TextureOpts_t opts;
		opts.type = TT_TEXTURE_3D;
		opts.magFilter = FM_LINEAR;
		opts.minFilter = FM_LINEAR;
		opts.wrapS = WM_CLAMP;
		opts.wrapT = WM_CLAMP;
		opts.wrapR = WM_CLAMP;
		opts.format = FMT_RGBA32F;
		opts.dimX = RES_MU_S * RES_NU;
		opts.dimY = RES_MU;
		opts.dimZ = RES_R;
		unsigned char * data_ptr = NULL;
		if ( g_deltaScatter ) {
			data_ptr = ( unsigned char * )g_deltaScatter;
		}
		deltaScatterTexture = textures->GetTexture( "_deltaS", opts, data_ptr );
	}
	
	{
		TextureOpts_t opts;
		opts.type = TT_TEXTURE_3D;
		opts.magFilter = FM_LINEAR;
		opts.minFilter = FM_LINEAR;
		opts.wrapS = WM_CLAMP;
		opts.wrapT = WM_CLAMP;
		opts.wrapR = WM_CLAMP;
		opts.format = FMT_RGBA32F;
		opts.dimX = RES_MU_S * RES_NU;
		opts.dimY = RES_MU;
		opts.dimZ = RES_R;
		unsigned char * data_ptr = NULL;
		if ( g_deltaLightTable ) {
			data_ptr = ( unsigned char * )g_deltaLightTable;
		}
		deltaJTexture = textures->GetTexture( "_deltaJ", opts, data_ptr );
	}

	delete[] nullData;
}

/*
 ===============================
 SetHeight
 ===============================
 */
void SetHeight( Shader * shader, const atmosphereBuildData_t & atmosData, const int layer ) {
	const float radiusTop = atmosData.radiusTop;
	const float radiusGround = atmosData.radiusGround;

    float radius = layer / ( RES_R - 1.0 );
    radius = sqrt( radiusGround * radiusGround + radius * radius * ( radiusTop * radiusTop - radiusGround * radiusGround ) );
	if ( layer == 0 ) {
		radius += 0.01f;
	}
	if ( layer == RES_R - 1 ) {
		radius -= 0.001f;
	}

	shader->SetUniform1f( "radius", 1, &radius );
}

void SetRadiusLimits( Shader * shader, const atmosphereBuildData_t & atmosData, const bool doGround, const bool doTop, const bool doScaleHeight, const bool doExtinction, const bool doScatter ) {
	const float radiusGround = atmosData.radiusGround;
	const float radiusTop = atmosData.radiusTop;
	const float scaleHeightRayleigh = atmosData.scaleHeightRayleigh;
	const float scaleHeightMie = atmosData.scaleHeightMie;
	const Vec3d betaRayleighExtinction = atmosData.betaRayleighExtinction;
	const Vec3d betaRayleighScatter = atmosData.betaRayleighScatter;
	const Vec3d betaMieExtinction = atmosData.betaMieExtinction;
	const Vec3d betaMieScatter = atmosData.betaMieScatter;

	if ( doGround ) {
		shader->SetUniform1f( "radiusGround", 1, &radiusGround );
	}
	if ( doTop ) {
		shader->SetUniform1f( "radiusTop", 1, &radiusTop );
	}
	if ( doScaleHeight ) {
		shader->SetUniform1f( "scaleHeightRayleigh", 1, &scaleHeightRayleigh );
		shader->SetUniform1f( "scaleHeightMie", 1, &scaleHeightMie );
	}
	if ( doExtinction ) {
		shader->SetUniform3f( "betaRayleighExtinction", 1, betaRayleighExtinction.ToPtr() );
		shader->SetUniform3f( "betaMieExtinction", 1, betaMieExtinction.ToPtr() );
	}
	if ( doScatter ) {
		shader->SetUniform3f( "betaRayleighScatter", 1, betaRayleighScatter.ToPtr() );
		shader->SetUniform3f( "betaMieScatter", 1, betaMieScatter.ToPtr() );
	}
}

/*
 ===============================
 ComputeScatteringCompute
 ===============================
 */
void ComputeScatteringCompute( const atmosphereBuildData_t & atmosData ) {
	const int numTramsittanceSamples = 500;
	const int numInscatterSamples = 50;
	const int numIrradianceSamples = 32;
	const int numInscatterSphericalSamples = 16;
	const float averageGroundReflectence = 0.1f;
	const float mieG = atmosData.mieG;
	const int workGroupSize = 16;
	const int workGroupSizeZ = 4;

	const Vec2d transmittanceDimensions = Vec2d( TRANSMITTANCE_WIDTH, TRANSMITTANCE_HEIGHT );
	const Vec2d irradianceDimensions = Vec2d( GROUND_IRRADIANCE_WIDTH, GROUND_IRRADIANCE_HEIGHT );
	const Vec3d scatterDimensions = Vec3d( RES_MU_S * RES_NU, RES_MU, RES_R );

	//
	// Calculate the Transmittance table
	//
	g_shaderTransmittance->UseProgram();
	g_shaderTransmittance->SetUniform1i( "numSamples", 1, &numTramsittanceSamples );
	SetRadiusLimits( g_shaderTransmittance, atmosData, true, true, true, true, false );
	g_shaderTransmittance->SetUniform2f( "dimensions", 1, transmittanceDimensions.ToPtr() );

	glBindImageTexture( 0, g_transmittanceTexture->GetName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F );
	g_shaderTransmittance->DispatchCompute( TRANSMITTANCE_WIDTH / workGroupSize, TRANSMITTANCE_HEIGHT / workGroupSize, 1 );

	//
	// Calculate the ground irradiance
	//
	g_shaderIrradiance1->UseProgram();
	SetRadiusLimits( g_shaderIrradiance1, atmosData, true, true, false, false, false );
	g_shaderIrradiance1->SetUniform2f( "dimensions", 1, irradianceDimensions.ToPtr() );
	g_shaderIrradiance1->SetAndBindUniformTexture( "transmittanceSampler", transmittanceUnit, GL_TEXTURE_2D, g_transmittanceTexture->GetName() );

	glBindImageTexture( 0, deltaGroundIrradianceTexture->GetName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F );
	g_shaderIrradiance1->DispatchCompute( GROUND_IRRADIANCE_WIDTH / workGroupSize, GROUND_IRRADIANCE_HEIGHT / workGroupSize, 1 );

	//
    // Compute single scattering Rayleigh and Mie separated in deltaSR + deltaSM
	//
	g_shaderInscatter1->UseProgram();
	g_shaderInscatter1->SetAndBindUniformTexture( "transmittanceSampler", transmittanceUnit, GL_TEXTURE_2D, g_transmittanceTexture->GetName() );
	g_shaderInscatter1->SetUniform1i( "numSamples", 1, &numInscatterSamples );
	SetRadiusLimits( g_shaderInscatter1, atmosData, true, true, true, false, true );

	glBindImageTexture( 0, deltaScatterTexture->GetName(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F );
	g_shaderInscatter1->DispatchCompute( RES_MU_S * RES_NU / workGroupSize, RES_MU / workGroupSize, RES_R / workGroupSizeZ );
	
	//
    // Calculate the delta scatter table
	//
	g_shaderCopyInscatter1->UseProgram();
	g_shaderCopyInscatter1->SetAndBindUniformTexture( "deltaScatterSampler", deltaScatterUnit, GL_TEXTURE_3D, deltaScatterTexture->GetName() );
	g_shaderCopyInscatter1->SetUniform3f( "dimensions", 1, scatterDimensions.ToPtr() );

	glBindImageTexture( 0, g_inscatterTexture->GetName(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F );
	g_shaderCopyInscatter1->DispatchCompute( RES_MU_S * RES_NU / workGroupSize, RES_MU / workGroupSize, RES_R / workGroupSizeZ );
	
	//
    // Multiple scattering
	//
    for ( int i = 0; i <= 6; ++i ) {
		//
        // Calculates the inscattered lighting at each point in the scattering table
		//
		g_shaderInscatterS->UseProgram();
		g_shaderInscatterS->SetUniform1i( "iteration", 1, &i );
		g_shaderInscatterS->SetAndBindUniformTexture( "transmittanceSampler", transmittanceUnit, GL_TEXTURE_2D, g_transmittanceTexture->GetName() );
		g_shaderInscatterS->SetAndBindUniformTexture( "deltaGroundIrradianceSampler", deltaEUnit, GL_TEXTURE_2D, deltaGroundIrradianceTexture->GetName() );
		g_shaderInscatterS->SetAndBindUniformTexture( "deltaScatterSampler", deltaScatterUnit, GL_TEXTURE_3D, deltaScatterTexture->GetName() );
		g_shaderInscatterS->SetUniform1i( "numSamples", 1, &numInscatterSphericalSamples );
		g_shaderInscatterS->SetUniform1f( "averageGroundReflectence", 1, &averageGroundReflectence );
		SetRadiusLimits( g_shaderInscatterS, atmosData, true, true, true, false, true );
		g_shaderInscatterS->SetUniform1f( "mieG", 1, &mieG );

		glBindImageTexture( 0, deltaJTexture->GetName(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F );
		g_shaderInscatterS->DispatchCompute( RES_MU_S * RES_NU / workGroupSize, RES_MU / workGroupSize, RES_R / workGroupSizeZ );

		//
        // Calculate the change in ground irradiance
		//
		g_shaderIrradianceN->UseProgram();
		g_shaderIrradianceN->SetUniform1i( "iteration", 1, &i );
		g_shaderIrradianceN->SetAndBindUniformTexture( "deltaScatterSampler", deltaScatterUnit, GL_TEXTURE_3D, deltaScatterTexture->GetName() );
		g_shaderIrradianceN->SetUniform1i( "numSamples", 1, &numIrradianceSamples );
		g_shaderIrradianceN->SetUniform1f( "mieG", 1, &mieG );
		SetRadiusLimits( g_shaderIrradianceN, atmosData, true, true, false, false, false );
		g_shaderIrradianceN->SetUniform2f( "dimensions", 1, irradianceDimensions.ToPtr() );

		glBindImageTexture( 0, deltaGroundIrradianceTexture->GetName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F );
		g_shaderIrradianceN->DispatchCompute( GROUND_IRRADIANCE_WIDTH / workGroupSize, GROUND_IRRADIANCE_HEIGHT / workGroupSize, 1 );

		//
        // Calculate the inscattering at each point in the scattering table along each and every view
		//
		g_shaderInscatterN->UseProgram();
		g_shaderInscatterN->SetAndBindUniformTexture( "transmittanceSampler", transmittanceUnit, GL_TEXTURE_2D, g_transmittanceTexture->GetName() );
		g_shaderInscatterN->SetAndBindUniformTexture( "deltaLightScatterSampler", deltaJUnit, GL_TEXTURE_3D, deltaJTexture->GetName() );
		g_shaderInscatterN->SetUniform1i( "numSamples", 1, &numInscatterSamples );
		SetRadiusLimits( g_shaderInscatterN, atmosData, true, true, false, false, false );

		glBindImageTexture( 0, deltaScatterTexture->GetName(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F );
		g_shaderInscatterN->DispatchCompute( RES_MU_S * RES_NU / workGroupSize, RES_MU / workGroupSize, RES_R / workGroupSizeZ );

		//
        // Append the change in ground irradiance to the ground irradiance
		//
		g_shaderCopyIrradiance->UseProgram();
		g_shaderCopyIrradiance->SetAndBindUniformTexture( "deltaESampler", deltaEUnit, GL_TEXTURE_2D, deltaGroundIrradianceTexture->GetName() );
		g_shaderCopyIrradiance->SetUniform2f( "dimensions", 1, irradianceDimensions.ToPtr() );

		glBindImageTexture( 0, g_groundIrradianceTexture->GetName(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F );
		g_shaderCopyIrradiance->DispatchCompute( GROUND_IRRADIANCE_WIDTH / workGroupSize, GROUND_IRRADIANCE_HEIGHT / workGroupSize, 1 );

		//
        // Append the change in scattering into the scattering table
		//
		g_shaderCopyInscatterN->UseProgram();
		g_shaderCopyInscatterN->SetAndBindUniformTexture( "deltaScatterSampler", deltaScatterUnit, GL_TEXTURE_3D, deltaScatterTexture->GetName() );
		g_shaderCopyInscatterN->SetUniform3f( "dimensions", 1, scatterDimensions.ToPtr() );

		glBindImageTexture( 0, g_inscatterTexture->GetName(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F );
		g_shaderCopyInscatterN->DispatchCompute( RES_MU_S * RES_NU / workGroupSize, RES_MU / workGroupSize, RES_R / workGroupSizeZ );
	}
}

/*
 ===============================
 ComputeCPU
 ===============================
 */
void ComputeCPU( const atmosphereBuildData_t & atmosData ) {
	BrunetonData_t data;
	data.radiusGround = atmosData.radiusGround;
	data.radiusTop = atmosData.radiusTop;
	
	data.betaRayleighExtinction = atmosData.betaRayleighExtinction;
	data.betaMieExtinction = atmosData.betaMieExtinction;
	data.scaleHeightRayleigh = atmosData.scaleHeightRayleigh;
	data.scaleHeightMie = atmosData.scaleHeightMie;
	data.betaRayleighScatter = atmosData.betaRayleighScatter;
	data.betaMieScatter = atmosData.betaMieScatter;

	data.dimTransmission = Vec2d( TRANSMITTANCE_WIDTH, TRANSMITTANCE_HEIGHT );
	data.numSamplesExtinction = 512;

	data.dimIrradiance = Vec2d( GROUND_IRRADIANCE_WIDTH, GROUND_IRRADIANCE_HEIGHT );
	data.numSamplesIrradiance = 32;

	data.dimScatter = Vec4d( RES_MU_S, RES_NU, RES_MU, RES_R );
	data.numSamplesScatter = 64;

	data.numSamplesScatterSpherical = 32;
	data.averageGroundReflectence = 0.1f;
	data.mieG = 0.8f;

	// Create the tables for holding the final data
	g_transmission = new Vec4d[ TRANSMITTANCE_WIDTH * TRANSMITTANCE_HEIGHT ];
	g_irradianceTable = new Vec4d[ GROUND_IRRADIANCE_WIDTH * GROUND_IRRADIANCE_HEIGHT ];
	g_scatterTable = new Vec4d[ RES_MU_S * RES_NU * RES_MU * RES_R ];

	// Temporary tables for calculating the multi-scattering
	g_deltaIrradiance = new Vec4d[ GROUND_IRRADIANCE_WIDTH * GROUND_IRRADIANCE_HEIGHT ];
	g_deltaScatter = new Vec4d[ RES_MU_S * RES_NU * RES_MU * RES_R ];
	g_deltaLightTable = new Vec4d[ RES_MU_S * RES_NU * RES_MU * RES_R ];

	// Build the transmission table... same as single scattering
	BuildTransmission( data, g_transmission );

	// Calculate the initial ground ambience from the extinction of sunlight through the atmosphere
	// (only do this to the temporary irradiance table... we'll do a direct lighting
	// calculation at render-time, and we don't want it in the pre-calculated table)
	InitIrradiance( data, g_transmission, g_deltaIrradiance );

	// Calculate the single scattering from the sun
	SingleScattering( data, g_transmission, g_scatterTable, g_deltaScatter );
	
//#define CPU_MULTISCATTERING	// uncomment to perform cpu multi-scattering
#if defined( CPU_MULTISCATTERING )
	// Perform the multi-scattering
	for ( int i = 0; i <= 6; ++i ) {
		//DeltaLightIrradiance( data, g_transmission, g_deltaIrradiance, g_deltaScatter, g_deltaLightTable );
		DeltaLightIrradianceMT( data, g_transmission, g_deltaIrradiance, g_deltaScatter, g_deltaLightTable );

		DeltaGroundIrradiance( data, g_deltaScatter, g_deltaIrradiance );

		DeltaScatter( data, g_transmission, g_deltaLightTable, g_deltaScatter );

		// Add the change in ground irradiance into the irradiance table
		for ( int j = 0; j < GROUND_IRRADIANCE_WIDTH * GROUND_IRRADIANCE_HEIGHT; ++j ) {
			g_irradianceTable[ j ] += g_deltaIrradiance[ j ];
		}

		// Add the change in scattering into the scattering table
		for ( int j = 0; j < RES_MU_S * RES_NU * RES_MU * RES_R; ++j ) {
			g_scatterTable[ j ] += g_deltaLightTable[ j ];
		}
	}
#endif

	// Re-sort the scatter table
	g_scatterTableResorted = new Vec4d[ RES_MU_S * RES_NU * RES_MU * RES_R ];

	for ( int x = 0; x < data.dimScatter.x; ++x ) {
		for ( int y = 0; y < data.dimScatter.y; ++y ) {
			for ( int z = 0; z < data.dimScatter.z; ++z ) {
				for ( int w = 0; w < data.dimScatter.w; ++w ) {
					//data.dimScatter = Vec4d( RES_MU_S, RES_NU, RES_MU, RES_R );
					//opts.dimX = RES_MU_S * RES_NU;
					//opts.dimY = RES_MU;
					//opts.dimZ = RES_R;
					int idx = x;
					idx += data.dimScatter.x * y;
					idx += data.dimScatter.x * data.dimScatter.y * z;
					idx += data.dimScatter.x * data.dimScatter.y * data.dimScatter.z * w;

					int mus = x;
					int nu = y;
					int mu = z;
					int r = w;
					
					//opts.dimX = RES_MU_S;
					//opts.dimY = RES_MU;
					//opts.dimZ = RES_R;
					int idxR = mus;
					idxR += RES_MU_S * mu;
					idxR += RES_MU_S * RES_MU * r;
					idxR += RES_MU_S * RES_MU * RES_R * nu;
					g_scatterTableResorted[ idxR ] = g_scatterTable[ idx ];					
				}
			}
		}
	}
}

//#define BRUNETON_CPU

/*
 ===============================
 BuildAtmosphereBruneton
 ===============================
 */
void BuildAtmosphereBruneton( const atmosphereBuildData_t & atmosData ) {
#if defined ( BRUNETON_CPU )
	const bool doCpu = true;
#else
	const bool doCpu = false;
#endif
	if ( doCpu ) {
		ComputeCPU( atmosData );
	}
	LoadShaders( g_shaderManager );
	CreateTextures( g_textureManager );
	if ( !doCpu ) {
		ComputeScatteringCompute( atmosData );
	}
	if ( g_transmission ) {
		delete[] g_transmission;
		g_transmission = NULL;
	}
	if ( g_scatterTable ) {
		delete[] g_scatterTable;
		g_scatterTable = NULL;
	}
	if ( g_deltaIrradiance ) {
		delete[] g_deltaIrradiance;
		g_deltaIrradiance = NULL;
	}
	if ( g_deltaScatter ) {
		delete[] g_deltaScatter;
		g_deltaScatter = NULL;
	}
	if ( g_deltaLightTable ) {
		delete[] g_deltaLightTable;
		g_deltaLightTable = NULL;
	}
	if ( g_irradianceTable ) {
		delete[] g_irradianceTable;
		g_irradianceTable = NULL;
	}
	if ( g_scatterTableResorted ) {
		delete[] g_scatterTableResorted;
		g_scatterTableResorted = NULL;
	}
}

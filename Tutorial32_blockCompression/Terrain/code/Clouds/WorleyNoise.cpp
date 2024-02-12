//
//  WorleyNoise.cpp
//
#include "Clouds/WorleyNoise.h"
#include "Math/Vector.h"
#include "Math/Random.h"
#include <math.h>

/*
The MIT License (MIT)

Copyright(c) 2017 Sébastien Hillaire

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated 
documentation files (the "Software"), to deal in the Software without restriction, including without limitation 
the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, 
and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED 
TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF 
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
DEALINGS IN THE SOFTWARE.

https://github.com/sebh/TileableVolumeNoise/tree/master
*/

// Perlin noise based on GLM http://glm.g-truc.net
// Worley noise based on https://www.shadertoy.com/view/Xl2XRR by Marc-Andre Loyer

static Vec3d floor( Vec3d x ) {
	Vec3d v;
	v.x = floorf( x.x );
	v.y = floorf( x.y );
	v.z = floorf( x.z );
	return v;
}

static float fractf( float x ) {
	float v = x - floor( x );
	return v;
}

static Vec3d fract( Vec3d x ) {
	Vec3d v;
	v.x = fractf( x.x );
	v.y = fractf( x.y );
	v.z = fractf( x.z );
	return v;
}

static float lerpf( float a, float b, float t ) {
	return a * ( 1.0f - t ) + b * t;
}

static Vec3d mod( Vec3d x, float y ) {
	Vec3d v;
	v.x = x.x - y * floorf( x.x / y );
	v.y = x.y - y * floorf( x.y / y );
	v.z = x.z - y * floorf( x.z / y );
	return v;
}

/*
=====================================
Tileable3dNoise::Hash
=====================================
*/
float Tileable3dNoise::Hash( float n ) {
	return fractf( sinf( n + 1.951f ) * 43758.5453f );
}

/*
=====================================
Tileable3dNoise::Noise
// hash based 3d value noise
=====================================
*/
float Tileable3dNoise::Noise( const Vec3d & x ) {
	Vec3d p = floor( x );
	Vec3d f = fract( x );

	f = f * f * ( Vec3d( 3.0f ) - Vec3d( 2.0f ) * f );
	float n = p.x + p.y * 57.0f + 113.0f * p.z;

	float a = lerpf( Hash( n + 0.0f ), Hash( n + 1.0f ), f.x );
	float b = lerpf( Hash( n + 57.0f ), Hash( n + 58.0f ), f.x );

	float c = lerpf( Hash( n + 113.0f ), Hash( n + 114.0f ), f.x );
	float d = lerpf( Hash( n + 170.0f ), Hash( n + 171.0f ), f.x );

	float ab = lerpf( a, b, f.y );
	float cd = lerpf( c, d, f.y );

	return lerpf( ab, cd, f.z );
}

/*
=====================================
GenerateBlueNoise32
Generates a 32 x 32 blue noise image by using a random generator and a high pass filter
=====================================
*/
float Tileable3dNoise::Cells( const Vec3d & p, float cellCount ) {
	const Vec3d pCell = p * cellCount;
	float d = 1.0e10;
	for ( int xo = -1; xo <= 1; xo++ ) {
		for ( int yo = -1; yo <= 1; yo++ ) {
			for ( int zo = -1; zo <= 1; zo++ ) {
				Vec3d tp = floor( pCell ) + Vec3d( xo, yo, zo );

				tp = pCell - tp - Noise( mod( tp, cellCount / 1 ) );

				d = std::fminf( d, tp.Dot( tp ) );
			}
		}
	}

	d = std::fminf( d, 1.0f );
	d = std::fmaxf( d, 0.0f );
	return d;
}

/*
=====================================
GenerateBlueNoise32
Generates a 32 x 32 blue noise image by using a random generator and a high pass filter
=====================================
*/
float Tileable3dNoise::WorleyNoise( const Vec3d & p, float cellCount ) {
	return Cells( p, cellCount );
}

static Vec4d mulComp( Vec4d a, Vec4d b ) {
	Vec4d v;
	v.x = a.x * b.x;
	v.y = a.y * b.y;
	v.z = a.z * b.z;
	v.w = a.w * b.w;
	return v;
}

/*
=====================================
GenerateBlueNoise32
Generates a 32 x 32 blue noise image by using a random generator and a high pass filter
=====================================
*/
float Tileable3dNoise::PerlinNoise( const Vec3d & pIn, float frequency, int octaveCount ) {
	const float octaveFrenquencyFactor = 2;			// noise frequency factor between octave, forced to 2

	// Compute the sum for each octave
	float sum = 0.0f;
	float weightSum = 0.0f;
	float weight = 0.5f;
	for ( int oct = 0; oct < octaveCount; oct++ ) {
		// Perlin vec3 is bugged in GLM on the Z axis :(, black stripes are visible
		// So instead we use 4d Perlin and only use xyz...
		//Vec3d p(x * freq, y * freq, z * freq);
		//float val = glm::perlin(p, Vec3d(freq)) *0.5 + 0.5;

		Vec4d p = mulComp( Vec4d( pIn.x, pIn.y, pIn.z, 0.0f ), Vec4d( frequency ) );
//		float val = glm::perlin( p, Vec4d( frequency ) );
		float val = 0;

		sum += val * weight;
		weightSum += weight;

		weight *= weight;
		frequency *= octaveFrenquencyFactor;
	}

	float noise = ( sum / weightSum ) * 0.5f + 0.5f;
	noise = std::fminf( noise, 1.0f );
	noise = std::fmaxf( noise, 0.0f );
	return noise;
 }












// the remap function used in the shaders as described in Gpu Pro 7. It must match when using pre packed textures
float remap(float originalValue, float originalMin, float originalMax, float newMin, float newMax)
{
	return newMin + (((originalValue - originalMin) / (originalMax - originalMin)) * (newMax - newMin));
}

#if 0
int main (int argc, char *argv[])
{   
	//
	// Exemple of tileable Perlin noise texture generation
	//

	/*
	unsigned int gPerlinNoiseTextureSize = 32;
	unsigned char* perlinNoiseTexels = (unsigned char*)malloc(gPerlinNoiseTextureSize*gPerlinNoiseTextureSize*gPerlinNoiseTextureSize * sizeof(unsigned char));

	// Generate Perlin noise source
	const glm::vec3 normFactPerlin = glm::vec3(1.0f / float(gPerlinNoiseTextureSize));
	parallel_for(int(0), int(gPerlinNoiseTextureSize), [&](int s) //for (int s=0; s<giPerlinNoiseTextureSize; s++)
	{
		for (int t = 0; t<gPerlinNoiseTextureSize; t++)
		{
			for (int r = 0; r<gPerlinNoiseTextureSize; r++)
			{
				glm::vec3 coord = glm::vec3(s, t, r) * normFactPerlin;

				const int octaveCount = 1;
				const float frequency = 8;
				float noise = Tileable3dNoise::PerlinNoise(coord, frequency, octaveCount);
				noise *= 255.0f;

				int addr = r*gPerlinNoiseTextureSize*gPerlinNoiseTextureSize + t*gPerlinNoiseTextureSize + s;
				perlinNoiseTexels[addr] = unsigned char(noise);

			}
		}
	}
	); // end parallel_for
	free(perlinNoiseTexels);

	//
	// Exemple of tileable Worley noise texture generation
	//
	unsigned int gWorleyNoiseTextureSize = 32;
	unsigned char* worleyNoiseTexels = (unsigned char*)malloc(gWorleyNoiseTextureSize*gWorleyNoiseTextureSize*gWorleyNoiseTextureSize * sizeof(unsigned char));

	const glm::vec3 normFactWorley = glm::vec3(1.0f / float(gWorleyNoiseTextureSize));
	parallel_for(int(0), int(gWorleyNoiseTextureSize), [&](int s) //for (int s = 0; s<giWorleyNoiseTextureSize; s++)
	{
		for (int t = 0; t<gWorleyNoiseTextureSize; t++)
		{
			for (int r = 0; r<gWorleyNoiseTextureSize; r++)
			{
				glm::vec3 coord = glm::vec3(s, t, r) * normFactPerlin;

				const float cellCount = 3;
				float noise = 1.0 - Tileable3dNoise::WorleyNoise(coord, cellCount);
				noise *= 255.0f;

				int addr = r*gWorleyNoiseTextureSize*gWorleyNoiseTextureSize + t*gWorleyNoiseTextureSize + s;
				worleyNoiseTexels[addr] = unsigned char(noise);
			}
		}
	}
	); // end parallel_for
	free(worleyNoiseTexels);
	*/



	//
	// Generate cloud shape and erosion texture similarly GPU Pro 7 chapter II-4
	//

	// Frequence multiplicator. No boudary check etc. but fine for this small tool.
	const float frequenceMul[6] = { 2.0f,8.0f,14.0f,20.0f,26.0f,32.0f };	// special weight for perling worley

														// Cloud base shape (will be used to generate PerlingWorley noise in he shader)
														// Note: all channels could be combined once here to reduce memory bandwith requirements.
	int cloudBaseShapeTextureSize = 128;				// !!! If this is reduce, you hsould also reduce the number of frequency in the fmb noise  !!!
	int gCloudBaseShapeTextureSize = cloudBaseShapeTextureSize;
	int cloudBaseShapeRowBytes = cloudBaseShapeTextureSize * sizeof(unsigned char) * 4;
	int cloudBaseShapeSliceBytes = cloudBaseShapeRowBytes * cloudBaseShapeTextureSize;
	int cloudBaseShapeVolumeBytes = cloudBaseShapeSliceBytes * cloudBaseShapeTextureSize;
	unsigned char* cloudBaseShapeTexels = (unsigned char*)malloc(cloudBaseShapeVolumeBytes);
	unsigned char* cloudBaseShapeTexelsPacked = (unsigned char*)malloc(cloudBaseShapeVolumeBytes);
	//parallel_for(int(0), int(cloudBaseShapeTextureSize), [&](int s) //
	for (int s = 0; s<gCloudBaseShapeTextureSize; s++)
	{
		const glm::vec3 normFact = glm::vec3(1.0f / float(cloudBaseShapeTextureSize));
		for (int t = 0; t<cloudBaseShapeTextureSize; t++)
		{
			for (int r = 0; r<cloudBaseShapeTextureSize; r++)
			{
				glm::vec3 coord = glm::vec3(s, t, r) * normFact;

				// Perlin FBM noise
				const int octaveCount = 3;
				const float frequency = 8.0f;
				float perlinNoise = Tileable3dNoise::PerlinNoise(coord, frequency, octaveCount);

				float PerlinWorleyNoise = 0.0f;
				{
					const float cellCount = 4;
					const float worleyNoise0 = (1.0f - Tileable3dNoise::WorleyNoise(coord, cellCount * frequenceMul[0]));
					const float worleyNoise1 = (1.0f - Tileable3dNoise::WorleyNoise(coord, cellCount * frequenceMul[1]));
					const float worleyNoise2 = (1.0f - Tileable3dNoise::WorleyNoise(coord, cellCount * frequenceMul[2]));
					const float worleyNoise3 = (1.0f - Tileable3dNoise::WorleyNoise(coord, cellCount * frequenceMul[3]));
					const float worleyNoise4 = (1.0f - Tileable3dNoise::WorleyNoise(coord, cellCount * frequenceMul[4]));
					const float worleyNoise5 = (1.0f - Tileable3dNoise::WorleyNoise(coord, cellCount * frequenceMul[5]));	// half the frequency of texel, we should not go further (with cellCount = 32 and texture size = 64)

					float worleyFBM = worleyNoise0*0.625f + worleyNoise1*0.25f + worleyNoise2*0.125f;

					// Perlin Worley is based on description in GPU Pro 7: Real Time Volumetric Cloudscapes.
					// However it is not clear the text and the image are matching: images does not seem to match what the result  from the description in text would give.
					// Also there are a lot of fudge factor in the code, e.g. *0.2, so it is really up to you to fine the formula you like.
					//PerlinWorleyNoise = remap(worleyFBM, 0.0, 1.0, 0.0, perlinNoise);	// Matches better what figure 4.7 (not the following up text description p.101). Maps worley between newMin as 0 and 
					PerlinWorleyNoise = remap(perlinNoise, 0.0f, 1.0f, worleyFBM, 1.0f);	// mapping perlin noise in between worley as minimum and 1.0 as maximum (as described in text of p.101 of GPU Pro 7) 
				}

				const float cellCount = 4;
				float worleyNoise0 = (1.0f - Tileable3dNoise::WorleyNoise(coord, cellCount * 1));
				float worleyNoise1 = (1.0f - Tileable3dNoise::WorleyNoise(coord, cellCount * 2));
				float worleyNoise2 = (1.0f - Tileable3dNoise::WorleyNoise(coord, cellCount * 4));
				float worleyNoise3 = (1.0f - Tileable3dNoise::WorleyNoise(coord, cellCount * 8));
				float worleyNoise4 = (1.0f - Tileable3dNoise::WorleyNoise(coord, cellCount * 16));
				//float worleyNoise5 = (1.0f - Tileable3dNoise::WorleyNoise(coord, cellCount * 32));	//cellCount=2 -> half the frequency of texel, we should not go further (with cellCount = 32 and texture size = 64)

				// Three frequency of Worley FBM noise
				float worleyFBM0 = worleyNoise1*0.625f + worleyNoise2*0.25f + worleyNoise3*0.125f;
				float worleyFBM1 = worleyNoise2*0.625f + worleyNoise3*0.25f + worleyNoise4*0.125f;
				//float worleyFBM2 = worleyNoise3*0.625f + worleyNoise4*0.25f + worleyNoise5*0.125f;
				float worleyFBM2 = worleyNoise3*0.75f + worleyNoise4*0.25f; // cellCount=4 -> worleyNoise5 is just noise due to sampling frequency=texel frequency. So only take into account 2 frequencies for FBM

				int addr = r*cloudBaseShapeTextureSize*cloudBaseShapeTextureSize + t*cloudBaseShapeTextureSize + s;

				addr *= 4;
				cloudBaseShapeTexels[addr] = unsigned char(255.0f*PerlinWorleyNoise);
				cloudBaseShapeTexels[addr + 1] = unsigned char(255.0f*worleyFBM0);
				cloudBaseShapeTexels[addr + 2] = unsigned char(255.0f*worleyFBM1);
				cloudBaseShapeTexels[addr + 3] = unsigned char(255.0f*worleyFBM2);

				float value = 0.0;
				{
					// pack the channels for direct usage in shader
					float lowFreqFBM = worleyFBM0*0.625f + worleyFBM1*0.25f + worleyFBM2*0.125f;
					float baseCloud = PerlinWorleyNoise;
					value = remap(baseCloud, -(1.0f - lowFreqFBM), 1.0f, 0.0f, 1.0f);
					// Saturate
					value = std::fminf(value, 1.0f);
					value = std::fmaxf(value, 0.0f);
				}
				cloudBaseShapeTexelsPacked[addr] = unsigned char(255.0f*value);
				cloudBaseShapeTexelsPacked[addr + 1] = unsigned char(255.0f*value);
				cloudBaseShapeTexelsPacked[addr + 2] = unsigned char(255.0f*value);
				cloudBaseShapeTexelsPacked[addr + 3] = unsigned char(255.0f);
			}
		}
	}
//	); // end parallel_for
	{
		int width = cloudBaseShapeTextureSize*cloudBaseShapeTextureSize;
		int height = cloudBaseShapeTextureSize;
		writeTGA("noiseShape.tga",       width, height, cloudBaseShapeTexels);
		writeTGA("noiseShapePacked.tga", width, height, cloudBaseShapeTexelsPacked);
	}






	// Detail texture behing different frequency of Worley noise
	// Note: all channels could be combined once here to reduce memory bandwith requirements.
	int cloudErosionTextureSize = 32;
	int cloudErosionRowBytes = cloudErosionTextureSize * sizeof(unsigned char) * 4;
	int cloudErosionSliceBytes = cloudErosionRowBytes * cloudErosionTextureSize;
	int cloudErosionVolumeBytes = cloudErosionSliceBytes * cloudErosionTextureSize;
	unsigned char* cloudErosionTexels = (unsigned char*)malloc(cloudErosionVolumeBytes);
	unsigned char* cloudErosionTexelsPacked = (unsigned char*)malloc(cloudErosionVolumeBytes);
	parallel_for(int(0), int(cloudErosionTextureSize), [&](int s) //for (int s = 0; s<gCloudErosionTextureSize; s++)
	{
		const glm::vec3 normFact = glm::vec3(1.0f / float(cloudErosionTextureSize));
		for (int t = 0; t<cloudErosionTextureSize; t++)
		{
			for (int r = 0; r<cloudErosionTextureSize; r++)
			{
				glm::vec3 coord = glm::vec3(s, t, r) * normFact;

#if 1
				// 3 octaves
				const float cellCount = 2;
				float worleyNoise0 = (1.0f - Tileable3dNoise::WorleyNoise(coord, cellCount * 1));
				float worleyNoise1 = (1.0f - Tileable3dNoise::WorleyNoise(coord, cellCount * 2));
				float worleyNoise2 = (1.0f - Tileable3dNoise::WorleyNoise(coord, cellCount * 4));
				float worleyNoise3 = (1.0f - Tileable3dNoise::WorleyNoise(coord, cellCount * 8));
				float worleyFBM0 = worleyNoise0*0.625f + worleyNoise1*0.25f + worleyNoise2*0.125f;
				float worleyFBM1 = worleyNoise1*0.625f + worleyNoise2*0.25f + worleyNoise3*0.125f;
				float worleyFBM2 = worleyNoise2*0.75f + worleyNoise3*0.25f; // cellCount=4 -> worleyNoise4 is just noise due to sampling frequency=texel freque. So only take into account 2 frequencies for FBM
#else
				// 2 octaves
				float worleyNoise0 = (1.0f - Tileable3dNoise::WorleyNoise(coord, 4));
				float worleyNoise1 = (1.0f - Tileable3dNoise::WorleyNoise(coord, 7));
				float worleyNoise2 = (1.0f - Tileable3dNoise::WorleyNoise(coord, 10));
				float worleyNoise3 = (1.0f - Tileable3dNoise::WorleyNoise(coord, 13));
				float worleyFBM0 = worleyNoise0*0.75f + worleyNoise1*0.25f;
				float worleyFBM1 = worleyNoise1*0.75f + worleyNoise2*0.25f;
				float worleyFBM2 = worleyNoise2*0.75f + worleyNoise3*0.25f;
#endif

				int addr = r*cloudErosionTextureSize*cloudErosionTextureSize + t*cloudErosionTextureSize + s;
				addr *= 4;
				cloudErosionTexels[addr] = unsigned char(255.0f*worleyFBM0);
				cloudErosionTexels[addr + 1] = unsigned char(255.0f*worleyFBM1);
				cloudErosionTexels[addr + 2] = unsigned char(255.0f*worleyFBM2);
				cloudErosionTexels[addr + 3] = unsigned char(255.0f);

				float value = 0.0;
				{
					value = worleyFBM0*0.625f + worleyFBM1*0.25f + worleyFBM2*0.125f;
				}
				cloudErosionTexelsPacked[addr] = unsigned char(255.0f * value);
				cloudErosionTexelsPacked[addr + 1] = unsigned char(255.0f * value);
				cloudErosionTexelsPacked[addr + 2] = unsigned char(255.0f * value);
				cloudErosionTexelsPacked[addr + 3] = unsigned char(255.0f);
			}
		}
	}
	); // end parallel_for

// 	{
// 		int width = cloudErosionTextureSize*cloudErosionTextureSize;
// 		int height = cloudErosionTextureSize;
// 		writeTGA("noiseErosion.tga",       width, height, cloudErosionTexels);
// 		writeTGA("noiseErosionPacked.tga", width, height, cloudErosionTexelsPacked);
// 	}

#if 0
	// Debug tileability using a 3x3 tile of the same slice, see if edges appears.

	auto debugPrintTileability = [&](auto addrSrc2, auto addrDst2, const char* debugStr) 
	{
		for (int r = 0; r < cloudBaseShapeTextureSize; r += cloudBaseShapeTextureSize / 8)
		{
			unsigned char* debugImg = (unsigned char*)malloc(9 * cloudBaseShapeSliceBytes);
			for (int i = 0; i < cloudBaseShapeTextureSize; i++)
			{
				int t = i;

				// copy the row into the 9 debug texture tiles
				auto addrSrc = [&]()
				{
					return addrSrc2(r, t, cloudBaseShapeSliceBytes, cloudBaseShapeRowBytes);
				};
				auto addrDst = [&](auto x, auto y)
				{
					return addrDst2(x, y, t, cloudBaseShapeSliceBytes, cloudBaseShapeRowBytes);
				};

				memcpy(debugImg + addrDst(0, 0), cloudBaseShapeTexelsPacked + addrSrc(), cloudBaseShapeRowBytes);
				memcpy(debugImg + addrDst(1, 0), cloudBaseShapeTexelsPacked + addrSrc(), cloudBaseShapeRowBytes);
				memcpy(debugImg + addrDst(2, 0), cloudBaseShapeTexelsPacked + addrSrc(), cloudBaseShapeRowBytes);
				memcpy(debugImg + addrDst(0, 1), cloudBaseShapeTexelsPacked + addrSrc(), cloudBaseShapeRowBytes);
				memcpy(debugImg + addrDst(1, 1), cloudBaseShapeTexelsPacked + addrSrc(), cloudBaseShapeRowBytes);
				memcpy(debugImg + addrDst(2, 1), cloudBaseShapeTexelsPacked + addrSrc(), cloudBaseShapeRowBytes);
				memcpy(debugImg + addrDst(0, 2), cloudBaseShapeTexelsPacked + addrSrc(), cloudBaseShapeRowBytes);
				memcpy(debugImg + addrDst(1, 2), cloudBaseShapeTexelsPacked + addrSrc(), cloudBaseShapeRowBytes);
				memcpy(debugImg + addrDst(2, 2), cloudBaseShapeTexelsPacked + addrSrc(), cloudBaseShapeRowBytes);


			}
			char fileName[256];
			sprintf_s(fileName, 256, "debugBase%s%i.tga", debugStr, r);
			writeTGA(fileName, cloudBaseShapeTextureSize * 3, cloudBaseShapeTextureSize * 3, debugImg);
		}

		for (int r = 0; r < cloudErosionTextureSize; r += cloudErosionTextureSize / 8)
		{
			unsigned char* debugImg = (unsigned char*)malloc(9*cloudErosionSliceBytes);
			for (int i = 0; i < cloudErosionTextureSize; i++)
			{
				int t = i;

				auto addrSrc = [&]()
				{
					return addrSrc2(r, t, cloudErosionSliceBytes, cloudErosionRowBytes);
				};
				auto addrDst = [&](auto x, auto y)
				{
					return addrDst2(x, y, t, cloudErosionSliceBytes, cloudErosionRowBytes);
				};

				memcpy(debugImg + addrDst(0,0), cloudErosionTexelsPacked + addrSrc(), cloudErosionRowBytes);
				memcpy(debugImg + addrDst(1,0), cloudErosionTexelsPacked + addrSrc(), cloudErosionRowBytes);
				memcpy(debugImg + addrDst(2,0), cloudErosionTexelsPacked + addrSrc(), cloudErosionRowBytes);
				memcpy(debugImg + addrDst(0,1), cloudErosionTexelsPacked + addrSrc(), cloudErosionRowBytes);
				memcpy(debugImg + addrDst(1,1), cloudErosionTexelsPacked + addrSrc(), cloudErosionRowBytes);
				memcpy(debugImg + addrDst(2,1), cloudErosionTexelsPacked + addrSrc(), cloudErosionRowBytes);
				memcpy(debugImg + addrDst(0,2), cloudErosionTexelsPacked + addrSrc(), cloudErosionRowBytes);
				memcpy(debugImg + addrDst(1,2), cloudErosionTexelsPacked + addrSrc(), cloudErosionRowBytes);
				memcpy(debugImg + addrDst(2,2), cloudErosionTexelsPacked + addrSrc(), cloudErosionRowBytes);
			}
			char fileName[256];
			sprintf_s(fileName, 256, "debugErosion%s%i.tga", debugStr, r);
			writeTGA(fileName, cloudErosionTextureSize*3, cloudErosionTextureSize*3, debugImg);
		}
	};

	auto addrDst = [](auto x, auto y, auto t, auto sliceBytes, auto rowBytes)
	{
		return x * rowBytes + y * sliceBytes * 3 + t * rowBytes * 3;
	};
	auto addrSrcXY = [](auto r, auto t, auto sliceBytes, auto rowBytes)
	{
		return r*sliceBytes + t*rowBytes;
	};
	auto addrSrcXZ = [](auto r, auto t, auto sliceBytes, auto rowBytes)
	{
		return t*sliceBytes + r*rowBytes;
	};

	debugPrintTileability(addrSrcXY, addrDst, "XY");
	debugPrintTileability(addrSrcXZ, addrDst, "XZ");
#endif

	free(cloudErosionTexels);
	free(cloudErosionTexelsPacked);
	free(cloudBaseShapeTexels);
	free(cloudBaseShapeTexelsPacked);

    return 0;
}
#endif
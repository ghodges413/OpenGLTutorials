//
//  Rasterization.cpp
//
#include "Rasterization/Rasterization.h"
#include "Math/MatrixOps.h"
#include "Math/Matrix.h"
#include "Graphics/Graphics.h"
#include "Graphics/TextureManager.h"
#include "Graphics/ShaderManager.h"
#include "Graphics/Targa.h"
#include <math.h>
#include <algorithm>
#include <stdlib.h>
#include <assert.h>


extern int g_screenWidth;
extern int g_screenHeight;

extern Mesh g_modelScreenSpaceFarPlane;
extern Mesh g_modelScreenSpaceNearPlane;

static Targa s_image;
static int s_maxMip = 0;
static uint8 * s_imageBuffer = NULL;
static uint8 * s_anisotropicBuffer = NULL;
static int s_anisotropicWidth = 0;

Texture * g_rasterTexture = NULL;

uint8 * g_rasterBuffer = NULL;

/*
================================
ClearBuffer
================================
*/
void ClearBuffer() {
	for ( int y = 0; y < g_screenHeight; y++ ) {
		for ( int x = 0; x < g_screenWidth; x++ ) {
			int idx = x + y * g_screenWidth;
			g_rasterBuffer[ 4 * idx + 0 ] = x % 255;
			g_rasterBuffer[ 4 * idx + 1 ] = y % 255;
			g_rasterBuffer[ 4 * idx + 2 ] = 255;
			g_rasterBuffer[ 4 * idx + 3 ] = 255;
		}
	}
}

/*
================================
InitRasterizer
================================
*/
void InitRasterizer() {
	TextureOpts_t opts;
	opts.type = TT_TEXTURE_2D;
	opts.magFilter = FM_NEAREST;
	opts.minFilter = FM_NEAREST;
	opts.wrapS = WM_CLAMP;
	opts.wrapT = WM_CLAMP;
	opts.format = FMT_RGBA8;
	opts.dimX = g_screenWidth;
	opts.dimY = g_screenHeight;
	g_rasterBuffer = (uint8 *)malloc( g_screenWidth * g_screenHeight * 4 );
	ClearBuffer();

	g_rasterTexture = g_textureManager->GetTexture( "_rasterBuffer", opts, g_rasterBuffer );
	//s_image.Load( "../../common/smiley_face.tga" );
	s_image.Load( "../../common/StoneBrick_d.tga" );

	// Calculate mips
	int w = s_image.GetWidth();
	int numTexels = 0;
	int totalWidth = 0;
	s_maxMip = 0;
	while ( w > 2 ) {
		totalWidth += w;
		numTexels += w * w;
		w >>= 1;
		s_maxMip++;
	}
	w = s_image.GetWidth();
	s_imageBuffer = (uint8 *)malloc( w * w * 4 * s_maxMip );
	s_anisotropicWidth = totalWidth;

	// Copy texture data and mip map
	w = s_image.GetWidth();
	for ( int p = 0; p < w * w; p++ ) {
		const uint8 * data = s_image.DataPtr();
		s_imageBuffer[ 4 * p + 0 ] = data[ 4 * p + 0 ];
		s_imageBuffer[ 4 * p + 1 ] = data[ 4 * p + 1 ];
		s_imageBuffer[ 4 * p + 2 ] = data[ 4 * p + 2 ];
		s_imageBuffer[ 4 * p + 3 ] = data[ 4 * p + 3 ];
	}

	for ( int mip = 1; mip < s_maxMip; mip++ ) {
		const int sourceMip = mip - 1;
		w = s_image.GetWidth() >> mip;
		const int W = s_image.GetWidth();
		const int sourceWidth = s_image.GetWidth() >> sourceMip;
		const int targetWidth = s_image.GetWidth() >> mip;

		for ( int y = 0; y < targetWidth; y++ ) {
			for ( int x = 0; x < targetWidth; x++ ) {
				int X = 2 * x;
				int Y = 2 * y;
				int sourceOffset = W * W * sourceMip;
				int targetOffset = W * W * mip;

				int idx0 = X + 0 + ( Y + 0 ) * sourceWidth + sourceOffset;
				int idx1 = X + 1 + ( Y + 0 ) * sourceWidth + sourceOffset;
				int idx2 = X + 0 + ( Y + 1 ) * sourceWidth + sourceOffset;
				int idx3 = X + 1 + ( Y + 1 ) * sourceWidth + sourceOffset;

				int R = 0;
				int G = 0;
				int B = 0;
				int A = 0;
				R += s_imageBuffer[ 4 * idx0 + 0 ];
				G += s_imageBuffer[ 4 * idx0 + 1 ];
				B += s_imageBuffer[ 4 * idx0 + 2 ];
				A += s_imageBuffer[ 4 * idx0 + 3 ];

				R += s_imageBuffer[ 4 * idx1 + 0 ];
				G += s_imageBuffer[ 4 * idx1 + 1 ];
				B += s_imageBuffer[ 4 * idx1 + 2 ];
				A += s_imageBuffer[ 4 * idx1 + 3 ];

				R += s_imageBuffer[ 4 * idx2 + 0 ];
				G += s_imageBuffer[ 4 * idx2 + 1 ];
				B += s_imageBuffer[ 4 * idx2 + 2 ];
				A += s_imageBuffer[ 4 * idx2 + 3 ];

				R += s_imageBuffer[ 4 * idx3 + 0 ];
				G += s_imageBuffer[ 4 * idx3 + 1 ];
				B += s_imageBuffer[ 4 * idx3 + 2 ];
				A += s_imageBuffer[ 4 * idx3 + 3 ];

				R = R >> 2;
				G = G >> 2;
				B = B >> 2;
				A = A >> 2;

				int idx = x + y * targetWidth + targetOffset;
				s_imageBuffer[ 4 * idx + 0 ] = R;
				s_imageBuffer[ 4 * idx + 1 ] = G;
				s_imageBuffer[ 4 * idx + 2 ] = B;
				s_imageBuffer[ 4 * idx + 3 ] = A;
			}
		}
	}

	// Create the Anisotropic Buffer
	s_anisotropicBuffer = (uint8 *)malloc( totalWidth * totalWidth * 4 );
	w = s_image.GetWidth();
	for ( int y = 0; y < w; y++ ) {
		for ( int x = 0; x < w; x++ ) {
			int p = x + y * w;
			int idx = x + y * totalWidth;
			const uint8 * data = s_image.DataPtr();
			s_anisotropicBuffer[ 4 * idx + 0 ] = data[ 4 * p + 0 ];
			s_anisotropicBuffer[ 4 * idx + 1 ] = data[ 4 * p + 1 ];
			s_anisotropicBuffer[ 4 * idx + 2 ] = data[ 4 * p + 2 ];
			s_anisotropicBuffer[ 4 * idx + 3 ] = data[ 4 * p + 3 ];
		}
	}

	// Mip along the vertical of the image
	const int width = s_image.GetWidth();
	int sourceOffsetY = 0;
	int targetOffsetY = width;
	for ( int mipy = 1; mipy < s_maxMip; mipy++ ) {
		int sourceHeight = width >> ( mipy - 1 );
		int targetHeight = width >> mipy;

		for ( int y = 0; y < targetHeight; y++ ) {
			for ( int x = 0; x < width; x++ ) {
				int idx0 = x + ( 2 * y + 0 + sourceOffsetY ) * totalWidth;
				int idx1 = x + ( 2 * y + 1 + sourceOffsetY ) * totalWidth;

				// Average colors
				uint32 colors[ 4 ] = { 0 };
				for ( int i = 0; i < 4; i++ ) {
					colors[ i ] += s_anisotropicBuffer[ 4 * idx0 + i ];
					colors[ i ] += s_anisotropicBuffer[ 4 * idx1 + i ];
					colors[ i ] >>= 1;
				}

				// Store colors
				int idx = x + ( y + targetOffsetY ) * totalWidth;
				for ( int i = 0; i < 4; i++ ) {
					s_anisotropicBuffer[ 4 * idx + i ] = (uint8)colors[ i ];
				}
			}
		}

		sourceOffsetY += sourceHeight;
		targetOffsetY += targetHeight;
	}

	// Mip along the horizontal of the image
	int sourceOffsetX = 0;
	int targetOffsetX = width;
	for ( int mipx = 1; mipx < s_maxMip; mipx++ ) {
		int sourceWidth = width >> ( mipx - 1 );
		int targetWidth = width >> mipx;

		for ( int y = 0; y < totalWidth; y++ ) {
			for ( int x = 0; x < targetWidth; x++ ) {
				int idx0 = ( 2 * x + 0 + sourceOffsetX ) + y * totalWidth;
				int idx1 = ( 2 * x + 1 + sourceOffsetX ) + y * totalWidth;

				// Average colors
				uint32 colors[ 4 ] = { 0 };
				for ( int i = 0; i < 4; i++ ) {
					colors[ i ] += s_anisotropicBuffer[ 4 * idx0 + i ];
					colors[ i ] += s_anisotropicBuffer[ 4 * idx1 + i ];
					colors[ i ] >>= 1;
				}

				// Store colors
				int idx = ( x + targetOffsetX ) + y * totalWidth;
				for ( int i = 0; i < 4; i++ ) {
					s_anisotropicBuffer[ 4 * idx + i ] = (uint8)colors[ i ];
				}
			}
		}

		sourceOffsetX += sourceWidth;
		targetOffsetX += targetWidth;
	}
}

/*
================================
SampleImage
================================
*/
void SampleImage( Vec2d st, uint8 color[ 4 ] ) {
	int width = s_image.GetWidth();
	int x = st.x * width;
	int y = st.y * width;
	x = x % width;
	y = y % width;

	int idx = x + y * width;
	color[ 0 ] = s_imageBuffer[ 4 * idx + 0 ];
	color[ 1 ] = s_imageBuffer[ 4 * idx + 1 ];
	color[ 2 ] = s_imageBuffer[ 4 * idx + 2 ];
	color[ 3 ] = s_imageBuffer[ 4 * idx + 3 ];
}

/*
================================
SampleImageMipMap
================================
*/
void SampleImageMipMap( Vec2d st, int mip, uint8 color[ 4 ] ) {
	if ( mip >= s_maxMip ) {
		mip = s_maxMip - 1;
	}
	int width = s_image.GetWidth() >> mip;
	int x = st.x * width;
	int y = st.y * width;
	x = x % width;
	y = y % width;

	int W = s_image.GetWidth();
	int offset = W * W * mip;

	int idx = x + y * width + offset;
	color[ 0 ] = s_imageBuffer[ 4 * idx + 0 ];
	color[ 1 ] = s_imageBuffer[ 4 * idx + 1 ];
	color[ 2 ] = s_imageBuffer[ 4 * idx + 2 ];
	color[ 3 ] = s_imageBuffer[ 4 * idx + 3 ];
}

/*
================================
MinPtsY
================================
*/
Vec2d MinPtsY( Vec2d a, Vec2d b ) {
	if ( a.y < b.y ) {
		return a;
	}
	return b;
}

/*
================================
OutputPixel
================================
*/
void OutputPixel( float x, float y, float r, float g, float b ) {
	int ix = ( x + 1.0f ) * 0.5f * g_screenWidth;
	int iy = ( y - 1.0f ) * -0.5f * g_screenHeight;
	if ( ix < 0 || ix >= g_screenWidth ) {
		return;
	}
	if ( iy < 0 || iy >= g_screenHeight ) {
		return;
	}

	int idx = ix + iy * g_screenWidth;
	g_rasterBuffer[ 4 * idx + 0 ] = 255.0f * r;
	g_rasterBuffer[ 4 * idx + 1 ] = 255.0f * g;
	g_rasterBuffer[ 4 * idx + 2 ] = 255.0f * b;
	g_rasterBuffer[ 4 * idx + 3 ] = 255;
}

/*
================================
RasterTop
================================
*/
void RasterTop( Vec2d a, Vec2d b, Vec2d c ) {
	// Assumed to be sorted from top to bottom by a, b, c
	if ( c.x < b.x ) {
		// b should be less than c in the x-axis for a ccw triangle
		return;
	}

	Vec2d ab = b - a;
	Vec2d ac = c - a;
	ab.Normalize();
	ac.Normalize();

	const float dy = 2.0f / (float)g_screenHeight;
	const float dx = 2.0f / (float)g_screenWidth;

	const float dAB = 1.0f / fabsf( ab.y );
	const float dAC = 1.0f / fabsf( ac.y );

	float tab = 0;
	float tac = 0;

	float y = a.y;
	while ( y <= b.y ) {
		Vec2d ab0 = a + ab * tab;
		Vec2d ac0 = a + ac * tac;

		float x = ab0.x;
		while ( x <= ac0.x ) {
			OutputPixel( x, y, 1, 0, 0 );
			x += dx;
		}

		y += dy;
		tab += dAB;
		tac += dAC;
	}
}

/*
================================
RasterBottom
================================
*/
void RasterBottom( Vec2d a, Vec2d b, Vec2d c ) {
	// Assumed to be sorted from top to bottom by a, b, c
	if ( c.x > b.x ) {
		// b should be less than c in the x-axis for a ccw triangle
		return;
	}

	Vec2d ab = b - a;
	Vec2d ac = c - a;
	ab.Normalize();
	ac.Normalize();

	const float dy = 2.0f / (float)g_screenHeight;
	const float dx = 2.0f / (float)g_screenWidth;

	const float dAB = 1.0f / fabsf( ab.y );
	const float dAC = 1.0f / fabsf( ac.y );

	float tab = 0;
	float tac = 0;

	float y = a.y;
	while ( y <= b.y ) {
		Vec2d ab0 = a + ab * tab;
		Vec2d ac0 = a + ac * tac;

		float x = ab0.x;
		while ( x <= ac0.x ) {
			OutputPixel( x, y, 0, 1, 0 );
			x += dx;
		}

		y -= dy;
		tab -= dAB;
		tac -= dAC;
	}
}

/*
================================
RayLineSegmentIntersect
================================
*/
float RayLineSegmentIntersect( Vec2d rayStart, Vec2d rayDir, Vec2d a, Vec2d b ) {
	Vec2d ab = b - a;

	Vec2d v1 = rayStart - a;
	Vec2d v2 = b - a;
	Vec2d v3 = Vec2d( -rayDir.y, rayDir.x );

	float dotdot = dot( v2, v3 );
	if ( fabsf( dotdot ) < 0.000001f ) {
		return 0;
	}

	float t1 = ( v2.x * v1.y - v2.y * v1.x ) / dotdot;
	float t2 = dot( v1, v3 ) / dotdot;

	if ( t1 >= 0 && ( t2 >= 0 && t2 <= 1 ) ) {
		return t1;
	}
	return 0;

// 	public double? GetRayToLineSegmentIntersection(Point rayOrigin, Vector rayDirection, Point point1, Point point2)
//     {
//         var v1 = rayOrigin - point1;
//         var v2 = point2 - point1;
//         var v3 = new Vector(-rayDirection.Y, rayDirection.X);
// 
// 
//         var dot = v2 * v3;
//         if (Math.Abs(dot) < 0.000001)
//             return null;
// 
//         var t1 = Vector.CrossProduct(v2, v1) / dot;
//         var t2 = (v1 * v3) / dot;
// 
//         if (t1 >= 0.0 && (t2 >= 0.0 && t2 <= 1.0))
//             return t1;
// 
//         return null;
//     }
}

/*
================================
TransformPoint
================================
*/
Vec2d TransformPoint( Vec2d a ) {
	a.x = ( a.x + 1.0f ) * 0.5f * (float)g_screenWidth;
	a.y = ( a.y - 1.0f ) * -0.5f * (float)g_screenHeight;
	return a;
}

/*
================================
MinsPt
================================
*/
Vec2d MinsPt( Vec2d a, Vec2d b ) {
	if ( a.x > b.x ) {
		a.x = b.x;
	}
	if ( a.y > b.y ) {
		a.y = b.y;
	}
	return a;
}

/*
================================
MaxsPt
================================
*/
Vec2d MaxsPt( Vec2d a, Vec2d b ) {
	if ( a.x < b.x ) {
		a.x = b.x;
	}
	if ( a.y < b.y ) {
		a.y = b.y;
	}
	return a;
}

/*
================================
Determinant2d
================================
*/
float Determinant2d( Vec2d a, Vec2d b ) {
	return a.x * b.y - a.y * b.x;
}

/*
================================
BarycentricCoord
================================
*/
Vec3d BarycentricCoord( Vec2d pt, Vec2d a, Vec2d b, Vec2d c ) {
	Vec2d ab = b - a;
	Vec2d bc = c - b;
	Vec2d ca = a - c;

	float area = Determinant2d( ca, ab );

	Vec3d bary;
	bary.x = Determinant2d( bc, ( pt - b ) ) / area;
	bary.y = Determinant2d( ca, ( pt - c ) ) / area;
	bary.z = Determinant2d( ab, ( pt - a ) ) / area;
	return bary;
}

/*
================================
IsPointInsideTriangle
================================
*/
bool IsPointInsideTriangle( Vec2d pt, Vec2d a, Vec2d b, Vec2d c ) {
	Vec2d ab = b - a;
	Vec2d bc = c - b;
	Vec2d ca = a - c;

	if ( Determinant2d( ab, ( pt - a ) ) > 0.0f ) {
		return false;
	}
	if ( Determinant2d( bc, ( pt - b ) ) > 0.0f ) {
		return false;
	}
	if ( Determinant2d( ca, ( pt - c ) ) > 0.0f ) {
		return false;
	}

	return true;
}

/*
================================
RasterTriangle
================================
*/
void RasterTriangle( Vec2d a, Vec2d b, Vec2d c ) {
	// Check the winding
	Vec2d ab = b - a;
	Vec2d ac = c - a;
	float det = ab.x * ac.y - ab.y * ac.x;
	if ( det < 0 ) {
		return;
	}

	a = TransformPoint( a );
	b = TransformPoint( b );
	c = TransformPoint( c );

	Vec2d mins = MinsPt( MinsPt( a, b ), c );
	Vec2d maxs = MaxsPt( MaxsPt( a, b ), c );
	
	for ( int y = (int)mins.y; y < (int)maxs.y && y < g_screenHeight; y++ ) {
		for ( int x = (int)mins.x; x < (int)maxs.x && x < g_screenWidth; x++ ) {
			if ( x < 0 || y < 0 ) {
				continue;
			}

			// Check if (x,y) is inside the triangle
			if ( !IsPointInsideTriangle( Vec2d( x, y ), a, b, c ) ) {
				continue;
			}

			int idx = x + y * g_screenWidth;

			g_rasterBuffer[ 4 * idx + 0 ] = 255;
			g_rasterBuffer[ 4 * idx + 1 ] = 0;
			g_rasterBuffer[ 4 * idx + 2 ] = 0;
			g_rasterBuffer[ 4 * idx + 3 ] = 255;
		}
	}
}

#define PERSPECTIVE_INTERPOLATION

/*
================================
CalculateST
================================
*/
Vec2d CalculateST( int x, int y, vert_t v0, vert_t v1, vert_t v2, Vec2d a, Vec2d b, Vec2d c, float w0, float w1, float w2 ) {
	Vec3d bary = BarycentricCoord( Vec2d( x, y ), a, b, c );
	Vec2d st = v0.st * bary.x + v1.st * bary.y + v2.st * bary.z;
#if defined( PERSPECTIVE_INTERPOLATION )
	float w = bary.x / w0 + bary.y / w1 + bary.z / w2;
	st /= w;
#endif
	st.x = fabsf( st.x );
	st.y = fabsf( st.y );
	return st;
}

/*
================================
CalculateMip
================================
*/
int CalculateMip( int x, int y, vert_t v0, vert_t v1, vert_t v2, Vec2d a, Vec2d b, Vec2d c, float w0, float w1, float w2 ) {
	Vec2d st0 = CalculateST( x + 0, y + 0, v0, v1, v2, a, b, c, w0, w1, w2 );
	Vec2d st1 = CalculateST( x + 1, y + 0, v0, v1, v2, a, b, c, w0, w1, w2 );
	Vec2d st2 = CalculateST( x + 0, y + 1, v0, v1, v2, a, b, c, w0, w1, w2 );
//	Vec2d st3 = CalculateST( x + 1, y + 1, v0, v1, v2, a, b, c, w0, w1, w2 );
// 	int stx = st.x * s_image.GetWidth();
// 	int sty = st.y * s_image.GetHeight();
// 	stx = stx % s_image.GetWidth();
// 	sty = sty % s_image.GetHeight();

	Vec2d stdx = st1 - st0;
	Vec2d stdy = st2 - st0;

	// Convert to texel space
	int w = s_image.GetWidth();
	int h = s_image.GetHeight();
	st0.x *= w;
	st0.y *= h;
	st1.x *= w;
	st1.y *= h;
	st2.x *= w;
	st2.y *= h;
// 	st3.x *= w;
// 	st3.y *= h;

	// NOTE: This is not in the correct space, we need to calculate the texel offset in the correct space

	// Get texel deltas
	float dx = fabsf( st0.x - st1.x );
	float dy = fabsf( st0.y - st2.y );

 	dx = fabsf( ( stdx.x - stdy.x ) * (float)w );
 	dy = fabsf( ( stdx.y - stdy.y ) * (float)h );

	int dxi = floorf( dx );// - 1;
	int dyi = floorf( dy );// - 1;

	if ( dxi < 0 ) {
		dxi = 0;
	}
	if ( dyi < 0 ) {
		dyi = 0;
	}

	// The maximum texel delta is the mip
	//return ( dxi > dyi ) ? dxi : dyi;
	stdx.x *= w;
	stdx.y *= h;
	stdy.x *= w;
	stdy.y *= h;
	float area = stdx.x * stdy.y - stdx.y * stdy.x;
	float h2 = sqrtf( dot( stdx, stdx ) + dot( stdy, stdy ) );
	float mip = ( fabsf( area ) );// / ( w * h );
	mip = h2;
	return (int)mip;
}

/*
================================
SampleImageMipMapLinear
================================
*/
void SampleImageMipMapLinear( Vec2d st, int mip, uint8 color[ 4 ] ) {
	if ( mip >= s_maxMip ) {
		mip = s_maxMip - 1;
	}
	int width = s_image.GetWidth() >> mip;
	st *= (float)width;

	int x0 = ( (int)floorf( st.x ) ) % width;
	int y0 = ( (int)floorf( st.y ) ) % width;

	int x1 = ( (int)ceilf( st.x ) ) % width;
	int y1 = ( (int)ceilf( st.y ) ) % width;
	
	int W = s_image.GetWidth();
	int offset = W * W * mip;

	int idx0 = x0 + y0 * width + offset;
	int idx1 = x1 + y0 * width + offset;
	int idx2 = x0 + y1 * width + offset;
	int idx3 = x1 + y1 * width + offset;

	extern float frac( float value );
	float tx = frac( st.x );
	float ty = frac( st.y );

	Vec4d colors;
	colors.x = (float)s_imageBuffer[ 4 * idx0 + 0 ] * ( 1.0f - tx ) * ( 1.0f - ty );
	colors.y = (float)s_imageBuffer[ 4 * idx0 + 1 ] * ( 1.0f - tx ) * ( 1.0f - ty );
	colors.z = (float)s_imageBuffer[ 4 * idx0 + 2 ] * ( 1.0f - tx ) * ( 1.0f - ty );
	colors.w = (float)s_imageBuffer[ 4 * idx0 + 3 ] * ( 1.0f - tx ) * ( 1.0f - ty );

	colors.x += (float)s_imageBuffer[ 4 * idx1 + 0 ] * tx * ( 1.0f - ty );
	colors.y += (float)s_imageBuffer[ 4 * idx1 + 1 ] * tx * ( 1.0f - ty );
	colors.z += (float)s_imageBuffer[ 4 * idx1 + 2 ] * tx * ( 1.0f - ty );
	colors.w += (float)s_imageBuffer[ 4 * idx1 + 3 ] * tx * ( 1.0f - ty );

	colors.x += (float)s_imageBuffer[ 4 * idx2 + 0 ] * ( 1.0f - tx ) * ty;
	colors.y += (float)s_imageBuffer[ 4 * idx2 + 1 ] * ( 1.0f - tx ) * ty;
	colors.z += (float)s_imageBuffer[ 4 * idx2 + 2 ] * ( 1.0f - tx ) * ty;
	colors.w += (float)s_imageBuffer[ 4 * idx2 + 3 ] * ( 1.0f - tx ) * ty;

	colors.x += (float)s_imageBuffer[ 4 * idx3 + 0 ] * tx * ty;
	colors.y += (float)s_imageBuffer[ 4 * idx3 + 1 ] * tx * ty;
	colors.z += (float)s_imageBuffer[ 4 * idx3 + 2 ] * tx * ty;
	colors.w += (float)s_imageBuffer[ 4 * idx3 + 3 ] * tx * ty;

	color[ 0 ] = (uint8)colors.x;
	color[ 1 ] = (uint8)colors.y;
	color[ 2 ] = (uint8)colors.z;
	color[ 3 ] = (uint8)colors.w;
}

/*
================================
CalculateMips
================================
*/
void CalculateMips( int x, int y, vert_t v0, vert_t v1, vert_t v2, Vec2d a, Vec2d b, Vec2d c, float w0, float w1, float w2, int & mipX, int & mipY ) {
	Vec2d st0 = CalculateST( x + 0, y + 0, v0, v1, v2, a, b, c, w0, w1, w2 );
	Vec2d st1 = CalculateST( x + 1, y + 0, v0, v1, v2, a, b, c, w0, w1, w2 );
	Vec2d st2 = CalculateST( x + 0, y + 1, v0, v1, v2, a, b, c, w0, w1, w2 );

	Vec2d stdx = st1 - st0;
	Vec2d stdy = st2 - st0;

	// Convert to texel space
	int w = s_image.GetWidth();
	int h = s_image.GetHeight();
	st0.x *= w;
	st0.y *= h;
	st1.x *= w;
	st1.y *= h;
	st2.x *= w;
	st2.y *= h;

	// Get texel deltas
	float dx = fabsf( st0.x - st1.x );
	float dy = fabsf( st0.y - st2.y );

 	dx = fabsf( ( stdx.x - stdy.x ) * (float)w );
 	dy = fabsf( ( stdx.y - stdy.y ) * (float)h );

	int dxi = floorf( dx ) - 1;
	int dyi = floorf( dy ) - 1;
// 	dxi *= 2;
// 	dyi *= 2;

	if ( dxi < 0 ) {
		dxi = 0;
	}
	if ( dyi < 0 ) {
		dyi = 0;
	}

// 	dxi >>= 1;
// 	dyi >>= 1;

	mipX = dxi;
	mipY = dyi;

	// The maximum texel delta is the mip
	//return ( dxi > dyi ) ? dxi : dyi;
// 	stdx.x *= w;
// 	stdx.y *= h;
// 	stdy.x *= w;
// 	stdy.y *= h;
// 	float area = stdx.x * stdy.y - stdx.y * stdy.x;
// 	float h2 = sqrtf( dot( stdx, stdx ) + dot( stdy, stdy ) );
// 	float mip = ( fabsf( area ) );// / ( w * h );
// 	mip = h2;
// 	return (int)mip;
}

/*
================================
SampleImageAnisotropic
================================
*/
void SampleImageAnisotropic( Vec2d st, int mipX, int mipY, uint8 color[ 4 ] ) {
	if ( mipX >= s_maxMip ) {
		mipX = s_maxMip - 1;
	}
	if ( mipY >= s_maxMip ) {
		mipY = s_maxMip - 1;
	}
	const int width = s_image.GetWidth();
	const int w = width >> mipX;
	const int h = width >> mipY;

	int offsetX = 0;
	int offsetY = 0;
	for ( int m = 0; m < mipX; m++ ) {
		offsetX += ( width >> m );
	}
	for ( int m = 0; m < mipY; m++ ) {
		offsetY += ( width >> m );
	}

	st.x *= w;
	st.y *= h;

	int x = (int)st.x;
	int y = (int)st.y;
	x = x % w;
	y = y % h;

	int idx = ( x + offsetX ) + ( y + offsetY ) * s_anisotropicWidth;
	color[ 0 ] = s_anisotropicBuffer[ 4 * idx + 0 ];
	color[ 1 ] = s_anisotropicBuffer[ 4 * idx + 1 ];
	color[ 2 ] = s_anisotropicBuffer[ 4 * idx + 2 ];
	color[ 3 ] = s_anisotropicBuffer[ 4 * idx + 3 ];
}

/*
================================
frac
================================
*/
float frac( float value ) {
	if ( value >= 0 ) {
		return value - floorf( value );
	}
	return value - ceilf( value );
}

/*
================================
SampleImageAnisotropicLinear
================================
*/
void SampleImageAnisotropicLinear( Vec2d st, int mipX, int mipY, uint8 color[ 4 ] ) {
	if ( mipX >= s_maxMip ) {
		mipX = s_maxMip - 1;
	}
	if ( mipY >= s_maxMip ) {
		mipY = s_maxMip - 1;
	}
	const int width = s_image.GetWidth();
	const int w = width >> mipX;
	const int h = width >> mipY;

	int offsetX = 0;
	int offsetY = 0;
	for ( int m = 0; m < mipX; m++ ) {
		offsetX += ( width >> m );
	}
	for ( int m = 0; m < mipY; m++ ) {
		offsetY += ( width >> m );
	}

	//st *= (float)width;
	st.x *= w;
	st.y *= h;

	int x0 = ( (int)floorf( st.x ) ) % w;
	int y0 = ( (int)floorf( st.y ) ) % h;

	int x1 = ( (int)ceilf( st.x ) ) % w;
	int y1 = ( (int)ceilf( st.y ) ) % h;

	int idx0 = ( x0 + offsetX ) + ( y0 + offsetY ) * s_anisotropicWidth;
	int idx1 = ( x1 + offsetX ) + ( y0 + offsetY ) * s_anisotropicWidth;
	int idx2 = ( x0 + offsetX ) + ( y1 + offsetY ) * s_anisotropicWidth;
	int idx3 = ( x1 + offsetX ) + ( y1 + offsetY ) * s_anisotropicWidth;
// 	idx1 = idx0;
// 	idx2 = idx0;
// 	idx3 = idx0;
	
// 	int W = s_image.GetWidth();
// 	int offset = W * W * mip;
// 
// 	int idx0 = x0 + y0 * width + offset;
// 	int idx1 = x1 + y0 * width + offset;
// 	int idx2 = x0 + y1 * width + offset;
// 	int idx3 = x1 + y1 * width + offset;

	extern float frac( float value );
	float tx = frac( st.x );
	float ty = frac( st.y );

	Vec4d colors;
	colors.x = (float)s_anisotropicBuffer[ 4 * idx0 + 0 ] * ( 1.0f - tx ) * ( 1.0f - ty );
	colors.y = (float)s_anisotropicBuffer[ 4 * idx0 + 1 ] * ( 1.0f - tx ) * ( 1.0f - ty );
	colors.z = (float)s_anisotropicBuffer[ 4 * idx0 + 2 ] * ( 1.0f - tx ) * ( 1.0f - ty );
	colors.w = (float)s_anisotropicBuffer[ 4 * idx0 + 3 ] * ( 1.0f - tx ) * ( 1.0f - ty );

	colors.x += (float)s_anisotropicBuffer[ 4 * idx1 + 0 ] * tx * ( 1.0f - ty );
	colors.y += (float)s_anisotropicBuffer[ 4 * idx1 + 1 ] * tx * ( 1.0f - ty );
	colors.z += (float)s_anisotropicBuffer[ 4 * idx1 + 2 ] * tx * ( 1.0f - ty );
	colors.w += (float)s_anisotropicBuffer[ 4 * idx1 + 3 ] * tx * ( 1.0f - ty );

	colors.x += (float)s_anisotropicBuffer[ 4 * idx2 + 0 ] * ( 1.0f - tx ) * ty;
	colors.y += (float)s_anisotropicBuffer[ 4 * idx2 + 1 ] * ( 1.0f - tx ) * ty;
	colors.z += (float)s_anisotropicBuffer[ 4 * idx2 + 2 ] * ( 1.0f - tx ) * ty;
	colors.w += (float)s_anisotropicBuffer[ 4 * idx2 + 3 ] * ( 1.0f - tx ) * ty;

	colors.x += (float)s_anisotropicBuffer[ 4 * idx3 + 0 ] * tx * ty;
	colors.y += (float)s_anisotropicBuffer[ 4 * idx3 + 1 ] * tx * ty;
	colors.z += (float)s_anisotropicBuffer[ 4 * idx3 + 2 ] * tx * ty;
	colors.w += (float)s_anisotropicBuffer[ 4 * idx3 + 3 ] * tx * ty;

	color[ 0 ] = (uint8)colors.x;
	color[ 1 ] = (uint8)colors.y;
	color[ 2 ] = (uint8)colors.z;
	color[ 3 ] = (uint8)colors.w;
}

/*
================================
RasterTriangle
================================
*/
void RasterTriangle( vert_t v0, vert_t v1, vert_t v2, float w0, float w1, float w2 ) {
	Vec2d a = Vec2d( v0.pos.x, v0.pos.y );
	Vec2d b = Vec2d( v1.pos.x, v1.pos.y );
	Vec2d c = Vec2d( v2.pos.x, v2.pos.y );

	// Check the winding
	Vec2d ab = b - a;
	Vec2d ac = c - a;
	float det = ab.x * ac.y - ab.y * ac.x;
	if ( det < 0 ) {
		return;
	}

	// Convert from NDC to Screen Space
	a = TransformPoint( a );
	b = TransformPoint( b );
	c = TransformPoint( c );

	Vec2d mins = MinsPt( MinsPt( a, b ), c );
	Vec2d maxs = MaxsPt( MaxsPt( a, b ), c );

	// The +1 lets us more conservatively rasterize the triangle
	maxs.x += 1;
	maxs.y += 1;
	
	for ( int y = (int)mins.y; y < (int)maxs.y && y < g_screenHeight; y++ ) {
		if ( y < 0 ) {
			continue;
		}

		for ( int x = (int)mins.x; x < (int)maxs.x && x < g_screenWidth; x++ ) {
			if ( x < 0 ) {
				continue;
			}
			
			// Check if (x,y) is inside the triangle
			if ( !IsPointInsideTriangle( Vec2d( x, y ), a, b, c ) ) {
				continue;
			}

			int idx = x + y * g_screenWidth;

			// Draw as a flat color
			g_rasterBuffer[ 4 * idx + 0 ] = 255;
			g_rasterBuffer[ 4 * idx + 1 ] = 0;
			g_rasterBuffer[ 4 * idx + 2 ] = 0;
			g_rasterBuffer[ 4 * idx + 3 ] = 255;

			// Draw colored faces
			for ( int i = 0; i < 4; i++ ) {
				g_rasterBuffer[ 4 * idx + i ] = v0.buff[ i ];
			}

			Vec2d st = CalculateST( x, y, v0, v1, v2, a, b, c, w0, w1, w2 );
			int mip = CalculateMip( x, y, v0, v1, v2, a, b, c, w0, w1, w2 );

			int mipX = 0;
			int mipY = 0;
			CalculateMips( x, y, v0, v1, v2, a, b, c, w0, w1, w2, mipX, mipY );

			uint8 color[ 4 ];
			//SampleImage( st, color );
			//SampleImageMipMap( st, mip, color );
			//SampleImageMipMapLinear( st, mip, color );
			//SampleImageAnisotropic( st, mipX, mipY, color );
			SampleImageAnisotropicLinear( st, mipX, mipY, color );
			g_rasterBuffer[ 4 * idx + 0 ] = color[ 0 ];
			g_rasterBuffer[ 4 * idx + 1 ] = color[ 1 ];
			g_rasterBuffer[ 4 * idx + 2 ] = color[ 2 ];
			g_rasterBuffer[ 4 * idx + 3 ] = color[ 3 ];
		}
	}
}

/*
================================
RasterTris
================================
*/
void RasterTris( vert_t * verts, int num, Mat4 mat ) {
	mat = mat.Transpose();

	for ( int i = 0; i < num / 3; i++ ) {
		vert_t v0 = verts[ 3 * i + 0 ];
		vert_t v1 = verts[ 3 * i + 1 ];
		vert_t v2 = verts[ 3 * i + 2 ];

		Vec4d a = mat * Vec4d( v0.pos.x, v0.pos.y, v0.pos.z, 1.0f );
		Vec4d b = mat * Vec4d( v1.pos.x, v1.pos.y, v1.pos.z, 1.0f );
		Vec4d c = mat * Vec4d( v2.pos.x, v2.pos.y, v2.pos.z, 1.0f );

		v0.pos = Vec3d( a.x, a.y, a.z ) / a.w;
		v1.pos = Vec3d( b.x, b.y, b.z ) / b.w;
		v2.pos = Vec3d( c.x, c.y, c.z ) / c.w;

#if defined( PERSPECTIVE_INTERPOLATION )
		v0.st = v0.st / a.w;
		v1.st = v1.st / b.w;
		v2.st = v2.st / c.w;
#endif

		RasterTriangle( v0, v1, v2, a.w, b.w, c.w );
	}
}

static vert_t s_cube[ 6 * 2 * 3 ];

/*
================================
InitCube
================================
*/
void InitCube() {
	// +Z Face
	s_cube[ 0 ].pos = Vec3d( 1, 1, 1 );
	s_cube[ 1 ].pos = Vec3d( -1, 1, 1 );
	s_cube[ 2 ].pos = Vec3d( -1, -1, 1 );

	s_cube[ 3 ].pos = Vec3d( -1, -1, 1 );
	s_cube[ 4 ].pos = Vec3d( 1, -1, 1 );
	s_cube[ 5 ].pos = Vec3d( 1, 1, 1 );

	s_cube[ 0 ].st = Vec2d( 0, 0 );
	s_cube[ 1 ].st = Vec2d( 1, 0 );
	s_cube[ 2 ].st = Vec2d( 1, 1 );

	s_cube[ 3 ].st = Vec2d( 1, 1 );
	s_cube[ 4 ].st = Vec2d( 0, 1 );
	s_cube[ 5 ].st = Vec2d( 0, 0 );

	// -Z Face
	s_cube[ 6 ].pos = Vec3d( -1, -1, -1 );
	s_cube[ 7 ].pos = Vec3d( -1, 1, -1 );
	s_cube[ 8 ].pos = Vec3d( 1, 1, -1 );

	s_cube[ 9 ].pos = Vec3d( 1, 1, -1 );
	s_cube[ 10 ].pos = Vec3d( 1, -1, -1 );
	s_cube[ 11 ].pos = Vec3d( -1, -1, -1 );

	s_cube[ 6 ].st = Vec2d( 1, 1 );
	s_cube[ 7 ].st = Vec2d( 1, 0 );
	s_cube[ 8 ].st = Vec2d( 0, 0 );

	s_cube[ 9 ].st = Vec2d( 0, 0 );
	s_cube[ 10 ].st = Vec2d( 0, 1 );
	s_cube[ 11 ].st = Vec2d( 1, 1 );

	// +X Face
	s_cube[ 12 ].pos = Vec3d( 1, 1, 1 );
	s_cube[ 13 ].pos = Vec3d( 1, -1, 1 );
	s_cube[ 14 ].pos = Vec3d( 1, -1, -1 );

	s_cube[ 15 ].pos = Vec3d( 1, -1, -1 );
	s_cube[ 16 ].pos = Vec3d( 1, 1, -1 );
	s_cube[ 17 ].pos = Vec3d( 1, 1, 1 );

	s_cube[ 12 ].st = Vec2d( 0, 0 );
	s_cube[ 13 ].st = Vec2d( 1, 0 );
	s_cube[ 14 ].st = Vec2d( 1, 1 );

	s_cube[ 15 ].st = Vec2d( 1, 1 );
	s_cube[ 16 ].st = Vec2d( 0, 1 );
	s_cube[ 17 ].st = Vec2d( 0, 0 );

	// -X Face
	s_cube[ 18 ].pos = Vec3d( -1, -1, 1 );
	s_cube[ 19 ].pos = Vec3d( -1, 1, 1 );
	s_cube[ 20 ].pos = Vec3d( -1, -1, -1 );

	s_cube[ 21 ].pos = Vec3d( -1, -1, -1 );
	s_cube[ 22 ].pos = Vec3d( -1, 1, 1 );
	s_cube[ 23 ].pos = Vec3d( -1, 1, -1 );

	s_cube[ 18 ].st = Vec2d( 1, 0 );
	s_cube[ 19 ].st = Vec2d( 0, 0 );
	s_cube[ 20 ].st = Vec2d( 1, 1 );

	s_cube[ 21 ].st = Vec2d( 1, 1 );
	s_cube[ 22 ].st = Vec2d( 0, 0 );
	s_cube[ 23 ].st = Vec2d( 0, 1 );

	// +Y Face
	s_cube[ 24 ].pos = Vec3d( 1, 1, 1 );
	s_cube[ 25 ].pos = Vec3d( 1, 1, -1 );
	s_cube[ 26 ].pos = Vec3d( -1, 1, -1 );

	s_cube[ 27 ].pos = Vec3d( -1, 1, -1 );
	s_cube[ 28 ].pos = Vec3d( -1, 1, 1 );
	s_cube[ 29 ].pos = Vec3d( 1, 1, 1 );

	s_cube[ 24 ].st = Vec2d( 0, 0 );
	s_cube[ 25 ].st = Vec2d( 1, 0 );
	s_cube[ 26 ].st = Vec2d( 1, 1 );

	s_cube[ 27 ].st = Vec2d( 1, 1 );
	s_cube[ 28 ].st = Vec2d( 0, 1 );
	s_cube[ 29 ].st = Vec2d( 0, 0 );

	// -Y Face
	s_cube[ 30 ].pos = Vec3d( 1, -1, -1 );
	s_cube[ 31 ].pos = Vec3d( 1, -1, 1 );
	s_cube[ 32 ].pos = Vec3d( -1, -1, -1 );

	s_cube[ 33 ].pos = Vec3d( -1, -1, -1 );
	s_cube[ 34 ].pos = Vec3d( 1, -1, 1 );
	s_cube[ 35 ].pos = Vec3d( -1, -1, 1 );

	s_cube[ 30 ].st = Vec2d( 0, 1 );
	s_cube[ 31 ].st = Vec2d( 0, 0 );
	s_cube[ 32 ].st = Vec2d( 1, 1 );

	s_cube[ 33 ].st = Vec2d( 1, 1 );
	s_cube[ 34 ].st = Vec2d( 0, 0 );
	s_cube[ 35 ].st = Vec2d( 1, 0 );

	for ( int i = 0; i < 6; i++ ) {
		s_cube[ i + 0 ].buff[ 0 ] = 255;
		s_cube[ i + 0 ].buff[ 1 ] = 0;
		s_cube[ i + 0 ].buff[ 2 ] = 0;
		s_cube[ i + 0 ].buff[ 3 ] = 255;

		s_cube[ i + 6 ].buff[ 0 ] = 0;
		s_cube[ i + 6 ].buff[ 1 ] = 255;
		s_cube[ i + 6 ].buff[ 2 ] = 0;
		s_cube[ i + 6 ].buff[ 3 ] = 255;

		s_cube[ i + 12 ].buff[ 0 ] = 0;
		s_cube[ i + 12 ].buff[ 1 ] = 0;
		s_cube[ i + 12 ].buff[ 2 ] = 255;
		s_cube[ i + 12 ].buff[ 3 ] = 255;

		s_cube[ i + 18 ].buff[ 0 ] = 255;
		s_cube[ i + 18 ].buff[ 1 ] = 0;
		s_cube[ i + 18 ].buff[ 2 ] = 255;
		s_cube[ i + 18 ].buff[ 3 ] = 255;

		s_cube[ i + 24 ].buff[ 0 ] = 255;
		s_cube[ i + 24 ].buff[ 1 ] = 255;
		s_cube[ i + 24 ].buff[ 2 ] = 0;
		s_cube[ i + 24 ].buff[ 3 ] = 255;

		s_cube[ i + 30 ].buff[ 0 ] = 255;
		s_cube[ i + 30 ].buff[ 1 ] = 255;
		s_cube[ i + 30 ].buff[ 2 ] = 255;
		s_cube[ i + 30 ].buff[ 3 ] = 255;
	}
}

/*
================================
DrawRasterizer
================================
*/
void DrawRasterizer( float time ) {
	InitCube();

	// Clear the render frame
	ClearBuffer();

	// This sets the perspective projection matrix
	float matProj[ 16 ] = { 0 };
	const float fieldOfViewDegrees = 45;
	const float aspectRatio = static_cast< float >( g_screenHeight ) / static_cast< float >( g_screenWidth );
	const float nearDepth = 1;
	const float farDepth = 1000;
	myPerspective( fieldOfViewDegrees, aspectRatio, nearDepth, farDepth, matProj );

	float matModel[ 16 ] = { 0 };
	myRotateZ( time * 10.01f, matModel );

	float matView[ 16 ] = { 0 };
	Vec3d camPos = Vec3d( 2, 2, 1 ) * 3.0f;
	Vec3d lookat = Vec3d( 0, 0, 0 );
	Vec3d camUp = Vec3d( 0, 0, 1 );
	myLookAt( camPos, lookat, camUp, matView );

	float matTemp[ 16 ] = { 0 };
	myMatrixMultiply( matModel, matView, matTemp );

	float matViewProj[ 16 ] = { 0 };
	myMatrixMultiply( matTemp, matProj, matViewProj );

	vert_t verts[ 3 ];
	verts[ 0 ].pos = Vec3d( 0.0f, 0.5f, 0.0f );
	verts[ 1 ].pos = Vec3d( -0.5f, -0.5f, 0.0f );
	verts[ 2 ].pos = Vec3d( 0.5f, -0.5f, 0.0f );
	Mat4 mat;
	mat.Identity();
	//RasterTris( verts, 3, mat );

	mat = Mat4( matViewProj );
	RasterTris( s_cube, 6 * 2 * 3, mat );
	//RasterTriangle( Vec2d( 0.0f, 0.5f ), Vec2d( -0.5f, -0.5f ), Vec2d( 0.5f, -0.5f ) );
	glBindTexture( GL_TEXTURE_2D, g_rasterTexture->GetName() );
	glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, g_screenWidth, g_screenHeight, GL_RGBA, GL_UNSIGNED_BYTE, g_rasterBuffer );
	//glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, s_anisotropicWidth, s_anisotropicWidth, GL_RGBA, GL_UNSIGNED_BYTE, s_anisotropicBuffer );
	glBindTexture( GL_TEXTURE_2D, 0 );

	glDepthMask( GL_FALSE );
	glDisable( GL_DEPTH_TEST );

	// Clear previous frame values
	glClear( GL_COLOR_BUFFER_BIT );
	glClear( GL_DEPTH_BUFFER_BIT );

	Shader * shader = g_shaderManager->GetAndUseShader( "fullscreen" );

	shader->SetAndBindUniformTexture( "rasterBuffer", 0, GL_TEXTURE_2D, g_rasterTexture->GetName() );

	g_modelScreenSpaceNearPlane.Draw();
	g_modelScreenSpaceFarPlane.Draw();
}

//
//  VirtualTexture.cpp
//
#include "VirtualTexture/VirtualTexture.h"
#include "VirtualTexture/PageTable.h"
#include "Math/MatrixOps.h"
#include "Graphics/Graphics.h"
#include <math.h>
#include <algorithm>
#include <stdlib.h>
#include <assert.h>

//static float s_virtualDistance = 10000.0f;	// The maximum distance for virtual texturing
static float s_virtualDistance = 100.0f;	// The maximum distance for virtual texturing

/*
 ================================================================
 
 VirtualTexture
 
 ================================================================
 */

/*
================================
VirtualTexture::VirtualTexture
================================
*/
VirtualTexture::VirtualTexture() {
	m_feedbackBuffer = NULL;
}

/*
================================
VirtualTexture::Init
================================
*/
void VirtualTexture::Init() {
	const char * fragShader = "data/Shaders/VirtualTexture/generatePageIDs_terrain.frag";
	const char * vertShader = "data/Shaders/VirtualTexture/generatePageIDs_terrain.vert";
	m_feedbackShader.LoadFromFile( fragShader, vertShader, NULL, NULL, NULL, NULL );

	fragShader = "data/Shaders/VirtualTexture/megatexture_terrain.frag";
	vertShader = "data/Shaders/VirtualTexture/megatexture_terrain.vert";
	m_megatextureShader.LoadFromFile( fragShader, vertShader, NULL, NULL, NULL, NULL );

	m_pageTable.InitPageTable();
}

/*
================================
VirtualTexture::Init
================================
*/
void VirtualTexture::Cleanup() {
	m_pageTable.CleanupPageTable();
}

/*
================================
VirtualTexture::InitSamplerFeedback
================================
*/
void VirtualTexture::InitSamplerFeedback( int width, int height ) {
	m_feedbackSurface.CreateSurface( RS_COLOR_BUFFER | RS_DEPTH_BUFFER, width, height );
	m_width = width;
	m_height = height;

	m_feedbackBuffer = (PixelFeedback_t *)malloc( width * height * sizeof( PixelFeedback_t ) );
}

/*
================================
VirtualTexture::BeginSamplerFeedBack
================================
*/
void VirtualTexture::BeginSamplerFeedBack() {
	// Bind the off screen render surface for rendering
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, m_feedbackSurface.GetFBO() );

	// A white pixel is equivalent to an invalid page.
	glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );

	glViewport( 0, 0, m_feedbackSurface.GetWidth(), m_feedbackSurface.GetHeight() );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

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

	// Clear previous frame values
	glClear( GL_COLOR_BUFFER_BIT );
	glClear( GL_DEPTH_BUFFER_BIT );

	// This will turn on back face culling.  Uncomment to turn on.
	glCullFace( GL_BACK );
	glEnable( GL_CULL_FACE );
	glDisable( GL_CULL_FACE );

	m_feedbackShader.UseProgram();

	// TODO: Draw the scene to fill the feedback buffer
	int virtualPagesWide = PAGE_COORD_WIDTH;
	int pageWidth = PAGE_WIDTH;
	//int pageCount = PAGE_COUNT;

	m_feedbackShader.SetUniform1i( "virtualPagesWide", 1, &virtualPagesWide );
	m_feedbackShader.SetUniform1i( "pageWidth", 1, &pageWidth );
	//m_megatextureShader.SetUniform1i( "pageCount", 1, &pageCount );
	m_feedbackShader.SetUniform1f( "virtualDistance", 1, &s_virtualDistance );
}

struct page_t {
	int x;
	int y;
	int mip;

	bool operator == ( const page_t & rhs ) const {
		if ( x != rhs.x ) {
			return false;
		}
		if ( y != rhs.y ) {
			return false;
		}
		if ( mip != rhs.mip ) {
			return false;
		}
		return true;
	}
};

/*
====================================================
MipSort
====================================================
*/
static int MipSort( const void * a, const void * b ) {
	const page_t * pa = (const page_t *)a;
	const page_t * pb = (const page_t *)b;

	// a goes before b
	if ( pa->mip > pb->mip ) {
		return -1;
	}

	// b goes before a
	return 1;
}

static void AppendUniquePage( std::vector< page_t > & pages, page_t p ) {
	for ( int i = 0; i < pages.size(); i++ ) {
		page_t e = pages[ i ];
		if ( e == p ) {
			return;
		}
	}
	pages.push_back( p );
}

/*
================================
VirtualTexture::EndSamplerFeedBack
================================
*/
void VirtualTexture::EndSamplerFeedBack() {
	memset( m_feedbackBuffer, 0, sizeof( PixelFeedback_t ) * m_width * m_height );

	// The scene will have been rendered to the feedback surface
	//glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, m_feedbackSurface.GetFBO() );
	const int width = m_feedbackSurface.GetWidth();
	const int height = m_feedbackSurface.GetHeight();
	glReadPixels( 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, m_feedbackBuffer );
	//glReadPixels( 0, 0, width, height, GL_RGBA, GL_UNSIGNED_SHORT, m_feedbackBuffer );
	//glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );

	// Process the sampler feedback buffer to acquire pages
	std::vector< page_t > pages;
	for ( int y = 0; y < height; y += 4 ) {
		for ( int x = 0; x < width; x += 4 ) {
			const int idx = x + y * width;
			PixelFeedback_t p = m_feedbackBuffer[ idx ];
			if ( !p.IsValid() ) {
				continue;
			}

			page_t page;
			page.x = ( p.r << 4 ) | ( p.b >> 4 );
			page.y = ( p.g << 4 ) | ( p.b & 15 );
			page.mip = p.a;
// 			page.x = p.r;
// 			page.y = p.g;
// 			page.mip = p.a;

			if ( page.mip >= MAX_MIPS ) {
				page.mip = MAX_MIPS - 1;
			}

			page.x >>= page.mip;
			page.y >>= page.mip;
			AppendUniquePage( pages, page );
		}
	}

	// Sort the page requests by mip level
	qsort( pages.data(), pages.size(), sizeof( page_t ), MipSort );

	// Request the page table to upload these pages
	m_pageTable.Reset();
	for ( int i = 0; i < pages.size(); i++ ) {
		page_t & p = pages[ i ];
		m_pageTable.UploadPage( p.x, p.y, p.mip );
	}
	m_pageTable.Finalize();

	myglGetError();
}

/*
================================
VirtualTexture::ProcessPageRequest
================================
*/
void VirtualTexture::BeginMegaTextureDraw() {
	m_megatextureShader.UseProgram();
	m_megatextureShader.SetAndBindUniformTexture( "pageCoordTexture", 0, GL_TEXTURE_2D, m_pageTable.GetPageCoordinatesTexture() );
	m_megatextureShader.SetAndBindUniformTexture( "scaleBiasTexture", 1, GL_TEXTURE_2D, m_pageTable.GetScaleAndBiasTexture() );
	m_megatextureShader.SetAndBindUniformTexture( "physicalTexture", 2, GL_TEXTURE_2D, m_pageTable.GetPhysicalTexture() );

	int virtualPagesWide = PAGE_COORD_WIDTH;
	int pageWidth = PAGE_WIDTH;
	int pageCount = PAGE_COUNT;

//  	m_megatextureShader.SetUniform1i( "virtualPagesWide", 1, &virtualPagesWide );
//  	m_megatextureShader.SetUniform1i( "pageWidth", 1, &pageWidth );
	m_megatextureShader.SetUniform1i( "pageCount", 1, &pageCount );
	m_megatextureShader.SetUniform1f( "virtualDistance", 1, &s_virtualDistance );
}
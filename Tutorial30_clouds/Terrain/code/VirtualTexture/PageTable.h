//
//  PageTable.h
//
#pragma once
#include "Miscellaneous/Types.h"
#include "Graphics/Graphics.h"
#include <string>
#include "Common.h"

#define PAGE_WIDTH 128				// Size used in RAGE (pixels of a single page)
//#define PHYSICAL_TEXTURE_WIDTH 4096	// Size used in RAGE (pixels of the physical texture)
//#define PAGE_COUNT 32				// PHYSICAL_TEXTURE_WIDTH / PAGE_WIDTH
#define PHYSICAL_TEXTURE_WIDTH ( 4096 )	// Size used in RAGE (pixels of the physical texture)
#define PAGE_COUNT ( PHYSICAL_TEXTURE_WIDTH / PAGE_WIDTH )


#define MAX_MIPS 8
//#define PAGE_COORD_WIDTH 1024		// The number of pages wide in the virtual texture
//#define SCALE_BIAS_WIDTH 32		// ( PHYSICAL_TEXTURE_WIDTH / PAGE_WIDTH )
//#define PAGE_COORD_WIDTH ( 1024 * MULTIPLIER )
#define PAGE_COORD_WIDTH ( 4096 )	// This is good per Terrain Tile (it's about the texel density we are looking for)
									// Which means we will need five times this for the whole terrain
									// This could require that we use a RGBA16 instead of RGBA8 feedback buffer
									// but we might be able to pack the extra bits into the mip (alpha) channel
									// It's probably going to be easier to use RGBA16... so we should start there.
//#define PAGE_COORD_WIDTH ( 8192 )
#define SCALE_BIAS_WIDTH ( PAGE_COUNT )		// ( PHYSICAL_TEXTURE_WIDTH / PAGE_WIDTH )



/*
================================
PhysicalTexture

The physical texture is the on gpu texture that stores the pages that are used for rendering.
	Each page is 128 x 128.  The physical texture is 4096 x 4096.  This means that there's a total
	of 32 x 32 pages that can be used for rendering at any one time (that's a total of 1024).
================================
*/
class PhysicalTexture {
public:
	PhysicalTexture() { m_textureID = 0; }
	~PhysicalTexture() {}

	unsigned int m_textureID;

	void Init();
	void Cleanup();

	void UploadPage( int vtX, int vtY, int ptX, int ptY, int mip, unsigned char * texture );
};

struct pageIdMRU_t {
	int vtX;
	int vtY;
	int mip;

	int ptX;
	int ptY;
	int counter;	// Counter for the number of contiguous frames used.  Goes to zero when not used.
	bool wasUsedLastFrame;
};

#define USE_MRU	// Enables most recently used caching policy.
				// When it's disabled, it overwrites the page cache every frame.

/*
================================
PageTable

The pagetable is split into two textures.
The first is mip-mapped with one texel per virtual page, it's 1024x1024 for mip0, filtered using nearest for texel/mip lookup.
	And the data corresponds to the virtual texture page for the desired LOD at the given virtual address.
	Each 2-byte texel of this texture stores the (x,y) coordinates of the physical page to be used for the virtual page.
	If the finer mip of a page isn't loaded yet, then the texel will point to a coarser mip

The scale and bias is not stored, it is calculated on the fly in the fragment shader

The second texture is not mip-mapped.  It is FP32x4 with one texel per physical page.
	Each texel is the ST-scale, S-bias, and T-bias.  Which is used to map the the virtual texture coordinate to the physical texture coordinate.
	This texture is 32x32 (4096/128 = 32).  Use the first texture to lookup which texel we need to examine.
================================
*/
class PageTable {
public:
	PageTable() { m_texturePageCoord = 0; m_textureScaleBias = 0; m_coordinatesTable = NULL; }
	~PageTable() {}

	void InitPageTable();
	void CleanupPageTable();

	void Reset();	// Reset the cache between frames... later we won't need to do this
	void UploadPage( const int vtX, const int vtY, int mip );
	void Finalize();
	
	unsigned int GetScaleAndBiasTexture() const { return m_textureScaleBias; }
	unsigned int GetPageCoordinatesTexture() const { return m_texturePageCoord; }
	unsigned int GetPhysicalTexture() const { return m_physicalTexture.m_textureID; }

private:
	void UpdatePageCoordinates( int vtX, int vtY, int ptX, int ptY, int mip );
	void UpdateScaleBias( int vtX, int vtY, int ptX, int ptY, int mip );

	unsigned int m_texturePageCoord;	// 1024x1024 mip mapped GL_LUMINANCE_ALPHA, L is x-coord, A is y-coord
	unsigned int m_textureScaleBias;	// 32x32 FP32x4

	PhysicalTexture m_physicalTexture;

	// debug page images
	unsigned char m_red[ PAGE_WIDTH * PAGE_WIDTH * 4 ];
	unsigned char m_green[ PAGE_WIDTH * PAGE_WIDTH * 4 ];
	unsigned char m_blue[ PAGE_WIDTH * PAGE_WIDTH * 4 ];
	unsigned char m_yellow[ PAGE_WIDTH * PAGE_WIDTH * 4 ];

	unsigned char m_texelBuffer[ PAGE_WIDTH * PAGE_WIDTH * 4 ];	// Used for the single threaded auto-gen virtual texture pages

	//unsigned char m_coordinatesTable[ PAGE_COORD_WIDTH * PAGE_COORD_WIDTH * 2 * MAX_MIPS ];
	unsigned char * m_coordinatesTable;//[ PAGE_COORD_WIDTH * PAGE_COORD_WIDTH * 2 * MAX_MIPS ];
	Vec4d m_scaleBiasTable[ PAGE_COUNT * PAGE_COUNT ];

#if defined( USE_MRU )
	pageIdMRU_t m_mruList[ PAGE_COUNT * PAGE_COUNT ];
#else
	int m_slots[ PAGE_COUNT * PAGE_COUNT ];	// slots in the physical texture
	int m_pageTable[ PAGE_COORD_WIDTH ][ PAGE_COORD_WIDTH ][ MAX_MIPS ];
#endif

	int m_numPageRequests;
};


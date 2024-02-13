//
//  Texture.h
//
#pragma once


enum TextureFormat_t {
	FMT_RGBA8 = 0,
	FMT_RGB8,
	FMT_RGBA32F,
	FMT_RGB32F,
	FMT_RG32F,
	FMT_R32F,
	FMT_RGBA16F,
	FMT_RGB16F,
	FMT_RG16F,
	FMT_R16F,
	FMT_BC1,
	FMT_BC3,
	//FMT_DEPTH,
};

enum WrapMode_t {
	WM_REPEAT = 0,
	WM_CLAMP,
};

enum FilertMode_t {
	FM_NEAREST = 0,
	FM_LINEAR,
	FM_NEAREST_MIPMAP_NEAREST,
	FM_LINEAR_MIPMAP_NEAREST,
	FM_NEAREST_MIPMAP_LINEAR,
	FM_LINEAR_MIPMAP_LINEAR,
};

enum TextureType_t {
	TT_TEXTURE_1D = 0,
	TT_TEXTURE_1D_ARRAY,
	TT_TEXTURE_2D,
	TT_TEXTURE_2D_ARRAY,
	TT_TEXTURE_3D,
//	TT_TEXTURE_CUBEMAP,
//	TT_TEXTURE_CUBEMAP_ARRAY,
};

struct TextureOpts_t {
	WrapMode_t wrapS;
	WrapMode_t wrapT;
	WrapMode_t wrapR;
	FilertMode_t minFilter;
	FilertMode_t magFilter;
	int dimX;
	int dimY;
	int dimZ;
	TextureType_t type;
	TextureFormat_t format;

	TextureOpts_t() {
		wrapS = WM_CLAMP;
		wrapT = WM_CLAMP;
		wrapR = WM_CLAMP;
		minFilter = FM_LINEAR;
		magFilter = FM_LINEAR;
		dimX = 0;
		dimY = 0;
		dimZ = 0;
		type = TT_TEXTURE_2D;
		format = FMT_RGBA8;
	}
};

/*
 ===============================
 Texture
 ===============================
 */
class Texture {
public:
	Texture();
	~Texture();
	
	void InitWithData( const void * data, const int width, const int height );
	void Init( TextureOpts_t opts, const void * data );
	
	unsigned int GetName()	const { return m_name; }

	int GetWidth()		const { return m_opts.dimX; }
	int GetHeight()		const { return m_opts.dimY; }

private:
	void InitializeFormat( const void * data );

private:
	unsigned int	m_name;

	TextureOpts_t m_opts;

	friend class TextureManager;
};

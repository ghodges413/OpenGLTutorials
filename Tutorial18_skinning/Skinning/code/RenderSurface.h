/*
 *  RenderSurface.h
 *
 */
#pragma once

enum RSFlags {
    RS_NONE             = 0,
    RS_DEPTH_BUFFER     = 1 << 0,
    RS_STENCIL_BUFFER   = 1 << 1,
    RS_COLOR_BUFFER     = 1 << 2,
    RS_POSITION_BUFFER  = 1 << 3,
    RS_NORMAL_BUFFER    = 1 << 4,
    RS_SPECULAR_BUFFER  = 1 << 5,
    RS_TANGENT_BUFFER   = 1 << 6,
};

/*
 ================================
 RenderSurface
 A complete surface that can be used for onscreen or offscreen rendering
 ================================
 */
class RenderSurface {
public:
	RenderSurface();	
	~RenderSurface();
	
	bool CreateSurface( const int flags, const int width, const int height );
	
	unsigned int	GetFBO() const { return mFBO; }	
	unsigned int	GetColorTexture() const { return mColorTexture; }
	unsigned int	GetDepthTexture() const { return mDepthTexture; }
	
	bool	HasColor() const { return ( 0 != mColorTexture ); }
	bool	HasDepth() const { return ( 0 != mDepthTexture ); }

	int		GetWidth() const { return mWidth; }
	int		GetHeight() const { return mHeight; }
    
private:
	unsigned int mFBO;
	unsigned int mColorTexture;
	unsigned int mDepthTexture;
	
	int mWidth;
	int mHeight;
};

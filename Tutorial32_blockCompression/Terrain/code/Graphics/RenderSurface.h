/*
 *  RenderSurface.h
 *
 */
#pragma once
#include "Graphics/Graphics.h"
#include "Miscellaneous/Pair.h"
#include "Miscellaneous/Array.h"

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


/*
 ================================
 GLSurface

 Abstract class for handling the common functionality of the glSurfaces
 ================================
 */
class GLSurface {
public:

	virtual bool CreateSurface( const int flags, const int * const dimensions ) = 0;

	unsigned int GetFBO() const { return mFBO; }
    unsigned int GetBuffer( const int buffID ) const;
    unsigned int GetTexture( const int buffID ) const;

	int GetWidth() const { return mWidth; }
	int GetHeight() const { return mHeight; }
    
	bool HasBuffer( const int buffID ) const;
    
    void SetDrawBuffers();

protected:
	void Cleanup();
    
private:
    int GetIndexFromBufferID( const int buffID ) const;
    
protected:
	unsigned int    mFBO;
    int             mWidth;
    int             mHeight;
	int				mDepth;
    
    // This should probably be a map
    typedef Pair< int, int > intPair_t;
    Array< intPair_t >   mIDPairMap;
    
    typedef Pair< unsigned int, unsigned int > uintPair_t;
    Array< uintPair_t > mBufferPairs;
    
    Array< GLenum > mBufferColorAttachments;
};

/*
 ================================
 GLSurface2d
 ================================
 */
class GLSurface2d : public GLSurface {
private:
	GLSurface2d( const GLSurface2d & rhs );
	const GLSurface2d & operator = ( const GLSurface2d & rhs );

public:
    GLSurface2d();
    ~GLSurface2d();
    
    virtual bool CreateSurface( const int flags, const int * const dimensions );
};


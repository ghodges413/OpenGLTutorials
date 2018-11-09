/*
 *  Targa.cpp
 *
 */
#include <stdio.h>
#include "Targa.h"
#include "Fileio.h"
#include <assert.h>
#include <errno.h>
#include <string>

template < class T >
void hbSwap( T & a, T & b ) {
	T tmp = b;
	b = a;
	a = tmp;
}

static const int MAXTEXTURESIZE = 1024;

static int ReadInt( const unsigned char * char4 ) {
    return char4[ 0 ] + char4[ 1 ] * 0x100 + char4[ 2 ] * 0x10000 + char4[ 3 ] * 0x1000000;
}
static short ReadShort( const unsigned char * char2 ) {
    return char2[ 0 ] + char2[ 1 ] * 0x100;
}

struct colorMapSpec_t {
    unsigned char	mFirstEntryIndex[ 2 ];	// offset into the color map table
    unsigned char	mColorMapLength[ 2 ];	// number of entries
    unsigned char	mColorMapSize;		// bits per pixel
};

struct imageSpec_t {
    unsigned char	mXorigin[ 2 ];
    unsigned char	mYorigin[ 2 ];
    unsigned char	mWidth[ 2 ];
    unsigned char	mHeight[ 2 ];
    unsigned char	mPixelDepth;        // bits per pixel
    unsigned char	mImageDescriptor;   // Describes the ordering (ie left-to-right + top-to-bottom)
};

struct targaHeader_t {
	unsigned char	id;
	unsigned char	colorMapType;	// unless 0, there is a color map (currently unsupported)
	unsigned char	imageType;		// 2 = color, 3 = greyscale
	colorMapSpec_t	colorMapSpec;
	imageSpec_t		imageSpec;
};

struct targaFooter_t {
    unsigned char	mExtensionOffset[ 4 ];  // bytes from beginning of file
    unsigned char	mDeveloperOffset[ 4 ];  // bytes from beginning of file
    char			mSignature[ 18 ];       // Always displays "TRUEVISION-XFILE"
};

struct targaExtension_t {
    unsigned char   mExtensionSize[ 2 ];    // size of extension, always 495 bytes
    char            mAuthorName[ 41 ];
    char            mAuthorComment[ 324 ];
    char            mTimeStamp[ 12 ];       // Date and time stamp
    unsigned char   mJobID[ 41 ];
    unsigned char   mJobTime[ 6 ];          // Hours/Time/Minutes making file
    char            mSoftwareID[ 41 ];      // Software Name that generated file
    unsigned char   mSoftwareVersion[ 3 ];
    unsigned char   mKeyColor[ 4 ];
    unsigned char   mPixelAspectRatio[ 4 ];
    unsigned char   mGammaValue[ 4 ];
    unsigned char   mColorCorrectionOffset[ 4 ];    // offset from beginning of a file to color correction table, if present
    unsigned char   mPostageStampOffset[ 4 ];       // offset from beginning of a file to postage stamp image (ie scaled down copy of image), if present
    unsigned char   mScanLineOffset[ 4 ];           // offset from beginning of a file to scan lines table, if present
    unsigned char   mAttributesType;                // specifies the alpha channel
};

/*
 ===============================
 Targa::Targa
 ===============================
 */
Targa::Targa() :
mWidth( 0 ),
mHeight( 0 ),
mBitsPerPixel( 0 ),
mOrdering( TARGA_ORDER_NONE ),
mData_ptr( NULL ) {
}

/*
 ===============================
 Targa::~Targa
 ===============================
 */
Targa::~Targa() {
    if ( mData_ptr ) {
        delete[] mData_ptr;
        mData_ptr = NULL;
    }
}

/*
 ===============================
 Targa::DisplayHeader
 ===============================
 */
void Targa::DisplayHeader( const unsigned char * data, const int size ) {
    //
    //  Read Header
    //
	{
		targaHeader_t header;
		memcpy( &header, data, sizeof( header ) );

		printf( "ID:  %i\n", header.id );
		printf( "Color Map Type: %i\n", header.colorMapType );
		printf( "Image Type: %i\n", header.imageType );

		{
			const int firstIndex  = ReadShort( header.colorMapSpec.mFirstEntryIndex );
			const int colorLength = ReadShort( header.colorMapSpec.mColorMapLength );
			printf( "Color Map Spec:\n   FirstIndex %i\n   ColorMapLength %i\n   bpp %i\n",
					firstIndex,
					colorLength,
					header.colorMapSpec.mColorMapSize );
		}
    
		{
			const int xorigin = ReadShort( header.imageSpec.mXorigin );
			const int yorigin = ReadShort( header.imageSpec.mYorigin );
			const int width   = ReadShort( header.imageSpec.mWidth );
			const int height  = ReadShort( header.imageSpec.mHeight );
			printf( "Image Spec:\n   x: %i  y: %i\n   width: %i  height: %i\n   bpp: %i  imageDescriptor: %i\n",
				   xorigin,
				   yorigin,
				   width,
				   height,
				   header.imageSpec.mPixelDepth,
				   header.imageSpec.mImageDescriptor );
		}
	}
    
    //
    //  Read footer
    //
    targaFooter_t footer;
	memcpy( &footer, data + size - sizeof( targaFooter_t ) - 1, sizeof( footer ) );
    
    char        signature[ 19 ] = { '\0' };
    const int   extensionOffset = ReadInt( footer.mExtensionOffset );
    const int   developerOffset = ReadInt( footer.mDeveloperOffset );
    {
        memcpy( signature, footer.mSignature, 18 );
        printf( "Footer:\n   ExtOffset: %i\n   DevOffset: %i\n   Signature: %s\n",
               extensionOffset,
               developerOffset,
               signature );
    }
    
    //
    //  Check if this targa file is version 2 or original
    //
    if ( 0 == strcmp( "TRUEVISION-XFILE.", signature ) ) {
        printf( "Targa is version 2.0\n" );
        
        //
        //  Read Extension Area
        //
        if ( extensionOffset > 0 ) {
            targaExtension_t extensionArea;
            const int headSize = ( sizeof( colorMapSpec_t ) + sizeof( imageSpec_t ) );
            printf( "Extension Area:\n  Header size: %i    ExtOffset: %i\n", headSize, extensionOffset );
            assert( extensionOffset >= headSize );
			memcpy( &extensionArea, data + extensionOffset, sizeof( extensionArea ) );
            
            const int extensionSize = ReadShort( extensionArea.mExtensionSize );
            char author[ 42 ] = { '\0' };
            memcpy( author, extensionArea.mAuthorName, 41 );
            char comment[ 325 ] = { '\0' };
            memcpy( comment, extensionArea.mAuthorComment, 324 );
            char jobID[ 42 ] = { '\0' };
            memcpy( jobID, extensionArea.mJobID, 41 );
            char softwareID[ 42 ] = { '\0' };
            memcpy( softwareID, extensionArea.mSoftwareID, 41 );
            
            const int keyColor          = ReadInt( extensionArea.mKeyColor );
            const int pixelAspect       = ReadInt( extensionArea.mPixelAspectRatio );
            const int gammValue         = ReadInt( extensionArea.mGammaValue );
            const int postagOffset      = ReadInt( extensionArea.mPostageStampOffset );
            const int scanLineOffset    = ReadInt( extensionArea.mScanLineOffset );
            const int attributType      = extensionArea.mAttributesType;

            printf( "   ExtSize:  %i\n   author: %s\n   comment: %s\n   jobID: %s\n   softwareID: %s\n  keyColor: %i\n  aspect: %i\n  gamma: %i\n",
                   extensionSize, author, comment, jobID, softwareID, keyColor, pixelAspect, gammValue );
            printf( "Postage Offset: %i\n   ScanLine Offset: %i\n   Attribute Type: %i\n",
                   postagOffset, scanLineOffset, attributType );
            printf( "End of Extension\n" );
        }
        
        //
        //  Read Developer Area
        //
        if ( developerOffset > 0 ) {
            //  There's really no need to bother... unless we pad it ourselves
        }
    }
}

/*
 ===============================
 Targa::Load
 ===============================
 */
bool Targa::Load( const char * fileName, const bool verbose /* = false */ ) {
	unsigned char type[ 4 ];
	unsigned char info[ 6 ];

	unsigned int size = 0;
	unsigned char * data = NULL;

	const bool didGetData = GetFileData( fileName, &data, size );
	
	if ( !didGetData ) {
		char errorMsg[ 2048 ] = { 0 };
		perror( errorMsg );
		printf( "ERROR: open file failed: %s\nWith ERROR: %s\n", fileName, errorMsg );
		return false;
	}

	if ( verbose ) {
		DisplayHeader( data, size );
	}
	
	unsigned char * data_ptr = data;

	memcpy( &type, data_ptr, 3 );
	data_ptr += 12;
	memcpy( &info, data_ptr, 6 );
	data_ptr += 6;
	
    // if type[ 1 ] is non-zero, there's a color map
	//  type[ 2 ] either 2 (color) or 3 (greyscale)
	if ( type[ 1 ] != 0 || ( type[ 2 ] != 2 && type[ 2 ] != 3 ) ) {
		free( data );
		return false;
	}
	
	mWidth          = ReadShort( info );
	mHeight         = ReadShort( info + 2 );
    mBitsPerPixel   = info[ 4 ];

	if ( verbose ) {
        printf( "width: %i\n", mWidth );
        printf( "height: %i\n", mHeight );
        printf( "bits per pixel: %i\n", mBitsPerPixel );
    }
	
	if ( mBitsPerPixel != 24 && mBitsPerPixel != 32 && mBitsPerPixel != 8 ) {
		free( data );
		return false;
	}
	
	//info[5] is our image descriptor
    mOrdering = TARGA_ORDER_NONE;
    if ( info[ 5 ] & ( 1 << 4 ) ) {
        mOrdering |= TARGA_ORDER_LEFT_TO_RIGHT;
    } else {
        mOrdering |= TARGA_ORDER_RIGHT_TO_LEFT;
    }
    if ( info[ 5 ] & ( 1 << 5 ) ) {
        mOrdering |= TARGA_ORDER_TOP_TO_BOTTOM;
    } else {
        mOrdering |= TARGA_ORDER_BOTTOM_TO_TOP;
    }
	mTargaOrdering = info[ 5 ];
    
    if ( verbose ) {
        if ( mOrdering & TARGA_ORDER_LEFT_TO_RIGHT ) {
            printf( "image is left-to-right\n" );
        }
        if ( mOrdering & TARGA_ORDER_RIGHT_TO_LEFT ) {
            printf( "image is right-to-left\n" );
        }
        if ( mOrdering & TARGA_ORDER_TOP_TO_BOTTOM ) {
            printf( "image is top-to-bottom\n" );
        }
        if ( mOrdering & TARGA_ORDER_BOTTOM_TO_TOP ) {
            printf( "image is bottom-to-top\n" );
        }
		if ( 0 == mOrdering ) {
			printf( "no ordering specified\n" );
		}
    }
    
    if ( type[ 0 ] > 0 ) {
        const int strLength = type[ 0 ] + 1;
        assert( type[ 0 ] <= 255 );
        char identifier[ 256 + 32 ];
        for ( int i = 0; i < strLength; ++i ) {
            identifier[ i ] = 0;
        }
		memcpy( identifier, data_ptr, type[ 0 ] );
        printf( "Targa contains identifier name: %s\n", identifier );
    }
	
	// read in image data
    const int imageSize = ( mWidth * mHeight * ( mBitsPerPixel >> 3 ) );
    unsigned char * pixelData = new unsigned char[ imageSize ];
	memcpy( pixelData, data_ptr, imageSize );
	
	// release the data that we no longer need
	free( data );
	
    
    const int finalImageSize = mWidth * mHeight * 4;
    mData_ptr = new unsigned char[ finalImageSize ];
    
    if ( 32 == mBitsPerPixel ) {
        assert( finalImageSize == imageSize );
        //
        //  Fill the array
        //
        for ( int i = 0; i < finalImageSize; i += 4 ) {
            mData_ptr[ i + 0 ]	= pixelData[ i + 0 ];
            mData_ptr[ i + 1 ]  = pixelData[ i + 1 ];
            mData_ptr[ i + 2 ]  = pixelData[ i + 2 ];
            mData_ptr[ i + 3 ]  = pixelData[ i + 3 ];
            mData_ptr[ i + 3 ]  = 255;
            
            // Convert from BGRA to RGBA
            hbSwap( mData_ptr[ i + 0 ], mData_ptr[ i + 2 ] );
        }
    } else if ( 24 == mBitsPerPixel ) {
        const int numPixels = mWidth * mHeight;
        for ( int i = 0; i < numPixels; ++i ) {
            const int offset32 = i * 4;
            const int offset24 = i * 3;
            mData_ptr[ offset32 + 0 ]	= pixelData[ offset24 + 0 ];
            mData_ptr[ offset32 + 1 ]	= pixelData[ offset24 + 1 ];
            mData_ptr[ offset32 + 2 ]	= pixelData[ offset24 + 2 ];
            mData_ptr[ offset32 + 3 ]	= 255;
            
            // Convert from BGRA to RGBA
            hbSwap( mData_ptr[ offset32 + 0 ], mData_ptr[ offset32 + 2 ] );
        }
    } else if ( 8 == mBitsPerPixel ) {
        const int numPixels = mWidth * mHeight;
        for ( int i = 0; i < numPixels; ++i ) {
            const int offset32 = i * 4;
            mData_ptr[ offset32 + 0 ]	= pixelData[ i ];
            mData_ptr[ offset32 + 1 ]	= pixelData[ i ];
            mData_ptr[ offset32 + 2 ]	= pixelData[ i ];
            mData_ptr[ offset32 + 3 ]	= 255;
        }
	}

	mBitsPerPixel = 32;
	delete[] pixelData;
	return true;
}

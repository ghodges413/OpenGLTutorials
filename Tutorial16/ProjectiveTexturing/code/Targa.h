/*
 *  Targa.h
 *
 */
#pragma once

enum targaOrdering_t {
    TARGA_ORDER_NONE          = 0,
    TARGA_ORDER_LEFT_TO_RIGHT = 1 << 0,
    TARGA_ORDER_RIGHT_TO_LEFT = 1 << 1,
    TARGA_ORDER_TOP_TO_BOTTOM = 1 << 2,
    TARGA_ORDER_BOTTOM_TO_TOP = 1 << 3,
};

/*
 ===============================
 Targa
 ===============================
 */
class Targa {
public:
	Targa();
	~Targa();
    
    bool Load( const char * filename, const bool verbose = false );

	const unsigned char * DataPtr() const { return mData_ptr; }

	int GetWidth() const    { return mWidth; }
	int GetHeight() const   { return mHeight; }

	int GetBitsPerPixel() const { return mBitsPerPixel; }

private:
    static void DisplayHeader( const unsigned char * data, const int size );
	
private:	
	int mWidth;
	int mHeight;
    int mBitsPerPixel;
    
    int mOrdering;
	int mTargaOrdering;
    
    unsigned char * mData_ptr;
};

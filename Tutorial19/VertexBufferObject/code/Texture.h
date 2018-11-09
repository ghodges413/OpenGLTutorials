/*
 *  Texture.h
 *
 */
#pragma once

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
	
	unsigned int GetName()	const { return mName; }

	int GetWidth()		const { return mWidth; }
	int GetHeight()		const { return mHeight; }

private:
	unsigned int	mName;
	
	int		mWidth;
	int		mHeight;
};

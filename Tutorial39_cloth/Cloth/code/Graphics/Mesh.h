//
//	Mesh.h
//
#pragma once
#include "Miscellaneous/Array.h"
#include "Math/Vector.h"
#include "Graphics/VertexArrayObject.h"
#include "Graphics/VertexBufferObject.h"

/*
====================================================
FloatToByte
// Assumes a float between [-1,1]
====================================================
*/
inline unsigned char FloatToByte_n11( const float f ) {
	int i = (int)(f * 127 + 128);
	return (unsigned char)i;
}
/*
====================================================
FloatToByte
// Assumes a float between [0,1]
====================================================
*/
inline unsigned char FloatToByte_01( const float f ) {
	int i = (int)(f * 255);
	return (unsigned char)i;
}

inline void Vec3ToByte4_n11( unsigned char * a, const Vec3 & b ) {
	a[ 0 ] = FloatToByte_n11( b.x );
	a[ 1 ] = FloatToByte_n11( b.y );
	a[ 2 ] = FloatToByte_n11( b.z );
	a[ 3 ] = 0;
}

inline void Vec3ToByte4_01( unsigned char * a, const Vec3 & b ) {
	a[ 0 ] = FloatToByte_01( b.x );
	a[ 1 ] = FloatToByte_01( b.y );
	a[ 2 ] = FloatToByte_01( b.z );
	a[ 3 ] = 0;
}

/*
====================================================
vert_t
// 8 * 4 = 32 bytes - data structure for drawable verts... this should be good for most things
====================================================
*/
struct vert_t {
	Vec3			pos;		// 12 bytes
	Vec2			st;			// 8 bytes
	unsigned char	norm[ 4 ];	// 4 bytes
	unsigned char	tang[ 4 ];	// 4 bytes
	unsigned char	buff[ 4 ];	// 4 bytes
};

/*
 ================================
 Mesh
 ================================
 */
class Mesh {
public:
	Mesh();

	bool Load( const char * name );
	bool LoadFromData( const vert_t * verts, int numVerts, const unsigned short * indices, int numIndices );
	void Draw() const;

	int NumVerts() const { return mVerts.Num(); }

	bool BuildFromCloth( float * verts, const int width, const int height, const int stride );
	void UpdateClothVerts( float * verts, const int width, const int height, const int stride );

private:
	void CalculateTangents();
	void MakeVBO();

private:
	Array< vert_t > mVerts;

	VertexBufferObject    mVBO;
    VertexArrayObject     mVAO;
};
//
//	String.h
//
#pragma once
#include <stddef.h>
#include <string>
#include "Assert.h"
#include "Miscellaneous/Array.h"

/*
 ====================================
 String
 ====================================
 */
class String {
public:
	String();
    String( const char character );
	String( const char * rhs );
	String( const char * rhs, const int length );
	String( const String & rhs );
	String & operator = ( const char * rhs );
	String & operator = ( const String & rhs );
	~String();

	static String va( const char * format, ... );
	
	bool operator == ( const char * rhs ) const;
	bool operator == ( const String & rhs ) const;
    bool operator != ( const char * rhs ) const;
    bool operator != ( const String & rhs ) const;
	String operator + ( const char * rhs ) const;
	String operator + ( const String & rhs ) const;
	const String & operator += ( const char * rhs );
	const String & operator += ( const String & rhs );

	bool operator < ( const String & rhs ) const;
	char operator [] ( const int idx ) const;
	
	const char * cstr() const { return mStr; }			// explicit conversion to c-string
	char * cstr() { return mStr; }
    operator const char *() const { return cstr(); }	// implicit conversion to c-string
	
	int		Length()	const { return mLength; }
	int		Size()		const { return mLength + 1; }		// actual size in bytes
	bool	IsEmpty()	const { return ( 0 == mLength ); }
    
    void ToLower();
    void ToUpper();
    
    static void ToArgumentList( const String & string, Array< String > & arguments );

	static void BackSlashesToForwardSlashes( char * str );
	static void ForwardSlashesToBackSlashes( char * str );
	void BackSlashesToForwardSlashes() { BackSlashesToForwardSlashes( mStr ); }
	void ForwardSlashesToBackSlashes() { ForwardSlashesToBackSlashes( mStr ); }
	void RemoveWhiteSpace();
	void RemoveCharacter( const char character );
    void RemoveEnclosingDoubleQuotes();
	String GetFilePath() const;

	int FindFirstStr( const char * str ) const;
    
    friend bool operator == ( const char * lhs, const String & rhs );
    friend bool operator != ( const char * lhs, const String & rhs );
	
protected:
	static const int sBaseSize = 64;
	char mStrBase[ sBaseSize ];

	char * mStr;	// Points to mStrBase, unless the string length is greater than sBaseSize
	int mLength;	// number of characters in string ( does not include the '\0' )
};

/*
 ====================================
 String::String
 ====================================
 */
inline String::String() :
mLength( 0 ) {
	mStrBase[ 0 ] = 0;
	mStr = mStrBase;
}

/*
 ====================================
 String::String
 ====================================
 */
inline String::String( const char character ) {
	mLength = 1;
	mStrBase[ 0 ] = character;
	mStrBase[ 1 ] = 0;
	mStr = mStrBase;
}

/*
 ====================================
 String::String
 ====================================
 */
inline String::String( const char * rhs ) {
	assert( rhs != NULL );
	mStrBase[ 0 ] = 0;
	mStr	= mStrBase;
	mLength	= 0;
	if ( NULL == rhs ) {
		return;
	}
	
	mLength = strlen( rhs );
	if ( mLength >= sBaseSize ) {
		mStr = (char * )malloc( mLength + 1 );
	}

	strcpy( mStr, rhs );	// double check the behavior of this function
}

/*
 ====================================
 String::String
 ====================================
 */
inline String::String( const char * rhs, const int length ) {
	assert( rhs != NULL );
	mStrBase[ 0 ] = 0;
	mStr	= mStrBase;
	mLength	= 0;
	if ( NULL == rhs ) {
		return;
	}

	mLength = strlen( rhs );
	if ( length < mLength ) {
		mLength = length;
	}

	if ( mLength >= sBaseSize ) {
		mStr = (char * )malloc( mLength + 1 );
	}

	memcpy( mStr, rhs, mLength );
	mStr[ mLength ] = '\0';
}

/*
 ====================================
 String::String
 ====================================
 */
inline String::String( const String & rhs ) {
	mStrBase[ 0 ] = 0;
	mStr = mStrBase;

	mLength = rhs.mLength;
	if ( NULL == rhs.mStr ) {
		mLength = 0;
		return;
	}

	if ( mLength >= sBaseSize ) {
		mStr = (char *)malloc( mLength + 1 );
	}
	strcpy( mStr, rhs.mStr );
}

/*
 ====================================
 String::operator =
 ====================================
 */
inline String & String::operator = ( const char * rhs ) {
	assert( rhs != NULL );
	if ( NULL == rhs ) {
		return *this;
	}

	mStrBase[ 0 ] = 0;
	if ( mStr != mStrBase ) {
		free( mStr );
		mStr = mStrBase;
	}

	mLength = strlen( rhs );
	if ( mLength >= sBaseSize ) {
		mStr = (char *)malloc( mLength + 1 );
	}

	strcpy( mStr, rhs );
	return *this;
}

/*
 ====================================
 String::operator =
 ====================================
 */
inline String & String::operator = ( const String & rhs ) {
	*this = rhs.mStr;
	return *this;
}

/*
 ====================================
 String::~String
 ====================================
 */
inline String::~String() {
	if ( mStr != mStrBase ) {
		free( mStr );
		mStr = NULL;
	}
	mLength = 0;
}

/*
 ====================================
 String::operator ==
 ====================================
 */
inline bool String::operator == ( const char * rhs ) const {
    assert( mStr );
    assert( rhs );
	return ( strcmp( mStr, rhs ) == 0 );
}

/*
 ====================================
 String::operator ==
 ====================================
 */
inline bool String::operator == ( const String & rhs ) const {
	if ( mLength != rhs.mLength ) {
		// this check makes this function faster, on average, than the above function
		return false;
	}
    assert( mStr );
    assert( rhs.mStr );
	return ( strcmp( mStr, rhs.mStr ) == 0 );
}

/*
 ====================================
 String::operator !=
 ====================================
 */
inline bool String::operator != ( const char * rhs ) const {
    assert( mStr );
    assert( rhs );
	return ( strcmp( mStr, rhs ) != 0 );
}

/*
 ====================================
 String::operator !=
 ====================================
 */
inline bool String::operator != ( const String & rhs ) const {
	if ( mLength != rhs.mLength ) {
		// this check makes this function faster, on average, than the above function
		return true;
	}
    assert( mStr );
    assert( rhs.mStr );
	return ( strcmp( mStr, rhs.mStr ) != 0 );
}

/*
 ====================================
 String::operator +
 ====================================
 */
inline String String::operator + ( const char * rhs ) const {
	String str = *this;
	str += rhs;
	return str;
}

/*
 ====================================
 String::operator +
 ====================================
 */
inline String String::operator + ( const String & rhs ) const {
    String str = *this;
	str += rhs;
	return str;
}

/*
 ====================================
 String::operator +=
 ====================================
 */
inline const String & String::operator += ( const char * rhs ) {
    assert( rhs );
	const int rhsLength = strlen( rhs );
	const int newLength = mLength + rhsLength;
	
	if ( newLength >= sBaseSize ) {
		char * tmp = (char*)malloc( newLength + 1 );
		memcpy( tmp, mStr, mLength );
		memcpy( tmp + mLength, rhs, rhsLength + 1 );

		mStrBase[ 0 ] = 0;
		if ( mStr != mStrBase ) {
			free( mStr );
		}
		mStr = tmp;
	} else {
		if ( mStr != mStrBase ) {
			// this really shouldn't be happening
			memcpy( mStrBase, mStr, mLength );
			free( mStr );
			mStr = mStrBase;
		}
		memcpy( mStr + mLength, rhs, rhsLength + 1 );
	}
	
	mLength = newLength;
	mStr[ mLength ] = '\0';
	
	return *this;
}

/*
 ====================================
 String::operator +=
 ====================================
 */
inline const String & String::operator += ( const String & rhs ) {
	*this += rhs.mStr;
	return *this;
}

/*
 ====================================
 String::operator <
 * used for sorting, this is useful in containers
 ====================================
 */
inline bool String::operator < ( const String & rhs ) const {
	int terminate( mLength );
	if ( rhs.mLength < mLength ) {
		terminate = rhs.mLength;
	}
	
	for ( int i = 0; i < terminate; ++i ) {
		if ( mStr[ i ] == rhs.mStr[ i ] ) {
			continue;
		}
		return ( mStr[ i ] < rhs.mStr[ i ] );
	}
	
	return ( mLength < rhs.mLength );
}

/*
 ====================================
 String::operator []
 ====================================
 */
inline char String::operator[]( const int idx ) const {
	return mStr[ idx ];
}
/*
 *  String.cpp
 *
 */

#include "String.h"
#include <stdarg.h>

/*
 ================================
 operator ==
 ================================
 */
bool operator == ( const char * lhs, const String & rhs ) {
    return ( rhs == lhs );
}

/*
 ================================
 operator !=
 ================================
 */
bool operator != ( const char * lhs, const String & rhs ) {
    return ( rhs != lhs );
}

/*
 ====================================
 String::va
 ====================================
 */
String String::va( const char * format, ... ) {
	char buffer[ 2048 ] = { 0 };
	
	// start va list
    va_list arg;
    va_start( arg, format );
    
	// print to buffer
	//int n = vsprintf( buffer, format, arg );
    vsprintf( buffer, format, arg );
        
    // end va list
    va_end( arg );

	String str = buffer;
	return str;
}

/*
 ====================================
 String::ToLower
 ====================================
 */
void String::ToLower() {
    for ( int i = 0; i < mLength; ++i ) {
        mStr[ i ] = tolower( mStr[ i ] );
    }
}

/*
 ====================================
 String::ToUpper
 ====================================
 */
void String::ToUpper() {
    for ( int i = 0; i < mLength; ++i ) {
        mStr[ i ] = toupper( mStr[ i ] );
    }
}

/*
 ====================================
 String::ToArgumentList
 ====================================
 */
void String::ToArgumentList( const String & string, Array< String > & arguments ) {
    arguments.Clear();
    
    int previousIdx = 0;
    for ( int i = 0; i < string.Length() + 1; ++i ) {
        const char letter = string[ i ];
        
        bool copyToken = false;
        if ( ' ' == letter || 0 == letter ) {
            if ( previousIdx == i ) {
                // if they're the same then we're dealing
                // with consecutive white space
                ++previousIdx;
                continue;
            }
            copyToken = true;
        }
        
        if ( string.Length() == i ) {
            // If we're at the end of the string, we should
            // go ahead and copy this last token the array
            copyToken = true;
        }

        // The check for i > previousIdx is to ensure that
        // the passed in string wasn't a long stretch of spaces
        if ( copyToken && i > previousIdx ) {
            // copy the token to the argument list
            const int length = i - previousIdx;
			const int maxLength = 2048;
			assert( length + 1 < maxLength );
            char str[ maxLength ];
            for ( int j = 0; j < length; ++j ) {
                str[ j ] = string[ previousIdx + j ];
            }
            str[ length ] = '\0';
            
            arguments.Append( str );
        } else if ( copyToken && letter != ' ' ) {
            arguments.Append( letter );
        }
        
        if ( copyToken ) {
            previousIdx = i + 1;
        }
    }
}

/*
 ====================================
 String::BackSlashesToForwardSlashes
 ====================================
 */
void String::BackSlashesToForwardSlashes( char * str ) {
	const int length = strlen( str );
	for ( int i = 0; i < length; ++i ) {
		if ( '\\' == str[ i ] ) {
			str[ i ] = '/';
		}
	}
}

/*
 ====================================
 String::ForwardSlashesToBackSlashes
 ====================================
 */
void String::ForwardSlashesToBackSlashes( char * str ) {
	const int length = strlen( str );
	for ( int i = 0; i < length; ++i ) {
		if ( '/' == str[ i ] ) {
			str[ i ] = '\\';
		}
	}
}

/*
 ====================================
 String::RemoveWhiteSpace
 * removes all whitespace from the string
 ====================================
 */
void String::RemoveWhiteSpace() {
	for ( int i = 0; i < mLength; ++i ) {
		if ( ' ' == mStr[ i ] ) {
			--mLength;
			if ( i == mLength ) {
				break;
			}
			for ( int j = i; j < mLength + 1; ++j ) {
				mStr[ j ] = mStr[ j + 1 ];
			}
			--i;
		}
	}
}

/*
 ====================================
 String::RemoveCharacter
 * removes all instances of the passed in character
 ====================================
 */
void String::RemoveCharacter( const char character ) {
	assert( character != '\0' );
	for ( int i = 0; i < mLength; ++i ) {
		if ( character == mStr[ i ] ) {
			--mLength;
			if ( i == mLength ) {
				break;
			}
			for ( int j = i; j < mLength + 1; ++j ) {
				mStr[ j ] = mStr[ j + 1 ];
			}
			--i;
		}
	}
}


/*
 ====================================
 String::RemoveEnclosingDoubleQuotes
 ====================================
 */
void String::RemoveEnclosingDoubleQuotes() {
    // remove trailing quote
    if ( '\"' == mStr[ mLength - 1 ] ) {
        mStr[ mLength - 1 ] = '\0';
        --mLength;
    }
    
    // remove leading quote
    if ( '\"' == mStr[ 0 ] ) {
        for ( int i = 0; i < mLength; ++i ) {
            mStr[ i ] = mStr[ i + 1 ];
        }
        --mLength;
    }
}

/*
 ====================================
 String::FindFirstStr
 ====================================
 */
int String::FindFirstStr( const char * str ) const {
	const int length = strlen( str );

	for ( int i = 0; i < mLength; ++i ) {
		bool didFind = true;

		for ( int j = 0; j < length; ++j ) {
			if ( i + j >= mLength ) {
				didFind = false;
				break;
			}

			if ( str[ j ] != mStr[ i + j ] ) {
				didFind = false;
				break;
			}
		}

		if ( didFind ) {
			return i;
		}
	}

	return -1;
}

/*
 ====================================
 String::GetFilePath
 ====================================
 */
String String::GetFilePath() const {
	String filePath = *this;

	int length = mLength;
	for ( int i = mLength - 1; i >= 0; --i ) {
		if ( '/' == mStr[ i ] || '\\' == mStr[ i ] ) {
			break;
		}
		--length;
	}

	filePath.mStr[ length ] = '\0';
	filePath.mLength = length;

	return filePath;
}



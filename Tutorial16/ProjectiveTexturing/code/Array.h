/*
 *  Array.h
 *
 */
#pragma once
#include <assert.h>
#ifdef WINDOWS
#include <stddef.h>
#endif

/*
 ================================
 Array
 ================================
 */
template < class T >
class Array {
public:
	Array( const int expansionStep = 16 );
    Array< T > & operator = ( const Array< T > & rhs );
	~Array();
	
	void	Clear() { mNumElements = 0; }
	void	Empty() { delete[] mArray; mArray = NULL; mSize = 0; mNumElements = 0; }
	int		Num() const { return mNumElements; }
	
	size_t	Allocated()		const { return sizeof( T ) * mSize; }						// Total memory reserved
	size_t	Size()			const { return sizeof( T ) * mNumElements; }				// memory used by the elements
	size_t	UnusedMemory()	const { return sizeof( T ) * ( mSize - mNumElements ); }	// Extra space that's currently not in use
	
	void	Append( const T & element );
	void	AppendUnique( const T & element );
	void	AppendArray( const Array< T > & rhs );
	void	Expand( const int & newSize );
	void	Shrink( const int & newSize );
	void	Resize( const int & newSize );
	
	const   T & operator[]( const int idx ) const;
            T & operator[]( const int idx );
	
	const   T * ToPtr() const   { return mArray; }
            T * ToPtr()         { return mArray; }
	
	void	Remove( const int idx );
	bool	RemoveElement( const T & element );
	void	Insert( const T & element, const int idx );
	
	int		Find( const T & element ) const;
    
    void    Reverse();  // reverse the order of the element
	
private:
	int mNumElements;	// number of elements in the array
	int mSize;			// number of elements that can be stored
	int mExpansionSize;	// number of elements to add when auto-expansion is required
	T * mArray;
};

/*
 ================================
 Array::Array
 ================================
 */
template < class T >
Array< T >::Array( const int expansionStep ) :
mArray( NULL ),
mSize( 0 ),
mNumElements( 0 ),
mExpansionSize( expansionStep ) {
}

/*
 ================================
 Array::operator =
 ================================
 */
template < class T >
Array< T > & Array< T >::operator=( const Array< T > & rhs ) {
    mNumElements = rhs.mNumElements;
    mSize = rhs.mSize;
    mExpansionSize = rhs.mExpansionSize;
    mArray = new T[ mSize ];
    for ( int i = 0; i < mNumElements; ++i ) {
        mArray[ i ] = rhs.mArray[ i ];
    }
    return *this;
}

/*
 ================================
 Array::~Array
 ================================
 */
template < class T >
Array< T >::~Array() {
	if ( mArray ) {
		delete[] mArray;
		mArray = NULL;
	}
	mSize = 0;
	mNumElements = 0;
}

/*
 ================================
 Array::Append
 ================================
 */
template < class T >
void Array<T>::Append( const T& element ) {
	if ( mNumElements == mSize ) {
		// Expand array
		Expand( mSize + mExpansionSize + 1 );
	}
	
	// copy element into array
	mArray[ mNumElements ] = element;
	++mNumElements;
}

/*
 ================================
 Array::AppendUnique
 * same as above, but does not allow duplicates
 ================================
 */
template < class T >
void Array< T >::AppendUnique( const T& element ) {
	for ( int i = 0; i < mNumElements; ++i ) {
		if ( element == mArray[ i ] ) {
			// if this element already exists, do not add
			return;
		}
	}
	
	Append( element );
}

/*
 ================================
 Array::AppendArray
 ================================
 */
template < class T >
void Array< T >::AppendArray( const Array< T > & rhs ) {
	for ( int i = 0; i < rhs.Num(); ++i ) {
		Append( rhs[ i ] );
	}
}

/*
 ================================
 Array::Expand
 ================================
 */
template < class T >
void Array< T >::Expand( const int& newSize ) {
	assert( newSize > mSize );
	
	// create a new array
	T* tmpArray = new T[ newSize ];
	
	// copy elements
	for ( int i = 0; i < mNumElements; ++i ) {
		tmpArray[ i ] = mArray[ i ];
	}
	
	// delete old array and set to the new one
	delete[] mArray;
	mArray = tmpArray;
	mSize = newSize;
}

/*
 ================================
 Array::Shrink
 ================================
 */
template < class T >
void Array< T >::Shrink( const int& newSize ) {
	assert( newSize < mSize );
	if ( newSize < 0 ) {
		Empty();
		return;
	}
	
	// create a new array
	T* tmpArray = new T[ newSize ];
	
	// copy elements
//	memcpy( tmpArray, mArray, sizeof( T ) * newSize );
	for ( int i = 0; i < mNumElements; ++i ) {
		tmpArray[ i ] = mArray[ i ];
	}
	
	// delete old array and set to the new one
	delete[] mArray;
	mArray = tmpArray;
	mSize = newSize;
}

/*
 ================================
 Array::Resize
 ================================
 */
template < class T >
void Array< T >::Resize( const int& newSize ) {
	if ( newSize > mSize ) {
		Expand( newSize );
	} else if ( newSize < mSize ) {
		Shrink( newSize );
	}
}

/*
 ================================
 Array::operator[]
 ================================
 */
template < class T >
const T & Array< T >::operator[]( const int idx ) const {
	assert( idx >= 0 ); 
//	assert( idx < mNumElements ); 
    assert( idx < mSize );
	
	return mArray[ idx ];
}

/*
 ================================
 Array::operator[]
 ================================
 */
template < class T >
T & Array< T >::operator[]( const int idx ) {
	assert( idx >= 0 ); 
//	assert( idx < mNumElements ); 
    assert( idx < mSize );
	
	return mArray[ idx ]; 
}

/*
 ================================
 Array::Remove
 ================================
 */
template < class T >
void Array< T >::Remove( const int idx ) {
	assert( idx >= 0 );
	assert( idx < mNumElements );
	
	--mNumElements;
	for ( int i = idx; i < mNumElements; ++i ) {
		mArray[ i ] = mArray[ i + 1 ];
	}
}

/*
 ================================
 Array::RemoveElement
 ================================
 */
template < class T >
bool Array< T >::RemoveElement( const T & element ) {
	for ( int i = 0; i < mNumElements; ++i ) {
		if ( mArray[ i ] == element ) {
			Remove( i );
			return true;
		}
	}
	
	return false;
}

/*
 ================================
 Array::Insert
 ================================
 */
template < class T >
void Array< T >::Insert( const T & element, const int idx ) {
	assert( idx >= 0 );
	assert( idx < mNumElements );
	
	// check if array needs to expand
	if ( mNumElements == mSize ) {
		Expand( mSize + mExpansionSize );
	}
	
	// shift elements down one
	++mNumElements;
	for ( int i = mNumElements; i > idx; --i ) {
		mArray[ i ] = mArray[ i - 1 ];
	}
	
	// insert element
	mArray[ idx ] = element;
}

/*
 ================================
 Array::Find
 * returns the index of the element found at the location
 * returns -1 if the index was not found
 ================================
 */
template < class T >
int Array< T >::Find( const T & element ) const {
	int idx = -1;
	for ( int i = 0; i < mNumElements; ++i ) {
		if ( mArray[ i ] == element ) {
			idx = i;
			break;
		}
	}
	
	return idx;
}

/*
 ================================
 Array::Reverse
 * reverse the ordering of the elements
 * swap the back elements with the front elements
 ================================
 */
template < class T >
void Array< T >::Reverse() {
    int num_1 = mNumElements - 1;
    int half_elements = mNumElements >> 1;
    for ( int i = 0; i < half_elements; ++i ) {
        hbSwap( mArray[ i ], mArray[ num_1 - i ] );
    }
}


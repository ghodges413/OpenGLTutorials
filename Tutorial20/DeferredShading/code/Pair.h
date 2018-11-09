/*
 *  Pair.h
 *
 */
#pragma once

/*
 ================================
 hbPair
 ================================
 */
template < class T, class R >
class hbPair {
public:
	hbPair() {}
    hbPair< T, R >( const hbPair< T, R > & rhs );
    hbPair< T, R >( const T & first, const R & second );
    hbPair< T, R > & operator=( const hbPair< T, R > & rhs );
	~hbPair() {}
	
public:
    T   mFirst;
	R   mSecond;
};

/*
 ================================
 hbPair::hbPair
 ================================
 */
template < class T, class R >
hbPair< T, R >::hbPair( const hbPair< T, R > & rhs ) :
mFirst( rhs.mFirst ),
mSecond( rhs.mSecond ) {
}

/*
 ================================
 hbPair::hbPair
 ================================
 */
template < class T, class R >
hbPair< T, R >::hbPair( const T & first, const R & second ) :
mFirst( first ),
mSecond( second ) {
}

/*
 ================================
 hbPair::operator =
 ================================
 */
template < class T, class R >
hbPair< T, R > & hbPair< T, R >::operator=( const hbPair< T, R > & rhs ) {
    mFirst  = rhs.mFirst;
    mSecond = rhs.mSecond;
    return *this;
}

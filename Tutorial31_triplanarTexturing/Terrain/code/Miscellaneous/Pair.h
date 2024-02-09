//
//  Pair.h
//
#pragma once

/*
 ================================
 Pair
 ================================
 */
template < class T, class R >
class Pair {
public:
	Pair() {}
    Pair< T, R >( const Pair< T, R > & rhs );
    Pair< T, R >( const T & first, const R & second );
    Pair< T, R > & operator=( const Pair< T, R > & rhs );
	~Pair() {}
	
public:
    T   mFirst;
	R   mSecond;
};

/*
 ================================
 Pair::Pair
 ================================
 */
template < class T, class R >
Pair< T, R >::Pair( const Pair< T, R > & rhs ) :
mFirst( rhs.mFirst ),
mSecond( rhs.mSecond ) {
}

/*
 ================================
 Pair::Pair
 ================================
 */
template < class T, class R >
Pair< T, R >::Pair( const T & first, const R & second ) :
mFirst( first ),
mSecond( second ) {
}

/*
 ================================
 Pair::operator =
 ================================
 */
template < class T, class R >
Pair< T, R > & Pair< T, R >::operator=( const Pair< T, R > & rhs ) {
    mFirst  = rhs.mFirst;
    mSecond = rhs.mSecond;
    return *this;
}

//
//	Comparison.h
//
#pragma once

template <class T>
inline void Clamp( T & v, const T & min, const T & max ) {
	if ( v < min ) {
		v = min;
	}
	if ( v > max ) {
		v = max;
	}
}

template <class T>
inline T Min( const T & a, const T & b ) {
	if ( a < b ) {
		return a;
	}
	return b;
}

template <class T>
inline T Max( const T & a, const T & b ) {
	if ( a > b ) {
		return a;
	}
	return b;
}
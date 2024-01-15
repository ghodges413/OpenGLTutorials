//
//  Atomics.h
//
#pragma once

/*
===============================
Atomics
===============================
*/
class Atomics {
public:
	static long Increment( long & value );
	static long Decrement( long & value );

	static long Add( long & value, long i );
	static long Sub( long & value, long i );

	static long Exchange( long & value, long exchange );
	static long CompareExchange( long & value, long compare, long exchange );

	static void * ExchangePointer( void ** ptr, void * exchange );
	static void * CompareExchangePointer( void ** ptr, void * compare, void * exchange );
};
//
//  Atomics.cpp
//
#include "Atomics.h"
#include "Common.h"

#if defined( WINDOWS_THREADS )
/*
===============================
Atomics::Increment
===============================
*/
long Atomics::Increment( long & value ) {
	return InterlockedIncrementAcquire( &value );
}
long Atomics::Decrement( long & value ) {
	return InterlockedDecrementRelease( &value );
}

/*
===============================
Atomics::Add
===============================
*/
long Atomics::Add( long & value, long i ) {
	return InterlockedExchangeAdd( &value, i );
}
long Atomics::Sub( long & value, long i ) {
	return InterlockedExchangeAdd( &value, -i );
}

/*
===============================
Atomics::Exchange
===============================
*/
long Atomics::Exchange( long & value, long exchange ) {
	return InterlockedExchange( &value, exchange );
}
long Atomics::CompareExchange( long & value, long compare, long exchange ) {
	return InterlockedCompareExchange( &value, exchange, compare );
}

/*
===============================
Atomics::ExchangePointer
===============================
*/
void * Atomics::ExchangePointer( void ** ptr, void * exchange ) {
	return InterlockedExchangePointer( ptr, exchange );
}
void * Atomics::CompareExchangePointer( void ** ptr, void * compare, void * exchange ) {
	return InterlockedCompareExchangePointer( ptr, exchange, compare );
}
#endif


#if defined( POSIX_THREADS )
/*
===============================
Atomics::Increment
===============================
*/
long Atomics::Increment( long & value ) {
	return __sync_add_and_fetch( &value, 1 );
}
long Atomics::Decrement( long & value ) {
	return __sync_sub_and_fetch( &value, 1 );
}

/*
===============================
Atomics::Add
===============================
*/
long Atomics::Add( long & value, long i ) {
	return __sync_add_and_fetch( &value, i );
}
long Atomics::Sub( long & value, long i ) {
	return __sync_sub_and_fetch( &value, -i );
}

/*
===============================
Atomics::Exchange
===============================
*/
long Atomics::Exchange( long & value, long exchange ) {
	return __sync_lock_test_and_set( &value, exchange );
}
long Atomics::CompareExchange( long & value, long compare, long exchange ) {
	return __sync_val_compare_and_swap( &value, exchange, compare );
}

/*
===============================
Atomics::ExchangePointer
===============================
*/
void * Atomics::ExchangePointer( void ** ptr, void * exchange ) {
	return __sync_lock_test_and_set( &ptr, exchange );
}
void * Atomics::CompareExchangePointer( void ** ptr, void * compare, void * exchange ) {
	return __sync_val_compare_and_swap( &ptr, exchange, compare );
}
#endif


#if defined( STD_THREADS )
/*
===============================
Atomics::Increment
===============================
*/
long Atomics::Increment( long & value ) {
	return std::atomic_fetch_add( value, 1 );
}
long Atomics::Decrement( long & value ) {
	return std::atomic_fetch_sub( value, 1 );
}

/*
===============================
Atomics::Add
===============================
*/
long Atomics::Add( long & value, long i ) {
	return std::atomic_fetch_add( value, i );
}
long Atomics::Sub( long & value, long i ) {
	return std::atomic_fetch_sub( value, i );
}

/*
===============================
Atomics::Exchange
===============================
*/
long Atomics::Exchange( long & value, long exchange ) {
	return std::atomic_exchange( &value, exchange );
}
long Atomics::CompareExchange( long & value, long compare, long exchange ) {
	return std::atomic_compare_exchange_weak( &value, exchange, compare );
}

/*
===============================
Atomics::ExchangePointer
===============================
*/
void * Atomics::ExchangePointer( void ** ptr, void * exchange ) {
	return std::atomic_exchange( &ptr, exchange );
}
void * Atomics::CompareExchangePointer( void ** ptr, void * compare, void * exchange ) {
	return std::atomic_compare_exchange_weak( &ptr, exchange, compare );
}
#endif
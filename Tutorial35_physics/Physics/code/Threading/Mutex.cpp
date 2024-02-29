//
//  Mutex.cpp
//
#include "Threading/Mutex.h"
#include <stdio.h>

/*
====================================================
Mutex::Mutex
====================================================
*/
Mutex::Mutex() {
#if defined( WINDOWS_THREADS )
	//m_mutex = CreateMutex( NULL, FALSE, L"ALGORITHMS" );
	m_mutex = CreateMutex( NULL, FALSE, "ALGORITHMS" );
#endif

#if defined( POSIX_THREADS )
	pthread_mutex_init( &m_mutex, NULL );
	//m_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif
}

/*
====================================================
Mutex::~Mutex
====================================================
*/
Mutex::~Mutex() {
#if defined( WINDOWS_THREADS )
	CloseHandle( m_mutex );
#endif

#if defined( POSIX_THREADS )
	pthread_mutex_destroy( &m_mutex );
#endif;
}

/*
====================================================
Mutex::Mutex
====================================================
*/
void Mutex::Lock() {
#if defined( WINDOWS_THREADS )
	WaitForSingleObject( m_mutex, INFINITE );
#endif

#if defined( POSIX_THREADS )
	pthread_mutex_lock( &m_mutex );
#endif

#if defined( STD_THREADS )
	m_mutex.lock();
#endif

//	printf( "Lock acquired\n" );
}

/*
====================================================
Mutex::Unlock
====================================================
*/
void Mutex::Unlock() {
#if defined( WINDOWS_THREADS )
	ReleaseMutex( m_mutex );
#endif

#if defined( POSIX_THREADS )
	pthread_mutex_unlock( &m_mutex );
#endif

#if defined( STD_THREADS )
	m_mutex.unlock();
#endif

//	printf( "Lock released\n" );
}

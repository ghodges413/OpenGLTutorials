//
//  Threads.cpp
//
#include "Threading/Threads.h"

/*
===============================
Thread::Create
===============================
*/
bool Thread::Create( ThreadWorkFunctor_t * functor, void * data ) {
#if defined( WINDOWS_THREADS )
// 	m_threadHandle = CreateThread( 
// 		NULL,						// default security attributes
// 		0,							// use default stack size  
// 		Thread::Main,				// thread function name
// 		this,						// argument to thread function 
// 		0,							// use default creation flags 
// 		&m_dwThreadID );			// returns the thread identifier
	m_threadHandle = CreateThread( 
		NULL,						// default security attributes
		0,							// use default stack size  
		functor,					// thread function name
		data,						// argument to thread function 
		0,							// use default creation flags 
		&m_dwThreadID );			// returns the thread identifier
#endif

#if defined( POSIX_THREADS )
	//m_returnCode = pthread_create( &m_threadHandle, NULL, Thread::Main, (void *)this );
	m_returnCode = pthread_create( &m_threadHandle, NULL, functor, data );
#endif

#if defined( STD_THREADS )
	//m_threadHandle = new std::thread( &Thread::Main, this );
	m_threadHandle = new std::thread( functor, data );
#endif

	return true;
}

/*
===============================
Thread::Join
===============================
*/
void Thread::Join() {
#if defined( WINDOWS_THREADS )
	// Wait until this thread has terminated.
	WaitForMultipleObjects( 1, &m_threadHandle, TRUE, INFINITE );
	CloseHandle( m_threadHandle );
#endif

#if defined( POSIX_THREADS )
	m_returnCode = pthread_join( m_threadHandle, NULL );
#endif

#if defined( STD_THREADS )
	m_threadHandle->join();
	delete m_threadHandle;
	m_threadHandle = NULL;
#endif
}

/*
===============================
Thread::YieldThread
===============================
*/
void Thread::YieldThread() {
#if defined( WINDOWS_THREADS )
	Yield();
#endif

#if defined( POSIX_THREADS )
	pthread_yield();
#endif

#if defined( STD_THREADS )
	std::this_thread::yield();
#endif
}

/*
===============================
Thread::NumHardwareThreads
===============================
*/
unsigned int Thread::NumHardwareThreads() {
	unsigned int numThreads = 1;

#if defined( WINDOWS_THREADS )
	SYSTEM_INFO sysinfo;
	GetSystemInfo( &sysinfo );
	numThreads = sysinfo.dwNumberOfProcessors;
#endif

#if defined( POSIX_THREADS )
//	numThreads = sysconf(_SC_NPROCESSORS_ONLN); // linux

	// osx
	{
		int mib[ 4 ];
		std::size_t len = sizeof( numThreads ); 

		mib[ 0 ] = CTL_HW;
		mib[ 1 ] = HW_AVAILCPU;

		sysctl( mib, 2, &numThreads, &len, NULL, 0 );

		if ( numThreads < 1 ) {
			mib[ 1 ] = HW_NCPU;
			sysctl( mib, 2, &numThreads, &len, NULL, 0 );
			if ( numThreads < 1 ) {
				numThreads = 1;
			}
		}
	}
#endif

#if defined( STD_THREADS )
	numThreads = std::thread::hardware_concurrency();
#endif

	return numThreads;
}
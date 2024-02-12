//
//  Mutex.h
//
#pragma once
#include "Threading/Common.h"

/*
===============================
Mutex
===============================
*/
class Mutex {
public:
	Mutex();
	~Mutex();

	void Lock();
	void Unlock();

private:
#if defined( WINDOWS_THREADS )
	HANDLE m_mutex;
#endif

#if defined( POSIX_THREADS )
	pthread_mutex_t m_mutex;
#endif

#if defined( STD_THREADS )
	std::mutex m_mutex;
#endif
};
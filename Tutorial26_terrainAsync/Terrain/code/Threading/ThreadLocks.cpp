//
//  ThreadLocks.cpp
//
#include "ThreadLocks.h"
#include "Mutex.h"

/*
===============================
ScopedLock::ScopedLock
===============================
*/
ScopedLock::ScopedLock( Mutex & mutex ) {
	m_mutexPtr = &mutex;
	m_mutexPtr->Lock();
}

/*
===============================
ScopedLock::~ScopedLock
===============================
*/
ScopedLock::~ScopedLock() {
	m_mutexPtr->Unlock();
}

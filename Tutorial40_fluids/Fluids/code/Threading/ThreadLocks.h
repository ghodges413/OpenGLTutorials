//
//  ThreadLocks.h
//
#pragma once
class Mutex;

/*
====================================================
ScopedLock
====================================================
*/
class ScopedLock {
private:
	ScopedLock();
	ScopedLock( const ScopedLock & rhs );
	ScopedLock & operator = ( const ScopedLock & rhs );

public:
	explicit ScopedLock( Mutex & mutex );
	~ScopedLock();	

private:
	Mutex * m_mutexPtr;
};
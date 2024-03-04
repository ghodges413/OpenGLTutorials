//
//  JobQueues.cpp
//
#include "JobSystem/JobQueues.h"
#include "Threading/ThreadLocks.h"
#include "Threading/Common.h"

/*
========================================================================================================

WorkStealingQueue

========================================================================================================
*/

/*
====================================================
WorkStealingQueue::Push
====================================================
*/
void WorkStealingQueue::Push( Job_t * job ) {
	ScopedLock lock( m_queueMutex );

	m_jobs[ m_bottom & ( JobQueue::MAX_JOBS - 1 ) ] = job;
	++m_bottom;
}

/*
====================================================
WorkStealingQueue::Pop
====================================================
*/
Job_t * WorkStealingQueue::Pop() {
	ScopedLock lock( m_queueMutex );

	const int jobCount = m_bottom - m_top;
	if ( jobCount <= 0 ) {
		return NULL;
	}

	--m_bottom;
	return m_jobs[ m_bottom & ( JobQueue::MAX_JOBS - 1 ) ];
}

/*
====================================================
WorkStealingQueue::Steal
====================================================
*/
Job_t * WorkStealingQueue::Steal() {
	ScopedLock lock( m_queueMutex );

	const int jobCount = m_bottom - m_top;
	if ( jobCount <= 0 ) {
		return NULL;
	}

	Job_t * job = m_jobs[ m_top & ( JobQueue::MAX_JOBS - 1 ) ];
	++m_top;
	return job;
}

/*
========================================================================================================

LocklessQueue

========================================================================================================
*/

/*
====================================================
LocklessQueue::Push
====================================================
*/
void LocklessQueue::Push( Job_t * job ) {
	long b = m_bottom;
	m_jobs[ b & ( JobQueue::MAX_JOBS - 1 ) ] = job;

	// ensure the job is written before b+1 is published to other threads.
	// on x86/64, a compiler barrier is enough.
	COMPILER_BARRIER();

	m_bottom = b + 1;
}

/*
====================================================
LocklessQueue::Pop
====================================================
*/
Job_t * LocklessQueue::Pop() {
#if 1
	long b = m_bottom - 1;
	_InterlockedExchange( &m_bottom, b );

	long t = m_top;
#elif 0
	long b = m_bottom - 1;
	m_bottom = b;

	MEMORY_BARRIER();

	long t = m_top;
#else
	long b = m_bottom - 1;
	m_bottom = b;

	long t = m_top;
#endif

	if ( t > b ) {
		// deque was already empty
		m_bottom = t;
		return NULL;
	}

	// non-empty queue
	Job_t * job = m_jobs[ b & ( JobQueue::MAX_JOBS - 1 ) ];
	if ( t != b ) {
		// there's still more than one item left in the queue
		return job;
	}

	// this is the last item in the queue
	if ( _InterlockedCompareExchange( &m_top, t + 1, t ) != t ) {
		// failed race against steal operation
		job = NULL;
	}

	m_bottom = t + 1;
	return job;
}

/*
====================================================
LocklessQueue::Steal
====================================================
*/
Job_t * LocklessQueue::Steal() {
	long t = m_top;

	// ensure that top is always read before bottom.
	// loads will not be reordered with other loads on x86, so a compiler barrier is enough.
	COMPILER_BARRIER();

	long b = m_bottom;

	// Check if the queue is empty
	if ( t >= b ) {
		return NULL;
	}

	// non-empty queue
	Job_t * job = m_jobs[ t & ( JobQueue::MAX_JOBS - 1 ) ];

	// the interlocked function serves as a compiler barrier, and guarantees that the read happens before the CAS.
	if ( _InterlockedCompareExchange( &m_top, t + 1, t ) != t ) {
		// a concurrent steal or pop operation removed an element from the deque in the meantime.
		return NULL;
	}

	return job;
}
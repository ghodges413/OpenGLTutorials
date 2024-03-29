//
//  JobAllocators.cpp
//
#include "JobSystem/JobSystem.h"
#include "JobSystem/JobAllocators.h"
#include "Threading/Atomics.h"

/*
========================================================================================================
JobAllocators

These job allocators are just pre-allocated ring buffers.
TODO: check that we don't over-allocate jobs, ie the next slot to be allocated
========================================================================================================
*/
Job_t GlobalSharedJobAllocator::s_jobAllocator[ GlobalSharedJobAllocator::MAX_JOBS ];
long GlobalSharedJobAllocator::s_allocatedJobs = 0;


/*
====================================================
GlobalSharedJobAllocator::AllocateJob
====================================================
*/
Job_t * GlobalSharedJobAllocator::AllocateJob() {
	const long index = Atomics::Increment( s_allocatedJobs ) - 1;
	return &s_jobAllocator[ index & ( MAX_JOBS - 1 ) ];
}

/*
====================================================
ThreadLocalJobAllocator::AllocateJob
====================================================
*/
Job_t * ThreadLocalJobAllocator::AllocateJob() {
	const unsigned int index = m_allocatedJobs++;
	return &m_jobAllocator[ index & ( MAX_JOBS - 1 ) ];
}

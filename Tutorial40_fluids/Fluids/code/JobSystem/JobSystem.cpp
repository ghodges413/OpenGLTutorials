//
//  JobSystem.cpp
//
#include "JobSystem/JobSystem.h"
#include "JobSystem/JobQueues.h"
#include "JobSystem/JobAllocators.h"
#include "Threading/Threads.h"
#include "JobSystem/JobThread.h"
#include "Threading/Atomics.h"
#include <stdio.h>

JobSystem * g_jobSystem = NULL;

/*
====================================================
JobSystem::JobSystem
====================================================
*/
JobSystem::JobSystem() {
	m_numThreads = Thread::NumHardwareThreads();

	printf( "Num Job System Threads: %i\n", m_numThreads );

	m_queue = new WorkStealingQueue;
	m_threads = new JobThread[ m_numThreads ];

	m_numJobsUnfinished = 0;
}

/*
====================================================
JobSystem::~JobSystem
====================================================
*/
JobSystem::~JobSystem() {
	delete[] m_threads;
	m_threads = NULL;
}

/*
====================================================
JobSystem::CreateJob
====================================================
*/
Job_t * JobSystem::CreateJob( JobFunction_t * functor ) {
	Job_t * job = GlobalSharedJobAllocator::AllocateJob();

	job->m_functor = functor;
	job->m_parent = NULL;
	job->m_unfinishedJobs = 1;

	return job;
}

/*
====================================================
JobSystem::CreateJobAsChild
====================================================
*/
Job_t * JobSystem::CreateJobAsChild( Job_t * parent, JobFunction_t * functor ) {
	Atomics::Increment( parent->m_unfinishedJobs );

	Job_t * job = GlobalSharedJobAllocator::AllocateJob();
	job->m_functor = functor;
	job->m_parent = parent;
	job->m_unfinishedJobs = 1;

	return job;
}

/*
====================================================
JobSystem::Run
====================================================
*/
void JobSystem::Run( Job_t * job ) {
	m_queue->Push( job );
}

/*
====================================================
JobSystem::Wait
====================================================
*/
void JobSystem::Wait( const Job_t * job ) {
	if ( NULL == job ) {
		// Wait until all jobs are complete
		while ( m_queue->Num() > 0 ) {
			Thread::YieldThread();
		}
		
		// We still need to wait until all jobs are completed.
		while ( m_numJobsUnfinished > 0 ) {
			Thread::YieldThread();
		}
		return;
	}

	// wait until the job has completed. in the meantime, work on any other job.
	while ( !HasJobCompleted( job ) ) {
		Thread::YieldThread();
	}
}

/*
====================================================
JobSystem::AddJob
====================================================
*/
void JobSystem::AddJoby( JobFunction_t * functor, void * data ) {
	Job_t * job = CreateJob( functor );
	job->m_data = data;
	job->m_numElements = 1;
	m_queue->Push( job );
	Atomics::Increment( m_numJobsUnfinished );
}

/*
====================================================
JobSystem::ParallelFor
====================================================
*/
void JobSystem::ParallelFor( JobFunction_t * functor, void * data, const int elementSize, const int numElements, int groupSize ) {
	// Determine the group size and number of groups
	int remainder = 0;
	int numGroups = m_numThreads;
	if ( groupSize <= 0 ) {
		groupSize = numElements / m_numThreads;
		remainder = ( numElements % numGroups );
	} else {
		numGroups = numElements / groupSize;
		remainder = ( numElements % groupSize );
	}

	for ( int i = 0; i < numGroups; i++ ) {
		Job_t * job = CreateJob( functor );
		job->m_data = (void*)(((char*)data) + i * groupSize * elementSize);
		job->m_numElements = groupSize;
		m_queue->Push( job );
		Atomics::Increment( m_numJobsUnfinished );
	}

	//const int remainder = ( numElements % groupSize );
	if ( remainder > 0 ) {
		Job_t * job = CreateJob( functor );
		job->m_data = (void*)(((char*)data) + numGroups * groupSize * elementSize);
		job->m_numElements = remainder;
		m_queue->Push( job );
		Atomics::Increment( m_numJobsUnfinished );
	}
}

/*
========================================================================================================

TestJobSystem

========================================================================================================
*/
/*
====================================================
TestParallelForJob
====================================================
*/
void TestParallelForJob( Job_t * job, void * data ) {
	int * dataInt = (int *)job->m_data;

	for ( int i = 0; i < job->m_numElements; i++ ) {
		for ( int j = 0; j < i; j++ ) {
			dataInt[ i ]++;
		}
	}
}

/*
====================================================
TestJobSystem
====================================================
*/
void TestJobSystem() {
	if ( NULL == g_jobSystem ) {
		g_jobSystem = new JobSystem;
	}

	const int numElements = 1024;
	int testData[ numElements ] { 0 };

	g_jobSystem->ParallelFor( TestParallelForJob, testData, sizeof( int ), numElements );
	g_jobSystem->Wait( NULL );

	for ( int i = 0; i < numElements; i++ ) {
		printf( "%i: %i\n", i, testData[ i ] );
	}
}




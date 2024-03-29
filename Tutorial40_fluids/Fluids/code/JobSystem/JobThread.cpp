//
//  JobThread.cpp
//
#include "JobSystem/JobThread.h"
#include "JobSystem/JobSystem.h"
#include "JobSystem/JobQueues.h"
#include "Threading/Atomics.h"

/*
====================================================
JobThread::JobThread
====================================================
*/
JobThread::JobThread() {
//	m_jobQueue = NULL;
	m_jobQueue = new WorkStealingQueue();

	// Activate this thread so it doesn't stop when we create it
	m_workerThreadActive = true;

	// Create the thread and enter the main loop for this thread
	m_thread.Create( JobThread::Main, this );
}

/*
====================================================
JobThread::~JobThread
====================================================
*/
JobThread::~JobThread() {
	// Deactivate the thread so the thread's main loop quits
	m_workerThreadActive = false;

	// Wait until the thread exits
	m_thread.Join();

	delete m_jobQueue;
	m_jobQueue = NULL;
}

/*
====================================================
JobThread::Main
====================================================
*/
ThreadReturnType_t JobThread::Main( ThreadInputType_t data ) {
	JobThread * jobThread = (JobThread *)data;

	// Loop forever and execute jobs as they become available
	while ( jobThread->m_workerThreadActive ) {
		Job_t * job = jobThread->GetJob();
		if ( NULL != job ) {
			jobThread->Execute( job );

			// Let the job system know this job is finished
			Atomics::Decrement( g_jobSystem->m_numJobsUnfinished );
		}
	}

	return NULL;
}

/*
====================================================
JobThread::GetJob
====================================================
*/
Job_t * JobThread::GetJob() {
	JobQueue * queue = GetJobQueue();
	if ( NULL == queue ) {
		return NULL;
	}

	Job_t * job = queue->Pop();
	if ( JobSystem::IsValidJob( job ) ) {
		return job;
	}

	return NULL;
}

/*
====================================================
JobThread::Execute
====================================================
*/
void JobThread::Execute( Job_t * job ) {
	job->m_functor( job, job->m_data );
	Finish( job );
}

/*
====================================================
JobThread::GetJobQueue
====================================================
*/
JobQueue * JobThread::GetJobQueue() {
	if ( NULL == g_jobSystem ) {
		return NULL;
	}
	if ( NULL != g_jobSystem->m_queue ) {
		return g_jobSystem->m_queue;
	}
	return m_jobQueue;
}

/*
====================================================
JobThread::Finish
Use this one in combination with the custom job allocators
====================================================
*/
void JobThread::Finish( Job_t * job ) {
	const long unfinishedJobs = Atomics::Decrement( job->m_unfinishedJobs );

	if ( 0 == unfinishedJobs ) {
		if ( NULL != job->m_parent ) {
			Finish( job->m_parent );
		}
	}
}
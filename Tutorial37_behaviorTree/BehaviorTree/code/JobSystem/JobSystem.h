//
//  JobSystem.h
//
#pragma once
#include <stddef.h>

/*
====================================================
JobSystem
====================================================
*/
struct Job_t;

typedef void JobFunction_t( Job_t * job, void * data );

struct Job_t {
	JobFunction_t * m_functor;
	void * m_data;
	int m_numElements;

	Job_t * m_parent;
	long m_unfinishedJobs;
};

class JobSystem {
public:
	JobSystem();
	~JobSystem();

	Job_t * CreateJob( JobFunction_t * functor );
	Job_t * CreateJobAsChild( Job_t * parent, JobFunction_t * functor );
	
	void Run( Job_t * job );
	void Wait( const Job_t * job );

	void AddJoby( JobFunction_t * functor, void * data );
	void ParallelFor( JobFunction_t * functor, void * data, const int elementSize, const int numElements, int groupSize = -1 );

	static bool HasJobCompleted( const Job_t * job ) { return ( job->m_unfinishedJobs <= 0 ); }
	static bool IsEmptyJob( const Job_t * job ) {
		return ( ( NULL == job ) || job->m_unfinishedJobs <= 0 );
	}
	static bool IsValidJob( const Job_t * job ) {
		return ( ( NULL != job ) && ( job->m_unfinishedJobs > 0 ) );
	}

	friend class JobThread;
private:
	unsigned int m_numThreads;

	class JobQueue *	m_queue;
	class JobThread *	m_threads;

	long m_numJobsUnfinished;
};

extern JobSystem * g_jobSystem;


void TestJobSystem();
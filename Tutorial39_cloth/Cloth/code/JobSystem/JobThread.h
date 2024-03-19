//
//  JobSystem.h
//
//
#pragma once
#include "Threading/Threads.h"

/*
====================================================
JobThread
====================================================
*/
struct Job_t;
class JobQueue;

class JobThread {
public:
	JobThread();
	~JobThread();

	static ThreadReturnType_t Main( ThreadInputType_t data );

private:
	Job_t * GetJob();
	JobQueue * GetJobQueue();

	static void Execute( Job_t * job );
	static void Finish( Job_t * job );

private:
	Thread m_thread;
	bool m_workerThreadActive;

	JobQueue * m_jobQueue;
};
 
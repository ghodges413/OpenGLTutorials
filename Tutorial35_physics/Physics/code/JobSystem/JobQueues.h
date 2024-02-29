//
//  JobQueues.h
//
#pragma once
#include "Threading/Mutex.h"

/*
========================================================================================================

These job queues are ring buffers

========================================================================================================
*/

struct Job_t;

/*
====================================================
JobQueue
====================================================
*/
class JobQueue {
public:
	virtual void Push( Job_t * job ) = 0;
	virtual Job_t * Pop() = 0;
	virtual Job_t * Steal() = 0;
	virtual int Num() const = 0;

	static const unsigned int MAX_JOBS = 128;
};

/*
====================================================
WorkStealingQueue
====================================================
*/
class WorkStealingQueue : public JobQueue {
public:
	WorkStealingQueue() : m_bottom( 0 ), m_top( 0 ) {}

	void Push( Job_t * job ) override;
	Job_t * Pop() override;
	Job_t * Steal() override;
	int Num() const override { return ( m_top - m_bottom ); }

private:
	long m_bottom;
	long m_top;

	Job_t * m_jobs[ JobQueue::MAX_JOBS ];

	Mutex m_queueMutex;
};

/*
====================================================
LocklessQueue
====================================================
*/
class LocklessQueue : public JobQueue {
public:
	LocklessQueue() : m_bottom( 0 ), m_top( 0 ) {}

	void Push( Job_t * job ) override;
	Job_t * Pop() override;
	Job_t * Steal() override;
	int Num() const override { return ( m_top - m_bottom ); }

private:
	long m_bottom;
	long m_top;

	Job_t * m_jobs[ JobQueue::MAX_JOBS ];
};
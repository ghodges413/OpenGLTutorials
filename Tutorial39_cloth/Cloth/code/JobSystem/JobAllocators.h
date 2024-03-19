//
//  JobAllocators.h
//
#pragma once

/*
========================================================================================================

These job allocators are ring buffers

========================================================================================================
*/

struct Job_t;

/*
====================================================
GlobalSharedJobAllocator
====================================================
*/
class GlobalSharedJobAllocator {
public:
	static Job_t * AllocateJob();

private:
	static const unsigned int MAX_JOBS = 1024;
	static Job_t s_jobAllocator[ MAX_JOBS ];
	static long s_allocatedJobs;
};

/*
====================================================
ThreadLocalJobAllocator
====================================================
*/
class ThreadLocalJobAllocator {
public:
	ThreadLocalJobAllocator() : m_allocatedJobs( 0 ) {}

	Job_t * AllocateJob();

private:
	static const unsigned int MAX_JOBS = 1024;
	Job_t m_jobAllocator[ MAX_JOBS ];
	unsigned int m_allocatedJobs;
};
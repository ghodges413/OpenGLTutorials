//
//  Common.h
//
#pragma once

#define WINDOWS_THREADS
//#define POSIX_THREADS
//#define STD_THREADS

#if defined( WINDOWS_THREADS )
	#include <Windows.h>
	#define ThreadReturnType_t DWORD WINAPI
	//typedef DWORD WINAPI ThreadReturnType_t;
	typedef LPVOID ThreadInputType_t;
#endif

#if defined( POSIX_THREADS )
	#include <pthread.h>
	typedef void * ThreadReturnType_t;
	typedef void * ThreadInputType_t;
#endif

#if defined( STD_THREADS )
	#include <thread>
	#include <mutex>
	#include <atomic>
#endif

/*
===============================
Barriers
===============================
*/

// _ReadWriteBarrier is a windows msvc compiler barrier
// Compiler barriers prevent the compiler from re-ordering the output assembly instructions
// MemoryBarrier adds an instruction to prevent the cpu from re-ordering the execution of instructions
#define MEMORY_BARRIER() MemoryBarrier()

#if defined( WINDOWS_THREADS )
	//#pragma intrinsic( _ReadWriteBarrier )
	#define COMPILER_BARRIER() _ReadWriteBarrier()
#else
	// This is the gcc version of a compiler barrier
	#define COMPILER_BARRIER() asm volatile("" ::: "memory")
#endif
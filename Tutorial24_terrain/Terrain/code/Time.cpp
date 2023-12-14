/*
 *  Time.cpp
 *
 */

#include "Time.h"
#ifdef WINDOWS
#include <time.h>
#include <windows.h>
#else
#include <sys/time.h>
#endif

#ifndef WINDOWS
static int gStartSeconds( 0 );

/*
 ====================================
 GetTimeMicroseconds
 ====================================
 */
int GetTimeMicroseconds() {
	struct timeval tp;
	
	gettimeofday( &tp, NULL );
	if ( gStartSeconds == 0 ) {
		gStartSeconds = tp.tv_sec;
		return tp.tv_usec;
	}
	return ( ( tp.tv_sec - gStartSeconds ) * 1000000 + tp.tv_usec );
}

/*
 ====================================
 GetTimeMilliseconds
 ====================================
 */
int GetTimeMilliseconds() {
	struct timeval tp;
	
	gettimeofday( &tp, NULL );
	if ( gStartSeconds == 0 ) {
		gStartSeconds = tp.tv_sec;
		return tp.tv_usec;
	}
	return ( ( tp.tv_sec - gStartSeconds ) * 1000 + tp.tv_usec / 1000 );
}

/*
 ====================================
 GetTimeSeconds
 ====================================
 */
int GetTimeSeconds() {
	struct timeval tp;
	
	gettimeofday( &tp, NULL );
	if ( gStartSeconds == 0 ) {
		gStartSeconds = tp.tv_sec;
		return 0;
	}
	return ( ( tp.tv_sec - gStartSeconds ) );
}
#endif

#ifdef WINDOWS
static bool gIsInitialized( false );
static unsigned __int64 gTicksPerSecond;
static unsigned __int64 gStartTicks;

/*
 ====================================
 GetTimeSeconds
 ====================================
 */
int GetTimeMicroseconds() {
	if ( false == gIsInitialized ) {
		gIsInitialized = true;

		// Get the high frequency counter's resolution
		QueryPerformanceFrequency( (LARGE_INTEGER *)&gTicksPerSecond );

		// Get the current time
		QueryPerformanceCounter( (LARGE_INTEGER *)&gStartTicks );

		return 0;
	}
	
	unsigned __int64 tick;
	QueryPerformanceCounter( (LARGE_INTEGER *)&tick );
	
	const double ticks_per_micro = (double)( gTicksPerSecond / 1000000 );

	const unsigned __int64 timeMicro = ( tick - gStartTicks ) / ticks_per_micro;
	return (int)timeMicro;
}

/*
 ====================================
 GetTimeMilliseconds
 ====================================
 */
int GetTimeMilliseconds() {
	if ( false == gIsInitialized ) {
		gIsInitialized = true;

		// Get the high frequency counter's resolution
		QueryPerformanceFrequency( (LARGE_INTEGER *)&gTicksPerSecond );

		// Get the current time
		QueryPerformanceCounter( (LARGE_INTEGER *)&gStartTicks );

		return 0;
	}
	
	unsigned __int64 tick;
	QueryPerformanceCounter( (LARGE_INTEGER *)&tick );
	
	const double ticks_per_milli = (double)( gTicksPerSecond / 1000 );

	const unsigned __int64 timeMilli = ( tick - gStartTicks ) / ticks_per_milli;
	return (int)timeMilli;
}

/*
 ====================================
 GetTimeSeconds
 ====================================
 */
int GetTimeSeconds() {
	if ( false == gIsInitialized ) {
		gIsInitialized = true;

		// Get the high frequency counter's resolution
		QueryPerformanceFrequency( (LARGE_INTEGER *)&gTicksPerSecond );

		// Get the current time
		QueryPerformanceCounter( (LARGE_INTEGER *)&gStartTicks );

		return 0;
	}
	
	unsigned __int64 tick;
	QueryPerformanceCounter( (LARGE_INTEGER *)&tick );
	
	const double ticks_per_second = (double)( gTicksPerSecond );

	const unsigned __int64 timeSeconds = ( tick - gStartTicks ) / ticks_per_second;
	return (int)timeSeconds;
}
#endif

#include "core.h"
#include "exceptionhandler.h"
#include "exceptiontrace.h"
#include "exceptionuploadlog.h"
#include "allocator.h"

#include <Windows.h>

#if BUILD_SDL
#include "SDL2/SDL.h"
#endif

// I always want to get crash logs in final. Not necessarily for dev.
//#define UPLOADLOG ( 1 || BUILD_FINAL )
#define UPLOADLOG 0	// Disabling for final Eldritch release because its usefulness has run its course.

static bool		gIsEnabled	= false;

LONG WINAPI CustomUnhandledExceptionFilter( EXCEPTION_POINTERS* pExceptionPointers )
{
	Unused( pExceptionPointers );

	ASSERT( gIsEnabled );

	// In case the exception was due to game being OOM.
	Allocator::GetDefault().Enable( false );

	PRINTF( "Exception handled:\n" );
	EXCEPTION_RECORD* pExceptionRecord = pExceptionPointers->ExceptionRecord;
	const DWORD FirstExceptionCode = pExceptionRecord ? pExceptionRecord->ExceptionCode : 0;
	Unused( FirstExceptionCode );

	while( pExceptionRecord )
	{
		PRINTF( "\tCode: 0x%08X\n", pExceptionRecord->ExceptionCode );
		PRINTF( "\tAddr: 0x%08X\n", pExceptionRecord->ExceptionAddress );
		pExceptionRecord = pExceptionRecord->ExceptionRecord;
	}

	ExceptionTrace::PrintTrace();

#if UPLOADLOG
	static const DWORD kBreakpointCode = 0x80000003;
	if( FirstExceptionCode == kBreakpointCode )
	{
		// Don't upload logs from asserts in dev mode.
	}
	else
	{
		ExceptionUploadLog::UploadLog();
	}
#endif

#if BUILD_SDL
	SDL_Quit();
#endif

	return EXCEPTION_CONTINUE_SEARCH;
}

void ExceptionHandler::Enable()
{
	ASSERT( !gIsEnabled );

	ExceptionTrace::Enable();

	gIsEnabled	= true;
	SetUnhandledExceptionFilter( CustomUnhandledExceptionFilter );
}

void ExceptionHandler::ShutDown()
{
	ASSERT( gIsEnabled );

	gIsEnabled	= false;
	SetUnhandledExceptionFilter( NULL );

	ExceptionTrace::ShutDown();
}
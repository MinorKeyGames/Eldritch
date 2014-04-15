#ifndef CORE_H
#define CORE_H

#include "versions.h"

#if BUILD_WINDOWS
// Requires Windows XP, needed for getting the console window, using SwitchToThread, etc.
#define _WIN32_WINNT 0x0501
#endif

#if BUILD_WINDOWS
#define FUNCTION_NAME __FUNCTION__
#else
#define FUNCTION_NAME "unknown function"	// __func__ exists but is a static variable instead of a string literal, so usage will need to change.
#endif

#if BUILD_WINDOWS
#define VSPRINTF_COUNT( fmt, args )		_vscprintf( fmt, args )
#define VSPRINTF( buf, len, fmt, args )	vsprintf_s( buf, len, fmt, args )
#define FOPEN( file, path, mode )		fopen_s( &file, path, mode )
#else
#define VSPRINTF_COUNT( fmt, args )		vsnprintf( NULL, 0, fmt, args )
#define VSPRINTF( buf, len, fmt, args )	vsnprintf( buf, len, fmt, args )
#define FOPEN( file, path, mode )		file = fopen( path, mode )
#endif

#if !BUILD_WINDOWS
#define memcpy_s( dst, size, src, count )	memcpy( dst, src, count );	Unused( size )
#define strcpy_s( dst, size, src )			strcpy( dst, src );			Unused( size )
#endif

#if BUILD_LINUX
// Used for raise( SIGINT ), which wasn't even working for me. :p
#include <csignal>
#endif

// This should be the first file included in every .cpp (even before that
// file's matching header).
// This should be kept a very lightweight include that can go anywhere.

#if BUILD_WINDOWS
#pragma warning( disable: 4127 )	// conditional expression is constant (thrown by while(0))
#pragma warning( disable: 4201 )	// nonstandard extension used : nameless struct/union
#pragma warning( disable: 4351 )	// new behavior: elements of array '...' will be default initialized
#endif

#ifdef _DEBUG
#define BUILD_DEBUG		1	// Defined in Debug only
#define BUILD_RELEASE	0	// Defined in Release and Final (same as !BUILD_DEBUG)
#else
#define BUILD_DEBUG		0	// Defined in Debug only
#define BUILD_RELEASE	1	// Defined in Release and Final (same as !BUILD_DEBUG)
#endif

#ifdef _FINAL
#define BUILD_FINAL		1	// Defined in Final only
#define BUILD_DEV		0	// Defined in Debug and Release (same as !BUILD_FINAL)
#else
#define BUILD_FINAL		0	// Defined in Final only
#define BUILD_DEV		1	// Defined in Debug and Release (same as !BUILD_FINAL)
#endif

typedef unsigned int	uint;
typedef unsigned long	uint32;
typedef unsigned short	uint16;
typedef unsigned char	uint8;
typedef long			int32;
typedef short			int16;
typedef char			int8;
typedef unsigned char	byte;

typedef unsigned long	u32;
typedef unsigned short	u16;
typedef unsigned char	u8;
typedef long			s32;
typedef short			s16;
typedef char			s8;

#define NULL 0

// See http://kernelnewbies.org/FAQ/DoWhile0 for the reason behind this syntax
#define SafeDelete( ptr )		do { if( ( ptr ) ) { delete ( ptr ); ( ptr ) = NULL; } } while(0)
#define SafeDeleteNoNull( ptr )	do { if( ( ptr ) ) { delete ( ptr ); } } while(0)
#define SafeDeleteArray( ptr )	do { if( ( ptr ) ) { delete[] ( ptr ); ( ptr ) = NULL; } } while(0)
#define SafeRelease( ptr )		do { if( ( ptr ) ) { ( ptr )->Release(); ( ptr ) = NULL; } } while(0) // For refcounting interfaces
#define Unused( exp )			do { ( void )sizeof( ( exp ) ); } while(0)
#define DoExp( exp )			do { ( exp ); } while(0)
#define DoNothing				do { ( void )0; } while(0)

#if BUILD_DEV
	#if BUILD_WINDOWS
		#define BREAKPOINT		DoExp( __debugbreak() )
	#elif BUILD_LINUX
		#define BREAKPOINT		raise( SIGINT )
	#else
		// TODO PORT LATER: Support breakpoints on other platforms
		#define BREAKPOINT		DoNothing
	#endif
#else
	#define BREAKPOINT		DoNothing
#endif

#if BUILD_DEBUG
#define DEBUGBREAKPOINT	BREAKPOINT
#else
#define DEBUGBREAKPOINT	DoNothing
#endif

// Other includes should go at the end of this file,
// so they can reference anything defined in here
#include "customassert.h"
#include "customnew.h"
#include "printmanager.h"
#include "profiler.h"
#include "exceptiontrace.h"

#if BUILD_WINDOWS
#include "exceptionhandler.h"
#endif

#include "color.h"

#else
#error core.h included twice, check structure.
#endif
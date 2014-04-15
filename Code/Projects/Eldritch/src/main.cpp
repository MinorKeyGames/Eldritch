#include "core.h"
#include "eldritchframework.h"
#include "filestream.h"
#include "configmanager.h"
#include "stringmanager.h"
#include "allocator.h"

#if BUILD_WINDOWS
#include <Windows.h>
#include <crtdbg.h>
#endif

#if BUILD_SDL
#include "SDL2/SDL.h"
#endif

#if BUILD_MAC
#include "file.h"
#include "objcjunk.h"
#include <CoreFoundation/CoreFoundation.h>
#endif

// Can easily disable my memory manager here if needed
#define ELDRITCH_USE_ALLOCATOR 1

#if BUILD_MAC
// TODO: Fix PrintManager to just keep a SimpleString so I don't have to do this gross hack.
void Initialize( SimpleString& LogFilename )
#else
void Initialize()
#endif
{
#if ELDRITCH_USE_ALLOCATOR
	// Default allocator must be initialized before creating any objects.
	// It should be large enough that I won't need to worry about fragmentation.
	{
		static const uint kNumChunks = 5;
		Allocator::SChunkDef ChunkDefs[ kNumChunks ];

		ChunkDefs[0].m_AllocSize = 16;
		ChunkDefs[0].m_ChunkSize = 6 * 1024 * 1024;
		ChunkDefs[1].m_AllocSize = 32;
		ChunkDefs[1].m_ChunkSize = 18 * 1024 * 1024;
		ChunkDefs[2].m_AllocSize = 256;
		ChunkDefs[2].m_ChunkSize = 10 * 1024 * 1024;
		ChunkDefs[3].m_AllocSize = 1024;
		ChunkDefs[3].m_ChunkSize = 5 * 1024 * 1024;
		ChunkDefs[4].m_AllocSize = 0;
		ChunkDefs[4].m_ChunkSize = 30 * 1024 * 1024;

		// Total: 6 + 18 + 10 + 5 + 30 = 69MB(!)

		Allocator::Enable( true );
		Allocator::GetDefault().Initialize( kNumChunks, ChunkDefs );
	}
#endif

#if BUILD_WINDOWS
	ExceptionHandler::Enable();
#endif
    
#if BUILD_MAC
	// Change working directory to the Resources path in the .app folder structure
	CFBundleRef MainBundle = CFBundleGetMainBundle();
	CFURLRef ResourceURL = CFBundleCopyResourcesDirectoryURL( MainBundle );
	char ResourcePath[ PATH_MAX ];
	CFURLGetFileSystemRepresentation( ResourceURL, TRUE, reinterpret_cast<UInt8*>( ResourcePath ), PATH_MAX );
	CFRelease( ResourceURL );
	chdir( ResourcePath );
#endif

#if BUILD_MAC
	LogFilename = ObjCJunk::GetUserDirectory() + SimpleString( "eldritch-log.txt" );
	PrintManager::GetInstance()->LogTo( LogFilename.CStr() );
#else
	// Do the default thing and emit log file in working directory.
	PRINTLOGS( eldritch );
#endif
}

void ShutDown()
{
#if BUILD_WINDOWS
	ExceptionHandler::ShutDown();
#endif

#if ELDRITCH_USE_ALLOCATOR
	if( Allocator::IsEnabled() )
	{
#if BUILD_DEV
		Allocator::GetDefault().Report( FileStream( "memory_exit_report.txt", FileStream::EFM_Write ) );
#endif

		ASSERT( Allocator::GetDefault().CheckForLeaks() );
		Allocator::GetDefault().ShutDown();
	}
#endif

#if BUILD_WINDOWS
	DEBUGASSERT( _CrtCheckMemory() );
	DEBUGASSERT( !_CrtDumpMemoryLeaks() );
#endif
}

#if BUILD_WINDOWS_NO_SDL
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
#elif BUILD_SDL
extern "C" int main( int argc, char* argv[] )
#endif
{
#if BUILD_WINDOWS_NO_SDL
	Unused( hPrevInstance );
	Unused( lpCmdLine );
#elif BUILD_SDL
	Unused( argc );
	Unused( argv );
#endif

#if BUILD_MAC
    // TODO: Fix this gross hack.
    // Keep log filename in scope
    SimpleString LogFilename;
    Initialize( LogFilename );
#else
	Initialize();
#endif

	EldritchFramework* pFramework = new EldritchFramework;
#if BUILD_WINDOWS_NO_SDL
	pFramework->SetInitializeParameters( hInstance, nCmdShow );
#endif
	pFramework->Main();
	SafeDelete( pFramework );

	ShutDown();

	return 0;
}
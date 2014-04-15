#include "core.h"
#include "printmanager.h"
#include "filestream.h"
#include "configmanager.h"
#include "map.h"

#include <stdio.h>
#include <stdarg.h>

#if BUILD_WINDOWS
#include <Windows.h>	// For OutputDebugString
#endif

#define STRINGBUFFERSIZE 2048

PrintManager* PrintManager::m_Instance = NULL;

static Map<HashedString, int>	sPrintLevels;

PrintManager* PrintManager::GetInstance()
{
	if( !m_Instance )
	{
		m_Instance = new PrintManager;
	}
	return m_Instance;
}

PrintManager* PrintManager::GetInstance_NoAlloc()
{
	return m_Instance;
}

void PrintManager::DeleteInstance()
{
	SafeDelete( m_Instance );
}

PrintManager::PrintManager()
:	m_StringBuffer( NULL )
,	m_Channels( PRINTCHANNEL_Console | PRINTCHANNEL_Output | PRINTCHANNEL_Log )
#if OPENLOGFILE
,	m_LogFilename( NULL )
#else
,	m_LogStream( NULL )
#endif
{
	m_StringBuffer = new char[ STRINGBUFFERSIZE ];
}

PrintManager::~PrintManager()
{
	sPrintLevels.Clear();
	SafeDeleteArray( m_StringBuffer );
#if OPENLOGFILE
	// Do nothing.
#else
	SafeDelete( m_LogStream );
#endif
}

void PrintManager::SetPrintLevel( const HashedString& Category, int Level )
{
	sPrintLevels[ Category ] = Level;
}

void PrintManager::Printf( const HashedString& Category, int Level, const char* Format, ... )
{
	Map< HashedString, int >::Iterator CatIter;
	if( Category )
	{
		CatIter = sPrintLevels.Search( Category );
	}
	// If we don't have a level defined for the given category, use the default
	if( CatIter.IsNull() )
	{
		CatIter = sPrintLevels.Search( HashedString::NullString );
	}
	int PrintLevel = CatIter.IsNull() ? 0 : *CatIter;

	if( Level <= PrintLevel )
	{
		va_list Args;

		va_start( Args, Format );
		VSPRINTF( m_StringBuffer, STRINGBUFFERSIZE, Format, Args );

		// Now send string wherever I want it to go...

		if( m_Channels & PRINTCHANNEL_Console )
		{
			fputs( m_StringBuffer, stdout );	// puts appends an \n
		}

#if BUILD_DEV
		if( m_Channels & PRINTCHANNEL_Output )
		{
#if BUILD_WINDOWS
			OutputDebugString( m_StringBuffer );
#else
			// TODO PORT LATER: Output to debugger on other platforms.
#endif
		}
#endif

		// Handle logging differently in Debug and Release
		if( m_Channels & PRINTCHANNEL_Log )
		{
#if OPENLOGFILE
			if( m_LogFilename )
			{
				FileStream LogStream( m_LogFilename, FileStream::EFM_Append );	// Append files for every separate print; slow.
				LogStream.PrintF( m_StringBuffer );
			}
#else
			if( m_LogStream )
			{
				m_LogStream->PrintF( m_StringBuffer );
			}
#endif
		}
	}
}

void PrintManager::SetChannels( int Channels )
{
	m_Channels = Channels;
}

void PrintManager::LogTo( const char* Filename )
{
#if OPENLOGFILE
	m_LogFilename = Filename;
	FileStream LogStream( m_LogFilename, FileStream::EFM_Write );	// Open the file for write just to clear it
#else
	// NOTE: I thought about using a buffered filestream to allow concurrent instances of the game,
	// but then the game would be accumulating all printing into a huge buffer at runtime. So no.
	m_LogStream = new FileStream( Filename, FileStream::EFM_Write );
#endif
}

void PrintManager::LoadPrintLevels()
{
	STATICHASH( NumPrintLevels );
	STATICHASH( PrintManager );

	int NumPrintLevels = ConfigManager::GetInt( sNumPrintLevels, 0, sPrintManager );
	for( int i = 0; i < NumPrintLevels; ++i )
	{
		const char* Category = ConfigManager::GetSequenceString( "PrintLevelCategory%d", i, NULL, sPrintManager );
		int Level = ConfigManager::GetSequenceInt( "PrintLevel%d", i, 0, sPrintManager );
		SetPrintLevel( Category, Level );
	}
}
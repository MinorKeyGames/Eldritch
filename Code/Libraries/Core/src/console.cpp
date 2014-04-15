#include "core.h"
#include "console.h"

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <Windows.h>

Console* Console::m_Instance = NULL;

Console::Console()
{
	AllocConsole();

	// Store off original stdin, stdout, and stderr
	m_StdIn = *stdin;
	m_StdOut = *stdout;
	m_StdErr = *stderr;

	intptr_t	StdHandle;
	int			ConHandle;
	FILE*		Stream;

	// Redirect stdout
	StdHandle = (intptr_t)GetStdHandle( STD_OUTPUT_HANDLE );
	ConHandle = _open_osfhandle( StdHandle, _O_TEXT );
	if( ConHandle != -1 )
	{
		Stream = _fdopen( ConHandle, "w" );
		*stdout = *Stream;
		setvbuf( stdout, NULL, _IONBF, 0 );
	}

	// Redirect stdin
	StdHandle = (intptr_t)GetStdHandle( STD_INPUT_HANDLE );
	ConHandle = _open_osfhandle( StdHandle, _O_TEXT );
	if( ConHandle != -1 )
	{
		Stream = _fdopen( ConHandle, "r" );
		*stdin = *Stream;
		setvbuf( stdin, NULL, _IONBF, 0 );
	}

	// Redirect stderr
	StdHandle = (intptr_t)GetStdHandle( STD_ERROR_HANDLE );
	ConHandle = _open_osfhandle( StdHandle, _O_TEXT );
	if( ConHandle != -1 )
	{
		Stream = _fdopen( ConHandle, "w" );
		*stderr = *Stream;
		setvbuf( stderr, NULL, _IONBF, 0 );
	}

	DEBUGPRINTF( "Console initialized\n" );
}

Console::~Console()
{
	FreeConsole();

	// Restore original stdin, stdout, and stderr
	*stdin = m_StdIn;
	*stdout = m_StdOut;
	*stderr = m_StdErr;
}

Console* Console::GetInstance()
{
	if( !m_Instance )
	{
		m_Instance = new Console;
	}
	return m_Instance;
}

void Console::DeleteInstance()
{
	SafeDelete( m_Instance );
}

bool Console::IsOpen()
{
	return ( m_Instance != NULL );
}

void Console::Toggle()
{
	if( IsOpen() )
	{
		DeleteInstance();
	}
	else
	{
		GetInstance();
	}
}

HWND Console::GetHWnd()
{
	return GetConsoleWindow();
}

void Console::SetPos( const int X, const int Y ) const
{
	SetWindowPos( GetConsoleWindow(), HWND_NOTOPMOST, X, Y, 0, 0, SWP_NOSIZE );
}
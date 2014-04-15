#include "core.h"
#include "consoleinput.h"

#define CONSOLE_INPUT_BUFFER_SIZE 1

ConsoleInputHandlerCallback ConsoleInput::m_HandlerCallback = NULL;

/*static*/ BOOL WINAPI ConsoleInput::ConsoleHandler( DWORD dwCtrlType )
{
	if( m_HandlerCallback )
	{
		return m_HandlerCallback( dwCtrlType );
	}
	else
	{
		return FALSE;
	}
}

/*static*/ void ConsoleInput::SetHandlerCallback( ConsoleInputHandlerCallback NewCallback )
{
	m_HandlerCallback = NewCallback;
}

ConsoleInput::ConsoleInput()
:	m_hConsoleInput( NULL )
,	m_CursorX(0)
,	m_CursorY(0)
,	m_HasFocus( true )	// NOTE: Assume we start with focus. Might cause problems?
{
	m_hConsoleInput = GetStdHandle( STD_INPUT_HANDLE );
	ASSERT( m_hConsoleInput );

	// TOTAL HACK!! Because the SDK I have doesn't define these, I need to do it myself
#define ENABLE_EXTENDED_FLAGS	0x0080
#define ENABLE_QUICK_EDIT_MODE	0x0040
#define ENABLE_INSERT_MODE		0x0020
	SetConsoleCtrlHandler( ConsoleHandler, TRUE );
	SetConsoleMode( m_hConsoleInput, ENABLE_EXTENDED_FLAGS | ENABLE_INSERT_MODE | ENABLE_PROCESSED_INPUT | ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_MOUSE_INPUT );
}

ConsoleInput::~ConsoleInput()
{
}

void ConsoleInput::Tick()
{
	ASSERT( m_hConsoleInput );

	DWORD			NumEvents;
	INPUT_RECORD	InputBuffer[ CONSOLE_INPUT_BUFFER_SIZE ];

	DWORD cMode = 0;
	GetConsoleMode( m_hConsoleInput, &cMode );

	// Get the cursor location (when the mouse moves)
	GetNumberOfConsoleInputEvents( m_hConsoleInput, &NumEvents );

	// Don't read until we have events, because ReadConsoleInput blocks until it can return something
	if( NumEvents > 0 )
	{
		ReadConsoleInput( m_hConsoleInput, InputBuffer, CONSOLE_INPUT_BUFFER_SIZE, &NumEvents );
		for( uint i = 0; i < NumEvents; ++i )
		{
			if( InputBuffer[i].EventType == MOUSE_EVENT )
			{
				if( InputBuffer[i].Event.MouseEvent.dwEventFlags == MOUSE_MOVED )
				{
					COORD CursorLoc = InputBuffer[i].Event.MouseEvent.dwMousePosition;
					m_CursorX = CursorLoc.X;
					m_CursorY = CursorLoc.Y;
				}
			}
			else if( InputBuffer[i].EventType == FOCUS_EVENT )
			{
				m_HasFocus = ( InputBuffer[i].Event.FocusEvent.bSetFocus == TRUE );
			}
		}
	}
}

int ConsoleInput::GetCursorX()
{
	return m_CursorX;
}

int ConsoleInput::GetCursorY()
{
	return m_CursorY;
}

bool ConsoleInput::HasFocus()
{
	return m_HasFocus;
}

SimpleString ConsoleInput::GetUserInput() const
{
	char Buffer[256];
	DWORD OutNum = 0;

	if( ReadConsole( m_hConsoleInput, Buffer, 256, &OutNum, NULL ) )
	{
		// The last two characters should be a CRLF, unless we Ctrl+C or Ctrl+Break out of the zinput.
		OutNum = OutNum >= 2 ? OutNum - 2 : OutNum;

		Buffer[ OutNum ] = '\0';
		return SimpleString( Buffer );
	}

	return SimpleString( "" );
}
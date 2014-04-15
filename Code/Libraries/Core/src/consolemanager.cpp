#include "core.h"
#include "consolemanager.h"
#include "console.h"
#include "configmanager.h"

#include <stdio.h>
#include <Windows.h>
#include <memory.h>

ConsoleManager* ConsoleManager::m_Instance = NULL;

char ShiftNumericMap[] = { ')', '!', '@', '#', '$', '%', '^', '&', '*', '(' };

ConsoleManager::ConsoleManager()
:	m_CommandQueue()
,	m_CommandBuffer()
,	m_Keyboard()
,	m_CommandHistory()
,	m_HistoryIndex( 0 )
,	m_DeferredCommands()
,	m_IsCheater( false )
{
}

ConsoleManager::~ConsoleManager()
{
	m_CommandQueue.Clear();
	m_CommandBuffer.Clear();
	m_CommandHistory.Clear();
}

ConsoleManager* ConsoleManager::GetInstance()
{
	if( !m_Instance )
	{
		m_Instance = new ConsoleManager;
	}
	return m_Instance;
}

void ConsoleManager::DeleteInstance()
{
	SafeDelete( m_Instance );
}

void ConsoleManager::PushCommands( const SimpleString& Commands )
{
	uint Length = Commands.Length() + 1;
	char* CommandBuffer = new char[ Length ];
	memcpy( CommandBuffer, Commands.CStr(), Length );

	// Separate tokens
	for( uint i = 0; i < Length; ++i )
	{
		if( CommandBuffer[i] == ' ' )
		{
			CommandBuffer[i] = '\0';
		}
	}
	for( uint i = 0; i < Length; )
	{
		while( CommandBuffer[i] == '\0' )
		{
			++i;
		}
		if( i >= Length )
		{
			break;
		}
		AddCommand( &CommandBuffer[i] );
		while( CommandBuffer[i] != '\0' )
		{
			++i;
		}
	}

	delete CommandBuffer;
}

void ConsoleManager::PushDeferredCommands( const SimpleString& Commands, uint Frames )
{
	SDeferredCommands DeferredCommands;
	DeferredCommands.m_Commands = Commands;
	DeferredCommands.m_Frames = Frames;
	m_DeferredCommands.PushBack( DeferredCommands );
}

void ConsoleManager::AddCommand( const SimpleString& Command )
{
	m_CommandQueue.PushBack( Command );
}

/*static*/ bool ConsoleManager::GetBool()
{
	return GetInstance()->GetNextCommand().AsBool();
}

/*static*/ int ConsoleManager::GetInt()
{
	return GetInstance()->GetNextCommand().AsInt();
}

/*static*/ float ConsoleManager::GetFloat()
{
	return GetInstance()->GetNextCommand().AsFloat();
}

/*static*/ SimpleString ConsoleManager::GetString()
{
	return GetInstance()->GetNextCommand();
}

SimpleString ConsoleManager::GetNextCommand()
{
	SimpleString RetVal;
	if( !m_CommandQueue.Empty() )
	{
		RetVal = *( m_CommandQueue.Front() );
		m_CommandQueue.PopFront();
	}
	return RetVal;
}

SimpleString ConsoleManager::PeekNextCommand() const
{
	SimpleString RetVal;
	if( !m_CommandQueue.Empty() )
	{
		RetVal = *( m_CommandQueue.Front() );
	}
	return RetVal;
}

void ConsoleManager::Tick( float DeltaTime, bool GetInput )
{
	for( List< SDeferredCommands >::Iterator DeferredCommandIter = m_DeferredCommands.Begin(); DeferredCommandIter != m_DeferredCommands.End(); )
	{
		SDeferredCommands& DeferredCommands = ( *DeferredCommandIter );
		if( DeferredCommands.m_Frames == 0 )
		{
			PushCommands( DeferredCommands.m_Commands );
			m_DeferredCommands.Pop( DeferredCommandIter );
		}
		else
		{
			--DeferredCommands.m_Frames;
			++DeferredCommandIter;
		}
	}

	if( GetInput )
	{
		m_Keyboard.Tick( DeltaTime );

		if( GetForegroundWindow() == Console::GetInstance()->GetHWnd() )
		{
			HandleInput();
		}
	}
}

bool ConsoleManager::IsCheater() const
{
	return m_IsCheater;
}

void ConsoleManager::SetIsCheater( bool Cheater )
{
	m_IsCheater = Cheater;
}

void ConsoleManager::HandleInput()
{
	bool Shift = m_Keyboard.IsHigh( Keyboard::EB_LeftShift );

	// Handle a-zA-Z
	for( uint8 i = 65; i < 91; ++i )
	{
		if( m_Keyboard.OnRise( i ) )
		{
			char c = i + ( Shift ? 0 : 32 );
			PRINTF( "%c", c );
			m_CommandBuffer.PushBack( c );
		}
	}
	// Handle 0-9
	for( uint8 i = 48; i < 58; ++i )
	{
		if( m_Keyboard.OnRise( i ) )
		{
			char c = Shift ? ShiftNumericMap[ i - 48 ] : i;
			PRINTF( "%c", c );
			m_CommandBuffer.PushBack( c );
		}
	}
	// Space
	if( m_Keyboard.OnRise( Keyboard::EB_Minus ) )
	{
		char c = Shift ? '_' : '-';
		PRINTF( "%c", c );
		m_CommandBuffer.PushBack( c );
	}
	// Space
	if( m_Keyboard.OnRise( Keyboard::EB_Space ) )
	{
		PRINTF( " " );
		m_CommandBuffer.PushBack( ' ' );
	}
	// Period
	if( m_Keyboard.OnRise( Keyboard::EB_Period ) )
	{
		PRINTF( "." );
		m_CommandBuffer.PushBack( '.' );
	}
	// Slash
	if( m_Keyboard.OnRise( Keyboard::EB_Slash ) )
	{
		PRINTF( "/" );
		m_CommandBuffer.PushBack( '/' );
	}
	// Backslash
	if( m_Keyboard.OnRise( Keyboard::EB_Backslash ) )
	{
		PRINTF( "\\" );
		m_CommandBuffer.PushBack( '\\' );
	}
	// Backspace/Ctrl+Backspace
	if( m_Keyboard.OnRise( Keyboard::EB_Backspace ) )
	{
		if( m_Keyboard.IsHigh( Keyboard::EB_LeftControl ) )
		{
			ClearBuffer();
		}
		else
		{
			PRINTF( "%c %c", 0x08, 0x08 );
			m_CommandBuffer.PopBack();
		}
	}
	// Up arrow
	if( m_Keyboard.OnRise( Keyboard::EB_Up ) )
	{
		GetPrevHistory();
	}
	// Down arrow
	if( m_Keyboard.OnRise( Keyboard::EB_Down ) )
	{
		GetNextHistory();
	}
	// Enter
	if( m_Keyboard.OnRise( Keyboard::EB_Enter ) )
	{
		PRINTF( "\n" );
		SubmitCommandString();
	}
}

void ConsoleManager::SubmitCommandString()
{
	m_CommandBuffer.PushBack( '\0' );
	SimpleString Commands( m_CommandBuffer.GetData() );
	m_CommandBuffer.Clear();

	// Entering a console command marks the player as a cheater
	// for purposes of leaderboards, records, etc.
	SetIsCheater( true );

	PushCommands( Commands );

	STATICHASH( CommandHistorySize );

	m_CommandHistory.PushBack( Commands );
	if( m_CommandHistory.Size() > ( uint )ConfigManager::GetInt( sCommandHistorySize, 10 ) )
	{
		m_CommandHistory.Remove( 0 );
	}
	m_HistoryIndex = m_CommandHistory.Size();
}

void ConsoleManager::ClearBuffer()
{
	while( m_CommandBuffer.Size() )
	{
		PRINTF( "%c %c", 0x08, 0x08 );
		m_CommandBuffer.PopBack();
	}
}

void ConsoleManager::GetNextHistory()
{
	if( m_CommandHistory.Size() && m_HistoryIndex < ( m_CommandHistory.Size() - 1 ) )
	{
		GetHistory( ++m_HistoryIndex );
	}
}

void ConsoleManager::GetPrevHistory()
{
	if( m_CommandHistory.Size() && m_HistoryIndex > 0 )
	{
		GetHistory( --m_HistoryIndex );
	}
}

void ConsoleManager::GetHistory( uint Index )
{
	ClearBuffer();
	ASSERT( Index < m_CommandHistory.Size() );
	SimpleString& Command = m_CommandHistory[ Index ];
	PRINTF( Command.CStr() );

	for( const char* c = Command.CStr(); *c != '\0'; ++c )
	{
		m_CommandBuffer.PushBack( *c );
	}
}
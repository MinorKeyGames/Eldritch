#ifndef CONSOLEMANAGER_H
#define CONSOLEMANAGER_H

// I'm extending this beyond just a "console manager" to using it as a
// general-purpose command buffer for all kinds of game messages. Just
// don't call Tick unless the console is supposed to be open.

#include "list.h"
#include "array.h"
#include "simplestring.h"
#include "keyboard.h"

#include <Windows.h>

class ConsoleManager
{
public:
	static ConsoleManager*	GetInstance();
	static void				DeleteInstance();

	// Utility functions to read the next command on the queue as a type
	static bool				GetBool();
	static int				GetInt();
	static float			GetFloat();
	static SimpleString		GetString();

	SimpleString			GetNextCommand();
	SimpleString			PeekNextCommand() const;
	void					PushCommands( const SimpleString& Commands );	// This can accept command strings (e.g. "world <worldfile>"), not just atomic commands
	void					PushDeferredCommands( const SimpleString& Commands, uint Frames );

	void					Tick( float DeltaTime, bool GetInput );

	bool					IsCheater() const;
	void					SetIsCheater( bool Cheater );

private:
	ConsoleManager();
	~ConsoleManager();

	struct SDeferredCommands
	{
		SDeferredCommands() : m_Commands( "" ), m_Frames( 0 ) {}
		SimpleString	m_Commands;
		uint			m_Frames;
	};

	void						HandleInput();
	void						SubmitCommandString();
	void						AddCommand( const SimpleString& Command );	// This only accepts unit words as commands; use PushCommands instead
	void						ClearBuffer();
	void						GetNextHistory();
	void						GetPrevHistory();
	void						GetHistory( uint Index );

	static ConsoleManager*		m_Instance;

	List< SimpleString >		m_CommandQueue;
	Array< char >				m_CommandBuffer;
	Keyboard					m_Keyboard;

	Array< SimpleString >		m_CommandHistory;
	uint						m_HistoryIndex;

	List< SDeferredCommands >	m_DeferredCommands;

	bool						m_IsCheater;
};

#endif // CONSOLEMANAGER_H
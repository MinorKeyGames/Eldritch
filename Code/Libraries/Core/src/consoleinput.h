#ifndef CONSOLEINPUT_H
#define CONSOLEINPUT_H

// This just grabs the console cursor location. Everything else should use my existing input classes.

#include "simplestring.h"

#include <Windows.h>

typedef bool ( *ConsoleInputHandlerCallback )( uint );

class ConsoleInput
{
public:
	ConsoleInput();
	~ConsoleInput();

	void	Tick();

	int		GetCursorX();
	int		GetCursorY();

	bool	HasFocus();

	SimpleString	GetUserInput() const;

	static BOOL WINAPI ConsoleHandler( DWORD dwCtrlType );
	static void SetHandlerCallback( ConsoleInputHandlerCallback NewCallback );

private:
	HANDLE	m_hConsoleInput;

	int		m_CursorX;
	int		m_CursorY;

	bool	m_HasFocus;

	static ConsoleInputHandlerCallback m_HandlerCallback;
};

#endif // CONSOLEINPUT_H
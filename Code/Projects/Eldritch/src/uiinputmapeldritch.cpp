#include "core.h"
#include "uiinputmapeldritch.h"
#include "eldritchframework.h"
#include "keyboard.h"
#include "xinputcontroller.h"
#include "inputsystem.h"

UIInputMapEldritch::UIInputMapEldritch( EldritchFramework* const pFramework )
:	m_Framework( pFramework )
{
}

UIInputMapEldritch::~UIInputMapEldritch()
{
}

bool UIInputMapEldritch::OnNext()
{
	Keyboard* const pKeyboard = m_Framework->GetKeyboard();

	if( pKeyboard->OnRise( Keyboard::EB_Tab ) &&
		!( pKeyboard->IsHigh( Keyboard::EB_LeftControl ) ||
		   pKeyboard->IsHigh( Keyboard::EB_RightControl ) ) &&
		!( pKeyboard->IsHigh( Keyboard::EB_LeftShift ) ||
		   pKeyboard->IsHigh( Keyboard::EB_RightShift ) ) )
	{
		return true;
	}

	return false;
}

bool UIInputMapEldritch::OnPrevious()
{
	Keyboard* const pKeyboard = m_Framework->GetKeyboard();

	if( pKeyboard->OnRise( Keyboard::EB_Tab ) &&
		( ( pKeyboard->IsHigh( Keyboard::EB_LeftControl ) ||
			pKeyboard->IsHigh( Keyboard::EB_RightControl ) ) ||
		  ( pKeyboard->IsHigh( Keyboard::EB_LeftShift ) ||
			pKeyboard->IsHigh( Keyboard::EB_RightShift ) ) ) )
	{
		return true;
	}

	return false;
}

bool UIInputMapEldritch::OnUp()
{
	Keyboard* const pKeyboard = m_Framework->GetKeyboard();
	XInputController * const pController = m_Framework->GetController();
	InputSystem* const pInputSystem = m_Framework->GetInputSystem();

	STATIC_HASHED_STRING( Forward );
	const uint Keyboard_Forward = pInputSystem->GetBoundKeyboardSignal( sForward );

	if( pKeyboard->OnRise( Keyboard_Forward ) )
	{
		return true;
	}

	if( pKeyboard->OnRise( Keyboard::EB_Up ) )
	{
		return true;
	}

	if( pController->OnRise( XInputController::EB_Up ) ||
		pController->OnRise( XInputController::EB_LeftThumbUp ) ||
		pController->OnRise( XInputController::EB_RightThumbUp ) )
	{
		return true;
	}

	return false;
}

bool UIInputMapEldritch::OnDown()
{
	Keyboard* const pKeyboard = m_Framework->GetKeyboard();
	XInputController * const pController = m_Framework->GetController();
	InputSystem* const pInputSystem = m_Framework->GetInputSystem();

	STATIC_HASHED_STRING( Back );
	const uint Keyboard_Back = pInputSystem->GetBoundKeyboardSignal( sBack );

	if( pKeyboard->OnRise( Keyboard_Back ) )
	{
		return true;
	}

	if( pKeyboard->OnRise( Keyboard::EB_Down ) )
	{
		return true;
	}

	if( pController->OnRise( XInputController::EB_Down ) ||
		pController->OnRise( XInputController::EB_LeftThumbDown ) ||
		pController->OnRise( XInputController::EB_RightThumbDown ) )
	{
		return true;
	}

	return false;
}

bool UIInputMapEldritch::OnLeft()
{
	Keyboard* const pKeyboard = m_Framework->GetKeyboard();
	XInputController * const pController = m_Framework->GetController();
	InputSystem* const pInputSystem = m_Framework->GetInputSystem();

	STATIC_HASHED_STRING( Left );
	const uint Keyboard_Left = pInputSystem->GetBoundKeyboardSignal( sLeft );

	if( pKeyboard->OnRise( Keyboard_Left ) )
	{
		return true;
	}

	if( pKeyboard->OnRise( Keyboard::EB_Left ) )
	{
		return true;
	}

	if( pController->OnRise( XInputController::EB_Left ) ||
		pController->OnRise( XInputController::EB_LeftThumbLeft ) ||
		pController->OnRise( XInputController::EB_RightThumbLeft ) )
	{
		return true;
	}

	return false;
}

bool UIInputMapEldritch::OnRight()
{
	Keyboard* const pKeyboard = m_Framework->GetKeyboard();
	XInputController * const pController = m_Framework->GetController();
	InputSystem* const pInputSystem = m_Framework->GetInputSystem();

	STATIC_HASHED_STRING( Right );
	const uint Keyboard_Right = pInputSystem->GetBoundKeyboardSignal( sRight );

	if( pKeyboard->OnRise( Keyboard_Right ) )
	{
		return true;
	}

	if( pKeyboard->OnRise( Keyboard::EB_Right ) )
	{
		return true;
	}

	if( pController->OnRise( XInputController::EB_Right ) ||
		pController->OnRise( XInputController::EB_LeftThumbRight ) ||
		pController->OnRise( XInputController::EB_RightThumbRight ) )
	{
		return true;
	}

	return false;
}

bool UIInputMapEldritch::OnAccept()
{
	Keyboard* const pKeyboard = m_Framework->GetKeyboard();
	XInputController * const pController = m_Framework->GetController();

	if( pKeyboard->OnRise( Keyboard::EB_Enter ) &&
		!( pKeyboard->IsHigh( Keyboard::EB_LeftAlt ) ||
		   pKeyboard->IsHigh( Keyboard::EB_RightAlt ) ) )
	{
		return true;
	}

	if( pController->OnRise( XInputController::EB_A ) || pController->OnRise( XInputController::EB_Start ) )
	{
		return true;
	}

	return false;
}

bool UIInputMapEldritch::OnCancel()
{
	Keyboard* const pKeyboard = m_Framework->GetKeyboard();
	XInputController * const pController = m_Framework->GetController();
	InputSystem* const pInputSystem = m_Framework->GetInputSystem();

	STATIC_HASHED_STRING( Frob );
	const uint Keyboard_Frob = pInputSystem->GetBoundKeyboardSignal( sFrob );
	if( pKeyboard->OnRise( Keyboard_Frob ) )
	{
		return true;
	}

	if( pKeyboard->OnRise( Keyboard::EB_Escape ) )
	{
		return true;
	}

	const uint Controller_Frob = pInputSystem->GetBoundControllerSignal( sFrob );
	if( pController->OnRise( Controller_Frob ) )
	{
		return true;
	}

	if( pController->OnRise( XInputController::EB_B ) )
	{
		return true;
	}

	return false;
}
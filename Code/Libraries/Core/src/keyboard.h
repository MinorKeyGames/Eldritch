#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "icontroller.h"

class Keyboard : public IController
{
public:
	enum EButtons
	{
		EB_None,
		EB_A,
		EB_B,
		EB_C,
		EB_D,
		EB_E,
		EB_F,
		EB_G,
		EB_H,
		EB_I,
		EB_J,
		EB_K,
		EB_L,
		EB_M,
		EB_N,
		EB_O,
		EB_P,
		EB_Q,
		EB_R,
		EB_S,
		EB_T,
		EB_U,
		EB_V,
		EB_W,
		EB_X,
		EB_Y,
		EB_Z,
		EB_Space,
		EB_LeftShift,
		EB_LeftControl,
		EB_LeftAlt,
		EB_RightShift,
		EB_RightControl,
		EB_RightAlt,
		EB_CapsLock,
		EB_Comma,
		EB_Period,
		EB_Slash,
		EB_Semicolon,
		EB_Apostrophe,
		EB_LeftBrace,
		EB_RightBrace,
		EB_Backslash,
		EB_Tilde,
		EB_Up,
		EB_Down,
		EB_Left,
		EB_Right,
		EB_Num0,
		EB_Num1,
		EB_Num2,
		EB_Num3,
		EB_Num4,
		EB_Num5,
		EB_Num6,
		EB_Num7,
		EB_Num8,
		EB_Num9,
		EB_NumDecimal,
		EB_NumMultiply,
		EB_NumAdd,
		EB_NumSubtract,
		EB_NumDivide,
		EB_Enter,
		EB_Insert,
		EB_Home,
		EB_PageUp,
		EB_Delete,
		EB_End,
		EB_PageDown,
		EB_Escape,
		EB_1,
		EB_2,
		EB_3,
		EB_4,
		EB_5,
		EB_6,
		EB_7,
		EB_8,
		EB_9,
		EB_0,
		EB_Backspace,
		EB_Minus,
		EB_Plus,
		EB_Tab,
		EB_F1,
		EB_F2,
		EB_F3,
		EB_F4,
		EB_F5,
		EB_F6,
		EB_F7,
		EB_F8,
		EB_F9,
		EB_F10,
		EB_F11,
		EB_F12,

		EB_Mouse_Left,
		EB_Mouse_Middle,
		EB_Mouse_Right,

		EB_Max,
	};

	struct SKeyboardState
	{
		bool	m_Buttons[ EB_Max ];
	};

	Keyboard();

	virtual void	Tick( float DeltaTime );

	virtual bool	IsHigh( uint Signal );
	virtual bool	IsLow( uint Signal );
	virtual bool	OnRise( uint Signal );
	virtual bool	OnFall( uint Signal );

	SKeyboardState	m_CurrentState;
	SKeyboardState	m_LastState;
};

#endif // KEYBOARD_H
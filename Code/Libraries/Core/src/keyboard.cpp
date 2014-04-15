#include "core.h"
#include "keyboard.h"

#include <memory.h>	// For memset

#if BUILD_WINDOWS_NO_SDL
#include <Windows.h>
#endif
#if BUILD_SDL
#include "SDL2/SDL.h"
#endif

#if BUILD_WINDOWS_NO_SDL
// Map Keyboard keys to VK keys.
int VKKeys[ Keyboard::EB_Max ] =
{
	0,
	'A',
	'B',
	'C',
	'D',
	'E',
	'F',
	'G',
	'H',
	'I',
	'J',
	'K',
	'L',
	'M',
	'N',
	'O',
	'P',
	'Q',
	'R',
	'S',
	'T',
	'U',
	'V',
	'W',
	'X',
	'Y',
	'Z',
	VK_SPACE,
	VK_LSHIFT,
	VK_LCONTROL,
	VK_LMENU,
	VK_RSHIFT,
	VK_RCONTROL,
	VK_RMENU,
	VK_CAPITAL,
	VK_OEM_COMMA,
	VK_OEM_PERIOD,
	VK_OEM_2,		// /?
	VK_OEM_1,		// ;:
	VK_OEM_7,		// '"
	VK_OEM_4,		// [{
	VK_OEM_6,		// ]}
	VK_OEM_5,		// \|
	VK_OEM_3,		// `~
	VK_UP,
	VK_DOWN,
	VK_LEFT,
	VK_RIGHT,
	VK_NUMPAD0,
	VK_NUMPAD1,
	VK_NUMPAD2,
	VK_NUMPAD3,
	VK_NUMPAD4,
	VK_NUMPAD5,
	VK_NUMPAD6,
	VK_NUMPAD7,
	VK_NUMPAD8,
	VK_NUMPAD9,
	VK_DECIMAL,
	VK_MULTIPLY,
	VK_ADD,
	VK_SUBTRACT,
	VK_DIVIDE,
	VK_RETURN,
	VK_INSERT,
	VK_HOME,
	VK_PRIOR,
	VK_DELETE,
	VK_END,
	VK_NEXT,
	VK_ESCAPE,
	'1',
	'2',
	'3',
	'4',
	'5',
	'6',
	'7',
	'8',
	'9',
	'0',
	VK_BACK,
	VK_OEM_MINUS,
	VK_OEM_PLUS,
	VK_TAB,
	VK_F1,
	VK_F2,
	VK_F3,
	VK_F4,
	VK_F5,
	VK_F6,
	VK_F7,
	VK_F8,
	VK_F9,
	VK_F10,
	VK_F11,
	VK_F12,

	VK_LBUTTON,
	VK_MBUTTON,
	VK_RBUTTON,
};
#endif // BUILD_WINDOWS_NO_SDL

#if BUILD_SDL
// Map Keyboard keys to SDL keys.
SDL_Scancode	SDLScancodes[ Keyboard::EB_Max ];
SDL_Keycode		SDLKeycodes[ Keyboard::EB_Max ] =
{
	SDLK_UNKNOWN,
	SDLK_a,
	SDLK_b,
	SDLK_c,
	SDLK_d,
	SDLK_e,
	SDLK_f,
	SDLK_g,
	SDLK_h,
	SDLK_i,
	SDLK_j,
	SDLK_k,
	SDLK_l,
	SDLK_m,
	SDLK_n,
	SDLK_o,
	SDLK_p,
	SDLK_q,
	SDLK_r,
	SDLK_s,
	SDLK_t,
	SDLK_u,
	SDLK_v,
	SDLK_w,
	SDLK_x,
	SDLK_y,
	SDLK_z,
	SDLK_SPACE,
	SDLK_LSHIFT,
	SDLK_LCTRL,
	SDLK_LALT,
	SDLK_RSHIFT,
	SDLK_RCTRL,
	SDLK_RALT,
	SDLK_CAPSLOCK,
	SDLK_COMMA,
	SDLK_PERIOD,
	SDLK_SLASH,			// /?
	SDLK_SEMICOLON,		// ;:
	SDLK_QUOTE,			// '"
	SDLK_LEFTBRACKET,	// [{
	SDLK_RIGHTBRACKET,	// ]}
	SDLK_BACKSLASH,		// \|
	SDLK_BACKQUOTE,		// `~
	SDLK_UP,
	SDLK_DOWN,
	SDLK_LEFT,
	SDLK_RIGHT,
	SDLK_KP_0,
	SDLK_KP_1,
	SDLK_KP_2,
	SDLK_KP_3,
	SDLK_KP_4,
	SDLK_KP_5,
	SDLK_KP_6,
	SDLK_KP_7,
	SDLK_KP_8,
	SDLK_KP_9,
	SDLK_KP_DECIMAL,
	SDLK_KP_MULTIPLY,
	SDLK_KP_PLUS,
	SDLK_KP_MINUS,
	SDLK_KP_DIVIDE,
	SDLK_RETURN,
	SDLK_INSERT,
	SDLK_HOME,
	SDLK_PAGEUP,
	SDLK_DELETE,
	SDLK_END,
	SDLK_PAGEDOWN,
	SDLK_ESCAPE,
	SDLK_1,
	SDLK_2,
	SDLK_3,
	SDLK_4,
	SDLK_5,
	SDLK_6,
	SDLK_7,
	SDLK_8,
	SDLK_9,
	SDLK_0,
	SDLK_BACKSPACE,
	SDLK_MINUS,
	SDLK_PLUS,
	SDLK_TAB,
	SDLK_F1,
	SDLK_F2,
	SDLK_F3,
	SDLK_F4,
	SDLK_F5,
	SDLK_F6,
	SDLK_F7,
	SDLK_F8,
	SDLK_F9,
	SDLK_F10,
	SDLK_F11,
	SDLK_F12,

	SDLK_UNKNOWN,	// SDL actually uses the mouse for mouse input. What a novel concept.
	SDLK_UNKNOWN,
	SDLK_UNKNOWN,
};
#endif // BUILD_SDL

Keyboard::Keyboard()
:	m_CurrentState()
,	m_LastState()
{
	memset( &m_CurrentState, 0, sizeof( SKeyboardState ) );
	memset( &m_LastState, 0, sizeof( SKeyboardState ) );

#if BUILD_SDL
	// Initialize the keycode -> scancode mapping
	for( uint KeyIndex = EB_None; KeyIndex < EB_Max; ++KeyIndex )
	{
		const SDL_Keycode	SDLKeycode	= SDLKeycodes[ KeyIndex ];
		const SDL_Scancode	SDLScancode	= SDL_GetScancodeFromKey( SDLKeycode );
		SDLScancodes[ KeyIndex ] = SDLScancode;
	}
#endif
}

void Keyboard::Tick( float DeltaTime )
{
	Unused( DeltaTime );

	m_LastState = m_CurrentState;

#if BUILD_WINDOWS_NO_SDL
	for( uint KeyIndex = EB_None; KeyIndex < EB_Max; ++KeyIndex )
	{
		const int	VKKey	= VKKeys[ KeyIndex ];
		const SHORT	VKState	= GetAsyncKeyState( VKKey );
		const bool	VKHigh	= ( VKState & 0x8000 ) != 0;
		m_CurrentState.m_Buttons[ KeyIndex ] = VKHigh;
	}
#endif

#if BUILD_SDL
	const Uint8* const SDLKeysState = SDL_GetKeyboardState( NULL );
	for( uint KeyIndex = EB_None; KeyIndex < EB_Max; ++KeyIndex )
	{
		const SDL_Scancode	SDLScancode	= SDLScancodes[ KeyIndex ];
		const Uint8			SDLState	= SDLKeysState[ SDLScancode ];
		const bool			SDLHigh		= ( SDLState != 0 );
		m_CurrentState.m_Buttons[ KeyIndex ] = SDLHigh;
	}
#endif
}

bool Keyboard::IsHigh( uint Signal )
{
	DEVASSERT( Signal > EB_None );
	DEVASSERT( Signal < EB_Max );
	return m_CurrentState.m_Buttons[ Signal ];
}

bool Keyboard::IsLow( uint Signal )
{
	DEVASSERT( Signal > EB_None );
	DEVASSERT( Signal < EB_Max );
	return !m_CurrentState.m_Buttons[ Signal ];
}

bool Keyboard::OnRise( uint Signal )
{
	DEVASSERT( Signal > EB_None );
	DEVASSERT( Signal < EB_Max );
	return m_CurrentState.m_Buttons[ Signal ] && !m_LastState.m_Buttons[ Signal ];
}

bool Keyboard::OnFall( uint Signal )
{
	DEVASSERT( Signal > EB_None );
	DEVASSERT( Signal < EB_Max );
	return !m_CurrentState.m_Buttons[ Signal ] && m_LastState.m_Buttons[ Signal ];
}
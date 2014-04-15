#include "core.h"
#include "mouse.h"
#include "display.h"
#include "windowwrapper.h"

#include <memory.h>

#if BUILD_SDL
#include "SDL2/SDL.h"
#endif

#if BUILD_WINDOWS_NO_SDL
Mouse::Mouse( HINSTANCE hInstance, HWND hWnd )
:	m_Active( true )
,	m_pDirectInput( NULL )
,	m_pMouse( NULL )
,	m_NotifiedMouseMoved( false )
,	m_ReceivedInputThisTick( false )
,	m_AllowCursor( true )
,	m_CursorShown( true )
{
	memset( &m_CurrentState, 0, sizeof( SMouseState ) );
	memset( &m_LastState, 0, sizeof( SMouseState ) );
	memset( &m_BufferedState, 0, sizeof( SMouseState ) );
	
	DirectInput8Create( hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**) &m_pDirectInput, NULL );
	m_pDirectInput->CreateDevice( GUID_SysMouse, &m_pMouse, NULL );
	m_pMouse->SetDataFormat( &c_dfDIMouse2 );
	m_pMouse->SetCooperativeLevel( hWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND );	// Could do non-exclusive if I don't need exclusive?

	m_pMouse->Acquire();
}
#endif

#if BUILD_SDL
Mouse::Mouse( Window* const pWindow )
:	m_Active( true )
,	m_Window( pWindow )
,	m_AdjustmentX( 0 )
,	m_AdjustmentY( 0 )
,	m_RelativeModeActive( false )
,	m_RelativeModeSupported( false )
,	m_NotifiedMouseMoved( false )
,	m_ReceivedInputThisTick( false )
,	m_AllowCursor( true )
,	m_CursorShown( true )
{
	memset( &m_CurrentState, 0, sizeof( SMouseState ) );
	memset( &m_LastState, 0, sizeof( SMouseState ) );
	memset( &m_BufferedState, 0, sizeof( SMouseState ) );

	const int Error = SDL_SetRelativeMouseMode( SDL_TRUE );
	m_RelativeModeSupported = ( 0 == Error );

	// This is redundant if m_RelativeModeSupported but whatever.
	SetRelativeMode( true );
}
#endif

Mouse::~Mouse()
{
#if BUILD_WINDOWS_NO_SDL
	if( m_pMouse )
	{
		m_pMouse->Unacquire();
	}
	SafeRelease( m_pMouse );
	SafeRelease( m_pDirectInput );
#endif
#if BUILD_SDL
	SetRelativeMode( false );
#endif
}

#if BUILD_SDL
void Mouse::SetRelativeMode( const bool Relative )
{
	if( m_RelativeModeSupported )
	{
		const int Error = SDL_SetRelativeMouseMode( Relative ? SDL_TRUE : SDL_FALSE );
		ASSERT( 0 == Error );

		m_RelativeModeActive = Relative;
	}
	else
	{
		ASSERT( m_Window );
		ASSERT( m_Window->GetSDLWindow() );

		SDL_ShowCursor( Relative ? SDL_DISABLE : SDL_ENABLE );
		SDL_SetWindowGrab( m_Window->GetSDLWindow(), Relative ? SDL_TRUE : SDL_FALSE );
	}
}
#endif

void Mouse::Tick( float DeltaTime )
{
	Unused( DeltaTime );

	m_LastState = m_CurrentState;

#if BUILD_WINDOWS_NO_SDL
	DIMOUSESTATE2 MouseState; 
	ZeroMemory( &MouseState, sizeof( DIMOUSESTATE2 ) );

	if( m_Active )
	{
		HRESULT hr = m_pMouse->GetDeviceState( sizeof( DIMOUSESTATE2 ), &MouseState );

		// Handle lost mouse
		while( FAILED( hr ) )
		{
			hr = m_pMouse->Acquire();
			while( hr == DIERR_INPUTLOST )
			{
				hr = m_pMouse->Acquire();
			}

			if( FAILED( hr ) )
			{
				// Couldn't acquire for some reason; zero current state
				memset( &m_CurrentState, 0, sizeof( SMouseState ) );
				return;
			}

			// Else, successfully reacquired device! Continue as before.
			hr = m_pMouse->GetDeviceState( sizeof( DIMOUSESTATE2 ), &MouseState );
		}
	}

	// MouseState returns the delta from the last state; this is
	// what I'd previously called velocity, but is now position.
	m_CurrentState.m_Axes[ EA_X ] = MouseState.lX;
	m_CurrentState.m_Axes[ EA_Y ] = MouseState.lY;

	m_CurrentState.m_Buttons[ EB_Left ]			= ( MouseState.rgbButtons[0] & 0x80 ) != 0;
	m_CurrentState.m_Buttons[ EB_Middle ]		= ( MouseState.rgbButtons[2] & 0x80 ) != 0;
	m_CurrentState.m_Buttons[ EB_Right ]		= ( MouseState.rgbButtons[1] & 0x80 ) != 0;

	m_CurrentState.m_Buttons[ EB_WheelUp ]		= MouseState.lZ > 0;
	m_CurrentState.m_Buttons[ EB_WheelDown ]	= MouseState.lZ < 0;
#endif // BUILD_WINDOWS_NO_SDL

#if BUILD_SDL
	int DeltaX = 0;
	int DeltaY = 0;
	const Uint32 MouseState = SDL_GetRelativeMouseState( &DeltaX, &DeltaY );

	m_CurrentState.m_Axes[ EA_X ] = DeltaX;
	m_CurrentState.m_Axes[ EA_Y ] = DeltaY;

	if( !m_RelativeModeActive )
	{
		m_CurrentState.m_Axes[ EA_X ] += m_AdjustmentX;
		m_CurrentState.m_Axes[ EA_Y ] += m_AdjustmentY;
	}

	m_AdjustmentX = 0;
	m_AdjustmentY = 0;

	m_CurrentState.m_Buttons[ EB_Left ]		= ( MouseState & SDL_BUTTON_LMASK ) != 0;
	m_CurrentState.m_Buttons[ EB_Middle ]	= ( MouseState & SDL_BUTTON_MMASK ) != 0;
	m_CurrentState.m_Buttons[ EB_Right ]	= ( MouseState & SDL_BUTTON_RMASK ) != 0;

	m_CurrentState.m_Buttons[ EB_WheelUp ]		= 0;
	m_CurrentState.m_Buttons[ EB_WheelDown ]	= 0;
#endif

	// Get buffered signal from Windows messages when needed (when DI is inactive,
	// because wheel isn't accessible through virtual keys like mouse buttons are).
	m_CurrentState.m_Buttons[ EB_WheelUp ]		|= m_BufferedState.m_Buttons[ EB_WheelUp ];
	m_CurrentState.m_Buttons[ EB_WheelDown ]	|= m_BufferedState.m_Buttons[ EB_WheelDown ];

	memset( &m_BufferedState, 0, sizeof( SMouseState ) );

	m_ReceivedInputThisTick = false;
	if( m_NotifiedMouseMoved )
	{
		m_NotifiedMouseMoved = false;
		m_ReceivedInputThisTick = true;
	}
	for( uint ButtonIndex = 1; ButtonIndex < EB_Max && !m_ReceivedInputThisTick; ++ButtonIndex )
	{
		if( IsHigh( ButtonIndex ) )
		{
			m_ReceivedInputThisTick = true;
		}
	}
	for( uint AxisIndex = 1; AxisIndex < EA_Max && !m_ReceivedInputThisTick; ++AxisIndex )
	{
		if( GetPosition( AxisIndex ) != 0.0f )
		{
			m_ReceivedInputThisTick = true;
		}
	}
}

bool Mouse::IsHigh( uint Signal )
{
	DEVASSERT( Signal > EB_None );
	DEVASSERT( Signal < EB_Max );
	return m_CurrentState.m_Buttons[ Signal ];
}

bool Mouse::IsLow( uint Signal )
{
	DEVASSERT( Signal > EB_None );
	DEVASSERT( Signal < EB_Max );
	return !m_CurrentState.m_Buttons[ Signal ];
}

bool Mouse::OnRise( uint Signal )
{
	DEVASSERT( Signal > EB_None );
	DEVASSERT( Signal < EB_Max );
	return m_CurrentState.m_Buttons[ Signal ] && !m_LastState.m_Buttons[ Signal ];
}

bool Mouse::OnFall( uint Signal )
{
	DEVASSERT( Signal > EB_None );
	DEVASSERT( Signal < EB_Max );
	return !m_CurrentState.m_Buttons[ Signal ] && m_LastState.m_Buttons[ Signal ];
}

void Mouse::Buffer( uint Signal )
{
	DEVASSERT( Signal > EB_None );
	DEVASSERT( Signal < EB_Max );
	m_BufferedState.m_Buttons[ Signal ] = true;
}

float Mouse::GetPosition( uint Axis )
{
	DEVASSERT( Axis > EA_None );
	DEVASSERT( Axis < EA_Max );
	// TODO: Normalize this somehow?
	return (float)m_CurrentState.m_Axes[ Axis ];
}

float Mouse::GetVelocity( uint Axis )
{
	DEVASSERT( Axis > EA_None );
	DEVASSERT( Axis < EA_Max );
	// TODO: Normalize this somehow?
	return (float)m_CurrentState.m_Axes[ Axis ] - (float)m_LastState.m_Axes[ Axis ];
}

void Mouse::SetPosition( int X, int Y, Window* const pWindow )
{
	XTRACE_FUNCTION;

#if BUILD_WINDOWS_NO_SDL
	POINT ClientPos;
	ClientPos.x = X;
	ClientPos.y = Y;
	ClientToScreen( pWindow->GetHWnd(), &ClientPos );
	SetCursorPos( ClientPos.x, ClientPos.y );
#endif
#if BUILD_SDL
	int CurrentX = 0;
	int CurrentY = 0;
	GetPosition( CurrentX, CurrentY, pWindow );

	SDL_WarpMouseInWindow( pWindow->GetSDLWindow(), X, Y );

	m_AdjustmentX += ( CurrentX - X );
	m_AdjustmentY += ( CurrentY - Y );
#endif
}

void Mouse::GetPosition( int& X, int& Y, Window* const pWindow )
{
#if BUILD_WINDOWS_NO_SDL
	POINT ClientPos;
	GetCursorPos( &ClientPos );
	ScreenToClient( pWindow->GetHWnd(), &ClientPos );
	X = ClientPos.x;
	Y = ClientPos.y;
#endif
#if BUILD_SDL
	Unused( pWindow );
	SDL_GetMouseState( &X, &Y );
#endif
}

void Mouse::SetActive( bool Active )
{
	if( !Active && m_Active )
	{
#if BUILD_WINDOWS_NO_SDL
		m_pMouse->Unacquire();
#endif
#if BUILD_SDL
		SetRelativeMode( false );
#endif
	}
#if BUILD_SDL
	if( Active && !m_Active )
	{
		SetRelativeMode( true );
	}
#endif
	m_Active = Active;

	// Update the cursor as needed if we're now in inactive mode.
	if( !m_Active )
	{
		ShowCursor( m_AllowCursor );
	}
}

bool Mouse::IsActive() const
{
	return m_Active;
}

void Mouse::SetAllowCursor( const bool AllowCursor )
{
	if( AllowCursor == m_AllowCursor )
	{
		return;
	}

	m_AllowCursor = AllowCursor;

	if( !m_Active )
	{
		ShowCursor( m_AllowCursor );
	}
}

void Mouse::ShowCursor( const bool Show )
{
	if( m_CursorShown == Show )
	{
		return;
	}

	m_CursorShown = Show;

#if BUILD_WINDOWS_NO_SDL
	::ShowCursor( Show ? TRUE : FALSE );
#endif
#if BUILD_SDL
	SDL_ShowCursor( Show ? SDL_ENABLE : SDL_DISABLE );
#endif
}
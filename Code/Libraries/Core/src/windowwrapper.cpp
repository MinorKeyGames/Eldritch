#include "core.h"
#include "windowwrapper.h"
#include "mathcore.h"

#if BUILD_WINDOWS_NO_SDL
#include <Windows.h>
#endif

#if BUILD_SDL
#include "SDL2/SDL.h"
#endif

#include <memory.h>

Window::Window()
#if BUILD_WINDOWS_NO_SDL
:	m_hWnd( NULL )
,	m_hDC( NULL )
,	m_hInstance( NULL )
,	m_WndClassEx()
,	m_Style( 0 )
#endif
#if BUILD_SDL
:	m_Window( NULL )
#endif
,	m_Inited( false )
{
#if BUILD_WINDOWS_NO_SDL
	memset( &m_WndClassEx, 0, sizeof( WNDCLASSEX ) );
#endif
}

Window::~Window()
{
	if( m_Inited )
	{
		Free();
	}
}

#if BUILD_WINDOWS_NO_SDL
// TODO: Add some failure handling, and meaningful return values
int Window::Init( const char* Title, const char* ClassName, DWORD Style, DWORD ExStyle, int Width, int Height, HINSTANCE hInstance, WNDPROC WndProc /*= NULL*/, uint Icon /*= 0*/, int ScreenWidth /*= 0*/, int ScreenHeight /*= 0*/ )
{
	XTRACE_FUNCTION;

	if( m_Inited )
	{
		return 1;
	}

	m_hInstance = hInstance;

	m_Style = Style;

	m_WndClassEx.cbSize = sizeof( WNDCLASSEX );
	m_WndClassEx.style = CS_HREDRAW | CS_VREDRAW;
	m_WndClassEx.lpfnWndProc = ( WndProc != NULL ) ? WndProc : DefWindowProc;
	m_WndClassEx.hInstance = hInstance;
	m_WndClassEx.hIcon = ( Icon == 0 ? NULL : LoadIcon( hInstance, MAKEINTRESOURCE( Icon ) ) );
	m_WndClassEx.hCursor = (HCURSOR)LoadImage( NULL, MAKEINTRESOURCE(IDC_ARROW), IMAGE_CURSOR, 0, 0, LR_SHARED );
	m_WndClassEx.hbrBackground = (HBRUSH)GetStockObject( WHITE_BRUSH );
	m_WndClassEx.lpszMenuName = NULL;
	m_WndClassEx.lpszClassName = ClassName;
	m_WndClassEx.hIconSm = NULL;
	RegisterClassEx( &m_WndClassEx );

	// Adjust dimensions to accommodate borders
	RECT WindowRect;
	WindowRect.left = 0;
	WindowRect.right = Width;
	WindowRect.top = 0;
	WindowRect.bottom = Height;
	AdjustWindowRect( &WindowRect, Style, false );
	int AdjustedWidth = WindowRect.right - WindowRect.left;
	int AdjustedHeight = WindowRect.bottom - WindowRect.top;

	// Center window, or just position in top-left if it won't fit
	const int WindowX = Max( 0, ( ScreenWidth - AdjustedWidth ) >> 1 );
	const int WindowY = Max( 0, ( ScreenHeight - AdjustedHeight ) >> 1 );

	// Let calling code know what the window will look like.
	// We don't have an hWnd yet, but we can call the WindowProc directly.
	if( WndProc )
	{
		POINT	NotifySize;
		NotifySize.x	= AdjustedWidth;
		NotifySize.y	= AdjustedHeight;
		WPARAM	wParam	= 0;
		LPARAM	lParam	= reinterpret_cast<LPARAM>( &NotifySize );
		WndProc( 0, WM_NOTIFY_SIZE, wParam, lParam );
	}

	m_hWnd = CreateWindowEx( ExStyle, ClassName, Title, Style, WindowX, WindowY, AdjustedWidth, AdjustedHeight, NULL, NULL, m_hInstance, NULL );
	ASSERT( m_hWnd );

#if BUILD_DEV
	GetWindowRect( m_hWnd, &WindowRect );
	ASSERT( WindowRect.right - WindowRect.left == AdjustedWidth );
	ASSERT( WindowRect.bottom - WindowRect.top == AdjustedHeight );
#endif

	m_hDC = GetDC( m_hWnd );

	m_Inited = true;

	return 0;
}
#endif

#if BUILD_SDL
int Window::Init( const char* Title, uint Flags, int Width, int Height )
{
	XTRACE_FUNCTION;

	if( m_Inited )
	{
		return 1;
	}

	m_Window = SDL_CreateWindow( Title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Width, Height, Flags );
	ASSERT( m_Window );

	m_Inited = true;

	return 0;
}
#endif

#if BUILD_WINDOWS_NO_SDL
// TODO: Add some failure handling, and meaningful return values
int Window::Show( int nCmdShow )
{
	ShowWindow( m_hWnd, nCmdShow );
	SetWindowPos( m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE );	// Make sure we're on top
	UpdateWindow( m_hWnd );
	return 0;
}

// TODO: Add some failure handling, and meaningful return values
int Window::SetStyle( DWORD Style )
{
	if( m_Inited )
	{
		m_Style = Style;
		SetWindowLongPtr( m_hWnd, GWL_STYLE, Style );
		ShowWindow( m_hWnd, SW_SHOWNA );
		//UpdateWindow( m_hWnd );
		//SetWindowPos( m_hWnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW );
	}
	return 0;
}
#endif

#if BUILD_SDL
int Window::Show()
{
	SDL_ShowWindow( m_Window );
	return 0;
}
#endif

// TODO: Add some failure handling, and meaningful return values
int Window::Free()
{
#if BUILD_WINDOWS_NO_SDL
	char TempName[256];
	GetClassName( m_hWnd, TempName, 256 );
	if( m_Inited )
	{
		ReleaseDC( m_hWnd, m_hDC );
		CHECK( DestroyWindow( m_hWnd ) );
		CHECK( UnregisterClass( TempName, m_hInstance ) );
		m_Inited = false;
		return 0;
	}
	return 1;
#endif
#if BUILD_SDL
	if( m_Window )
	{
		SDL_DestroyWindow( m_Window );
		m_Inited = false;
		return 0;
	}
	else
	{
		return 1;
	}
#endif
}

// TODO: Add some failure handling, and meaningful return values
int Window::SetPosition( int x, int y )
{
	if( m_Inited )
	{
#if BUILD_WINDOWS_NO_SDL
		SetWindowPos( m_hWnd, HWND_NOTOPMOST, x, y, 0, 0, SWP_NOSIZE );
#endif
#if BUILD_SDL
		SDL_SetWindowPosition( m_Window, x, y );
#endif
	}
	return 0;
}

// TODO: Add some failure handling, and meaningful return values
int Window::SetSize( int Width, int Height, bool AdjustForStyle )
{
	if( !m_Inited )
	{
		return 0;
	}

#if BUILD_WINDOWS_NO_SDL
	RECT WindowRect;
	WindowRect.left		= 0;
	WindowRect.right	= Width;
	WindowRect.top		= 0;
	WindowRect.bottom	= Height;

	if( AdjustForStyle )
	{
		AdjustWindowRect( &WindowRect, m_Style, false );
		Width = WindowRect.right - WindowRect.left;
		Height = WindowRect.bottom - WindowRect.top;
	}

	{
		POINT	NotifySize;
		NotifySize.x	= Width;
		NotifySize.y	= Height;
		WPARAM	wParam	= 0;
		LPARAM	lParam	= reinterpret_cast<LPARAM>( &NotifySize );
		SendMessage( m_hWnd, WM_NOTIFY_SIZE, wParam, lParam );
	}

	// NOTE: This is just to update the size, it does *not* position window at (0, 0) (SWP_NOMOVE flag).
	const BOOL Success = SetWindowPos( m_hWnd, HWND_NOTOPMOST, 0, 0, Width, Height, SWP_NOMOVE );
	ASSERT( Success );
	Unused( Success );

#if BUILD_DEV
	GetWindowRect( m_hWnd, &WindowRect );
	ASSERT( WindowRect.right - WindowRect.left == Width );
	ASSERT( WindowRect.bottom - WindowRect.top == Height );
#endif
#endif // BUILD_WINDOWS_NO_SDL

#if BUILD_SDL
	Unused( AdjustForStyle );
	SDL_SetWindowSize( m_Window, Width, Height );
#endif

	return 0;
}

int Window::SetSize( int Width, int Height, int ScreenWidth, int ScreenHeight, bool AdjustForStyle )
{
	if( !m_Inited )
	{
		return 0;
	}

#if BUILD_WINDOWS_NO_SDL
	RECT WindowRect;
	WindowRect.left		= 0;
	WindowRect.right	= Width;
	WindowRect.top		= 0;
	WindowRect.bottom	= Height;

	if( AdjustForStyle )
	{
		AdjustWindowRect( &WindowRect, m_Style, false );
		Width = WindowRect.right - WindowRect.left;
		Height = WindowRect.bottom - WindowRect.top;
	}

	// Center window, or just position in top-left if it won't fit
	const int WindowX = Max( 0, ( ScreenWidth - Width ) >> 1 );
	const int WindowY = Max( 0, ( ScreenHeight - Height ) >> 1 );

	{
		POINT	NotifySize;
		NotifySize.x		= Width;
		NotifySize.y		= Height;
		WPARAM	wParam		= 0;
		LPARAM	lParam		= reinterpret_cast<LPARAM>( &NotifySize );
		SendMessage( m_hWnd, WM_NOTIFY_SIZE, wParam, lParam );
	}

	const BOOL Success = SetWindowPos( m_hWnd, HWND_NOTOPMOST, WindowX, WindowY, Width, Height, 0 );
	ASSERT( Success );
	Unused( Success );

#if BUILD_DEV
	GetWindowRect( m_hWnd, &WindowRect );
	ASSERT( WindowRect.right - WindowRect.left == Width );
	ASSERT( WindowRect.bottom - WindowRect.top == Height );
#endif
#endif // BUILD_WINDOWS_NO_SDL

#if BUILD_SDL
	Unused( AdjustForStyle );
	Unused( ScreenWidth );
	Unused( ScreenHeight );
	SDL_SetWindowSize( m_Window, Width, Height );
	SDL_SetWindowPosition( m_Window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED );
#endif

	return 0;
}

// TODO: Add some failure handling, and meaningful return values
int Window::SetTitleText( const char* Text )
{
	if( m_Inited )
	{
#if BUILD_WINDOWS_NO_SDL
		SetWindowText( m_hWnd, Text );
#endif
#if BUILD_SDL
		SDL_SetWindowTitle( m_Window, Text );
#endif
	}
	return 0;
}

bool Window::HasFocus() const
{
#if BUILD_WINDOWS_NO_SDL
	return GetForegroundWindow() == m_hWnd;
#endif
#if BUILD_SDL
	if( m_Inited )
	{
		const Uint32 WindowFlags = SDL_GetWindowFlags( m_Window );
		return ( WindowFlags & SDL_WINDOW_INPUT_FOCUS ) != 0;
	}
	else
	{
		return false;
	}
#endif
}

void Window::SetBordered( const bool Bordered )
{
#if BUILD_WINDOWS_NO_SDL
	const DWORD StyleFlags = Bordered ? ( WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX ) : WS_POPUP;
	SetStyle( StyleFlags );
#endif

#if BUILD_SDL
	SDL_SetWindowBordered( m_Window, Bordered ? SDL_TRUE : SDL_FALSE );
#endif
}

// NOTE: Used with SDL in place of managing display directly.
void Window::SetFullscreen( const bool Fullscreen )
{
#if BUILD_WINDOWS_NO_SDL
	Unused( Fullscreen );
#endif
#if BUILD_SDL
	// NOTE: SDL also has a SDL_WINDOW_FULLSCREEN_DESKTOP for borderless "fullscreen windowed" mode,
	// but I assume that would be redundant with the stuff I'm already doing to update window border.
	SDL_SetWindowDisplayMode( m_Window, NULL );
	SDL_SetWindowFullscreen( m_Window, Fullscreen ? SDL_WINDOW_FULLSCREEN : 0 );
#endif
}
#ifndef WINDOWWRAPPER_H
#define WINDOWWRAPPER_H

#if BUILD_WINDOWS_NO_SDL
#include <Windows.h>

#define WM_NOTIFY_SIZE	( WM_USER + 0x0010 )
// WM_NOTIFY_SIZE is used to notify a program of the intended size of a window prior to its creation.
// This allows the window's max size to be properly adjusted when WM_GETMINMAXINFO is received.
// wParam: Unused
// lParam: Pointer to a POINT representing the requested window's dimensions.

// Style notes:
// For basic window (thick border, resizing, min/max buttons): WS_OVERLAPPEDWINDOW
// For no resizing: WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX
// For no border: WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN
#endif

#if BUILD_SDL
struct SDL_Window;
#endif

class Window
{
public:
	Window();
	~Window();

#if BUILD_WINDOWS_NO_SDL
	int		Init( const char* Title, const char* ClassName, DWORD Style, DWORD ExStyle, int Width, int Height, HINSTANCE hInstance, WNDPROC WndProc = NULL, uint Icon = 0, int ScreenWidth = 0, int ScreenHeight = 0 );
#endif
#if BUILD_SDL
	// TODO SDL: If possible, unify interface to simplify calling code.
	int		Init( const char* Title, uint Flags, int Width, int Height );
#endif

#if BUILD_WINDOWS_NO_SDL
	int		Show( int nCmdShow = SW_SHOWNORMAL );
	int		SetStyle( DWORD Style );
#endif
#if BUILD_SDL
	// TODO SDL: If possible, unify interface to simplify calling code.
	int		Show();
#endif

	int		SetPosition( int x, int y );
	int		SetSize( int Width, int Height, bool AdjustForStyle );
	int		SetSize( int Width, int Height, int ScreenWidth, int ScreenHeight, bool AdjustForStyle );	// Resize and center window
	int		SetTitleText( const char* Text );
	int		Free();

	void	SetBordered( const bool Bordered );
	void	SetFullscreen( const bool Fullscreen );

#if BUILD_WINDOWS_NO_SDL
	HWND	GetHWnd() const { return m_hWnd; }
	HDC		GetHDC() const { return m_hDC; }
#endif
#if BUILD_SDL
	SDL_Window*	GetSDLWindow() const { return m_Window; }
#endif

	bool	HasFocus() const;

private:
#if BUILD_WINDOWS_NO_SDL
	HWND		m_hWnd;
	HDC			m_hDC;
	HINSTANCE	m_hInstance;
	WNDCLASSEX	m_WndClassEx;
	DWORD		m_Style;
#endif
#if BUILD_SDL
	SDL_Window*	m_Window;
#endif
	bool		m_Inited;
};

#endif // WINDOWWRAPPER_H
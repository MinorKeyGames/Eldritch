#ifndef FRAMEWORK3D_H
#define FRAMEWORK3D_H

#include "iwbeventobserver.h"

#if BUILD_WINDOWS_NO_SDL
#include <Windows.h>
#endif

class Display;
class Window;
class Keyboard;
class Mouse;
class IUIInputMap;
class Clock;
class AudiereDevice;
class UIManagerCommon;
class IRenderer;
class TargetManager;
class ShaderManager;
class IAudioSystem;
class WBEventManager;
class WBEvent;

class Framework3D : public IWBEventObserver
{
public:
	Framework3D();
	virtual ~Framework3D();

#if BUILD_WINDOWS_NO_SDL
	void			SetInitializeParameters( HINSTANCE hInstance, int nCmdShow );
#endif

	void			Main();

	// IWBEventObserver
	virtual void	HandleEvent( const WBEvent& Event );

	WBEventManager*	GetEventManager() const { return m_EventManager; }
	Display*		GetDisplay() const { return m_Display; }
	Window*			GetWindow() const { return m_Window; }
	Keyboard*		GetKeyboard() const { return m_Keyboard; }
	Mouse*			GetMouse() const { return m_Mouse; }
	IUIInputMap*	GetUIInputMap() const { return m_UIInputMap; }
	Clock*			GetClock() const { return m_Clock; }
	IRenderer*		GetRenderer() const { return m_Renderer; }
	IAudioSystem*	GetAudioSystem() const { return m_AudioSystem; }

	UIManagerCommon*	GetUIManager() const { return m_UIManager; }
	TargetManager*		GetTargetManager() const { return m_TargetManager; }

	// Where does this framework save files by default
	virtual SimpleString	GetSaveLoadPath();

	bool			HasFocus() const { return m_HasFocus; }

	void			TakeScreenshot() const;

#if BUILD_WINDOWS_NO_SDL
	// Hacky sack to allow windows larger than the screen size, which fixes ScreenToClient offset and UI problems.
	void			SetLastWindowSize( const POINT& WindowSize ) { m_LastWindowSize = WindowSize; }
	POINT			GetLastWindowSize() const { return m_LastWindowSize; }
#endif

	// Singleton accessor
	static Framework3D*	GetInstance();
	static void			SetInstance( Framework3D* const pFramework );

protected:
	virtual void	Initialize();
	virtual void	ShutDown();

	bool			HasRequestedExit();

	void			ResetRenderer();
	void			ConditionalRefreshDisplay();
	virtual void	RefreshDisplay( const bool Fullscreen, const uint DisplayWidth, const uint DisplayHeight );
	void			RefreshWindowSize();

	// Tick functions return true if the program should continue running.
	virtual bool	Tick();
	virtual bool	TickSim( float DeltaTime );
	virtual bool	TickGame( float DeltaTime );
	virtual void	OnUnpaused();
	virtual void	TickDevices();
	virtual bool	TickInput( float DeltaTime );
	virtual void	TickPausedInput();
	virtual void	TickRender();
	virtual void	DropRender();

#if BUILD_SDL
	virtual bool	TickSDLEvents();
#endif

	// Initialization hooks for framework implementations
	virtual void	CreateSplashWindow( const uint WindowIcon, const char* const Title );
	virtual void	GetInitialDisplaySize( uint& DisplayWidth, uint& DisplayHeight );
	virtual void	GetInitialWindowTitle( SimpleString& WindowTitle );
	virtual void	GetInitialWindowIcon( uint& WindowIcon );
	virtual void	GetUIManagerDefinitionName( SimpleString& DefinitionName );
	virtual void	InitializeUIInputMap();
	virtual bool	UseClassicTargetManager() { return true; }
	virtual bool	ShowWindowASAP() { return true; }
	virtual void	InitializeAudioSystem();
	virtual void	InitializeRender();
	virtual void	ToggleFullscreen();
	virtual void	SetResolution( const uint DisplayWidth, const uint DisplayHeight );

	static void		RendererRestoreDeviceCallback( void* pVoid );

#if BUILD_WINDOWS_NO_SDL
	static LRESULT CALLBACK WindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
#endif

	WBEventManager*			m_EventManager;	// Event manager for framework only, not for game.
	Display*				m_Display;
	Window*					m_Window;
	Window*					m_SplashWindow;
	Keyboard*				m_Keyboard;
	Mouse*					m_Mouse;
	Clock*					m_Clock;
	IAudioSystem*			m_AudioSystem;
	UIManagerCommon*		m_UIManager;

	IUIInputMap*			m_UIInputMap;

	IRenderer*				m_Renderer;
	TargetManager*			m_TargetManager;

	bool					m_HasFocus;
	float					m_FrameTimeAccumulator;

	bool					m_UseFixedFrameTime;
	float					m_FixedFrameTime;
	float					m_FrameTimeLimit;

	bool					m_DoVideoCapture;
	float					m_VideoCaptureFixedFrameTime;

#if BUILD_WINDOWS_NO_SDL
	HINSTANCE				m_hInstance;
	int						m_CmdShow;
	// Hacky sack, see above
	POINT					m_LastWindowSize;
#endif

	bool					m_IsInitializing;
	bool					m_IsShuttingDown;
};

#endif
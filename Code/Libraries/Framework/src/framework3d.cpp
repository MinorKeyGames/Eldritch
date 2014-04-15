#include "core.h"
#include "framework3d.h"

#include "keyboard.h"
#include "mouse.h"
#include "display.h"
#include "windowwrapper.h"
#include "mathcore.h"
#include "mathfunc.h"
#include "clock.h"
#include "stringmanager.h"
#include "3d.h"
#include "targetmanager.h"
#include "configmanager.h"
#include "filestream.h"
#include "packstream.h"
#include "uimanagerframework.h"
#include "uistack.h"
#include "uiinputmapframework.h"
#include "iaudiosystem.h"
#include "uiscreen.h"
#include "irenderer.h"
#include "file.h"
#include "wbeventmanager.h"
#include "surface.h"

#if BUILD_WINDOWS
#include "console.h"
#include "timedate.h"
#include "exceptionuploadlog.h"
#endif

#if BUILD_SDL
#include "SDL2/SDL.h"
#endif

// I always want *the option* to get logs in final, but not dev.
// (Actual behavior is controlled by a config var.)
#define UPLOADLOG ( ( 0 || BUILD_FINAL ) && BUILD_WINDOWS )

// Singleton accessor
static Framework3D* gSingletonFramework = NULL;

/*static*/ Framework3D* Framework3D::GetInstance()
{
	return gSingletonFramework;
}

/*static*/ void Framework3D::SetInstance( Framework3D* const pFramework )
{
	ASSERT( gSingletonFramework == NULL );
	gSingletonFramework = pFramework;
}

Framework3D::Framework3D()
:	m_EventManager( NULL )
,	m_Display( NULL )
,	m_Window( NULL )
,	m_SplashWindow( NULL )
,	m_Keyboard( NULL )
,	m_Mouse( NULL )
,	m_Clock( NULL )
,	m_AudioSystem( NULL )
,	m_UIManager( NULL )
,	m_UIInputMap( NULL )
,	m_Renderer( NULL )
,	m_TargetManager( NULL )
,	m_HasFocus( false )
,	m_FrameTimeAccumulator( 0.0f )
,	m_UseFixedFrameTime( false )
,	m_FixedFrameTime( 0.0f )
,	m_FrameTimeLimit( 0.0f )
,	m_DoVideoCapture( false )
,	m_VideoCaptureFixedFrameTime( 0.0f )
#if BUILD_WINDOWS_NO_SDL
,	m_hInstance( NULL )
,	m_CmdShow( 0 )
,	m_LastWindowSize()
#endif
,	m_IsInitializing( false )
,	m_IsShuttingDown( false )
{
	m_EventManager = new WBEventManager;
}

Framework3D::~Framework3D()
{
	m_EventManager->Destroy();
	m_EventManager = NULL;
}

#if BUILD_WINDOWS_NO_SDL
void Framework3D::SetInitializeParameters( HINSTANCE hInstance, int nCmdShow )
{
	m_hInstance	= hInstance;
	m_CmdShow	= nCmdShow;
}
#endif

void Framework3D::Main()
{
	XTRACE_FUNCTION;

	Initialize();
	while( Tick() );
	ShutDown();
}

/*virtual*/ void Framework3D::GetInitialDisplaySize( uint& DisplayWidth, uint& DisplayHeight )
{
	ASSERT( m_Display );
	DisplayWidth	= m_Display->m_Width;
	DisplayHeight	= m_Display->m_Height;
}

/*virtual*/ void Framework3D::GetInitialWindowTitle( SimpleString& WindowTitle )
{
	WindowTitle = "UNTITLED";
}

/*virtual*/ void Framework3D::GetInitialWindowIcon( uint& WindowIcon )
{
	WindowIcon = 101;
}

/*virtual*/ void Framework3D::InitializeAudioSystem()
{
	WARN;
}

/*virtual*/ void Framework3D::InitializeRender()
{
}

/*virtual*/ void Framework3D::GetUIManagerDefinitionName( SimpleString& DefinitionName )
{
	DefinitionName = "Framework3D";
}

/*virtual*/ void Framework3D::InitializeUIInputMap()
{
	m_UIInputMap = new UIInputMapFramework( this );
}

/*virtual*/ void Framework3D::Initialize()
{
	XTRACE_FUNCTION;

	m_IsInitializing = true;

#if BUILD_SDL
	const int Error = SDL_Init( SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS | SDL_INIT_NOPARACHUTE );
	ASSERT( 0 == Error );
	Unused( Error );
	if( 0 != Error )
	{
		PRINTF( "SDL_Init: %s\n", SDL_GetError() );
	}

	SDL_DisableScreenSaver();
#endif

	STATICHASH( Framework );

#if BUILD_WINDOWS
#if BUILD_FINAL
	STATICHASH( ShowConsole );
	const bool ShowConsole = ConfigManager::GetBool( sShowConsole, false, sFramework );
	if( ShowConsole )
#endif
	{
		Console::GetInstance()->SetPos( 0, 0 );
	}
#endif

	STATICHASH( UseRandomSeed );
	const bool UseRandomSeed = ConfigManager::GetBool( sUseRandomSeed, false, sFramework );

	STATICHASH( RandomSeed );
	const int RandomSeed = ConfigManager::GetInt( sRandomSeed, 0, sFramework );

	if( UseRandomSeed )
	{
		Math::SeedGenerator( RandomSeed );
	}
	else
	{
		Math::SeedGenerator();
	}

	STATICHASH( UseFixedFrameTime );
	m_UseFixedFrameTime	= ConfigManager::GetBool( sUseFixedFrameTime, true, sFramework );

	STATICHASH( FixedFrameTime );
	m_FixedFrameTime = ConfigManager::GetFloat( sFixedFrameTime, 1.0f / 60.0f, sFramework );

	STATICHASH( FramesLimit );
	const int FramesLimit = ConfigManager::GetInt( sFramesLimit, 5, sFramework );
	m_FrameTimeLimit = m_FixedFrameTime * static_cast<float>( FramesLimit );

	STATICHASH( DoVideoCapture );
	m_DoVideoCapture = ConfigManager::GetBool( sDoVideoCapture, false, sFramework );

	STATICHASH( VideoCaptureFixedFrameTime );
	m_VideoCaptureFixedFrameTime = ConfigManager::GetFloat( sVideoCaptureFixedFrameTime, 1.0f / 30.0f, sFramework );

	uint DisplayWidth = 0;
	uint DisplayHeight = 0;
	SimpleString WindowTitle;

	// Loads display parameters from config, so GetInitialDisplaySize can use that.
	m_Display = new Display;

	// Make sure that we use a supported resolution regardless of what the config file said.
	const SDisplayMode BestDisplayMode = m_Display->GetBestDisplayMode( m_Display->m_Width, m_Display->m_Height );
	m_Display->SetResolution( BestDisplayMode.Width, BestDisplayMode.Height );

	uint WindowIcon = 0;
	GetInitialWindowIcon( WindowIcon );
	GetInitialDisplaySize( DisplayWidth, DisplayHeight );
	GetInitialWindowTitle( WindowTitle );

	CreateSplashWindow( WindowIcon, WindowTitle.CStr() );

	m_Window = new Window;

#if BUILD_WINDOWS_NO_SDL
	DWORD WindowStyle = 0;
	if( m_Display->m_Fullscreen ||
		(	m_Display->m_ScreenWidth == m_Display->m_Width &&
			m_Display->m_ScreenHeight == m_Display->m_Height ) )
	{
		WindowStyle = WS_POPUP;
	}
	else
	{
		WindowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	}

	m_Window->Init( WindowTitle.CStr(), "Class1", WindowStyle, 0, DisplayWidth, DisplayHeight, m_hInstance, WindowProc, WindowIcon, m_Display->m_ScreenWidth, m_Display->m_ScreenHeight );
#elif BUILD_SDL
	uint WindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN;
	if( m_Display->m_Fullscreen ||
		(	m_Display->m_ScreenWidth == m_Display->m_Width &&
			m_Display->m_ScreenHeight == m_Display->m_Height ) )
	{
		WindowFlags |= SDL_WINDOW_BORDERLESS;
	}
	// TODO SDL: Unify interface?
	m_Window->Init( WindowTitle.CStr(), WindowFlags, DisplayWidth, DisplayHeight );

	STATICHASH( IconImage );
	const char* const	pIconImage	= ConfigManager::GetString( sIconImage, NULL, sFramework );
	ASSERT( pIconImage );
	const Surface		IconSurface	= Surface( PackStream( pIconImage ), Surface::ESFT_BMP );
	SDL_SetWindowIcon( m_Window->GetSDLWindow(), IconSurface.GetSDLSurface() );
#endif

	m_Window->SetFullscreen( m_Display->m_Fullscreen );
	if( m_Display->m_Fullscreen )
	{
		m_Window->SetPosition( 0, 0 );
	}

	m_Clock = new Clock;

	XTRACE_BEGIN( InitializeDevices );
		m_Keyboard	= new Keyboard;
#if BUILD_WINDOWS_NO_SDL
		m_Mouse		= new Mouse( m_hInstance, m_Window->GetHWnd() );
#elif BUILD_SDL
		m_Mouse		= new Mouse( m_Window );
#endif
	XTRACE_END;

	InitializeUIInputMap();

	XTRACE_BEGIN( InitializeAudioSystem );
		InitializeAudioSystem();
	XTRACE_END;

	XTRACE_BEGIN( InitializeRenderer );
#if BUILD_WINDOWS_NO_SDL
		STATICHASH( OpenGL );
		const bool OpenGL = ConfigManager::GetBool( sOpenGL );
		if( OpenGL )
#endif
		{
			PRINTF( "Using OpenGL renderer.\n" );
			m_Renderer = CreateGL2Renderer( m_Window );
		}
#if BUILD_WINDOWS_NO_SDL
		else
		{
			PRINTF( "Using Direct3D renderer.\n" );
			m_Renderer = CreateD3D9Renderer( m_Window->GetHWnd(), m_Display->m_Fullscreen );
		}
#endif

		IRenderer::SRestoreDeviceCallback Callback;
		Callback.m_Callback = &Framework3D::RendererRestoreDeviceCallback;
		Callback.m_Void = this;
		m_Renderer->SetRestoreDeviceCallback( Callback );

		InitializeRender();
		m_Renderer->Initialize();
		m_Renderer->SetDisplay( m_Display );
		m_Renderer->SetClock( m_Clock );
	XTRACE_END;

	if( ShowWindowASAP() )
	{
		SafeDelete( m_SplashWindow );
#if BUILD_WINDOWS_NO_SDL
		m_Window->Show( m_CmdShow );
#elif BUILD_SDL
		m_Window->Show();
#endif

		// Reattach GL context if needed.
		m_Renderer->Refresh();
	}

	if( UseClassicTargetManager() )
	{
		m_TargetManager = new TargetManager( m_Renderer, DisplayWidth, DisplayHeight );
	}

	XTRACE_BEGIN( InitializeUI );
		SimpleString UIManagerDefinitionName;
		GetUIManagerDefinitionName( UIManagerDefinitionName );

		m_UIManager = new UIManagerFramework( this );
		m_UIManager->InitializeFromDefinition( UIManagerDefinitionName );
		m_UIManager->GetUIStack()->SetFadeOverlay( m_UIManager->GetScreen( "Fade" ) );
		UIScreen::UpdateMouseButtonsSwapped();
	XTRACE_END;

	m_IsInitializing = false;
}

void Framework3D::CreateSplashWindow( const uint WindowIcon, const char* const Title )
{
	XTRACE_FUNCTION;

	STATICHASH( Framework );
	STATICHASH( SplashImage );
	const char* const	pSplashImage		= ConfigManager::GetString( sSplashImage, NULL, sFramework );
	if( !pSplashImage )
	{
		return;
	}

	const Surface		SplashSurface		= Surface( PackStream( pSplashImage ), Surface::ESFT_BMP );
	const int			SplashWindowWidth	= SplashSurface.GetWidth();
	const int			SplashWindowHeight	= SplashSurface.GetHeight();

	ASSERT( !m_SplashWindow );
	m_SplashWindow = new Window;

#if BUILD_WINDOWS_NO_SDL
	const DWORD			WindowStyle			= WS_POPUP;
	const DWORD			WindowExStyle		= WS_EX_TOOLWINDOW;	// Prevents this window appearing in the taskbar
	const int			ScreenWidth			= m_Display->m_Fullscreen ? m_Display->m_Width : m_Display->m_ScreenWidth;
	const int			ScreenHeight		= m_Display->m_Fullscreen ? m_Display->m_Height : m_Display->m_ScreenHeight;

	m_SplashWindow->Init( Title, "SplashWindowClass", WindowStyle, WindowExStyle, SplashWindowWidth, SplashWindowHeight, m_hInstance, NULL, WindowIcon, ScreenWidth, ScreenHeight );

	// The window needs to be shown before we can blit to it.
	m_SplashWindow->Show( m_CmdShow );
#endif
#if BUILD_SDL
	// TODO SDL: Unify interface?
	const uint Flags = SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS;
	m_SplashWindow->Init( Title, Flags, SplashWindowWidth, SplashWindowHeight );

	// Load icon from package file instead of compiled resource.
	Unused( WindowIcon );
	STATICHASH( IconImage );
	const char* const	pIconImage	= ConfigManager::GetString( sIconImage, NULL, sFramework );
	ASSERT( pIconImage );
	const Surface		IconSurface	= Surface( PackStream( pIconImage ), Surface::ESFT_BMP );
	SDL_SetWindowIcon( m_SplashWindow->GetSDLWindow(), IconSurface.GetSDLSurface() );
#endif

	SplashSurface.BlitToWindow( m_SplashWindow );
}

/*virtual*/ void Framework3D::ShutDown()
{
	XTRACE_FUNCTION;

	m_IsShuttingDown = true;

	PRINTF( "Shutting down framework\n" );

#if UPLOADLOG
	STATICHASH( Framework );
	STATICHASH( SendLogsOnQuit );
	if( ConfigManager::GetBool( sSendLogsOnQuit, false, sFramework ) )
	{
		ExceptionUploadLog::UploadLog();
	}
#endif

#if BUILD_WINDOWS
	Console::DeleteInstance();
#endif

	SafeDelete( m_Display );
	SafeDelete( m_SplashWindow );
	SafeDelete( m_Clock );
	SafeDelete( m_Keyboard );
	SafeDelete( m_Mouse );
	SafeDelete( m_AudioSystem );
	SafeDelete( m_UIManager );
	SafeDelete( m_UIInputMap );

	SafeDelete( m_TargetManager );
	SafeDelete( m_Renderer );
	SafeDelete( m_Window );			// Destroy after renderer, else I get GL errors with SDL.

	StringManager::DeleteInstance();
	PrintManager::DeleteInstance();
	ConfigManager::DeleteInstance();

#if DO_PROFILING
	Profiler::GetInstance()->Dump( FileStream( "profiler.txt", FileStream::EFM_Write ) );
	Profiler::DeleteInstance();
#endif

#if BUILD_SDL
	SDL_Quit();
#endif

	m_IsShuttingDown = false;
}

/*virtual*/ bool Framework3D::Tick()
{
	XTRACE_FUNCTION;
	PROFILE_FUNCTION;

	bool DoNextTick = true;

	if( HasRequestedExit() )
	{
		DoNextTick = false;
	}

	m_HasFocus = m_Window->HasFocus();

	StringManager::FlushStrings( StringManager::ESL_Transient );

	m_Clock->Tick( m_UIManager->GetUIStack()->PausesGame(), false );

	m_EventManager->Tick();

	if( m_DoVideoCapture )
	{
		// One sim tick, one render tick
		DoNextTick &= TickSim( m_VideoCaptureFixedFrameTime );
		TickRender();
	}
	else if( m_UseFixedFrameTime )
	{
		m_FrameTimeAccumulator += m_Clock->GetMachineDeltaTime();

		if( m_FrameTimeAccumulator > m_FrameTimeLimit )
		{
			// We've hit a very long frame, maybe we were sitting in the debugger.
			// Don't bother simming for this duration, it will just cause more hitches.
			m_FrameTimeAccumulator = 0.0f;
		}

		const int NumSimTicks = static_cast<int>( m_FrameTimeAccumulator / m_FixedFrameTime );
		const int LastSimTick = NumSimTicks - 1;
		for( int SimTickIndex = 0; SimTickIndex <= LastSimTick && DoNextTick; ++SimTickIndex )
		{
			ASSERT( m_FrameTimeAccumulator >= m_FixedFrameTime );
			m_FrameTimeAccumulator -= m_FixedFrameTime;

			DoNextTick &= TickSim( m_FixedFrameTime );

			if( SimTickIndex == LastSimTick )
			{
				TickRender();
			}
			else
			{
				DropRender();
			}
		}
	}
	else
	{
		// One sim tick, one render tick
		DoNextTick &= TickSim( m_Clock->GetMachineDeltaTime() );
		TickRender();
	}

#if BUILD_WINDOWS_NO_SDL
	{
		XTRACE_NAMED( SwitchToThread );	// Crashing within this trace name would typically indicate a crash in another thread.
		const BOOL SwitchedToThread = SwitchToThread();
		if( 0 == SwitchedToThread )
		{
			// Prevent hogging the CPU, maybe?
			Sleep( 0 );
		}
	}
#endif
#if BUILD_SDL
	{
		SDL_Delay( 0 );
	}
#endif

	return DoNextTick;
}

/*virtual*/ bool Framework3D::TickSim( float DeltaTime )
{
	XTRACE_FUNCTION;

	const bool WasPaused = m_UIManager->GetUIStack()->PausesGame();
	m_Clock->GameTick( WasPaused, DeltaTime );

#if BUILD_SDL
	if( !TickSDLEvents() )
	{
		return false;
	}
#endif

	TickDevices();

#if !BUILD_STEAM
	// Bit of a hack... And would conflict with Steam's F12 key function.
	if( m_Keyboard->OnRise( Keyboard::EB_F12 ) )
	{
		TakeScreenshot();
	}
#endif

	// Man I'm really ruining the notion of "TickInput" huh.
	if(	( m_Keyboard->IsHigh( Keyboard::EB_LeftAlt ) ||
		  m_Keyboard->IsHigh( Keyboard::EB_RightAlt ) ) &&
		m_Keyboard->OnRise( Keyboard::EB_Enter ) )
	{
		ToggleFullscreen();
	}

#if !BUILD_MAC
	// This causes problems on Mac, but is essential on Linux and maybe Windows.
	if( m_Mouse->IsActive() && m_Window->HasFocus() )
	{
		// Fix invisible cursor from affecting other windows
		m_Mouse->SetPosition( m_Display->m_Width / 2, m_Display->m_Height / 2, m_Window );
	}
#endif

	XTRACE_BEGIN( TickUI );
		m_UIManager->ProcessEvents();
		m_UIManager->GetUIStack()->Tick( DeltaTime, DeltaTime, false );
	XTRACE_END;

	const bool Unpaused = WasPaused && !m_UIManager->GetUIStack()->PausesGame();
	if( Unpaused )
	{
		OnUnpaused();
		m_Clock->GameTick( false, DeltaTime );
	}

	if( m_HasFocus )
	{
		if( WasPaused )
		{
			TickPausedInput();
		}
		else
		{
			if( !TickInput( DeltaTime ) )
			{
				return false;
			}
		}
	}

	if( !WasPaused )
	{
		TickGame( DeltaTime );
	}

	XTRACE_BEGIN( TickAudio );
		m_AudioSystem->Tick( DeltaTime, WasPaused );
	XTRACE_END;

	return true;
}

/*virtual*/ void Framework3D::OnUnpaused()
{
	XTRACE_FUNCTION;

	// HACK: If we've just unpaused, tick devices again so we ignore rising inputs on the next frame.
	TickDevices();
}

/*virtual*/ void Framework3D::TickDevices()
{
	XTRACE_FUNCTION;

	m_Keyboard->Tick( 0.0f );
	m_Mouse->Tick( 0.0f );
}

#if BUILD_SDL
// Replacement for WindowProc, basically.
/*virtual*/ bool Framework3D::TickSDLEvents()
{
	XTRACE_FUNCTION;

	SDL_Event Event;
	while( SDL_PollEvent( &Event ) )
	{
		if( Event.type == SDL_QUIT )
		{
			return false;
		}

		if( Event.type == SDL_MOUSEWHEEL )
		{
			const Sint32 WheelDeltaY = Event.wheel.y;
			if( WheelDeltaY > 0 )
			{
				ASSERT( m_Mouse );
				m_Mouse->Buffer( Mouse::EB_WheelUp );
			}
			else if( WheelDeltaY < 0 )
			{
				ASSERT( m_Mouse );
				m_Mouse->Buffer( Mouse::EB_WheelDown );
			}
		}

		if( Event.type == SDL_MOUSEMOTION )
		{
			m_Mouse->NotifyMouseMoved();
		}
	}

	return true;
}
#endif

/*virtual*/ bool Framework3D::TickGame( float DeltaTime )
{
	Unused( DeltaTime );
	return true;
}

/*virtual*/ bool Framework3D::TickInput( float DeltaTime )
{
	Unused( DeltaTime );
	return true;
}

/*virtual*/ void Framework3D::TickPausedInput()
{
}

/*virtual*/ void Framework3D::TickRender()
{
	XTRACE_FUNCTION;

	m_UIManager->GetUIStack()->Render();
	m_Renderer->Tick();
}

/*virtual*/ void Framework3D::DropRender()
{
	m_Renderer->FlushBuckets();
}

void Framework3D::TakeScreenshot() const
{
	// TODO PORT LATER: Support screenshots on other platforms.
#if BUILD_WINDOWS
	if( !FileUtil::PathExists( "Screens" ) )
	{
		FileUtil::MakePath( "Screens" );
	}

	const SimpleString TimeDateString = TimeDate::GetTimeDateString();
	const SimpleString ScreenshotName = SimpleString::PrintF( "Screens/%s.png", TimeDateString.CStr() );
	m_Renderer->SaveScreenshot( ScreenshotName );
#endif
}

/*virtual*/ void Framework3D::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	STATIC_HASHED_STRING( QuitGame );
	STATIC_HASHED_STRING( ResetRenderer );
	STATIC_HASHED_STRING( ConditionalRefreshDisplay );
	STATIC_HASHED_STRING( RefreshDisplay );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sQuitGame )
	{
#if BUILD_WINDOWS_NO_SDL
		XTRACE_NAMED( PostQuitMessage );
		PostQuitMessage( 0 );
#endif
#if BUILD_SDL
		SDL_Event QuitEvent;
		QuitEvent.type = SDL_QUIT;
		const int Success = SDL_PushEvent( &QuitEvent );
		ASSERT( Success );
		Unused( Success );
#endif
	}
	else if( EventName == sResetRenderer )
	{
		ResetRenderer();
	}
	else if( EventName == sConditionalRefreshDisplay )
	{
		ConditionalRefreshDisplay();
	}
	else if( EventName == sRefreshDisplay )
	{
		RefreshDisplay( m_Display->m_Fullscreen, m_Display->m_Width, m_Display->m_Height );
	}
}

void Framework3D::RefreshWindowSize()
{
	if( m_Display->m_Fullscreen )
	{
		m_Window->SetBordered( false );
		m_Window->SetSize( m_Display->m_Width, m_Display->m_Height, true );
		m_Window->SetPosition( 0, 0 );
	}
	else
	{
		// If display size is same as desktop size, and we're not fullscreen, automatically switch to a borderless window.
		if( m_Display->m_ScreenWidth == m_Display->m_Width &&
			m_Display->m_ScreenHeight == m_Display->m_Height )
		{
			m_Window->SetBordered( false );
		}
		else
		{
			m_Window->SetBordered( true );
		}

		m_Window->SetSize( m_Display->m_Width, m_Display->m_Height, m_Display->m_ScreenWidth, m_Display->m_ScreenHeight, true );
	}
}

/*virtual*/ void Framework3D::ToggleFullscreen()
{
	bool NewFullScreen = !m_Display->m_Fullscreen;

	// HACKHACK: At least for now, Linux doesn't do fullscreen at non desktop resolutions well.
	// So switch to desktop res when going fullscreen.
#if BUILD_LINUX
	if( NewFullScreen )
	{
		SetResolution( m_Display->m_ScreenWidth, m_Display->m_ScreenHeight );
	}
#endif

	PRINTF( "Framework3D::ToggleFullscreen %s\n", NewFullScreen ? "true" : "false" );

	if( !NewFullScreen &&
		(	m_Display->m_Width > m_Display->m_ScreenWidth ||
			m_Display->m_Height > m_Display->m_ScreenHeight ) )
	{
		PRINTF( "Automatically setting resolution to screen size.\n" );
		SetResolution( m_Display->m_ScreenWidth, m_Display->m_ScreenHeight );
	}

	m_Display->SetFullScreen( NewFullScreen );
	m_Display->UpdateDisplay();

	m_Window->SetFullscreen( NewFullScreen );

	m_Renderer->SetFullScreen( NewFullScreen );
	m_Renderer->Reset();

	// Set size after resetting renderer to be sure screen-to-client transform is correct
	RefreshWindowSize();
}

/*virtual*/ void Framework3D::SetResolution( const uint DisplayWidth, const uint DisplayHeight )
{
	// HACKHACK: At least for now, Linux doesn't do fullscreen at non desktop resolutions well.
	// So if we're in fullscreen mode, promote all resolution changes to desktop resolution.
#if BUILD_LINUX
	if( m_Display->m_Fullscreen &&
		( DisplayWidth != m_Display->m_ScreenWidth || DisplayHeight != m_Display->m_ScreenHeight ) )
	{
		SetResolution( m_Display->m_ScreenWidth, m_Display->m_ScreenHeight );
		return;
	}
#endif

	PRINTF( "Framework3D::SetResolution %d %d\n", DisplayWidth, DisplayHeight );

	ASSERT( DisplayWidth <= m_Display->m_ScreenWidth || m_Display->m_Fullscreen );
	ASSERT( DisplayHeight <= m_Display->m_ScreenHeight || m_Display->m_Fullscreen );

	m_Display->SetResolution( DisplayWidth, DisplayHeight );
	m_Display->UpdateDisplay();

	// HACKHACK: SDL
	m_Window->SetFullscreen( false );

	RefreshWindowSize();

	// HACKHACK: SDL
	m_Window->SetFullscreen( m_Display->m_Fullscreen );

	m_Renderer->Reset();

	if( m_TargetManager )
	{
		m_TargetManager->ReleaseTargets();
		m_TargetManager->CreateTargets( m_Display->m_Width, m_Display->m_Height );
	}

	m_UIManager->Reinitialize();
}

void Framework3D::ResetRenderer()
{
	XTRACE_FUNCTION;

	if( m_Renderer->Reset() )
	{
		// TODO: RefreshDisplay also resets the renderer--could that be problematic?
		WB_MAKE_EVENT( RefreshDisplay, NULL );
		WB_DISPATCH_EVENT( GetEventManager(), RefreshDisplay, this );
	}
	else
	{
		// Defer the command
		WB_MAKE_EVENT( ResetRenderer, NULL );
		WB_QUEUE_EVENT( GetEventManager(), ResetRenderer, this );
	}
}

void Framework3D::ConditionalRefreshDisplay()
{
	XTRACE_FUNCTION;

	// We shouldn't mess with this during initialization!
	if( m_IsInitializing || m_IsShuttingDown )
	{
		return;
	}

	// Resync display after changing config vars out from under it.
	STATICHASH( Fullscreen );
	const bool Fullscreen		= ConfigManager::GetBool( sFullscreen );

	STATICHASH( DisplayWidth );
	const uint DisplayWidth		= ConfigManager::GetInt( sDisplayWidth );

	STATICHASH( DisplayHeight );
	const uint DisplayHeight	= ConfigManager::GetInt( sDisplayHeight );

	ASSERT( m_Display );
	if( m_Display->NeedsUpdate()					||
		m_Display->m_Fullscreen	!= Fullscreen		||
		m_Display->m_Width		!= DisplayWidth		||
		m_Display->m_Height		!= DisplayHeight	)
	{
		RefreshDisplay( Fullscreen, DisplayWidth, DisplayHeight );
	}
}

/*virtual*/ void Framework3D::RefreshDisplay( const bool Fullscreen, const uint DisplayWidth, const uint DisplayHeight )
{
	XTRACE_FUNCTION;

	PRINTF( "Framework3D::RefreshDisplay %s %d %d\n", Fullscreen ? "true" : "false", DisplayWidth, DisplayHeight );

	ASSERT( m_Display );
	m_Display->SetFullScreen( Fullscreen );
	m_Display->SetResolution( DisplayWidth, DisplayHeight );
	m_Display->UpdateDisplay();

	// HACKHACK: SDL
	m_Window->SetFullscreen( false );

	RefreshWindowSize();

	// HACKHACK: SDL
	m_Window->SetFullscreen( Fullscreen );

	ASSERT( m_Renderer );
	m_Renderer->SetFullScreen( m_Display->m_Fullscreen );
	m_Renderer->Reset();

	if( m_TargetManager )
	{
		m_TargetManager->ReleaseTargets();
		m_TargetManager->CreateTargets( m_Display->m_Width, m_Display->m_Height );
	}

	ASSERT( m_UIManager );
	m_UIManager->Reinitialize();
}

// By default, just save to the working directory.
/*virtual*/ SimpleString Framework3D::GetSaveLoadPath()
{
	return SimpleString( "./" );
}

bool Framework3D::HasRequestedExit()
{
#if BUILD_WINDOWS_NO_SDL
	MSG Msg;
	while( PeekMessage( &Msg, NULL, 0, 0, PM_REMOVE ) )
	{
		// WM_QUIT is not associated with a window and must be processed here, not WndProc
		if( Msg.message == WM_QUIT )
		{
			return true;
		}

		TranslateMessage( &Msg );
		DispatchMessage( &Msg );
	}
#endif

#if BUILD_SDL && BUILD_WINDOWS
	// SDL overrides default Alt+F4 behavior, so I handle it myself.
	if( m_Keyboard->IsHigh( Keyboard::EB_Alt ) && m_Keyboard->OnRise( Keyboard::EB_F4 ) )
	{
		return true;
	}
#endif

	return false;
}

/*static*/ void Framework3D::RendererRestoreDeviceCallback( void* pVoid )
{
	Framework3D* pFramework3D = static_cast<Framework3D*>( pVoid );
	if( pFramework3D )
	{
		WB_MAKE_EVENT( ResetRenderer, NULL );
		WB_DISPATCH_EVENT( pFramework3D->GetEventManager(), ResetRenderer, pFramework3D );
	}
}

#if BUILD_WINDOWS_NO_SDL
LRESULT CALLBACK Framework3D::WindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_CLOSE:
	case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}

	case WM_SYSCOMMAND:
		{
			// Ignored so that releasing Alt doesn't cause game to hang
			if( wParam == SC_KEYMENU )
			{
				return 0;
			}
			break;
		}

	case WM_ACTIVATE:
		{
			if( wParam != WA_INACTIVE )
			{
				// Restore the resolution if it was changed from under us
				ASSERT( gSingletonFramework );
				WB_MAKE_EVENT( ConditionalRefreshDisplay, NULL );
				WB_DISPATCH_EVENT( gSingletonFramework->GetEventManager(), ConditionalRefreshDisplay, gSingletonFramework );
			}
			break;
		}

	case WM_NOTIFY_SIZE:
		{
			POINT* const	pSize = reinterpret_cast<POINT*>( lParam );

			ASSERT( gSingletonFramework );
			gSingletonFramework->SetLastWindowSize( *pSize );

			return 0;
		}

	case WM_GETMINMAXINFO:
		{
			MINMAXINFO* const pInfo = reinterpret_cast<MINMAXINFO*>( lParam );
			ASSERT( pInfo );

			ASSERT( gSingletonFramework );
			POINT LastWindowSize = gSingletonFramework->GetLastWindowSize();

			pInfo->ptMaxTrackSize.x = Max( pInfo->ptMaxTrackSize.x, LastWindowSize.x );
			pInfo->ptMaxTrackSize.y = Max( pInfo->ptMaxTrackSize.y, LastWindowSize.y );

			return 0;
		}

	case WM_MOUSEWHEEL:
		{
			const short WheelDelta = GET_WHEEL_DELTA_WPARAM( wParam );
			if( WheelDelta > 0 )
			{
				ASSERT( gSingletonFramework );
				gSingletonFramework->m_Mouse->Buffer( Mouse::EB_WheelUp );
			}
			else if( WheelDelta < 0 )
			{
				ASSERT( gSingletonFramework );
				gSingletonFramework->m_Mouse->Buffer( Mouse::EB_WheelDown );
			}
		}

	case WM_MOUSEMOVE:
		{
			ASSERT( gSingletonFramework );
			gSingletonFramework->m_Mouse->NotifyMouseMoved();
		}
	}

	return DefWindowProc( hWnd, uMsg, wParam, lParam );
}
#endif // BUILD_WINDOWS_NO_SDL
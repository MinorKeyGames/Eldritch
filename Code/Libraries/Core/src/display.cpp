#include "core.h"
#include "display.h"
#include "configmanager.h"

#if BUILD_WINDOWS_NO_SDL
#include <Windows.h>
#endif

#if BUILD_SDL
#include "SDL2/SDL.h"
#endif

Display::Display()
:	m_Width( 0 )
,	m_Height( 0 )
,	m_ScreenWidth( 0 )
,	m_ScreenHeight( 0 )
,	m_AspectRatio( 0.0f )
,	m_Fullscreen( false )
,	m_DisplayModes()
{
	STATICHASH( DisplayWidth );
	STATICHASH( DisplayHeight );
	STATICHASH( FOV );
	STATICHASH( Fullscreen );

	m_Width = ConfigManager::GetInt( sDisplayWidth );
	m_Height = ConfigManager::GetInt( sDisplayHeight );
	m_AspectRatio = ( float )m_Width / ( float )m_Height;
	m_Fullscreen = ConfigManager::GetBool( sFullscreen );

	// Get the stored resolution
#if BUILD_WINDOWS_NO_SDL
	DEVMODE DevMode;
	XTRACE_BEGIN( EnumDisplaySettings );
		EnumDisplaySettings( NULL, ENUM_REGISTRY_SETTINGS, &DevMode );
	XTRACE_END;
	m_ScreenWidth	= DevMode.dmPelsWidth;
	m_ScreenHeight	= DevMode.dmPelsHeight;
#endif
#if BUILD_SDL
	SDL_DisplayMode DisplayMode;
	SDL_GetDesktopDisplayMode( 0, &DisplayMode );
	m_ScreenWidth	= DisplayMode.w;
	m_ScreenHeight	= DisplayMode.h;
#endif

	UpdateDisplay();
}

Display::~Display()
{
	if( m_Fullscreen )
	{
		SetDisplay( true );
	}

	m_DisplayModes.Clear();
}

void Display::SetFullScreen( bool FullScreen )
{
	m_Fullscreen = FullScreen;
	STATICHASH( Fullscreen );
	ConfigManager::SetBool( sFullscreen, FullScreen );	// Keep config var in sync with display
}

void Display::SetResolution( uint Width, uint Height )
{
	m_Width = Width;
	m_Height = Height;
	m_AspectRatio = ( float )m_Width / ( float )m_Height;

	// Keep config vars in sync with display
	STATICHASH( DisplayWidth );
	STATICHASH( DisplayHeight );
	ConfigManager::SetInt( sDisplayWidth, m_Width );
	ConfigManager::SetInt( sDisplayHeight, m_Height );
}

void Display::UpdateDisplay()
{
	STATICHASH( DisplayDepth );
	STATICHASH( DisplayRate );
	if( m_Fullscreen )
	{
		SetDisplay( false, m_Fullscreen, m_Width, m_Height, ConfigManager::GetInt( sDisplayDepth, 32 ), ConfigManager::GetInt( sDisplayRate, 60 ) );
	}
	else
	{
		SetDisplay( true );
	}
}

bool Display::NeedsUpdate()
{
#if BUILD_WINDOWS_NO_SDL
	DEVMODE DevMode;
	ZeroMemory( &DevMode, sizeof( DEVMODE ) );

	EnumDisplaySettings( NULL, ENUM_CURRENT_SETTINGS, &DevMode );

	return m_Fullscreen && ( DevMode.dmPelsWidth != m_Width || DevMode.dmPelsHeight != m_Height );
#endif
#if BUILD_SDL
	SDL_DisplayMode DisplayMode;
	SDL_GetCurrentDisplayMode( 0, &DisplayMode );

	return m_Fullscreen && ( DisplayMode.w != static_cast<int>( m_Width ) || DisplayMode.h != static_cast<int>( m_Height ) );
#endif
}

// TODO: Check return value of ChangeDisplaySettings and react appropriately
/*static*/ void Display::SetDisplay( bool Reset /*= true*/, bool FullScreen /*= false*/, int Width /*= 0*/, int Height /*= 0*/, int BitDepth /*= 0*/, int Frequency /*= 0*/ )
{
	XTRACE_FUNCTION;

#if BUILD_SDL
	// SDL manages display through its window system.
	Unused( Reset );
	Unused( FullScreen );
	Unused( Width );
	Unused( Height );
	Unused( BitDepth );
	Unused( Frequency );
	return;
#endif

#if BUILD_WINDOWS_NO_SDL
	if( Reset )
	{
		DEBUGCATPRINTF( "Core", 1, "Resetting display\n" );

		ChangeDisplaySettings(NULL, 0);
	}
	else
	{
		DEBUGCATPRINTF( "Core", 1, "Setting display:\n\t%dx%d %dbpp %dHz\n", Width, Height, BitDepth, Frequency );

		DEVMODE DevMode;
		ZeroMemory( &DevMode, sizeof( DEVMODE ) );

		DevMode.dmSize = sizeof( DEVMODE );
		DevMode.dmPelsWidth = Width;
		DevMode.dmPelsHeight = Height;
		DevMode.dmBitsPerPel = BitDepth;
		DevMode.dmDisplayFrequency = Frequency;
		DevMode.dmFields = 0;
		DevMode.dmFields |= ( Width > 0 ) ? DM_PELSWIDTH : 0;
		DevMode.dmFields |= ( Height > 0 ) ? DM_PELSHEIGHT : 0;
		DevMode.dmFields |= ( BitDepth > 0 ) ? DM_BITSPERPEL : 0;
		DevMode.dmFields |= ( Frequency > 0 ) ? DM_DISPLAYFREQUENCY : 0;

		DWORD Flags = ( FullScreen ) ? CDS_FULLSCREEN : 0;
		if( DISP_CHANGE_SUCCESSFUL == ChangeDisplaySettings( &DevMode, Flags | CDS_TEST ) )
		{
			ChangeDisplaySettings( &DevMode, Flags );
		}
	}
#endif
}

/*static*/ void Display::EnumerateDisplayModes( Array<SDisplayMode>& DisplayModes )
{
	DisplayModes.Clear();

#if BUILD_WINDOWS_NO_SDL
	DEVMODE	DevMode;
	uint	ModeIndex	= 0;
	BOOL	Valid		= TRUE;
	for( ; Valid; ++ModeIndex )
	{
		Valid = EnumDisplaySettings( NULL, ModeIndex, &DevMode );

		// Some users have problems if their refresh rate is set to 59 Hz. Maybe I should reconsider this?
		if( DevMode.dmBitsPerPel == 32 )
		{
			SDisplayMode Mode;
			Mode.Width	= DevMode.dmPelsWidth;
			Mode.Height	= DevMode.dmPelsHeight;
			DisplayModes.PushBackUnique( Mode );
		}
	}
#endif
#if BUILD_SDL
	PRINTF( "Enumerating SDL display modes...\n" );
	const int NumDisplays		= SDL_GetNumVideoDisplays();
	for( int DisplayIndex = 0; DisplayIndex < NumDisplays; ++DisplayIndex )
	{
		const int NumModes = SDL_GetNumDisplayModes( DisplayIndex );
		for( int ModeIndex = 0; ModeIndex < NumModes; ++ModeIndex )
		{
			SDL_DisplayMode DisplayMode;
			SDL_GetDisplayMode( DisplayIndex, ModeIndex, &DisplayMode );

			if( SDL_BYTESPERPIXEL( DisplayMode.format ) == 4 )
			{
				SDisplayMode Mode;
				Mode.Width	= DisplayMode.w;
				Mode.Height	= DisplayMode.h;
				DisplayModes.PushBackUnique( Mode );

				PRINTF( "Enumerated mode %dx%d\n", Mode.Width, Mode.Height );
			}
		}
	}
#endif

	ASSERT( DisplayModes.Size() );
}

/*static*/ SDisplayMode Display::GetBestDisplayMode( const uint DesiredWidth, const uint DesiredHeight )
{
	Array<SDisplayMode> DisplayModes;
	EnumerateDisplayModes( DisplayModes );

	SDisplayMode BestDisplayMode;
	FOR_EACH_ARRAY( ModeIter, DisplayModes, SDisplayMode )
	{
		const SDisplayMode& Mode = ModeIter.GetValue();
		if( Mode.Width	>= BestDisplayMode.Width &&
			Mode.Height	>= BestDisplayMode.Height &&
			Mode.Width	<= DesiredWidth &&
			Mode.Height	<= DesiredHeight )
		{
			BestDisplayMode = Mode;
		}
	}

	// This case can occur if our desired dimensions are *smaller* than any available display mode.
	if( BestDisplayMode.Width == 0 ||
		BestDisplayMode.Height == 0 )
	{
		PRINTF( "Could not find a more suitable display mode.\n" );
		BestDisplayMode.Width   = DesiredWidth;
		BestDisplayMode.Height  = DesiredHeight;
	}

	PRINTF( "Using display mode %dx%d\n", BestDisplayMode.Width, BestDisplayMode.Height );
	return BestDisplayMode;
}
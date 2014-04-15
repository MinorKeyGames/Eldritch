#include "core.h"
#include "eldritchframework.h"

#if BUILD_WINDOWS
#include "../resource.h"
#endif

#if BUILD_MAC
#include "objcjunk.h"
#endif

#include "array.h"
#include "vector.h"
#include "view.h"
#include "bucket.h"
#include "angles.h"
#include "keyboard.h"
#include "mouse.h"
#include "xinputcontroller.h"
#include "eldritchgame.h"
#include "eldritchworld.h"
#include "eldritchsaveload.h"
#include "eldritchbank.h"
#include "eldritchpersistence.h"
#include "material.h"
#include "wbevent.h"
#include "wbeventmanager.h"
#include "reversehash.h"
#include "clock.h"
#include "frameworkutil.h"
#include "packstream.h"
#include "configmanager.h"
#include "filestream.h"
#include "fmodaudiosystem.h"
#include "eldritchmesh.h"
#include "dynamicmeshmanager.h"
#include "inputsystem.h"
#include "file.h"
#include "windowwrapper.h"
#include "irenderer.h"
#include "irendertarget.h"
#include "eldritchsound3dlistener.h"
#include "eldritchparticles.h"
#include "uiinputmapeldritch.h"
#include "eldritchtargetmanager.h"
#include "shadermanager.h"
#include "texturemanager.h"

#if BUILD_WINDOWS
#include "checkforupdates.h"
#endif

#if BUILD_DEV
#include "eldritchtools.h"
#endif

#include "sdpfactory.h"
#include "eldritchsdps.h"

#include "Common/uimanagercommon.h"
#include "uistack.h"

#include "uifactory.h"
#include "uiscreens.h"
#include "eldritchuiscreens.h"

#include "wbentity.h"
#include "wbscene.h"
#include "wbcomponent.h"
#include "rodinwbcomponents.h"
#include "eldritchwbcomponents.h"

#include "wbparamevaluatorfactory.h"
#include "rodinwbpes.h"
#include "eldritchwbpes.h"

#include "wbactionfactory.h"
#include "uiwbactions.h"
#include "rodinwbactions.h"
#include "eldritchwbactions.h"

#include "rodinbtnodefactory.h"
#include "eldritchrodinbtnodes.h"

#include "animeventfactory.h"
#include "eldritchanimevents.h"

#include "wbactionstack.h"

#define COOL_SIDE_VIEW_THING 0

// Singleton accessor
static EldritchFramework*	gSingletonFramework = NULL;

/*static*/ EldritchFramework* EldritchFramework::GetInstance()
{
	return gSingletonFramework;
}

/*static*/ void EldritchFramework::SetInstance( EldritchFramework* const pFramework )
{
	ASSERT( gSingletonFramework == NULL );
	gSingletonFramework = pFramework;
}

EldritchFramework::EldritchFramework()
:	m_Game( NULL )
,	m_World( NULL )
#if BUILD_DEV
,	m_Tools( NULL )
#endif
,	m_Controller( NULL )
,	m_InputSystem( NULL )
#if BUILD_WINDOWS
,	m_CheckForUpdates( NULL )
#endif
,	m_DisplayWidth( 0 )
,	m_DisplayHeight( 0 )
,	m_TargetManager( NULL )
,	m_MainView( NULL )
,	m_FGView( NULL )
,	m_HUDView( NULL )
,	m_MirrorView( NULL )
,	m_MinimapView( NULL )
,	m_Audio3DListener( NULL )
{
	EldritchFramework::SetInstance( this );
	Framework3D::SetInstance( this );
}

EldritchFramework::~EldritchFramework()
{
	ASSERT( gSingletonFramework == this );
	gSingletonFramework = NULL;
}

/*virtual*/ void EldritchFramework::GetInitialWindowTitle( SimpleString& WindowTitle )
{
	STATICHASH( WindowTitle );
	WindowTitle = ConfigManager::GetLocalizedString( sWindowTitle, "Eldritch" );
}

/*virtual*/ void EldritchFramework::GetInitialWindowIcon( uint& WindowIcon )
{
#if BUILD_WINDOWS
	WindowIcon = IDI_ICON1;
#else
	Unused( WindowIcon );
#endif
}

/*virtual*/ void EldritchFramework::GetUIManagerDefinitionName( SimpleString& DefinitionName )
{
	DefinitionName = "EldritchUI";
}

/*virtual*/ void EldritchFramework::InitializeUIInputMap()
{
	m_UIInputMap = new UIInputMapEldritch( this );
}

void EldritchFramework::InitializePackages()
{
	STATICHASH( NumPackages );
	const uint NumPackages = ConfigManager::GetInt( sNumPackages );
	for( uint PackageIndex = 0; PackageIndex < NumPackages; ++PackageIndex )
	{
		const SimpleString PackageName = ConfigManager::GetSequenceString( "Package%d", PackageIndex, "" );

		// Don't preempt the base package, there should be no conflicts.
		PackStream::StaticAddPackageFile( PackageName.CStr(), false );
	}
}

void EldritchFramework::InitializeDLC()
{
	STATICHASH( NumDLC );
	const uint NumDLC = ConfigManager::GetInt( sNumDLC );
	for( uint DLCIndex = 0; DLCIndex < NumDLC; ++DLCIndex )
	{
		const SimpleString DLCName = ConfigManager::GetSequenceString( "DLC%d", DLCIndex, "" );

		MAKEHASH( DLCName );

		STATICHASH( PackageFilename );
		const SimpleString PackageFilename = ConfigManager::GetString( sPackageFilename, "", sDLCName );

		// DLC will always preempt base content (so it can be used for patching as well as DLC per se).
		PackStream::StaticAddPackageFile( PackageFilename.CStr(), true );

		// Load config files for DLC, if DLC was successfully loaded.
		// (We can't check a return value from StaticAddPackageFile, because I won't have package
		// files during development but it still needs to load loose DLC files.)
		STATICHASH( NumConfigFiles );
		const uint NumConfigFiles = ConfigManager::GetInt( sNumConfigFiles, 0, sDLCName );
		for( uint ConfigFileIndex = 0; ConfigFileIndex < NumConfigFiles; ++ConfigFileIndex )
		{
			const SimpleString ConfigFile = ConfigManager::GetSequenceString( "ConfigFile%d", ConfigFileIndex, "", sDLCName );
			if( PackStream::StaticFileExists( ConfigFile.CStr() ) )
			{
				ConfigManager::Load( PackStream( ConfigFile.CStr() ) );
			}
		}
	}
}

/*virtual*/ void EldritchFramework::Initialize()
{
	XTRACE_FUNCTION;

	XTRACE_BEGIN( PreFramework3D );
		ReverseHash::Initialize();

		PackStream::StaticAddPackageFile( "eldritch-base.cpk" );
		FrameworkUtil::MinimalLoadConfigFiles( "Config/default.ccf" );

		InitializePackages();

		InitializeDLC();

		// Load prefs over anything in the defaults.
		LoadPrefsConfig();

		LOADPRINTLEVELS;

		STATICHASH( Version );
		STATICHASH( ContentSyncer );
		SimpleString LocalVersion = ConfigManager::GetString( sVersion, "", sContentSyncer );
		PRINTF( "Version: %s\n", LocalVersion.CStr() );

		XTRACE_BEGIN( InitializeFactories );
			PRINTF( "Initializing factories...\n" );

			PRINTF( "Initializing SDP factories.\n" );
			SDPFactory::InitializeBaseFactories();
#define ADDSDPFACTORY( type ) SDPFactory::RegisterSDPFactory( #type, SDP##type::Factory );
#include "eldritchsdps.h"
#undef ADDSDPFACTORY

			PRINTF( "Initializing UI factories.\n" );
			UIFactory::InitializeBaseFactories();
#define ADDUISCREENFACTORY( type ) UIFactory::RegisterUIScreenFactory( #type, UIScreen##type::Factory );
#include "eldritchuiscreens.h"
#undef ADDUISCREENFACTORY

			PRINTF( "Initializing anim event factories.\n" );
#define ADDANIMEVENTFACTORY( type ) AnimEventFactory::GetInstance()->Register( #type, AnimEvent##type::Factory );
#include "eldritchanimevents.h"
#undef ADDANIMEVENTFACTORY

			PRINTF( "Initializing PE factories.\n" );
			WBParamEvaluatorFactory::InitializeBaseFactories();
#define ADDWBPEFACTORY( type ) WBParamEvaluatorFactory::RegisterFactory( #type, WBPE##type::Factory );
#include "rodinwbpes.h"
#include "eldritchwbpes.h"
#undef ADDWBPEFACTORY

			PRINTF( "Initializing action factories.\n" );
			WBActionFactory::InitializeBaseFactories();
#define ADDWBACTIONFACTORY( type ) WBActionFactory::RegisterFactory( #type, WBAction##type::Factory );
#include "uiwbactions.h"
#include "rodinwbactions.h"
#include "eldritchwbactions.h"
#undef ADDWBPEFACTORY

			PRINTF( "Initializing BT factories.\n" );
			RodinBTNodeFactory::InitializeBaseFactories();
#define ADDRODINBTNODEFACTORY( type ) RodinBTNodeFactory::RegisterFactory( #type, RodinBTNode##type::Factory );
#include "eldritchrodinbtnodes.h"
#undef ADDRODINBTNODEFACTORY

			// Initialize core and Eldritch Workbench component factories.
			PRINTF( "Initializing component factories.\n" );
			WBComponent::InitializeBaseFactories();
#define ADDWBCOMPONENT( type ) WBComponent::RegisterWBCompFactory( #type, WBComp##type::Factory );
#include "rodinwbcomponents.h"
#include "eldritchwbcomponents.h"
#undef ADDWBCOMPONENT
		XTRACE_END;

		PRINTF( "Factories initialized.\n" );

		// Create input system before framework so it will exist for UI. But don't attach devices yet, as they don't exist.
		PRINTF( "Initializing input system.\n" );
		m_InputSystem = new InputSystem;
		m_InputSystem->Initialize( "EldritchInput" );
	XTRACE_END;

	Framework3D::Initialize();

	STATICHASH( DisplayWidth );
	STATICHASH( DisplayHeight );
	m_DisplayWidth	= ConfigManager::GetInt( sDisplayWidth );
	m_DisplayHeight	= ConfigManager::GetInt( sDisplayHeight );

#if BUILD_WINDOWS
	m_CheckForUpdates = new CheckForUpdates( m_UIManager );
#endif

	m_Controller = new XInputController;

	m_TargetManager = new EldritchTargetManager( m_Renderer );
	m_TargetManager->CreateTargets( m_Display->m_Width, m_Display->m_Height );

	m_Audio3DListener = new EldritchSound3DListener;
	m_Audio3DListener->Initialize();

	ASSERT( m_AudioSystem );
	m_AudioSystem->Set3DListener( m_Audio3DListener );

	STATICHASH( FOV );
	const float FOV = ConfigManager::GetFloat( sFOV, 90.0f );

	STATICHASH( ForegroundFOV );
	const float FGFOV = ConfigManager::GetFloat( sForegroundFOV, 60.0f );

	STATICHASH( NearClip );
	const float NearClip = ConfigManager::GetFloat( sNearClip, 0.1f );

	STATICHASH( FarClip );
	const float FarClip = ConfigManager::GetFloat( sFarClip, 0.1f );

	const float fDisplayWidth = static_cast<float>( m_DisplayWidth );
	const float fDisplayHeight = static_cast<float>( m_DisplayHeight );
	const float AspectRatio = fDisplayWidth / fDisplayHeight;

	m_MainView	= new View( Vector(), Angles(), FOV, AspectRatio, NearClip, FarClip );
	m_FGView	= new View( Vector(), Angles(), FGFOV, AspectRatio, NearClip, FarClip );
	CreateHUDView();
	CreateMirrorView();
	CreateMinimapView();

	CreateBuckets();

	m_InputSystem->SetKeyboard( m_Keyboard );
	m_InputSystem->SetMouse( m_Mouse );
	m_InputSystem->SetController( m_Controller );
	m_InputSystem->SetClock( m_Clock );

	WBActionStack::Initialize();

	PRINTF( "Initializing Eldritch.\n" );

	InitializeWorld( HashedString(), false );

	m_Game = new EldritchGame;
	m_Game->RefreshRTDependentSystems();
	m_Game->Initialize();

	// Initialize config stuff
	{
		STATICHASH( InvertY );
		const bool InvertY = ConfigManager::GetBool( sInvertY );

		STATIC_HASHED_STRING( TurnY );
		m_InputSystem->SetMouseInvert( sTurnY, InvertY );
		m_InputSystem->SetControllerInvert( sTurnY, InvertY );

		STATICHASH( ControllerPower );
		const float ControllerPower = ConfigManager::GetFloat( sControllerPower );

		STATIC_HASHED_STRING( MoveX );
		m_InputSystem->SetControllerPower( sMoveX, ControllerPower );
		STATIC_HASHED_STRING( MoveY );
		m_InputSystem->SetControllerPower( sMoveY, ControllerPower );
		STATIC_HASHED_STRING( TurnX );
		m_InputSystem->SetControllerPower( sTurnX, ControllerPower );
		m_InputSystem->SetControllerPower( sTurnY, ControllerPower );
	}

	// Initialize UI sliders. This could be neater.
	// This also pushes the initial values to their respective systems, which is pret-ty terrible design.
	{
		WBEventManager* pEventManager = WBWorld::GetInstance()->GetEventManager();

		{
			STATICHASH( MouseSpeed );
			const float MouseSpeed = ConfigManager::GetFloat( sMouseSpeed, 1.0f );

			STATIC_HASHED_STRING( ControlsOptionsScreen );
			STATIC_HASHED_STRING( MouseSpeedSlider );
			WB_MAKE_EVENT( SetUISliderValue, NULL );
			WB_SET_AUTO( SetUISliderValue, Hash, Screen, sControlsOptionsScreen );
			WB_SET_AUTO( SetUISliderValue, Hash, Widget, sMouseSpeedSlider );
			WB_SET_AUTO( SetUISliderValue, Float, SliderValue, GetSliderValueFromMouseSpeed( MouseSpeed ) );
			WB_DISPATCH_EVENT( pEventManager, SetUISliderValue, NULL );
		}

		{
			STATICHASH( ControllerSpeed );
			const float ControllerSpeed = ConfigManager::GetFloat( sControllerSpeed, 1.0f );

			STATIC_HASHED_STRING( ControlsOptionsScreen );
			STATIC_HASHED_STRING( ControllerSpeedSlider );
			WB_MAKE_EVENT( SetUISliderValue, NULL );
			WB_SET_AUTO( SetUISliderValue, Hash, Screen, sControlsOptionsScreen );
			WB_SET_AUTO( SetUISliderValue, Hash, Widget, sControllerSpeedSlider );
			WB_SET_AUTO( SetUISliderValue, Float, SliderValue, GetSliderValueFromControllerSpeed( ControllerSpeed ) );
			WB_DISPATCH_EVENT( pEventManager, SetUISliderValue, NULL );
		}

		{
			STATICHASH( Brightness );
			const float Brightness = ConfigManager::GetFloat( sBrightness, 1.0f );

			STATIC_HASHED_STRING( BrightnessScreen );
			STATIC_HASHED_STRING( BrightnessSlider );
			WB_MAKE_EVENT( SetUISliderValue, NULL );
			WB_SET_AUTO( SetUISliderValue, Hash, Screen, sBrightnessScreen );
			WB_SET_AUTO( SetUISliderValue, Hash, Widget, sBrightnessSlider );
			WB_SET_AUTO( SetUISliderValue, Float, SliderValue, GetSliderValueFromBrightness( Brightness ) );
			WB_DISPATCH_EVENT( pEventManager, SetUISliderValue, NULL );
		}

		{
			STATIC_HASHED_STRING( DisplayOptionsScreen );
			STATIC_HASHED_STRING( FOVSlider );
			WB_MAKE_EVENT( SetUISliderValue, NULL );
			WB_SET_AUTO( SetUISliderValue, Hash, Screen, sDisplayOptionsScreen );
			WB_SET_AUTO( SetUISliderValue, Hash, Widget, sFOVSlider );
			WB_SET_AUTO( SetUISliderValue, Float, SliderValue, GetSliderValueFromFOV( FOV ) );
			WB_DISPATCH_EVENT( pEventManager, SetUISliderValue, NULL );
		}

		{
			STATICHASH( MasterVolume );
			const float MasterVolume = ConfigManager::GetFloat( sMasterVolume );

			STATIC_HASHED_STRING( AudioOptionsScreen );
			STATIC_HASHED_STRING( VolumeSlider );
			WB_MAKE_EVENT( SetUISliderValue, NULL );
			WB_SET_AUTO( SetUISliderValue, Hash, Screen, sAudioOptionsScreen );
			WB_SET_AUTO( SetUISliderValue, Hash, Widget, sVolumeSlider );
			WB_SET_AUTO( SetUISliderValue, Float, SliderValue, MasterVolume );
			WB_DISPATCH_EVENT( pEventManager, SetUISliderValue, NULL );
		}

		{
			STATICHASH( MusicVolume );
			const float MusicVolume = ConfigManager::GetFloat( sMusicVolume );

			STATIC_HASHED_STRING( AudioOptionsScreen );
			STATIC_HASHED_STRING( MusicVolumeSlider );
			WB_MAKE_EVENT( SetUISliderValue, NULL );
			WB_SET_AUTO( SetUISliderValue, Hash, Screen, sAudioOptionsScreen );
			WB_SET_AUTO( SetUISliderValue, Hash, Widget, sMusicVolumeSlider );
			WB_SET_AUTO( SetUISliderValue, Float, SliderValue, MusicVolume );
			WB_DISPATCH_EVENT( pEventManager, SetUISliderValue, NULL );
		}
	}

	// Initialize UI callbacks
	{
		UIScreenEldSetRes* pSetRes = m_UIManager->GetScreen<UIScreenEldSetRes>( "SetResScreen" );
		pSetRes->SetUICallback( SUICallback( EldritchFramework::OnSetRes, NULL ) );

		UIScreenFade* pFade = m_UIManager->GetScreen<UIScreenFade>( "Fade" );
		pFade->SetFadeCallback( SUICallback( EldritchFramework::OnFadeFinished, NULL ) );
	}

	WB_MAKE_EVENT( ResetToInitialScreens, NULL );
	WB_DISPATCH_EVENT( WBWorld::GetInstance()->GetEventManager(), ResetToInitialScreens, NULL );

	{
		// HACK: Too content aware
		STATIC_HASHED_STRING( MKGLogoScreen );
		WB_MAKE_EVENT( PushUIScreen, NULL );
		WB_SET_AUTO( PushUIScreen, Hash, Screen, sMKGLogoScreen );
		WB_DISPATCH_EVENT( WBWorld::GetInstance()->GetEventManager(), PushUIScreen, NULL );
	}

	// Tick world once to pump the event queue. Fixes title screen bugs.
	m_World->Tick( 0.0f );

	// All done, show the window finally.
	SafeDelete( m_SplashWindow );
#if BUILD_WINDOWS_NO_SDL
	m_Window->Show( m_CmdShow );
#elif BUILD_SDL
	m_Window->Show();
#endif

	// Reattach GL context if needed.
	m_Renderer->Refresh();

	PRINTF( "Eldritch initialization complete.\n" );
}

/*static*/ void EldritchFramework::OnSetRes( void* pUIElement, void* pVoid )
{
	Unused( pVoid );

	EldritchFramework* const	pFramework	= EldritchFramework::GetInstance();
	Display* const				pDisplay	= pFramework->GetDisplay();
	UIWidget* const				pWidget		= static_cast<UIWidget*>( pUIElement );
	UIScreenEldSetRes* const	pSetRes		= pFramework->GetUIManager()->GetScreen<UIScreenEldSetRes>( "SetResScreen" );
	const SDisplayMode			ChosenRes	= pSetRes->GetRes( pWidget->m_Name );

	if( !pDisplay->m_Fullscreen && ( ChosenRes.Width > pDisplay->m_ScreenWidth || ChosenRes.Height > pDisplay->m_ScreenHeight ) )
	{
		WARNDESC( "Mode too large for screen." );
	}
	else
	{
		pFramework->SetResolution( ChosenRes.Width, ChosenRes.Height );
	}
}

/*static*/ void EldritchFramework::OnFadeFinished( void* pUIElement, void* pVoid )
{
	Unused( pVoid );
	Unused( pUIElement );

	// HACKHACK: Handle logo fade sequence here because event manager isn't meant for timed UI stuff.
	enum EIntroFadePhases
	{
		ELFP_FadingInLogo,
		ELFP_WaitingLogo,
		ELFP_FadingOutLogo,
		ELFP_Waiting,
		ELFP_Finished,
	};

	WBEventManager* const pEventManager = WBWorld::GetInstance()->GetEventManager();
	ASSERT( pEventManager );

	static EIntroFadePhases sPhase = ELFP_FadingInLogo;
	if( sPhase == ELFP_FadingInLogo )
	{
		DEBUGPRINTF( "Faded in\n" );
		sPhase = ELFP_WaitingLogo;

		static const float kWaitDuration = 0.75f;
		WB_MAKE_EVENT( Fade, NULL );
		WB_SET_AUTO( Fade, Float, Duration, kWaitDuration );
		WB_SET_AUTO( Fade, Float, FadeColorAStart, 0.0f );
		WB_SET_AUTO( Fade, Float, FadeColorAEnd, 0.0f );
		WB_DISPATCH_EVENT( pEventManager, Fade, NULL );
	}
	else if( sPhase == ELFP_WaitingLogo )
	{
		DEBUGPRINTF( "Waited\n" );
		sPhase = ELFP_FadingOutLogo;

		static const float kFadeOutDuration = 0.5f;
		WB_MAKE_EVENT( FadeOut, NULL );
		WB_SET_AUTO( FadeOut, Float, Duration, kFadeOutDuration );
		WB_DISPATCH_EVENT( pEventManager, FadeOut, NULL );
	}
	else if( sPhase == ELFP_FadingOutLogo )
	{
		DEBUGPRINTF( "Faded out\n" );
		sPhase = ELFP_Waiting;

		static const float kWaitDuration = 0.25f;
		WB_MAKE_EVENT( Fade, NULL );
		WB_SET_AUTO( Fade, Float, Duration, kWaitDuration );
		WB_SET_AUTO( Fade, Float, FadeColorAStart, 1.0f );
		WB_SET_AUTO( Fade, Float, FadeColorAEnd, 1.0f );
		WB_DISPATCH_EVENT( pEventManager, Fade, NULL );
	}
	else if( sPhase == ELFP_Waiting )
	{
		DEBUGPRINTF( "Waited\n" );
		sPhase = ELFP_Finished;

		WB_MAKE_EVENT( PopUIScreen, NULL );
		WB_DISPATCH_EVENT( pEventManager, PopUIScreen, NULL );

		static const float kFadeInDuration = 0.5f;
		WB_MAKE_EVENT( FadeIn, NULL );
		WB_SET_AUTO( FadeIn, Float, Duration, kFadeInDuration );
		WB_DISPATCH_EVENT( pEventManager, FadeIn, NULL );

		// HACKHACK: Moved here so it doesn't conflict with logos.
		WB_MAKE_EVENT( CheckForUpdates, NULL );
		WB_DISPATCH_EVENT( pEventManager, CheckForUpdates, NULL );
	}
	else
	{
		// Do nothing, intro has elapsed.
	}
}

/*virtual*/ void EldritchFramework::InitializeAudioSystem()
{
	m_AudioSystem = new FMODAudioSystem;
}

// Also used to reinitialize world.
void EldritchFramework::InitializeWorld( const HashedString& WorldDef, const bool CreateWorld )
{
	XTRACE_FUNCTION;

	PrepareForLoad();

	ShutDownWorld();

	WBWorld::CreateInstance();
	WBWorld::GetInstance()->SetClock( GetClock() );

	RegisterForEvents();

	m_World = new EldritchWorld;
	m_World->SetCurrentWorld( WorldDef );
	m_World->Initialize();

	m_Audio3DListener->SetWorld( m_World );

	m_UIManager->RegisterForEvents();

	if( m_Game )
	{
		m_Game->GetBank()->RegisterForEvents();
		m_Game->GetPersistence()->RegisterForEvents();
	}

	if( CreateWorld )
	{
		m_World->Create();
		InitializeTools();
	}
}

void EldritchFramework::RegisterForEvents()
{
	STATIC_HASHED_STRING( QuitGame );
	WBWorld::GetInstance()->GetEventManager()->AddObserver( sQuitGame, this, NULL );

	STATIC_HASHED_STRING( ToggleInvertY );
	WBWorld::GetInstance()->GetEventManager()->AddObserver( sToggleInvertY, this, NULL );

	STATIC_HASHED_STRING( ToggleFullscreen );
	WBWorld::GetInstance()->GetEventManager()->AddObserver( sToggleFullscreen, this, NULL );

	STATIC_HASHED_STRING( OnSliderChanged );
	WBWorld::GetInstance()->GetEventManager()->AddObserver( sOnSliderChanged, this, NULL );

	STATIC_HASHED_STRING( WritePrefsConfig );
	WBWorld::GetInstance()->GetEventManager()->AddObserver( sWritePrefsConfig, this, NULL );

	STATIC_HASHED_STRING( CheckForUpdates );
	WBWorld::GetInstance()->GetEventManager()->AddObserver( sCheckForUpdates, this, NULL );
}

/*virtual*/ void EldritchFramework::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Framework3D::HandleEvent( Event );

	STATIC_HASHED_STRING( ToggleInvertY );
	STATIC_HASHED_STRING( ToggleFullscreen );
	STATIC_HASHED_STRING( OnSliderChanged );
	STATIC_HASHED_STRING( WritePrefsConfig );
	STATIC_HASHED_STRING( CheckForUpdates );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sToggleInvertY )
	{
		XTRACE_NAMED( ToggleInvertY );

		// TODO: Also invert controller? Or on a separate button?

		STATIC_HASHED_STRING( TurnY );
		const bool InvertY = !m_InputSystem->GetMouseInvert( sTurnY );
		m_InputSystem->SetMouseInvert( sTurnY, InvertY );
		m_InputSystem->SetControllerInvert( sTurnY, InvertY );

		// Publish config var so UI can reflect the change.
		// I could make input system publish config vars for each adjustment, but
		// that seems wasteful since most inputs currently have no adjustment.
		STATICHASH( InvertY );
		ConfigManager::SetBool( sInvertY, InvertY );
	}
	else if( EventName == sToggleFullscreen )
	{
		XTRACE_NAMED( ToggleFullscreen );
		ToggleFullscreen();
	}
	else if( EventName == sOnSliderChanged )
	{
		XTRACE_NAMED( OnSliderChanged );

		STATIC_HASHED_STRING( SliderName );
		const HashedString SliderName = Event.GetHash( sSliderName );

		STATIC_HASHED_STRING( SliderValue );
		const float SliderValue = Event.GetFloat( sSliderValue );

		HandleUISliderEvent( SliderName, SliderValue );
	}
	else if( EventName == sWritePrefsConfig )
	{
		XTRACE_NAMED( WritePrefsConfig );
		WritePrefsConfig();
	}
	else if( EventName == sCheckForUpdates )
	{
#if BUILD_WINDOWS && !BUILD_STEAM
		XTRACE_NAMED( CheckForUpdates );

		// So I can compile updating out of certain builds.
		STATICHASH( Framework );
		STATICHASH( CheckForUpdates );
		const bool CheckForUpdates = ConfigManager::GetBool( sCheckForUpdates, true, sFramework );
		if( CheckForUpdates )
		{
			m_CheckForUpdates->Check( false, true );
		}
#endif
	}
}

void EldritchFramework::HandleUISliderEvent( const HashedString& SliderName, const float SliderValue )
{
	STATIC_HASHED_STRING( MouseSpeedSlider );
	STATIC_HASHED_STRING( ControllerSpeedSlider );
	STATIC_HASHED_STRING( BrightnessSlider );
	STATIC_HASHED_STRING( FOVSlider );
	STATIC_HASHED_STRING( VolumeSlider );
	STATIC_HASHED_STRING( MusicVolumeSlider );

	if( SliderName == sMouseSpeedSlider )
	{
		const float MouseSpeed = GetMouseSpeedFromSliderValue( SliderValue );

		STATIC_HASHED_STRING( TurnX );
		m_InputSystem->SetMouseScale( sTurnX, MouseSpeed );

		STATIC_HASHED_STRING( TurnY );
		m_InputSystem->SetMouseScale( sTurnY, MouseSpeed );

		// Publish config var.
		STATICHASH( MouseSpeed );
		ConfigManager::SetFloat( sMouseSpeed, MouseSpeed );
	}
	else if( SliderName == sControllerSpeedSlider )
	{
		const float ControllerSpeed = GetControllerSpeedFromSliderValue( SliderValue );

		STATICHASH( ControllerSpeedX );
		const float ControllerSpeedX = ControllerSpeed * ConfigManager::GetFloat( sControllerSpeedX );

		STATICHASH( ControllerSpeedY );
		const float ControllerSpeedY = -ControllerSpeed * ConfigManager::GetFloat( sControllerSpeedY ); // HACK! Controller Y axis is inverted from mouse.

		STATIC_HASHED_STRING( TurnX );
		m_InputSystem->SetControllerScale( sTurnX, ControllerSpeedX );

		STATIC_HASHED_STRING( TurnY );
		m_InputSystem->SetControllerScale( sTurnY, ControllerSpeedY );

		// Publish config var.
		STATICHASH( ControllerSpeed );
		ConfigManager::SetFloat( sControllerSpeed, ControllerSpeed );
	}
	else if( SliderName == sBrightnessSlider )
	{
		const float Brightness = GetBrightnessFromSliderValue( SliderValue );
		m_Game->SetGamma( Brightness );

		// Publish config var.
		STATICHASH( Brightness );
		ConfigManager::SetFloat( sBrightness, Brightness );
	}
	else if( SliderName == sFOVSlider )
	{
		const float FOV = GetFOVFromSliderValue( SliderValue );

		ASSERT( m_MainView );
		m_MainView->SetFOV( FOV );

		// Publish config var.
		STATICHASH( FOV );
		ConfigManager::SetFloat( sFOV, FOV );
	}
	else if( SliderName == sVolumeSlider )
	{
		const float MasterVolume = SliderValue;

		m_AudioSystem->SetMasterVolume( MasterVolume );

		// Publish config var.
		STATICHASH( MasterVolume );
		ConfigManager::SetFloat( sMasterVolume, MasterVolume );
	}
	else if( SliderName == sMusicVolumeSlider )
	{
		const float MusicVolume = SliderValue;

		STATIC_HASHED_STRING( Music );
		m_AudioSystem->SetCategoryVolume( sMusic, MusicVolume, 0.0f );

		// Publish config var.
		STATICHASH( MusicVolume );
		ConfigManager::SetFloat( sMusicVolume, MusicVolume );
	}
}

// TODO: Parameterize these functions and move to mathcore, they're pretty useful.
inline float EldritchFramework::GetMouseSpeedFromSliderValue( const float SliderValue )
{
	// TODO: Parameterize this so I can tune it.
	// This should scale neatly from 1/8 to 8 on a power curve, with 1 in the middle.
	static const float	sMouseScaleRange	= 8.0f;
	const float			AdjustedSliderValue	= ( SliderValue * 2.0f ) - 1.0f;
	const float			MouseSpeed			= Pow( sMouseScaleRange, AdjustedSliderValue );
	return MouseSpeed;
}

inline float EldritchFramework::GetSliderValueFromMouseSpeed( const float MouseSpeed )
{
	static const float	sMouseScaleRange	= 8.0f;
	const float			AdjustedSliderValue	= LogBase( MouseSpeed, sMouseScaleRange );
	const float			SliderValue			= ( AdjustedSliderValue + 1.0f ) * 0.5f;
	return SliderValue;
}

inline float EldritchFramework::GetControllerSpeedFromSliderValue( const float SliderValue )
{
	// TODO: Parameterize this so I can tune it.
	// This should scale neatly from 1/2 to 2 on a power curve, with 1 in the middle.
	static const float	sControllerScaleRange	= 2.0f;
	const float			AdjustedSliderValue	= ( SliderValue * 2.0f ) - 1.0f;
	const float			ControllerSpeed			= Pow( sControllerScaleRange, AdjustedSliderValue );
	return ControllerSpeed;
}

inline float EldritchFramework::GetSliderValueFromControllerSpeed( const float ControllerSpeed )
{
	static const float	sControllerScaleRange	= 2.0f;
	const float			AdjustedSliderValue	= LogBase( ControllerSpeed, sControllerScaleRange );
	const float			SliderValue			= ( AdjustedSliderValue + 1.0f ) * 0.5f;
	return SliderValue;
}

inline float EldritchFramework::GetBrightnessFromSliderValue( const float SliderValue )
{
	// TODO: Parameterize this so I can tune it.
	// This should scale neatly from 2 to 1/2 on a power curve, with 1 in the middle.
	static const float	sBrightnessScaleRange	= 2.0f;
	const float			AdjustedSliderValue		= ( SliderValue * -2.0f ) + 1.0f;
	const float			Brightness				= Pow( sBrightnessScaleRange, AdjustedSliderValue );
	return Brightness;
}

inline float EldritchFramework::GetSliderValueFromBrightness( const float Brightness )
{
	static const float	sBrightnessScaleRange	= 2.0f;
	const float			AdjustedSliderValue		= LogBase( Brightness, sBrightnessScaleRange );
	const float			SliderValue				= ( AdjustedSliderValue - 1.0f ) * -0.5f;
	return SliderValue;
}

inline float EldritchFramework::GetFOVFromSliderValue( const float SliderValue )
{
	// TODO: Configurate
	static const float LoFOV = 60.0f;
	static const float HiFOV = 120.0f;
	return Lerp( LoFOV, HiFOV, SliderValue );
}

inline float EldritchFramework::GetSliderValueFromFOV( const float FOV )
{
	// TODO: Configurate
	static const float LoFOV = 60.0f;
	static const float HiFOV = 120.0f;
	return InvLerp( FOV, LoFOV, HiFOV );
}

void EldritchFramework::InitializeTools()
{
#if BUILD_ELDRITCH_TOOLS
	SafeDelete( m_Tools );

	m_Tools = new EldritchTools;
	m_Tools->InitializeFromDefinition( "EldritchTools" );
#endif
}

void EldritchFramework::ShutDownWorld()
{
	XTRACE_FUNCTION;

	SafeDelete( m_World );
#if BUILD_ELDRITCH_TOOLS
	SafeDelete( m_Tools );
#endif

	m_Audio3DListener->SetWorld( NULL );

	WBWorld::DeleteInstance();
}

/*virtual*/ void EldritchFramework::ShutDown()
{
	XTRACE_FUNCTION;

	// Shutting down game also saves the game in progress.
	m_Game->ShutDown();

	WritePrefsConfig();

	PackStream::StaticShutDown();

	WBComponent::ShutDownBaseFactories();
	RodinBTNodeFactory::ShutDown();
	WBActionFactory::ShutDown();
	WBParamEvaluatorFactory::ShutDown();
	AnimEventFactory::DeleteInstance();
	UIFactory::ShutDown();
	WBActionStack::ShutDown();
	EldritchParticles::FlushConfigCache();
	SDPFactory::ShutDown();

	ShutDownWorld();

#if BUILD_WINDOWS
	SafeDelete( m_CheckForUpdates );
#endif
	SafeDelete( m_Game );
	SafeDelete( m_MainView );
	SafeDelete( m_FGView );
	SafeDelete( m_HUDView );
	SafeDelete( m_MirrorView );
	SafeDelete( m_MinimapView );
	SafeDelete( m_TargetManager );
	SafeDelete( m_InputSystem );
	SafeDelete( m_Controller );
	SafeDelete( m_Audio3DListener );

	DynamicMeshManager::DeleteInstance();

	Framework3D::ShutDown();

	ReverseHash::ShutDown();
}

void EldritchFramework::LoadPrefsConfig()
{
	const SimpleString PrefsConfigFilename = GetSaveLoadPath() + SimpleString( "prefs.cfg" );
	if( FileUtil::Exists( PrefsConfigFilename.CStr() ) )
	{
		const FileStream PrefsConfigStream = FileStream( PrefsConfigFilename.CStr(), FileStream::EFM_Read );
		ConfigManager::Load( PrefsConfigStream );
	}
}

void EldritchFramework::WritePrefsConfig()
{
	const SimpleString PrefsConfigFilename = GetSaveLoadPath() + SimpleString( "prefs.cfg" );
	const FileStream PrefsConfigStream = FileStream( PrefsConfigFilename.CStr(), FileStream::EFM_Write );

	PrefsConfigStream.PrintF( "# This file is automatically generated.\n# You may delete it to restore defaults.\n\n" );

	ConfigManager::BeginWriting();

	ConfigManager::Write( PrefsConfigStream, "Language" );
	ConfigManager::Write( PrefsConfigStream, "DisplayWidth" );
	ConfigManager::Write( PrefsConfigStream, "DisplayHeight" );
	ConfigManager::Write( PrefsConfigStream, "Fullscreen" );
	ConfigManager::Write( PrefsConfigStream, "OpenGL" );
	ConfigManager::Write( PrefsConfigStream, "Brightness" );
	ConfigManager::Write( PrefsConfigStream, "FOV" );
	ConfigManager::Write( PrefsConfigStream, "VSync" );
	ConfigManager::Write( PrefsConfigStream, "MouseSpeed" );
	ConfigManager::Write( PrefsConfigStream, "ControllerSpeed" );
	ConfigManager::Write( PrefsConfigStream, "InvertY" );
	ConfigManager::Write( PrefsConfigStream, "MasterVolume" );
	ConfigManager::Write( PrefsConfigStream, "MusicVolume" );

	m_InputSystem->WriteConfigBinds( PrefsConfigStream );
}

/*virtual*/ bool EldritchFramework::TickSim( float DeltaTime )
{
	XTRACE_FUNCTION;

#if BUILD_WINDOWS
	m_CheckForUpdates->Tick();
#endif

	if( !HasFocus() && !m_UIManager->GetUIStack()->PausesGame() )
	{
		Pause();
	}

	return Framework3D::TickSim( DeltaTime );
}

/*virtual*/ bool EldritchFramework::TickGame( float DeltaTime )
{
	XTRACE_FUNCTION;

	// Oh this is horrible. But it works. Since InputSystem is a "game" system
	// (i.e., part of the realtime sim and not the UI, etc.), tick it here.
	// This way, we *do* update the input system on the same frame that the
	// game gets unpaused, even though we *don't* call TickInput (because we
	// want to ignore the Escape press). This whole system probably needs to
	// be reconsidered, but whatever. It gets this game finished.
	m_InputSystem->Tick();

	m_Game->Tick();

#if BUILD_ELDRITCH_TOOLS
	if( m_Tools->IsInToolMode() )
	{
		m_Tools->Tick( DeltaTime );
	}
	else
#endif
	{
		m_World->Tick( DeltaTime );
	}

	return true;
}

/*virtual*/ void EldritchFramework::OnUnpaused()
{
	Framework3D::OnUnpaused();

	// HACK: If we've just unpaused, tick input system again so we ignore rising inputs on the next frame.
	m_InputSystem->Tick();
}

/*virtual*/ void EldritchFramework::TickDevices()
{
	XTRACE_FUNCTION;

	Framework3D::TickDevices();

	m_Controller->Tick( 0.0f );

	// If we're using either controller or mouse exclusively, allow or disallow the cursor as needed.
	const bool IsUsingController	= m_Controller->ReceivedInputThisTick();
	const bool IsUsingMouse			= m_Mouse->ReceivedInputThisTick();
	if( IsUsingController != IsUsingMouse )
	{
		m_Mouse->SetAllowCursor( IsUsingMouse );
	}
}

void EldritchFramework::Pause()
{
	XTRACE_FUNCTION;

	if( EldritchGame::IsPlayerAlive() && !EldritchGame::IsPlayerDisablingPause() )
	{
		UIScreen* const pPauseScreen = m_UIManager->GetScreen( "PauseScreen" );
		ASSERT( pPauseScreen );
		m_UIManager->GetUIStack()->Repush( pPauseScreen );
	}
}

/*virtual*/ bool EldritchFramework::TickInput( float DeltaTime )
{
	XTRACE_FUNCTION;

	if( !Framework3D::TickInput( DeltaTime ) )
	{
		return false;
	}

	if( m_Keyboard->OnRise( Keyboard::EB_Escape ) || m_Controller->OnRise( XInputController::EB_Start ) )
	{
		Pause();
	}

#if BUILD_ELDRITCH_TOOLS
	if( m_Keyboard->OnRise( Keyboard::EB_Tab ) )
	{
		m_Tools->ToggleToolMode();
	}

	if( m_Tools->IsInToolMode() )
	{
		m_Tools->TickInput();
		return true;
	}
#endif

#if BUILD_FINAL
	if( m_Keyboard->OnRise( Keyboard::EB_F6 ) )
	{
		m_Game->Checkpoint();
	}
#else
	{
		if( m_Keyboard->OnRise( Keyboard::EB_F6 ) )	// Quicksave
		{
			m_Game->GetSaveLoad()->SaveMaster( "quick.eldritchmastersave" );
		}

		if( m_Keyboard->OnRise( Keyboard::EB_F9 ) )	// Quickload
		{
			PrepareForLoad();
			m_Game->GetSaveLoad()->TryLoadMaster( "quick.eldritchmastersave" );
		}

#if BUILD_WINDOWS_NO_SDL
		// Alt + S saves
		if( m_Keyboard->IsHigh( Keyboard::EB_LeftAlt ) && m_Keyboard->OnRise( Keyboard::EB_S ) )
		{
			SimpleString SaveFileName;
			if( FileUtil::GetSaveFile( GetWindow()->GetHWnd(), "Save Files", "eldritchmastersave", SaveFileName ) )
			{
				m_Game->GetSaveLoad()->SaveMaster( SaveFileName );
			}
		}

		// Alt + L loads
		if( m_Keyboard->IsHigh( Keyboard::EB_LeftAlt ) && m_Keyboard->OnRise( Keyboard::EB_L ) )
		{
			SimpleString LoadFileName;
			if( FileUtil::GetLoadFile( GetWindow()->GetHWnd(), "Save Files", "eldritchmastersave", LoadFileName ) )
			{
				PrepareForLoad();
				m_Game->GetSaveLoad()->TryLoadMaster( LoadFileName );
			}
		}
#endif // BUILD_WINDOWS_NO_SDL
	}
#endif

#if BUILD_DEV
	if( m_Keyboard->OnRise( Keyboard::EB_R ) )
	{
		WBWorld::GetInstance()->Report();
		ReverseHash::ReportSize();
	}

	// Ctrl + Alt + Backspace: Invoke crash
	if( m_Keyboard->IsHigh( Keyboard::EB_LeftAlt ) && m_Keyboard->IsHigh( Keyboard::EB_LeftControl ) && m_Keyboard->OnRise( Keyboard::EB_Backspace ) )
	{
		WBEntity* const pEntity = NULL;
		pEntity->Tick( 0.0f );
	}

	// Shift + Alt + Backspace: Invoke crash by allocating all the memory
	if( m_Keyboard->IsHigh( Keyboard::EB_LeftAlt ) && m_Keyboard->IsHigh( Keyboard::EB_LeftShift ) && m_Keyboard->OnRise( Keyboard::EB_Backspace ) )
	{
		for(;;)
		{
			byte* pArray = new byte[ 32 ];
			pArray[0] = pArray[31];
		}
	}

	if( m_Keyboard->OnRise( Keyboard::EB_Backspace ) )
	{
		m_World->GatherStats();
	}

	// Shift + Ctrl + T is a full restart (new hub), like Resurrect.
	// Ctrl + T is a proxy return to hub (new worlds, same hub), like Return to Library.
	// Alt + T is a travel to next level.
	// Shift + Alt + T is a travel to prev level.
	if( m_Keyboard->OnRise( Keyboard::EB_T ) && m_Keyboard->IsHigh( Keyboard::EB_LeftControl ) )
	{
		WB_MAKE_EVENT( ReturnToHub, NULL );
		WB_SET_AUTO( ReturnToHub, Bool, Restart, true );
		WB_SET_AUTO( ReturnToHub, Bool, FlushHub, m_Keyboard->IsHigh( Keyboard::EB_LeftShift ) );
		WB_DISPATCH_EVENT( WBWorld::GetInstance()->GetEventManager(), ReturnToHub, m_Game );
	}
	else if( m_Keyboard->OnRise( Keyboard::EB_T ) && m_Keyboard->IsHigh( Keyboard::EB_LeftAlt ) )
	{
		if( m_Keyboard->IsHigh( Keyboard::EB_LeftShift ) )
		{
			WB_MAKE_EVENT( GoToPrevLevel, NULL );
			WB_DISPATCH_EVENT( WBWorld::GetInstance()->GetEventManager(), GoToPrevLevel, m_Game );
		}
		else
		{
			WB_MAKE_EVENT( GoToNextLevel, NULL );
			WB_DISPATCH_EVENT( WBWorld::GetInstance()->GetEventManager(), GoToNextLevel, m_Game );
		}
	}
	else if( m_Keyboard->OnRise( Keyboard::EB_T ) )
	{
		RegenerateWorld();
	}
	else if( m_Keyboard->IsHigh( Keyboard::EB_T ) &&
		m_Keyboard->IsHigh( Keyboard::EB_LeftShift ) &&
		m_Keyboard->IsLow( Keyboard::EB_LeftControl ) &&
		m_Keyboard->IsLow( Keyboard::EB_LeftAlt ) )
	{
		RegenerateWorld();
	}
#endif

	return true;
}

/*virtual*/ void EldritchFramework::TickPausedInput()
{
	XTRACE_FUNCTION;

	Framework3D::TickPausedInput();

	if( m_InputSystem->IsBinding() )
	{
		m_InputSystem->Tick();

		// HACK: If we finished or canceled binding, pop the bind dialog.
		if( !m_InputSystem->IsBinding() )
		{
			STATIC_HASHED_STRING( BindDialog );
			if( m_UIManager->GetUIStack()->Top() == m_UIManager->GetScreen( sBindDialog ) )
			{
				m_UIManager->GetUIStack()->Pop();
			}
		}
	}
}

/*virtual*/ void EldritchFramework::TickRender()
{
	XTRACE_FUNCTION;

#if BUILD_ELDRITCH_TOOLS
	if( m_Tools->IsInToolMode() )
	{
		m_Tools->TickRender();
	}
	else
#endif
	{
		m_World->Render();

#if BUILD_DEV
		m_World->DebugRender();
#endif

		WBWorld::GetInstance()->Render();

#if BUILD_DEV
		WBWorld::GetInstance()->DebugRender();
#endif
	}

	m_Renderer->AddMesh( m_Game->GetPostQuad() );

	Framework3D::TickRender();
}

void EldritchFramework::SetMainViewTransform( const Vector& Location, const Angles& Orientation )
{
	m_MainView->m_Location = Location;
	m_MainView->m_Rotation = Orientation;

	m_FGView->m_Location = Location;
	m_FGView->m_Rotation = Orientation;

	m_Audio3DListener->SetLocation( Location );
	m_Audio3DListener->SetRotation( Orientation );

#if COOL_SIDE_VIEW_THING
	new( m_FGView ) View( Vector(), Vector(), SRect(), 0.0f, 0.0f );
	new( m_MainView ) View( Vector( 25.0f, 25.0f, 13.0f ), Vector( 0.0f, 1.0f, 0.0f ), SRect( -25.0f, 13.0f, 25.0f, -13.0f ), 0.0f, 50.0f );
#endif
}

void EldritchFramework::PrepareForLoad()
{
	XTRACE_FUNCTION;

	m_InputSystem->PopAllContexts();

	static bool FirstLoad = true;
	if( FirstLoad )
	{
		FirstLoad = false;
	}
	else
	{
		STATIC_HASHED_STRING( HUD );
		m_UIManager->GetUIStack()->Clear();
		m_UIManager->GetUIStack()->Push( m_UIManager->GetScreen( sHUD ) );
	}
}

/*virtual*/ void EldritchFramework::ToggleFullscreen()
{
	PRINTF( "EldritchFramework::ToggleFullscreen\n" );

	Framework3D::ToggleFullscreen();

	ASSERT( m_Game );
	m_Game->RefreshRTDependentSystems();
}

/*virtual*/ void EldritchFramework::SetResolution( const uint DisplayWidth, const uint DisplayHeight )
{
	PRINTF( "EldritchFramework::SetResolution\n" );

	m_DisplayWidth	= DisplayWidth;
	m_DisplayHeight	= DisplayHeight;

	Framework3D::SetResolution( DisplayWidth, DisplayHeight );

	m_TargetManager->CreateTargets( DisplayWidth, DisplayHeight );

	UpdateViews();

	ASSERT( m_Game );
	m_Game->RefreshRTDependentSystems();

	CreateBuckets();
}

void EldritchFramework::CreateBuckets()
{
	PRINTF( "EldritchFramework::CreateBuckets\n" );

	IRenderTarget* const pMainRT = m_TargetManager->GetPrimaryRenderTarget();
	IRenderTarget* const pScrnRT = m_TargetManager->GetOriginalRenderTarget();
	IRenderTarget* const pMirrRT = m_TargetManager->GetMirrorRenderTarget();
	IRenderTarget* const pMMapRT = m_TargetManager->GetMinimapRenderTarget();

	m_Renderer->FreeBuckets();

#define ADDBUCKET m_Renderer->AddBucket
#define BUCKET new Bucket

										// View			// RT		// Flags					// Filter					// Excl	// Clear
#if COOL_SIDE_VIEW_THING
	ADDBUCKET( "Main",			BUCKET( m_MainView,		pMainRT,	MAT_WORLD,					MAT_ALPHA | MAT_DYNAMIC,	true,	CLEAR_DEPTH | CLEAR_COLOR ) );
	ADDBUCKET( "MainDynamic",	BUCKET( NULL,			NULL,		MAT_WORLD | MAT_DYNAMIC,	MAT_ALPHA,					true ) );
#else
	ADDBUCKET( "Main",			BUCKET( m_MainView,		pMainRT,	MAT_WORLD,					MAT_ALPHA | MAT_DYNAMIC,	true,	CLEAR_DEPTH ) );
	ADDBUCKET( "MainDynamic",	BUCKET( NULL,			NULL,		MAT_WORLD | MAT_DYNAMIC,	MAT_ALPHA,					true ) );
#endif
#if BUILD_DEV
	ADDBUCKET( "MainDebug",		BUCKET(	NULL,			NULL,		MAT_DEBUG_WORLD,			MAT_NONE,					true ) );
#endif
	ADDBUCKET( "MainAlpha",		BUCKET( NULL,			NULL,		MAT_WORLD | MAT_ALPHA,		MAT_NONE,					true ) );
	ADDBUCKET( "MainFG",		BUCKET( m_FGView,		NULL,		MAT_FOREGROUND,				MAT_ALPHA,					true,	CLEAR_DEPTH ) );
	ADDBUCKET( "MainFGAlpha",	BUCKET( NULL,			NULL,		MAT_FOREGROUND | MAT_ALPHA,	MAT_NONE,					true,	CLEAR_DEPTH ) );
	ADDBUCKET( "Mirror",		BUCKET( m_MirrorView,	pMirrRT,	MAT_OFFSCREEN_0,			MAT_NONE,					true,	CLEAR_DEPTH ) );
	ADDBUCKET( "Minimap",		BUCKET( m_MinimapView,	pMMapRT,	MAT_OFFSCREEN_1,			MAT_NONE,					true,	CLEAR_DEPTH | CLEAR_COLOR ) );
	ADDBUCKET( "Post",			BUCKET( m_HUDView,		pScrnRT,	MAT_POSTFX,					MAT_NONE,					true,	CLEAR_DEPTH ) );
	ADDBUCKET( "HUD",			BUCKET( NULL,			NULL,		MAT_HUD,					MAT_NONE,					true,	CLEAR_DEPTH ) );
#if BUILD_DEV
	ADDBUCKET( "HUDDebug",		BUCKET( NULL,			NULL,		MAT_DEBUG_HUD,				MAT_NONE,					true ) );
#endif

#undef ADDBUCKET
#undef BUCKET

	// Get buckets by name and set properties here because I didn't want to change that horrid bucket constructor
	m_Renderer->GetBucket( "MainDynamic" )->m_SortByMaterial = true;
}

void EldritchFramework::UpdateViews()
{
	PRINTF( "EldritchFramework::UpdateViews\n" );

	const float		fDisplayWidth	= static_cast<float>( m_DisplayWidth );
	const float		fDisplayHeight	= static_cast<float>( m_DisplayHeight );
	const float		AspectRatio		= fDisplayWidth / fDisplayHeight;

	m_MainView->SetAspectRatio( AspectRatio );
	m_FGView->SetAspectRatio( AspectRatio );

	CreateHUDView();
	CreateMirrorView();
	CreateMinimapView();
}

void EldritchFramework::CreateHUDView()
{
	PRINTF( "EldritchFramework::CreateHUDView\n" );

	SafeDelete( m_HUDView );

	const float		fDisplayWidth	= static_cast<float>( m_DisplayWidth );
	const float		fDisplayHeight	= static_cast<float>( m_DisplayHeight );
	const Vector	EyePosition		= Vector( 0.0f, -1.0f, 0.0f );
	const Vector	EyeDirection	= Vector( 0.0f, 1.0f, 0.0f );
	const float		NearClip		= 0.01f;
	const float		FarClip			= 2.0f;

	m_HUDView						= new View( EyePosition, EyeDirection, SRect( 0.0f, 0.0f, fDisplayWidth, fDisplayHeight ), NearClip, FarClip );
}

void EldritchFramework::CreateMirrorView()
{
	PRINTF( "EldritchFramework::CreateMirrorView\n" );

	SafeDelete( m_MirrorView );

	UIScreenEldMirror* const	pMirrorScreen	= EldritchGame::GetMirrorScreen();

	const float		fMirrorRTWidth	= static_cast<float>( pMirrorScreen->GetMirrorRTWidth() );
	const float		fMirrorRTHeight	= static_cast<float>( pMirrorScreen->GetMirrorRTHeight() );
	const float		AspectRatio		= fMirrorRTWidth / fMirrorRTHeight;
	const Matrix	MirrorRotation	= Matrix::CreateRotationAboutZ( pMirrorScreen->GetMirrorYaw() );	// For beauty lighting with cube lights
	const Vector	EyePosition		= MirrorRotation * Vector( 0.0f, pMirrorScreen->GetMirrorViewDistance(), pMirrorScreen->GetMirrorViewHeight() );
	const Vector	EyeDirection	= MirrorRotation * Vector( 0.0f, -1.0f, 0.0f );
	const float		NearClip		= pMirrorScreen->GetMirrorViewNearClip();
	const float		FarClip			= pMirrorScreen->GetMirrorViewFarClip();
	const float		FOV				= pMirrorScreen->GetMirrorViewFOV();

	m_MirrorView					= new View( EyePosition, EyeDirection, FOV, AspectRatio, NearClip, FarClip );
}

void EldritchFramework::CreateMinimapView()
{
	PRINTF( "EldritchFramework::CreateMinimapView\n" );

	SafeDelete( m_MinimapView );

	STATICHASH( EldMinimap );

	STATICHASH( MinimapRTWidth );
	const float		fMinimapRTWidth		= ConfigManager::GetFloat( sMinimapRTWidth, 0.0f, sEldMinimap );

	STATICHASH( MinimapRTHeight );
	const float		fMinimapRTHeight	= ConfigManager::GetFloat( sMinimapRTHeight, 0.0f, sEldMinimap );

	STATICHASH( MinimapViewDistance );
	const float		ViewDistance		= ConfigManager::GetFloat( sMinimapViewDistance, 0.0f, sEldMinimap );

	STATICHASH( MinimapViewPitch );
	const float		ViewPitch			= DEGREES_TO_RADIANS( ConfigManager::GetFloat( sMinimapViewPitch, 0.0f, sEldMinimap ) );

	STATICHASH( MinimapViewFOV );
	const float		FOV					= ConfigManager::GetFloat( sMinimapViewFOV, 0.0f, sEldMinimap );

	STATICHASH( MinimapViewNearClip );
	const float		NearClip			= ConfigManager::GetFloat( sMinimapViewNearClip, 0.0f, sEldMinimap );

	STATICHASH( MinimapViewFarClip );
	const float		FarClip				= ConfigManager::GetFloat( sMinimapViewFarClip, 0.0f, sEldMinimap );

	const Angles	EyeOrientation		= Angles( ViewPitch, 0.0f, 0.0f );
	const Vector	EyeDirection		= EyeOrientation.ToVector();
	const Vector	EyePosition			= -EyeDirection * ViewDistance;
	const float		AspectRatio			= fMinimapRTWidth / fMinimapRTHeight;

	m_MinimapView						= new View( EyePosition, EyeOrientation, FOV, AspectRatio, NearClip, FarClip );
}

/*virtual*/ void EldritchFramework::RefreshDisplay( const bool Fullscreen, const uint DisplayWidth, const uint DisplayHeight )
{
	PRINTF( "EldritchFramework::RefreshDisplay\n" );

	Framework3D::RefreshDisplay( Fullscreen, DisplayWidth, DisplayHeight );

	m_TargetManager->CreateTargets( DisplayWidth, DisplayHeight );

	UpdateViews();

	ASSERT( m_Game );
	m_Game->RefreshRTDependentSystems();

	CreateBuckets();
}

void EldritchFramework::RegenerateWorld()
{
	ASSERT( m_World );
	InitializeWorld( m_World->GetCurrentWorld(), true );
}

void EldritchFramework::GoToLevel( const HashedString& WorldDef )
{
	InitializeWorld( WorldDef, true );
}

// Save to working directory on Windows and Linux. On Mac, save in proper location.
/*virtual*/ SimpleString EldritchFramework::GetSaveLoadPath()
{
#if BUILD_MAC
	return ObjCJunk::GetUserDirectory();
#else
	return SimpleString( "./" );
#endif
}
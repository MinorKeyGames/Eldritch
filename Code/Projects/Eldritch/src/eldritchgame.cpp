#include "core.h"
#include "eldritchgame.h"
#include "wbeventmanager.h"
#include "eldritchframework.h"
#include "eldritchworld.h"
#include "wbworld.h"
#include "eldritchsaveload.h"
#include "eldritchpersistence.h"
#include "eldritchbank.h"
#include "eldritchmusic.h"
#include "configmanager.h"
#include "texturemanager.h"
#include "irenderer.h"
#include "mesh.h"
#include "eldritchtargetmanager.h"
#include "meshfactory.h"
#include "irendertarget.h"
#include "shadermanager.h"
#include "Common/uimanagercommon.h"
#include "uiscreen.h"
#include "uiwidget.h"
#include "wbscene.h"
#include "Components/wbcompeldhealth.h"
#include "Components/wbcompeldtransform.h"
#include "Components/wbcompeldcamera.h"
#include "Components/wbcompeldplayer.h"
#include "Components/wbcompeldvisible.h"
#include "Screens/uiscreen-eldmirror.h"
#include "Widgets/uiwidget-image.h"
#include "allocator.h"
#include "wbcomponentarrays.h"

#if BUILD_WINDOWS
#include <Windows.h>	// For ShellExecute
#endif

EldritchGame::EldritchGame()
:	m_GoToLevelOnNextTick( false )
,	m_IsRestarting( false )
,	m_FlushHubOnRestart( false )
,	m_RestoreSpawnPoint( false )
,	m_NextLevelName()
,	m_NextWorldDef()
,	m_CurrentLevelName()
,	m_TravelPersistence()
,	m_SaveLoad( NULL )
,	m_GenerationPersistence( NULL )
,	m_Bank( NULL )
,	m_Music( NULL )
,	m_Gamma( 0.0f )
,	m_PostQuad( NULL )
,	m_ColorGradingTexture()
,	m_FogTexture( NULL )
,	m_FogParams()
{
	m_ColorGradingTexture	= DEFAULT_TEXTURE;
	m_SaveLoad				= new EldritchSaveLoad;
	m_GenerationPersistence	= new EldritchPersistence;
	m_Bank					= new EldritchBank;
	m_Music					= new EldritchMusic;
}

EldritchGame::~EldritchGame()
{
	SafeDelete( m_SaveLoad );
	SafeDelete( m_GenerationPersistence );
	SafeDelete( m_Bank );
	SafeDelete( m_Music );
	SafeDelete( m_PostQuad );
}

void EldritchGame::SetCurrentLevelName( const SimpleString& LevelName )
{
	XTRACE_FUNCTION;

	m_CurrentLevelName = LevelName;

	MAKEHASH( m_CurrentLevelName );
	STATICHASH( Name );
	const SimpleString LevelFriendlyName = ConfigManager::GetString( sName, "", sm_CurrentLevelName );

	STATICHASH( HUD );
	STATICHASH( WorldName );
	ConfigManager::SetString( sWorldName, LevelFriendlyName.CStr(), sHUD );
}

/*static*/ TPersistence& EldritchGame::StaticGetTravelPersistence()
{
	return EldritchFramework::GetInstance()->GetGame()->GetTravelPersistence();
}

/*virtual*/ void EldritchGame::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	STATIC_HASHED_STRING( ReturnToHub );
	STATIC_HASHED_STRING( GoToNextLevel );
	STATIC_HASHED_STRING( GoToPrevLevel );
	STATIC_HASHED_STRING( GoToLevel );
	STATIC_HASHED_STRING( Checkpoint );
	STATIC_HASHED_STRING( TweetRIP );
	STATIC_HASHED_STRING( PlayMusic );
	STATIC_HASHED_STRING( StopMusic );
	STATIC_HASHED_STRING( LaunchWebSite );
	STATIC_HASHED_STRING( GoToLevelImmediate );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sReturnToHub )
	{
		STATIC_HASHED_STRING( Restart );
		const bool Restart = Event.GetBool( sRestart );

		STATIC_HASHED_STRING( FlushHub );
		const bool FlushHub = Event.GetBool( sFlushHub );

		RequestReturnToHub( Restart, FlushHub );
	}
	else if( EventName == sGoToNextLevel )
	{
		RequestGoToNextLevel();
	}
	else if( EventName == sGoToPrevLevel )
	{
		RequestGoToPrevLevel();
	}
	else if( EventName == sGoToLevel )
	{
		STATIC_HASHED_STRING( Level );
		const SimpleString Level = Event.GetString( sLevel );

		STATIC_HASHED_STRING( WorldDef );
		const HashedString WorldDef = Event.GetHash( sWorldDef );

		RequestGoToLevel( Level, WorldDef, true );
	}
	else if( EventName == sCheckpoint )
	{
		Checkpoint();
	}
	else if( EventName == sTweetRIP )
	{
		LaunchRIPTweet();
	}
	else if( EventName == sLaunchWebSite )
	{
		LaunchWebSite();
	}
	else if( EventName == sPlayMusic )
	{
		STATIC_HASHED_STRING( Music );
		const SimpleString Music = Event.GetString( sMusic );

		m_Music->PlayMusic( ( Music == "" ) ? m_CurrentMusic : Music );
	}
	else if( EventName == sStopMusic )
	{
		m_Music->StopMusic();
	}
	else if( EventName == sGoToLevelImmediate )
	{
		// This should only be called after we've queued a go-to.
		ASSERT( m_GoToLevelOnNextTick );
		if( m_GoToLevelOnNextTick )
		{
			GoToLevel();
		}

		// HACK: Tick world once to pump the event queue. Fixes title screen bugs. (Hack because this assumes we only ever use this when returning to title screen.)
		EldritchFramework* const pFramework = EldritchFramework::GetInstance();
		EldritchWorld* const pWorld = pFramework->GetWorld();
		pWorld->Tick( 0.0f );
	}
}

void EldritchGame::Initialize()
{
	if( GetSaveLoad()->TryLoadMaster() )
	{
		// We're good! Don't flush the master file; if the game crashes, we still want it around!
	}
	else
	{
		// There was no master file, or the master file didn't have a world state (was saved
		// when the player was dead, etc.). Create a new world, but don't mess with persistence.

		STATICHASH( EldritchWorld );
		STATICHASH( InitialLevel );
		SetCurrentLevelName( ConfigManager::GetString( sInitialLevel, "", sEldritchWorld ) );

		MAKEHASH( m_CurrentLevelName );

		STATICHASH( WorldDef );
		const HashedString InitialWorldDef = ConfigManager::GetHash( sWorldDef, HashedString::NullString, sm_CurrentLevelName );

		EldritchFramework* const pFramework = EldritchFramework::GetInstance();
		EldritchWorld* const pWorld = pFramework->GetWorld();
		pWorld->SetCurrentWorld( InitialWorldDef );
		pWorld->Create();
		pFramework->InitializeTools();

		RefreshUIReturnToHubEnabled();
	}
}

void EldritchGame::ShutDown()
{
	PRINTF( "Shutting down game\n" );

	GetSaveLoad()->SaveMaster();
	GetSaveLoad()->FlushWorldFiles();
}

void EldritchGame::Tick()
{
	XTRACE_FUNCTION;

	// This stuff is done in a tick instead of being event-driven,
	// because it needs to happen before the world tick.

	if( m_GoToLevelOnNextTick )
	{
		GoToLevel();
	}
}

void EldritchGame::ClearTravelPersistence()
{
	m_TravelPersistence.Clear();
}

// Restart means we're going to flush all the dungeon world files and travel back to the old hub (e.g., "Return to Hub" from menu).
// FlushHub means we're also flushing the hub, and not traveling (e.g., restarting after death).
// If neither of these is set, then we're traveling back and keeping dungeon world files (e.g., returning after finishing a world).
void EldritchGame::RequestReturnToHub( const bool Restart, const bool FlushHub )
{
	XTRACE_FUNCTION;

	const SimpleString InitialLevelName = GetHubLevelName();

	MAKEHASH( InitialLevelName );

	STATICHASH( WorldDef );
	const HashedString InitialWorldDef = ConfigManager::GetHash( sWorldDef, HashedString::NullString, sInitialLevelName );

	RequestGoToLevel( InitialLevelName, InitialWorldDef, false );

	if( Restart )
	{
		m_IsRestarting = true;

		if( FlushHub )
		{
			m_FlushHubOnRestart = true;
			ClearTravelPersistence();
		}
	}
}

void EldritchGame::RequestGoToNextLevel()
{
	XTRACE_FUNCTION;

	MAKEHASH( m_CurrentLevelName );

	STATICHASH( NextLevel );
	const SimpleString NextLevelName = ConfigManager::GetString( sNextLevel, "", sm_CurrentLevelName );

	MAKEHASH( NextLevelName );

	STATICHASH( WorldDef );
	const HashedString NextWorldDef = ConfigManager::GetHash( sWorldDef, HashedString::NullString, sNextLevelName );

	RequestGoToLevel( NextLevelName, NextWorldDef, true );
}

void EldritchGame::RequestGoToPrevLevel()
{
	XTRACE_FUNCTION;

	MAKEHASH( m_CurrentLevelName );

	STATICHASH( PrevLevel );
	const SimpleString PrevLevelName = ConfigManager::GetString( sPrevLevel, "", sm_CurrentLevelName );

	MAKEHASH( PrevLevelName );

	STATICHASH( WorldDef );
	const HashedString PrevWorldDef = ConfigManager::GetHash( sWorldDef, HashedString::NullString, sPrevLevelName );

	RequestGoToLevel( PrevLevelName, PrevWorldDef, false );
}

void EldritchGame::RequestGoToLevel( const SimpleString& NextLevel, const HashedString& NextWorldDef, const bool RestoreSpawnPoint )
{
	XTRACE_FUNCTION;

	if( NextLevel == "" || NextWorldDef == HashedString::NullString )
	{
		WARN;
		return;
	}

	// Make a checkpoint save, for crash protection
	Checkpoint();

	ASSERT( !m_GoToLevelOnNextTick );
	m_GoToLevelOnNextTick	= true;
	m_RestoreSpawnPoint		= RestoreSpawnPoint;
	m_NextLevelName			= NextLevel;
	m_NextWorldDef			= NextWorldDef;

	WBEventManager* const pEventManager = WBWorld::GetInstance()->GetEventManager();

	WB_MAKE_EVENT( PreLevelTransition, NULL );
	WB_DISPATCH_EVENT( pEventManager, PreLevelTransition, NULL );
}

SimpleString EldritchGame::DecorateWorldFileName( const SimpleString& LevelName ) const
{
	XTRACE_FUNCTION;
	const SimpleString SaveLoadPath			= EldritchFramework::GetInstance()->GetSaveLoadPath();
	const SimpleString DecoratedFileName	= SimpleString::PrintF( "%s%s.eldritchworldsave", SaveLoadPath.CStr(), LevelName.CStr() );
	return DecoratedFileName;
}

void EldritchGame::Checkpoint() const
{
	XTRACE_FUNCTION;

	if( GetSaveLoad()->ShouldSaveCurrentWorld() )
	{
		GetSaveLoad()->SaveMaster();
	}
}

void EldritchGame::GoToLevel()
{
	XTRACE_FUNCTION;

	PRINTF( "Traveling to level %s\n", m_NextLevelName.CStr() );

	const bool IsRestarting			= m_IsRestarting;
	const bool FlushHubOnRestart	= m_FlushHubOnRestart;

	if( IsRestarting )
	{
		if( FlushHubOnRestart )
		{
			// Don't save the current world, and flush everything.
			GetSaveLoad()->FlushWorldFiles();
		}
		else
		{
			// Don't save the current world, but wait to flush until we're done traveling.
		}
	}
	else
	{
		// Store a record of the world we're leaving so we can come back to it.
		GetSaveLoad()->SaveWorld( DecorateWorldFileName( m_CurrentLevelName ) );
	}

	m_GoToLevelOnNextTick	= false;
	m_IsRestarting			= false;
	m_FlushHubOnRestart		= false;
	SetCurrentLevelName( m_NextLevelName );

	if( GetSaveLoad()->TryLoadWorld( DecorateWorldFileName( m_NextLevelName ) ) )
	{
		// We're good!
	}
	else
	{
		EldritchFramework::GetInstance()->GoToLevel( m_NextWorldDef );
	}

	RefreshUIReturnToHubEnabled();

	if( IsRestarting && !FlushHubOnRestart )
	{
		// Flush worlds now that we're back in the library.
		GetSaveLoad()->FlushWorldFiles();
	}

	WBEventManager* const pEventManager = WBWorld::GetInstance()->GetEventManager();

	WB_MAKE_EVENT( PostLevelTransition, NULL );
	WB_SET_AUTO( PostLevelTransition, Bool, RestoreSpawnPoint, m_RestoreSpawnPoint );
	WB_DISPATCH_EVENT( pEventManager, PostLevelTransition, NULL );

	// Top off player's health if we're returning to the hub.
	if( IsInHub() )
	{
		WB_MAKE_EVENT( RestoreHealth, NULL );
		WB_LOG_EVENT( RestoreHealth );
		WB_DISPATCH_EVENT( pEventManager, RestoreHealth, GetPlayer() );
	}

	// Make a checkpoint save, for crash protection
	// (Queued because we want to pump the event queue once to initialize the world.)
	WB_MAKE_EVENT( Checkpoint, NULL );
	WB_LOG_EVENT( Checkpoint );
	WB_QUEUE_EVENT( pEventManager, Checkpoint, this );

	// Clear travel persistence now that we've successfully traveled and don't need it.
	ClearTravelPersistence();

	// And fade up.
	static const float kFadeInDuration = 0.5f;
	WB_MAKE_EVENT( FadeIn, NULL );
	WB_SET_AUTO( FadeIn, Float, Duration, kFadeInDuration );
	WB_DISPATCH_EVENT( pEventManager, FadeIn, NULL );

	PRINTF( "Travel finished\n" );
}

void EldritchGame::RefreshRTDependentSystems()
{
	XTRACE_FUNCTION;

	CreatePostQuad();
	UpdateMirror();
	UpdateMinimap();
}

void EldritchGame::UpdateMirror()
{
	XTRACE_FUNCTION;

	UIScreenEldMirror* const		pMirrorScreen	= GetMirrorScreen();
	ASSERT( pMirrorScreen );

	pMirrorScreen->OnMirrorRTUpdated();
}

void EldritchGame::UpdateMinimap()
{
	XTRACE_FUNCTION;

	UIWidgetImage* const pMinimapImage = GetMinimapImage();
	if( !pMinimapImage )
	{
		return;
	}

	EldritchFramework* const		pFramework		= EldritchFramework::GetInstance();
	ASSERT( pFramework );

	EldritchTargetManager* const	pTargetManager	= pFramework->GetTargetManager();
	ASSERT( pTargetManager );

	IRenderTarget* const			pMinimapRT		= pTargetManager->GetMinimapRenderTarget();
	ASSERT( pMinimapRT );

	ITexture* const					pTexture		= pMinimapRT->GetColorTextureHandle();
	ASSERT( pTexture );

	pMinimapImage->SetTexture( pTexture );
}

void EldritchGame::CreatePostQuad()
{
	XTRACE_FUNCTION;

	SafeDelete( m_PostQuad );

	EldritchFramework* const		pFramework		= EldritchFramework::GetInstance();
	ASSERT( pFramework );

	Display* const					pDisplay		= pFramework->GetDisplay();
	ASSERT( pDisplay );

	IRenderer* const				pRenderer		= pFramework->GetRenderer();
	ASSERT( pRenderer );

	EldritchTargetManager* const	pTargetManager	= pFramework->GetTargetManager();
	ASSERT( pTargetManager );

	// Half-pixel offset to align the pixels on the grid
	const float DisplayWidth	= static_cast<float>( pDisplay->m_Width );
	const float DisplayHeight	= static_cast<float>( pDisplay->m_Height );
	const float PixelGridOffset	= pRenderer->GetPixelGridOffset();
	const float OffsetWidth		= ( 0.5f * DisplayWidth ) - PixelGridOffset;
	const float OffsetHeight	= ( 0.5f * DisplayHeight ) - PixelGridOffset;

	m_PostQuad = pRenderer->GetMeshFactory()->CreatePlane( DisplayWidth, DisplayHeight, 1, 1, XZ_PLANE, false );
	m_PostQuad->m_Location = Vector( OffsetWidth, 0.0f, OffsetHeight );
	m_PostQuad->SetTexture( 0, pTargetManager->GetPrimaryRenderTarget()->GetColorTextureHandle() );
	m_PostQuad->SetTexture( 1, pRenderer->GetTextureManager()->GetTexture( m_ColorGradingTexture.CStr() ) );
	m_PostQuad->SetMaterialFlags( MAT_POSTFX );
	m_PostQuad->SetMaterialDefinition( "Material_Post", pRenderer );
}

void EldritchGame::SetColorGradingTexture( const SimpleString& TextureFilename )
{
	XTRACE_FUNCTION;

	m_ColorGradingTexture = TextureFilename;

	if( m_PostQuad )
	{
		EldritchFramework* const pFramework = EldritchFramework::GetInstance();
		ASSERT( pFramework );

		TextureManager* const pTextureManager = pFramework->GetRenderer()->GetTextureManager();
		ASSERT( pTextureManager );

		m_PostQuad->SetTexture( 1, pTextureManager->GetTexture( TextureFilename.CStr() ) );
	}
}

void EldritchGame::SetFogParams( const float FogNear, const float FogFar, const SimpleString& FogTextureFilename )
{
	XTRACE_FUNCTION;

	EldritchFramework* const pFramework = EldritchFramework::GetInstance();
	ASSERT( pFramework );

	TextureManager* const pTextureManager = pFramework->GetRenderer()->GetTextureManager();
	ASSERT( pTextureManager );

	m_FogParams.x	= FogNear;
	m_FogParams.y	= 1.0f / ( FogFar - FogNear );
	m_FogTexture	= pTextureManager->GetTexture( FogTextureFilename.CStr() );
}

SimpleString EldritchGame::GetHubLevelName() const
{
	STATICHASH( EldritchWorld );
	STATICHASH( InitialLevel );
	return ConfigManager::GetString( sInitialLevel, "", sEldritchWorld );
}

bool EldritchGame::IsInHub() const
{
	return m_CurrentLevelName == GetHubLevelName();
}

void EldritchGame::SetUIReturnToHubDisabled( const bool Disabled )
{
	STATIC_HASHED_STRING( PauseScreen );
	STATIC_HASHED_STRING( PausedReturnButton );

	WB_MAKE_EVENT( SetWidgetDisabled, NULL );
	WB_SET_AUTO( SetWidgetDisabled, Hash, Screen, sPauseScreen );
	WB_SET_AUTO( SetWidgetDisabled, Hash, Widget, sPausedReturnButton );
	WB_SET_AUTO( SetWidgetDisabled, Bool, Disabled, Disabled );
	WB_DISPATCH_EVENT( WBWorld::GetInstance()->GetEventManager(), SetWidgetDisabled, NULL );
}

void EldritchGame::RefreshUIReturnToHubEnabled()
{
	SetUIReturnToHubDisabled( IsInHub() );
}

const SimpleString EldritchGame::GetRIPDamage()
{
	XTRACE_FUNCTION;

	WBEntity* const			pPlayer		= GetPlayer();
	WBCompEldPlayer* const	pPlayerComp	= GET_WBCOMP( pPlayer, EldPlayer );

	const SimpleString DamageDesc = pPlayerComp->GetLastDamageDesc();

	MAKEHASH( DamageDesc );
	const SimpleString RIPDamage = ConfigManager::GetLocalizedString( sDamageDesc, "" );

	return RIPDamage;
}

const SimpleString EldritchGame::GetRIPLevel()
{
	STATICHASH( EldritchWorld );
	STATICHASH( DefaultRIPName );
	const SimpleString DefaultRIPName = ConfigManager::GetString( sDefaultRIPName, "", sEldritchWorld );

	MAKEHASH( m_CurrentLevelName );
	STATICHASH( RIPName );
	const SimpleString RIPLevelName = ConfigManager::GetString( sRIPName, DefaultRIPName.CStr(), sm_CurrentLevelName );

	MAKEHASH( RIPLevelName );
	const SimpleString RIPLevel = ConfigManager::GetLocalizedString( sRIPLevelName, "" );

	return RIPLevel;
}

// For a purchase link in the demo, if I do that.
void EldritchGame::LaunchWebSite()
{
	STATICHASH( WebSiteURL );
	const SimpleString WebSiteURL = ConfigManager::GetString( sWebSiteURL, "" );

#if BUILD_WINDOWS
	ShellExecute( NULL, "open", WebSiteURL.CStr(), NULL, NULL, SW_SHOWNORMAL );
#elif BUILD_MAC
	const SimpleString Command = SimpleString::PrintF( "open %s", WebSiteURL.CStr() );
	system( Command.CStr() );
#elif BUILD_LINUX
	const SimpleString Command = SimpleString::PrintF( "xdg-open %s", WebSiteURL.CStr() );
	system( Command.CStr() );
#endif
}

void EldritchGame::LaunchRIPTweet()
{
	STATICHASH( RIPFormat );
	const SimpleString RIPFormat = ConfigManager::GetLocalizedString( sRIPFormat, "" );

	STATICHASH( RIPURL );
	const SimpleString RIPURL = ConfigManager::GetLocalizedString( sRIPURL, "" );

	STATICHASH( RIPHashtags );
	const SimpleString RIPHashtags = ConfigManager::GetLocalizedString( sRIPHashtags, "" );

	STATICHASH( RIPTwitter );
	const SimpleString RIPTwitter = ConfigManager::GetLocalizedString( sRIPTwitter, "" );

	const SimpleString RIPDamage	= GetRIPDamage();
	const SimpleString RIPLevel		= GetRIPLevel();

	const SimpleString RIPFormatted	= SimpleString::PrintF( RIPFormat.CStr(), RIPDamage.CStr(), RIPLevel.CStr() );

	const SimpleString RIPEncoded	= RIPFormatted.URLEncodeUTF8();
	const SimpleString RIPTweet		= SimpleString::PrintF( RIPTwitter.CStr(), RIPEncoded.CStr(), RIPHashtags.CStr(), RIPURL.CStr() );

#if BUILD_WINDOWS
	ShellExecute( NULL, "open", RIPTweet.CStr(), NULL, NULL, SW_SHOWNORMAL );
#elif BUILD_MAC
	const SimpleString Command = SimpleString::PrintF( "open %s", RIPTweet.CStr() );
	system( Command.CStr() );
#elif BUILD_LINUX
	const SimpleString Command = SimpleString::PrintF( "xdg-open %s", RIPTweet.CStr() );
	system( Command.CStr() );
#endif
}

/*static*/ WBEntity* EldritchGame::GetPlayer()
{
	const Array<WBCompEldPlayer*>* const pPlayers = WBComponentArrays::GetComponents<WBCompEldPlayer>();
	if( !pPlayers )
	{
		return NULL;
	}

	const Array<WBCompEldPlayer*>& Players = *pPlayers;
	if( Players.Empty() )
	{
		return NULL;
	}

	WBCompEldPlayer* const pPlayer = Players[0];
	ASSERT( pPlayer );

	return pPlayer->GetEntity();
}

/*static*/ Vector EldritchGame::GetPlayerLocation()
{
	WBEntity* const pPlayer = GetPlayer();
	ASSERT( pPlayer );

	WBCompEldTransform* const pTransform = pPlayer->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	return pTransform->GetLocation();
}

/*static*/ Vector EldritchGame::GetPlayerViewLocation()
{
	WBEntity* const pPlayer = GetPlayer();
	ASSERT( pPlayer );

	WBCompEldTransform* const pTransform = pPlayer->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	WBCompEldCamera* const pCamera = GET_WBCOMP( pPlayer, EldCamera );
	ASSERT( pCamera );

	return pTransform->GetLocation() + pCamera->GetViewTranslationOffset( WBCompEldCamera::EVM_All );
}

/*static*/ Angles EldritchGame::GetPlayerViewOrientation()
{
	WBEntity* const pPlayer = GetPlayer();
	ASSERT( pPlayer );

	WBCompEldTransform* const pTransform = pPlayer->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	WBCompEldCamera* const pCamera = GET_WBCOMP( pPlayer, EldCamera );
	ASSERT( pCamera );

	return pTransform->GetOrientation() + pCamera->GetViewOrientationOffset( WBCompEldCamera::EVM_All );
}

/*static*/ bool EldritchGame::IsPlayerAlive()
{
	WBEntity* const pPlayer = GetPlayer();
	if( !pPlayer )
	{
		return false;
	}

	WBCompEldHealth* const pHealth = GET_WBCOMP( pPlayer, EldHealth );
	ASSERT( pHealth );
	if( pHealth->IsDead() )
	{
		return false;
	}

	return true;
}

/*static*/ bool EldritchGame::IsPlayerDisablingPause()
{
	WBEntity* const pPlayer = GetPlayer();
	if( !pPlayer )
	{
		return false;
	}

	WBCompEldPlayer* const pPlayerComponent = GET_WBCOMP( pPlayer, EldPlayer );
	ASSERT( pPlayerComponent );

	return pPlayerComponent->IsDisablingPause();
}

/*static*/ bool EldritchGame::IsPlayerVisible()
{
	WBEntity* const pPlayer = GetPlayer();
	if( !pPlayer )
	{
		return false;
	}

	WBCompEldVisible* const pVisible = GET_WBCOMP( pPlayer, EldVisible );
	ASSERT( pVisible );

	return pVisible->IsVisible();
}

/*static*/ UIScreenEldMirror* EldritchGame::GetMirrorScreen()
{
	EldritchFramework* const	pFramework		= EldritchFramework::GetInstance();
	ASSERT( pFramework );

	UIManager* const			pUIManager		= pFramework->GetUIManager();
	ASSERT( pUIManager );

	STATIC_HASHED_STRING( MirrorScreen );
	UIScreenEldMirror* const	pMirrorScreen	= pUIManager->GetScreen<UIScreenEldMirror>( sMirrorScreen );
	ASSERT( pMirrorScreen );

	return pMirrorScreen;
}

/*static*/ UIWidgetImage* EldritchGame::GetMinimapImage()
{
	EldritchFramework* const	pFramework		= EldritchFramework::GetInstance();
	ASSERT( pFramework );

	UIManager* const			pUIManager		= pFramework->GetUIManager();
	ASSERT( pUIManager );

	STATIC_HASHED_STRING( HUD );
	UIScreen* const				pHUDScreen		= pUIManager->GetScreen( sHUD );
	if( !pHUDScreen )
	{
		return NULL;
	}

	STATIC_HASHED_STRING( Minimap );
	UIWidgetImage* const		pMinimap		= pHUDScreen->GetWidget<UIWidgetImage>( sMinimap );
	return pMinimap;
}
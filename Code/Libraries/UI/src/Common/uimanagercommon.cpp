#include "core.h"
#include "uimanagercommon.h"
#include "uiscreen.h"
#include "uiwidget.h"
#include "uistack.h"
#include "configmanager.h"
#include "uifactory.h"
#include "wbeventmanager.h"

#include "Screens/uiscreen-okdialog.h"
#include "Screens/uiscreen-waitdialog.h"
#include "Screens/uiscreen-yesnodialog.h"

#include "Screens/uiscreen-fade.h"

UIManagerCommon::UIManagerCommon()
:	m_InitialScreens()
{
}

UIManagerCommon::~UIManagerCommon()
{
}

/*virtual*/ void UIManagerCommon::Initialize()
{
	UIManager::Initialize();

	AddScreen( "OKDialog",		new UIScreenOKDialog );
	AddScreen( "WaitDialog",	new UIScreenWaitDialog );
	AddScreen( "YesNoDialog",	new UIScreenYesNoDialog );
	AddScreen( "Fade",			new UIScreenFade( "Fade" ) );
	AddScreen( "GameFade",		new UIScreenFade( "GameFade" ) );
}

/*virtual*/ void UIManagerCommon::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Initialize();

	MAKEHASH( DefinitionName );

	STATICHASH( NumUIScreens );
	const uint NumUIScreens = ConfigManager::GetInt( sNumUIScreens, 0, sDefinitionName );
	for( uint UIScreenIndex = 0; UIScreenIndex < NumUIScreens; ++UIScreenIndex )
	{
		const SimpleString UIScreenName = ConfigManager::GetSequenceString( "UIScreen%d", UIScreenIndex, "", sDefinitionName );
		UIScreen* const pUIScreen = UIFactory::CreateScreen( UIScreenName );
		ASSERT( pUIScreen );

		AddScreen( UIScreenName, pUIScreen );
	}

	STATICHASH( NumInitialScreens );
	const uint NumInitialScreens = ConfigManager::GetInt( sNumInitialScreens, 0, sDefinitionName );
	for( uint InitialScreenIndex = 0; InitialScreenIndex < NumInitialScreens; ++InitialScreenIndex )
	{
		const HashedString InitialScreen = ConfigManager::GetSequenceHash( "InitialScreen%d", InitialScreenIndex, HashedString::NullString, sDefinitionName );
		UIScreen* const pInitialScreen = GetScreen( InitialScreen );
		ASSERT( pInitialScreen );

		m_InitialScreens.PushBack( pInitialScreen );
	}

	STATICHASH( PushInitialScreensOnInitialize );
	if( ConfigManager::GetBool( sPushInitialScreensOnInitialize, true, sDefinitionName ) )
	{
		ResetToInitialScreens();
	}
}

/*virtual*/ void UIManagerCommon::RegisterForEvents()
{
	UIManager::RegisterForEvents();

	STATIC_HASHED_STRING( ShowYesNoDialog );
	WBWorld::GetInstance()->GetEventManager()->AddObserver( sShowYesNoDialog, this, NULL );

	STATIC_HASHED_STRING( Fade );
	WBWorld::GetInstance()->GetEventManager()->AddObserver( sFade, this, NULL );

	STATIC_HASHED_STRING( FadeIn );
	WBWorld::GetInstance()->GetEventManager()->AddObserver( sFadeIn, this, NULL );

	STATIC_HASHED_STRING( FadeOut );
	WBWorld::GetInstance()->GetEventManager()->AddObserver( sFadeOut, this, NULL );

	STATIC_HASHED_STRING( ResetToInitialScreens );
	WBWorld::GetInstance()->GetEventManager()->AddObserver( sResetToInitialScreens, this, NULL );
}

/*virtual*/ void UIManagerCommon::UnregisterForEvents()
{
	UIManager::UnregisterForEvents();

	WBEventManager* const pEventManager = WBWorld::GetInstance()->GetEventManager();
	if( pEventManager )
	{
		STATIC_HASHED_STRING( ShowYesNoDialog );
		WBWorld::GetInstance()->GetEventManager()->RemoveObserver( sShowYesNoDialog, this, NULL );

		STATIC_HASHED_STRING( Fade );
		WBWorld::GetInstance()->GetEventManager()->RemoveObserver( sFade, this, NULL );

		STATIC_HASHED_STRING( FadeIn );
		WBWorld::GetInstance()->GetEventManager()->RemoveObserver( sFadeIn, this, NULL );

		STATIC_HASHED_STRING( FadeOut );
		WBWorld::GetInstance()->GetEventManager()->RemoveObserver( sFadeOut, this, NULL );

		STATIC_HASHED_STRING( ResetToInitialScreens );
		WBWorld::GetInstance()->GetEventManager()->RemoveObserver( sResetToInitialScreens, this, NULL );
	}
}

/*virtual*/ void UIManagerCommon::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	UIManager::HandleEvent( Event );

	STATIC_HASHED_STRING( ShowYesNoDialog );
	STATIC_HASHED_STRING( Fade );
	STATIC_HASHED_STRING( FadeIn );
	STATIC_HASHED_STRING( FadeOut );
	STATIC_HASHED_STRING( ResetToInitialScreens );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sShowYesNoDialog )
	{
		STATIC_HASHED_STRING( YesNoString );
		const SimpleString YesNoString = Event.GetString( sYesNoString );

		STATIC_HASHED_STRING( YesActions );
		const Array<WBAction*>* const pYesActions = Event.GetPointer<Array<WBAction*> >( sYesActions );

		// TODO: Parameterize these as needed.
		const bool						PauseGame			= true;
		const SimpleString				YesNoDynamicString	= "";
		const Array<WBAction*>* const	pNoActions			= NULL;

		ShowYesNoDialog( PauseGame, YesNoString, YesNoDynamicString, pYesActions, pNoActions );
	}
	else if( EventName == sFade )
	{
		STATIC_HASHED_STRING( FadeColorR );
		const float	FadeColorR		= Event.GetFloat( sFadeColorR );

		STATIC_HASHED_STRING( FadeColorG );
		const float	FadeColorG		= Event.GetFloat( sFadeColorG );

		STATIC_HASHED_STRING( FadeColorB );
		const float	FadeColorB		= Event.GetFloat( sFadeColorB );

		STATIC_HASHED_STRING( FadeColorAStart );
		const float	FadeColorAStart	= Event.GetFloat( sFadeColorAStart );

		STATIC_HASHED_STRING( FadeColorAEnd );
		const float	FadeColorAEnd	= Event.GetFloat( sFadeColorAEnd );

		STATIC_HASHED_STRING( Duration );
		const float Duration = Event.GetFloat( sDuration );

		const Vector4		StartColor	= Vector4( FadeColorR, FadeColorG, FadeColorB, FadeColorAStart );
		const Vector4		EndColor	= Vector4( FadeColorR, FadeColorG, FadeColorB, FadeColorAEnd );
		static const bool	kGameFade	= false;

		Fade( StartColor, EndColor, Duration, kGameFade );
	}
	else if( EventName == sFadeIn )
	{
		STATIC_HASHED_STRING( FadeColorR );
		const float	FadeColorR	= Event.GetFloat( sFadeColorR );

		STATIC_HASHED_STRING( FadeColorG );
		const float	FadeColorG	= Event.GetFloat( sFadeColorG );

		STATIC_HASHED_STRING( FadeColorB );
		const float	FadeColorB	= Event.GetFloat( sFadeColorB );

		STATIC_HASHED_STRING( Duration );
		const float Duration = Event.GetFloat( sDuration );

		const Vector4		StartColor	= Vector4( FadeColorR, FadeColorG, FadeColorB, 1.0f );
		const Vector4		EndColor	= Vector4( FadeColorR, FadeColorG, FadeColorB, 0.0f );
		static const bool	kGameFade	= false;

		Fade( StartColor, EndColor, Duration, kGameFade );
	}
	else if( EventName == sFadeOut )
	{
		STATIC_HASHED_STRING( FadeColorR );
		const float	FadeColorR	= Event.GetFloat( sFadeColorR );

		STATIC_HASHED_STRING( FadeColorG );
		const float	FadeColorG	= Event.GetFloat( sFadeColorG );

		STATIC_HASHED_STRING( FadeColorB );
		const float	FadeColorB	= Event.GetFloat( sFadeColorB );

		STATIC_HASHED_STRING( Duration );
		const float Duration = Event.GetFloat( sDuration );

		const Vector4		StartColor	= Vector4( FadeColorR, FadeColorG, FadeColorB, 0.0f );
		const Vector4		EndColor	= Vector4( FadeColorR, FadeColorG, FadeColorB, 1.0f );
		static const bool	kGameFade	= false;

		Fade( StartColor, EndColor, Duration, kGameFade );
	}
	else if( EventName == sResetToInitialScreens )
	{
		ResetToInitialScreens();
	}
}

void UIManagerCommon::ResetToInitialScreens()
{
	UIStack* const pStack = GetUIStack();

	pStack->Clear();

	FOR_EACH_ARRAY( InitialScreenIter, m_InitialScreens, UIScreen* )
	{
		UIScreen* const pInitialScreen = InitialScreenIter.GetValue();
		pStack->Push( pInitialScreen );
	}
}

void UIManagerCommon::ShowOKDialog(
	bool				PauseGame,
	const SimpleString&	OKString,
	const SimpleString&	OKDynamicString,
	const SimpleString&	OKEvent,
	const SimpleString&	OKCommand,
	const SUICallback&	OKCallback /*= SUICallback()*/ )
{
	static const HashedString sOKDialog( "OKDialog" );
	UIScreenOKDialog* pOKDialog = GetScreen<UIScreenOKDialog>( sOKDialog );

	pOKDialog->SetParameters( PauseGame, OKString, OKDynamicString, OKEvent, OKCommand, OKCallback );
	m_UIStack->Push( pOKDialog );
}

void UIManagerCommon::ShowWaitDialog(
	bool				PauseGame,
	const SimpleString&	WaitString,
	const SimpleString&	WaitDynamicString )
{
	static const HashedString sWaitDialog( "WaitDialog" );
	UIScreenWaitDialog* pWaitDialog = GetScreen<UIScreenWaitDialog>( sWaitDialog );

	pWaitDialog->SetParameters( PauseGame, WaitString, WaitDynamicString );
	m_UIStack->Push( pWaitDialog );
}

void UIManagerCommon::HideWaitDialog()
{
	static const HashedString sWaitDialog( "WaitDialog" );
	UIScreenWaitDialog* pWaitDialog = GetScreen<UIScreenWaitDialog>( sWaitDialog );

	if( m_UIStack->Top() == pWaitDialog )
	{
		m_UIStack->Pop();
	}
}

void UIManagerCommon::ShowYesNoDialog(
	bool				PauseGame,
	const SimpleString&	YesNoString,
	const SimpleString&	YesNoDynamicString,
	const SimpleString&	YesEvent,
	const SimpleString&	NoEvent,
	const SimpleString&	YesCommand,
	const SimpleString&	NoCommand,
	const SUICallback&	YesCallback /*= SUICallback()*/,
	const SUICallback&	NoCallback /*= SUICallback()*/ )
{
	STATIC_HASHED_STRING( YesNoDialog );
	UIScreenYesNoDialog* const pYesNoDialog = GetScreen<UIScreenYesNoDialog>( sYesNoDialog );
	ASSERT( pYesNoDialog );

	pYesNoDialog->SetParameters( PauseGame, YesNoString, YesNoDynamicString, YesEvent, NoEvent, YesCommand, NoCommand, YesCallback, NoCallback, NULL, NULL );
	m_UIStack->Push( pYesNoDialog );
}

void UIManagerCommon::ShowYesNoDialog(
	bool							PauseGame,
	const SimpleString&				YesNoString,
	const SimpleString&				YesNoDynamicString,
	const Array<WBAction*>* const	pYesActions,
	const Array<WBAction*>* const	pNoActions )
{
	STATIC_HASHED_STRING( YesNoDialog );
	UIScreenYesNoDialog* const pYesNoDialog = GetScreen<UIScreenYesNoDialog>( sYesNoDialog );
	ASSERT( pYesNoDialog );

	STATIC_HASHED_STRING( Pop );
	const SimpleString	UnusedCommand;
	const SUICallback	UnusedCallback;

	pYesNoDialog->SetParameters(
		PauseGame,
		YesNoString,
		YesNoDynamicString,
		sPop,
		sPop,
		UnusedCommand,
		UnusedCommand,
		UnusedCallback,
		UnusedCallback,
		pYesActions,
		pNoActions );

	m_UIStack->Push( pYesNoDialog );
}

void UIManagerCommon::Fade( const Vector4& StartColor, const Vector4& EndColor, const float Duration, const bool GameFade )
{
	static const HashedString sGameFade( "GameFade" );
	static const HashedString sFade( "Fade" );
	UIScreenFade* const pFade = GetScreen<UIScreenFade>( GameFade ? sGameFade : sFade );
	ASSERT( pFade );

	pFade->Fade( StartColor, EndColor, Duration );
}
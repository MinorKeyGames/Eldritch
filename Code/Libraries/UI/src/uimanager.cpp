#include "core.h"
#include "uimanager.h"
#include "uiscreen.h"
#include "uiwidget.h"
#include "Widgets/uiwidget-image.h"
#include "Widgets/uiwidget-slider.h"
#include "uistack.h"
#include "wbeventmanager.h"

UIManager::UIManager()
:	m_UIStack( NULL )
,	m_Screens()
,	m_Events()
#if BUILD_DEBUG
,	m_DEBUGIsSafeToUpdateRender( true )
,	m_DEBUGLastSafeToUpdateRenderTime( 0 )
#endif
{
	UIWidget::SetUIManager( this );
	UIScreen::SetUIManager( this );

	m_UIStack = new UIStack;
}

UIManager::~UIManager()
{
	SafeDelete( m_UIStack );

	FOR_EACH_MAP( ScreenIter, m_Screens, HashedString, UIScreen* )
	{
		SafeDelete( *ScreenIter );
	}
	m_Screens.Clear();

	m_Events.Clear();
}

/*virtual*/ void UIManager::Initialize()
{
}

void UIManager::Reinitialize()
{
	FOR_EACH_MAP( ScreenIter, m_Screens, HashedString, UIScreen* )
	{
		UIScreen* const pScreen = ScreenIter.GetValue();
		pScreen->Reinitialize();
	}
}

void UIManager::PostEvent( const HashedString& EventName )
{
	m_Events.PushBack( EventName );
}

void UIManager::ProcessEvents()
{
	XTRACE_FUNCTION;

	for( uint i = 0; i < m_Events.Size(); ++i )
	{
		ProcessEvent( m_Events[i] );
	}
	m_Events.Clear();
}

void UIManager::ProcessEvent( const HashedString& EventName )
{
	CHECKDESC( ProcessEventInternal( EventName ), "Unhandled UI event" );
}

/*virtual*/ bool UIManager::ProcessEventInternal( const HashedString& EventName )
{
	ASSERT( m_UIStack );

	static const HashedString shPop( "Pop" );

	if( EventName == shPop )
	{
		m_UIStack->Pop();
	}
	else
	{
		return false;
	}

	return true;
}

UIScreen* UIManager::AddScreen( const HashedString& Name, UIScreen* pScreen )
{
	ASSERT( !GetScreen( Name ) );
	m_Screens[ Name ] = pScreen;
	return pScreen;
}

UIScreen* UIManager::GetScreen( const HashedString& Name )
{
	Map< HashedString, UIScreen* >::Iterator ScreenIter = m_Screens.Search( Name );
	if( ScreenIter.IsNull() )
	{
		return NULL;
	}
	else
	{
		return *ScreenIter;
	}
}

/*virtual*/ void UIManager::RegisterForEvents()
{
	STATIC_HASHED_STRING( ReinitializeUI );
	WBWorld::GetInstance()->GetEventManager()->AddObserver( sReinitializeUI, this, NULL );

	STATIC_HASHED_STRING( PushUIScreen );
	WBWorld::GetInstance()->GetEventManager()->AddObserver( sPushUIScreen, this, NULL );

	STATIC_HASHED_STRING( PopUIScreen );
	WBWorld::GetInstance()->GetEventManager()->AddObserver( sPopUIScreen, this, NULL );

	STATIC_HASHED_STRING( ClearUIStack );
	WBWorld::GetInstance()->GetEventManager()->AddObserver( sClearUIStack, this, NULL );

	STATIC_HASHED_STRING( RemoveUIScreen );
	WBWorld::GetInstance()->GetEventManager()->AddObserver( sRemoveUIScreen, this, NULL );

	STATIC_HASHED_STRING( SetWidgetHidden );
	WBWorld::GetInstance()->GetEventManager()->AddObserver( sSetWidgetHidden, this, NULL );

	STATIC_HASHED_STRING( SetWidgetImage );
	WBWorld::GetInstance()->GetEventManager()->AddObserver( sSetWidgetImage, this, NULL );

	STATIC_HASHED_STRING( SetUISliderValue );
	WBWorld::GetInstance()->GetEventManager()->AddObserver( sSetUISliderValue, this, NULL );

	STATIC_HASHED_STRING( SetWidgetDisabled );
	WBWorld::GetInstance()->GetEventManager()->AddObserver( sSetWidgetDisabled, this, NULL );

	STATIC_HASHED_STRING( SetWidgetLocation );
	WBWorld::GetInstance()->GetEventManager()->AddObserver( sSetWidgetLocation, this, NULL );

	STATIC_HASHED_STRING( SetWidgetAlpha );
	WBWorld::GetInstance()->GetEventManager()->AddObserver( sSetWidgetAlpha, this, NULL );
}

/*virtual*/ void UIManager::UnregisterForEvents()
{
	WBEventManager* const pEventManager = WBWorld::GetInstance()->GetEventManager();
	if( pEventManager )
	{
		STATIC_HASHED_STRING( ReinitializeUI );
		WBWorld::GetInstance()->GetEventManager()->RemoveObserver( sReinitializeUI, this, NULL );

		STATIC_HASHED_STRING( PushUIScreen );
		WBWorld::GetInstance()->GetEventManager()->RemoveObserver( sPushUIScreen, this, NULL );

		STATIC_HASHED_STRING( PopUIScreen );
		WBWorld::GetInstance()->GetEventManager()->RemoveObserver( sPopUIScreen, this, NULL );

		STATIC_HASHED_STRING( ClearUIStack );
		WBWorld::GetInstance()->GetEventManager()->RemoveObserver( sClearUIStack, this, NULL );

		STATIC_HASHED_STRING( RemoveUIScreen );
		WBWorld::GetInstance()->GetEventManager()->RemoveObserver( sRemoveUIScreen, this, NULL );

		STATIC_HASHED_STRING( SetWidgetHidden );
		WBWorld::GetInstance()->GetEventManager()->RemoveObserver( sSetWidgetHidden, this, NULL );

		STATIC_HASHED_STRING( SetWidgetImage );
		WBWorld::GetInstance()->GetEventManager()->RemoveObserver( sSetWidgetImage, this, NULL );

		STATIC_HASHED_STRING( SetUISliderValue );
		WBWorld::GetInstance()->GetEventManager()->RemoveObserver( sSetUISliderValue, this, NULL );

		STATIC_HASHED_STRING( SetWidgetDisabled );
		WBWorld::GetInstance()->GetEventManager()->RemoveObserver( sSetWidgetDisabled, this, NULL );

		STATIC_HASHED_STRING( SetWidgetLocation );
		WBWorld::GetInstance()->GetEventManager()->RemoveObserver( sSetWidgetLocation, this, NULL );

		STATIC_HASHED_STRING( SetWidgetAlpha );
		WBWorld::GetInstance()->GetEventManager()->RemoveObserver( sSetWidgetAlpha, this, NULL );
	}
}

/*virtual*/ void UIManager::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	STATIC_HASHED_STRING( ReinitializeUI );
	STATIC_HASHED_STRING( PushUIScreen );
	STATIC_HASHED_STRING( PopUIScreen );
	STATIC_HASHED_STRING( ClearUIStack );
	STATIC_HASHED_STRING( RemoveUIScreen );
	STATIC_HASHED_STRING( SetWidgetHidden );
	STATIC_HASHED_STRING( SetWidgetImage );
	STATIC_HASHED_STRING( SetUISliderValue );
	STATIC_HASHED_STRING( SetWidgetDisabled );
	STATIC_HASHED_STRING( SetWidgetLocation );
	STATIC_HASHED_STRING( SetWidgetAlpha );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sReinitializeUI )
	{
		Reinitialize();
	}
	else if( EventName == sPushUIScreen )
	{
		XTRACE_NAMED( PushUIScreen );

		STATIC_HASHED_STRING( Screen );
		const HashedString Screen = Event.GetHash( sScreen );

		UIScreen* const pScreen = GetScreen( Screen );
		if( pScreen )
		{
			ASSERT( m_UIStack );
			m_UIStack->Push( pScreen );
		}
	}
	else if( EventName == sPopUIScreen )
	{
		XTRACE_NAMED( PopUIScreen );

		ASSERT( m_UIStack );
		m_UIStack->Pop();
	}
	else if( EventName == sClearUIStack )
	{
		XTRACE_NAMED( ClearUIStack );

		ASSERT( m_UIStack );
		m_UIStack->Clear();
	}
	else if( EventName == sRemoveUIScreen )
	{
		XTRACE_NAMED( RemoveUIScreen );

		STATIC_HASHED_STRING( Screen );
		const HashedString Screen = Event.GetHash( sScreen );

		UIScreen* const pScreen = GetScreen( Screen );
		if( pScreen )
		{
			ASSERT( m_UIStack );
			m_UIStack->Remove( pScreen );
		}
	}
	else if( EventName == sSetWidgetHidden )
	{
		XTRACE_NAMED( SetWidgetHidden );

		STATIC_HASHED_STRING( Screen );
		const HashedString Screen = Event.GetHash( sScreen );

		STATIC_HASHED_STRING( Widget );
		const HashedString Widget = Event.GetHash( sWidget );

		STATIC_HASHED_STRING( Hidden );
		const bool Hidden = Event.GetBool( sHidden );

		UIScreen* const pScreen = GetScreen( Screen );
		if( pScreen )
		{
			UIWidget* const pWidget = pScreen->GetWidget( Widget );
			if( pWidget )
			{
				pWidget->SetHidden( Hidden );
			}
		}
	}
	else if( EventName == sSetWidgetImage )
	{
		XTRACE_NAMED( SetWidgetImage );

		STATIC_HASHED_STRING( Screen );
		const HashedString Screen = Event.GetHash( sScreen );

		STATIC_HASHED_STRING( Widget );
		const HashedString Widget = Event.GetHash( sWidget );

		STATIC_HASHED_STRING( Image );
		const SimpleString Image = Event.GetString( sImage );

		UIScreen* const pScreen = GetScreen( Screen );
		if( pScreen )
		{
			UIWidgetImage* const pWidget = pScreen->GetWidget<UIWidgetImage>( Widget );
			if( pWidget )
			{
				pWidget->SetTexture( Image.CStr() );
				pWidget->UpdateRender();
			}
		}
	}
	else if( EventName == sSetUISliderValue )
	{
		XTRACE_NAMED( SetUISliderValue );

		STATIC_HASHED_STRING( Screen );
		const HashedString Screen = Event.GetHash( sScreen );

		STATIC_HASHED_STRING( Widget );
		const HashedString Widget = Event.GetHash( sWidget );

		STATIC_HASHED_STRING( Image );
		const SimpleString Image = Event.GetString( sImage );

		STATIC_HASHED_STRING( SliderValue );
		const float SliderValue = Event.GetFloat( sSliderValue );

		UIScreen* const pScreen = GetScreen( Screen );
		if( pScreen )
		{
			UIWidgetSlider* const pWidget = pScreen->GetWidget<UIWidgetSlider>( Widget );
			if( pWidget )
			{
				pWidget->SetSliderValue( SliderValue );
				pWidget->UpdateRender();
			}
		}
	}
	else if( EventName == sSetWidgetDisabled )
	{
		XTRACE_NAMED( SetWidgetDisabled );

		STATIC_HASHED_STRING( Screen );
		const HashedString Screen = Event.GetHash( sScreen );

		STATIC_HASHED_STRING( Widget );
		const HashedString Widget = Event.GetHash( sWidget );

		STATIC_HASHED_STRING( Disabled );
		const bool Disabled = Event.GetBool( sDisabled );

		UIScreen* const pScreen = GetScreen( Screen );
		if( pScreen )
		{
			UIWidget* const pWidget = pScreen->GetWidget( Widget );
			if( pWidget )
			{
				pWidget->SetDisabled( Disabled );
			}
		}
	}
	else if( EventName == sSetWidgetLocation )
	{
		XTRACE_NAMED( SetWidgetLocation );

		STATIC_HASHED_STRING( Screen );
		const HashedString Screen = Event.GetHash( sScreen );

		STATIC_HASHED_STRING( Widget );
		const HashedString Widget = Event.GetHash( sWidget );

		STATIC_HASHED_STRING( X );
		const float X = Event.GetFloat( sX );

		STATIC_HASHED_STRING( Y );
		const float Y = Event.GetFloat( sY );

		UIScreen* const pScreen = GetScreen( Screen );
		if( pScreen )
		{
			UIWidget* const pWidget = pScreen->GetWidget( Widget );
			if( pWidget )
			{
				pWidget->SetLocation( X, Y );
			}
		}
	}
	else if( EventName == sSetWidgetAlpha )
	{
		XTRACE_NAMED( SetWidgetAlpha );

		STATIC_HASHED_STRING( Screen );
		const HashedString Screen = Event.GetHash( sScreen );

		STATIC_HASHED_STRING( Widget );
		const HashedString Widget = Event.GetHash( sWidget );

		STATIC_HASHED_STRING( Alpha );
		const float Alpha = Event.GetFloat( sAlpha );

		UIScreen* const pScreen = GetScreen( Screen );
		if( pScreen )
		{
			UIWidget* const pWidget = pScreen->GetWidget( Widget );
			if( pWidget )
			{
				pWidget->SetAlpha( Alpha );
			}
		}
	}
}

#if BUILD_DEBUG
void UIManager::DEBUGSetIsSafeToUpdateRender( bool IsSafeToUpdateRender, debug_int64_t Time /*= 0*/ )
{
	m_DEBUGIsSafeToUpdateRender = IsSafeToUpdateRender;
	if( IsSafeToUpdateRender )
	{
		m_DEBUGLastSafeToUpdateRenderTime = Time;
	}
}

bool UIManager::DEBUGIsSafeToUpdateRender( debug_int64_t LastRenderedTime ) const
{
	return m_DEBUGIsSafeToUpdateRender || ( LastRenderedTime < m_DEBUGLastSafeToUpdateRenderTime );
}
#endif
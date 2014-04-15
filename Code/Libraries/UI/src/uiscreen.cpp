#include "core.h"
#include "3d.h"
#include "uiscreen.h"
#include "uiwidget.h"
#include "uimanager.h"
#include "iuiinputmap.h"
#include "mouse.h"
#include "keyboard.h"
#include "windowwrapper.h"
#include "configmanager.h"
#include "stringmanager.h"
#include "iaudiosystem.h"
#include "linebatcher.h"
#include "uifactory.h"
#include "wbaction.h"
#include "wbactionfactory.h"
#include "wbactionstack.h"
#include "wbevent.h"
#include "mathcore.h"

// For old non-factory based code.
#include "Widgets/uiwidget-image.h"
#include "Widgets/uiwidget-text.h"
#include "Widgets/uiwidget-slider.h"

// TODO: Generalize this beyond UI. This will fix it for a lot of cases, though.
#define LEFT_MOUSE_KEY		( UIScreen::m_MouseButtonsSwapped ? Keyboard::EB_Mouse_Right	: Keyboard::EB_Mouse_Left )
#define LEFT_MOUSE_BUTTON	( UIScreen::m_MouseButtonsSwapped ? Mouse::EB_Right				: Mouse::EB_Left )

UIManager*	UIScreen::m_UIManager = NULL;
bool		UIScreen::m_MouseButtonsSwapped = false;

UIScreen::UIScreen()
:	m_RenderWidgets()
,	m_FocusWidgets()
,	m_FocusedWidget( NULL )
,	m_TakesFocus( false )
,	m_PausesGame( false )
,	m_SeizesMouse( false )
,	m_HasSeizedMouse( false )
,	m_PushedActions()
,	m_AllowClose( false )
,	m_ClosedActions()
,	m_GameTick( false )
,	m_FocusXShift( 0 )
,	m_FocusYShift( 0 )
,	m_LastMousePosX( 0 )
,	m_LastMousePosY( 0 )
,	m_ClickedWidget( NULL )
,	m_PushedSound( "" )
,	m_PoppedSound( "" )
,	m_OnStack( false )
,	m_Hidden( false )
{
}

UIScreen::UIScreen( const SimpleString& DefinitionName )
:	m_RenderWidgets()
,	m_FocusWidgets()
,	m_FocusedWidget( NULL )
,	m_TakesFocus( false )
,	m_PausesGame( false )
,	m_SeizesMouse( false )
,	m_HasSeizedMouse( false )
,	m_PushedActions()
,	m_AllowClose( false )
,	m_ClosedActions()
,	m_GameTick( false )
,	m_FocusXShift( 0 )
,	m_FocusYShift( 0 )
,	m_LastMousePosX( 0 )
,	m_LastMousePosY( 0 )
,	m_ClickedWidget( NULL )
,	m_PushedSound( "" )
,	m_PoppedSound( "" )
,	m_OnStack( false )
,	m_Hidden( false )
{
	InitializeFromDefinition( DefinitionName );
}

UIScreen::~UIScreen()
{
	FlushWidgets();

	FOR_EACH_ARRAY( ActionIter, m_PushedActions, WBAction* )
	{
		WBAction* pAction = ActionIter.GetValue();
		SafeDelete( pAction );
	}

	FOR_EACH_ARRAY( ActionIter, m_ClosedActions, WBAction* )
	{
		WBAction* pAction = ActionIter.GetValue();
		SafeDelete( pAction );
	}
}

void UIScreen::FlushWidgets()
{
	for( uint i = 0; i < m_RenderWidgets.Size(); ++i )
	{
		SafeDelete( m_RenderWidgets[i] );
	}
	m_RenderWidgets.Clear();
	m_FocusWidgets.Clear();
}

void UIScreen::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	STATICHASH( UI );
	STATICHASH( UIScreenVersion );
	const uint UIScreenVersion = ConfigManager::GetInt( sUIScreenVersion, 0, sUI );

	STATICHASH( TakesFocus );
	STATICHASH( PausesGame );
	STATICHASH( SeizesMouse );
	STATICHASH( GameTick );
	STATICHASH( PushedSound );
	STATICHASH( PoppedSound );
	STATICHASH( NumWidgets );
	MAKEHASH( DefinitionName );

	m_Name = DefinitionName;

	m_TakesFocus = ConfigManager::GetBool( sTakesFocus, false, sDefinitionName );
	m_PausesGame = ConfigManager::GetBool( sPausesGame, false, sDefinitionName );
	m_SeizesMouse = ConfigManager::GetBool( sSeizesMouse, false, sDefinitionName );
	m_GameTick = ConfigManager::GetBool( sGameTick, false, sDefinitionName );

	STATICHASH( FocusXShift );
	m_FocusXShift = ConfigManager::GetInt( sFocusXShift, 1, sDefinitionName );

	STATICHASH( FocusYShift );
	m_FocusYShift = ConfigManager::GetInt( sFocusYShift, 1, sDefinitionName );

	STATICHASH( NumPushedActions );
	const uint NumPushedActions = ConfigManager::GetInt( sNumPushedActions, 0, sDefinitionName );
	for( uint PushedActionIndex = 0; PushedActionIndex < NumPushedActions; ++PushedActionIndex )
	{
		const SimpleString Action = ConfigManager::GetSequenceString( "PushedAction%d", PushedActionIndex, "", DefinitionName );
		WBAction* const pAction = WBActionFactory::Create( Action );
		if( pAction )
		{
			m_PushedActions.PushBack( pAction );
		}
	}

	STATICHASH( AllowClose );
	m_AllowClose = ConfigManager::GetBool( sAllowClose, true, sDefinitionName );

	STATICHASH( NumClosedActions );
	const uint NumClosedActions = ConfigManager::GetInt( sNumClosedActions, 0, sDefinitionName );
	for( uint ClosedActionIndex = 0; ClosedActionIndex < NumClosedActions; ++ClosedActionIndex )
	{
		const SimpleString Action = ConfigManager::GetSequenceString( "ClosedAction%d", ClosedActionIndex, "", DefinitionName );
		WBAction* const pAction = WBActionFactory::Create( Action );
		if( pAction )
		{
			m_ClosedActions.PushBack( pAction );
		}
	}

	m_PushedSound = ConfigManager::GetString( sPushedSound, "", sDefinitionName );
	m_PoppedSound = ConfigManager::GetString( sPoppedSound, "", sDefinitionName );

	int NumWidgets = ConfigManager::GetInt( sNumWidgets, 0, sDefinitionName );
	for( int WidgetIndex = 0; WidgetIndex < NumWidgets; ++WidgetIndex )
	{
		if( UIScreenVersion < 2 )
		{
			// OLD SYSTEM
			const SimpleString WidgetClass = ConfigManager::GetSequenceString( "WidgetClass%d", WidgetIndex, "", sDefinitionName );
			const SimpleString WidgetName = ConfigManager::GetSequenceString( "WidgetName%d", WidgetIndex, "", sDefinitionName );

			if( WidgetClass == "Image"
#if BUILD_DEV
				|| WidgetClass == "DevImage"
#endif
				)
			{
				AddWidget( new UIWidgetImage( WidgetName ) );
			}
			else if( WidgetClass == "Text"
#if BUILD_DEV
				|| WidgetClass == "DevText"
#endif
				)
			{
				AddWidget( new UIWidgetText( WidgetName ) );
			}
			else if( WidgetClass == "Slider"
#if BUILD_DEV
				|| WidgetClass == "DevSlider"
#endif
				)
			{
				AddWidget( new UIWidgetSlider( WidgetName ) );
			}
			else if( WidgetClass == "None" )
			{
				// Do nothing--this is so widgets can be "commented out" in config without throwing asserts in dev builds
			}
			else
			{
				DEBUGLEVELPRINTF( PRINTLEVEL_Info, "Bad widget class name \"%s\" in definition \"%s\"\n", WidgetName.CStr(), DefinitionName.CStr() );
				WARNDESC( "Bad widget class name in configuration file" );
			}
		}
		else
		{
			// NEW SYSTEM
			// Dev widgets are supported with a DevOnly bool in the config, see UIFactory::CreateWidget.
			const SimpleString WidgetName = ConfigManager::GetSequenceString( "Widget%d", WidgetIndex, "", sDefinitionName );
			UIWidget* const pUIWidget = UIFactory::CreateWidget( WidgetName, this );
			if( pUIWidget )
			{
				AddWidget( pUIWidget );
			}
		}
	}

	UpdateRender();

	// Initialize focus to the first element (so that we're never unfocused,
	// because that doesn't make sense for controllers).
	if( m_FocusWidgets.Size() > 0 )
	{
		m_FocusedWidget = m_FocusWidgets[0];
		// NOTE: Intentionally not calling SetFocus here, so effects don't happen; we
		// just need an initial focus for controllers, it's not really *getting* focus
	}
}

void UIScreen::Reinitialize()
{
	for( uint i = 0; i < m_RenderWidgets.Size(); ++i )
	{
		UIWidget* const pWidget = m_RenderWidgets[i];
		pWidget->Reinitialize();
	}

	UpdateRender();
}

bool UIScreen::TakesFocus()
{
	return m_TakesFocus;
}

bool UIScreen::PausesGame()
{
	return m_PausesGame;
}

void UIScreen::TickMouse()
{
	int MousePosX = 0;
	int MousePosY = 0;
	m_UIManager->GetMouse()->GetPosition( MousePosX, MousePosY, m_UIManager->GetWindow() );

	const int MouseOffsetX = MousePosX - m_LastMousePosX;
	const int MouseOffsetY = MousePosY - m_LastMousePosY;
	m_LastMousePosX = MousePosX;
	m_LastMousePosY = MousePosY;
	const float MouseX = (float)MousePosX;
	const float MouseY = (float)MousePosY;

	// Only activate the focused widget on a mouse click if
	// the widget is still under the cursor when it is clicked.
	// Use keyboard to query mouse buttons because mouse is
	// deactivated (from DirectInput) for UI control.
	if( m_FocusedWidget )	// Don't do mouse UI stuff if we're unfocused or binding keys
	{
		if( m_UIManager->GetKeyboard()->OnRise( LEFT_MOUSE_KEY ) ||
			m_UIManager->GetMouse()->OnRise( LEFT_MOUSE_BUTTON ) )
		{
			SRect WidgetBounds;
			m_FocusedWidget->GetBounds( WidgetBounds );
			if(
				MouseX >= WidgetBounds.m_Left &&
				MouseX <= WidgetBounds.m_Right &&
				MouseY >= WidgetBounds.m_Top &&
				MouseY <= WidgetBounds.m_Bottom )
			{
				m_ClickedWidget = m_FocusedWidget;
			}
			else
			{
				m_ClickedWidget = NULL;
			}
		}
		else if(
			m_UIManager->GetKeyboard()->OnFall( LEFT_MOUSE_KEY ) ||
			m_UIManager->GetMouse()->OnFall( LEFT_MOUSE_BUTTON ) )
		{
			if( m_ClickedWidget == m_FocusedWidget )
			{
				SRect WidgetBounds;
				m_FocusedWidget->GetBounds( WidgetBounds );
				if(
					MouseX >= WidgetBounds.m_Left &&
					MouseX <= WidgetBounds.m_Right &&
					MouseY >= WidgetBounds.m_Top &&
					MouseY <= WidgetBounds.m_Bottom )
				{
					m_ClickedWidget->OnTrigger();
					m_ClickedWidget = NULL;
				}
				else
				{
					m_ClickedWidget->Released();
					m_ClickedWidget = NULL;
				}
			}
		}
		else if(
			m_UIManager->GetKeyboard()->IsHigh( LEFT_MOUSE_KEY ) ||
			m_UIManager->GetMouse()->IsHigh( LEFT_MOUSE_BUTTON ) )
		{
			if( m_ClickedWidget )
			{
				m_ClickedWidget->Drag( MouseX, MouseY );
			}
		}
	}

	if( !m_UIManager->GetKeyboard()->IsHigh( LEFT_MOUSE_KEY ) &&
		!m_UIManager->GetMouse()->IsHigh( LEFT_MOUSE_BUTTON ) )
	{
		// Only refocus under the cursor when the mouse moves and the button is not held down
		if( MouseOffsetX || MouseOffsetY ||
			m_UIManager->GetKeyboard()->OnFall( LEFT_MOUSE_KEY ) ||
			m_UIManager->GetMouse()->OnFall( LEFT_MOUSE_BUTTON ) )
		{
			for( uint i = 0; i < m_FocusWidgets.Size(); ++i )
			{
				UIWidget* const pWidget = m_FocusWidgets[i];

				if( pWidget->IsDisabled() )
				{
					continue;
				}

				SRect WidgetBounds;
				pWidget->GetBounds( WidgetBounds );
				if(	// TODO: Make this test a function of SRect, and maybe make SRect a full-fledged class
					MouseX >= WidgetBounds.m_Left &&
					MouseX <= WidgetBounds.m_Right &&
					MouseY >= WidgetBounds.m_Top &&
					MouseY <= WidgetBounds.m_Bottom )
				{
					SetFocus( pWidget );
					break;
				}
			}
		}
	}
}

void UIScreen::TickWidgets( const float DeltaTime )
{
	for( uint i = 0; i < m_RenderWidgets.Size(); ++i )
	{
		UIWidget* pWidget = m_RenderWidgets[i];
		pWidget->Tick( DeltaTime );
	}
}

void UIScreen::RefreshWidgets()
{
	for( uint i = 0; i < m_RenderWidgets.Size(); ++i )
	{
		UIWidget* pWidget = m_RenderWidgets[i];
		pWidget->Refresh();
	}
}

UIScreen::ETickReturn UIScreen::Tick( float DeltaTime, bool HasFocus )
{
	XTRACE_FUNCTION;

	Unused( DeltaTime );

	ETickReturn RetVal = ETR_None;

	if( !HasFocus )
	{
		m_ClickedWidget = NULL;
	}

	RefreshWidgets();
	TickWidgets( DeltaTime );

	if( HasFocus && m_UIManager->GetWindow()->HasFocus() )
	{
		// Mouse input has to be handled different than keyboard
		// and controller input, because we have a cursor.
		TickMouse();

		if( m_FocusedWidget && !m_FocusedWidget->IsDisabled() && m_FocusedWidget->HandleInput() )
		{
			// Widget consumed input, don't do anything here.
		}
		else
		{
			IUIInputMap* const pInputMap = m_UIManager->GetUIInputMap();
			if( pInputMap )
			{
				if( pInputMap->OnNext() )
				{
					FocusNext();
				}
				else if( pInputMap->OnPrevious() )
				{
					FocusPrevious();
				}
				else if( pInputMap->OnUp() )
				{
					FocusUp();
				}
				else if( pInputMap->OnDown() )
				{
					FocusDown();
				}
				else if( pInputMap->OnLeft() )
				{
					FocusLeft();
				}
				else if( pInputMap->OnRight() )
				{
					FocusRight();
				}
				else if( pInputMap->OnCancel() )
				{
					RetVal = ETR_Close;
				}
			}
		}
	}

	return RetVal;
}

void UIScreen::Pushed()
{
	m_OnStack = true;
	if( m_UIManager->GetAudioSystem() && m_PushedSound != "" )
	{
		m_UIManager->GetAudioSystem()->Play( m_PushedSound, Vector() );
	}

	if( m_SeizesMouse )
	{
		Mouse* pMouse = m_UIManager->GetMouse();
		if( pMouse->IsActive() )
		{
			pMouse->SetActive( false );
			m_HasSeizedMouse = true;
		}
		else
		{
			m_HasSeizedMouse = false;
		}
	}

	RefreshWidgets();

	FOR_EACH_ARRAY( ActionIter, m_PushedActions, WBAction* )
	{
		WBAction* const pAction = ActionIter.GetValue();
		ASSERT( pAction );

		WBActionStack::Push( WBEvent() );
		pAction->Execute();
		WBActionStack::Pop();
	}
}

void UIScreen::Popped()
{
	m_OnStack = false;
	m_ClickedWidget = NULL;
	if( m_UIManager->GetAudioSystem() && m_PoppedSound != "" )
	{
		m_UIManager->GetAudioSystem()->Play( m_PoppedSound, Vector() );
	}

	if( m_SeizesMouse && m_HasSeizedMouse )
	{
		m_UIManager->GetMouse()->SetActive( true );
		m_HasSeizedMouse = false;
	}
}

void UIScreen::Closed()
{
	FOR_EACH_ARRAY( ActionIter, m_ClosedActions, WBAction* )
	{
		WBAction* const pAction = ActionIter.GetValue();
		ASSERT( pAction );

		WBActionStack::Push( WBEvent() );
		pAction->Execute();
		WBActionStack::Pop();
	}
}

void UIScreen::AddWidget( UIWidget* pNewWidget )
{
	ASSERT( pNewWidget );

	m_RenderWidgets.PushBack( pNewWidget );
	if( pNewWidget->m_CanBeFocused )
	{
		m_FocusWidgets.PushBack( pNewWidget );
	}

	m_Widgets[ pNewWidget->m_Name ] = pNewWidget;
}

/*virtual*/ void UIScreen::Render( bool HasFocus )
{
	XTRACE_FUNCTION;

	Unused( HasFocus );

	const uint NumRenderWidgets = m_RenderWidgets.Size();
	for( uint RenderWidgetIndex = 0; RenderWidgetIndex < NumRenderWidgets; ++RenderWidgetIndex )
	{
		UIWidget* const pWidget = m_RenderWidgets[ RenderWidgetIndex ];
		if( pWidget->IsHidden() )
		{
			// Skip
		}
		else
		{
			pWidget->Render( ( m_FocusedWidget == pWidget ) );
		}
	}

#if BUILD_DEBUG
	STATICHASH( DebugRenderUIBounds );
	if( HasFocus && ConfigManager::GetBool( sDebugRenderUIBounds, false ) )
	{
		if( m_UIManager->GetLineBatcher() )
		{
			for( uint i = 0; i < m_FocusWidgets.Size(); ++i )
			{
				UIWidget* pWidget = m_FocusWidgets[i];
				SRect WidgetBounds;
				pWidget->GetBounds( WidgetBounds );
				m_UIManager->GetLineBatcher()->DrawBox(
					Vector( WidgetBounds.m_Left, 0.0f, WidgetBounds.m_Top ),
					Vector( WidgetBounds.m_Right, 0.0f, WidgetBounds.m_Bottom ),
					ARGB_TO_COLOR( 255, 255, 255, 0 ) );
			}
		}
		else
		{
			WARNDESC( "UI manager does not have a line batcher and Cannot render debug bounds." );
		}
	}
#endif
}

void UIScreen::UpdateRender()
{
	SortWidgets();
}

void UIScreen::SortWidgets()
{
	// Simple insertion sort. Stable, doesn't reorder widgets with same priority.

	// Sort render widgets by render priority
	for( uint i = 0; i < m_RenderWidgets.Size(); ++i )
	{
		UIWidget* pWidget = m_RenderWidgets[i];
		int j = i - 1;
		while( j >= 0 && m_RenderWidgets[j]->m_RenderPriority > pWidget->m_RenderPriority )
		{
			m_RenderWidgets[ j + 1 ] = m_RenderWidgets[j];
			--j;
		}
		m_RenderWidgets[ j + 1 ] = pWidget;
	}

	// Sort focus widgets by focus order
	for( uint i = 0; i < m_FocusWidgets.Size(); ++i )
	{
		UIWidget* pWidget = m_FocusWidgets[i];
		int j = i - 1;
		while( j >= 0 && m_FocusWidgets[j]->m_FocusOrder > pWidget->m_FocusOrder )
		{
			m_FocusWidgets[ j + 1 ] = m_FocusWidgets[j];
			--j;
		}
		m_FocusWidgets[ j + 1 ] = pWidget;
	}
}

UIWidget* UIScreen::GetWidget( const HashedString& Name )
{
	Map<HashedString, UIWidget*>::Iterator WidgetIter = m_Widgets.Search( Name );
	if( WidgetIter.IsValid() )
	{
		return *WidgetIter;
	}
	else
	{
		return NULL;
	}
}

void UIScreen::SetFocus( UIWidget* pWidget )
{
	ASSERT( pWidget );
	ASSERT( !pWidget->IsDisabled() );

	if( m_FocusedWidget != pWidget )
	{
		m_FocusedWidget = pWidget;
		m_FocusedWidget->GetFocus();
	}
}

void UIScreen::FocusNext()
{
	ShiftFocus( 1 );
}

void UIScreen::FocusPrevious()
{
	ShiftFocus( -1 );
}

void UIScreen::FocusUp()
{
	const int FocusShift = m_FocusedWidget ? m_FocusedWidget->OverrideFocusUp( -m_FocusYShift ) : -m_FocusYShift;
	ShiftFocus( FocusShift );
}

void UIScreen::FocusDown()
{
	const int FocusShift = m_FocusedWidget ? m_FocusedWidget->OverrideFocusDown( m_FocusYShift ) : m_FocusYShift;
	ShiftFocus( FocusShift );
}

void UIScreen::FocusLeft()
{
	const int FocusShift = m_FocusedWidget ? m_FocusedWidget->OverrideFocusLeft( -m_FocusXShift ) : -m_FocusXShift;
	ShiftFocus( FocusShift );
}

void UIScreen::FocusRight()
{
	const int FocusShift = m_FocusedWidget ? m_FocusedWidget->OverrideFocusRight( m_FocusXShift ) : m_FocusXShift;
	ShiftFocus( FocusShift );
}

void UIScreen::ShiftFocus( int Amount )
{
	const uint NumFocusWidgets = m_FocusWidgets.Size();
	if( NumFocusWidgets > 0 )
	{
		uint		FocusIndex		= GetFocusIndex();
		UIWidget*	pFocusWidget	= NULL;

		do 
		{
			// Add size to focus index to prevent subtracting from zero
			FocusIndex = ( FocusIndex + NumFocusWidgets + Amount ) % NumFocusWidgets;
			pFocusWidget	= m_FocusWidgets[ FocusIndex ];

			// HACK: Go to the next widget in order instead of continuing to skip by the shift amount.
			// This is specifically to deal with a special case in Eldritch (gear icons on the pause
			// screen being disabled in any combination), and shouldn't break anything anywhere else.
			// Generally, I don't know enough about the layout of a page to infer what should happen.
			Amount = Sign( Amount );
		}
		while ( pFocusWidget->IsDisabled() );

		SetFocus( pFocusWidget );
	}
}

uint UIScreen::GetFocusIndex()
{
	const uint NumFocusWidgets = m_FocusWidgets.Size();
	for( uint i = 0; i < NumFocusWidgets; ++i )
	{
		if( m_FocusWidgets[i] == m_FocusedWidget )
		{
			return i;
		}
	}
	return 0;
}

/*static*/ void UIScreen::SetUIManager( UIManager* pManager )
{
	m_UIManager = pManager;
}

/*static*/ void UIScreen::UpdateMouseButtonsSwapped()
{
#if BUILD_WINDOWS_NO_SDL
	m_MouseButtonsSwapped = ( GetSystemMetrics( SM_SWAPBUTTON ) != 0 );
#endif
	// NOTE: SDL does this automatically, no need to support it.
}
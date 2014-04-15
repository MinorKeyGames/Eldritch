#include "core.h"
#include "3d.h"
#include "uiwidget.h"
#include "uimanager.h"
#include "uiscreen.h"
#include "configmanager.h"
#include "iaudiosystem.h"
#if BUILD_WINDOWS	// TODO: Just remove this stuff, I don't use it anymore.
#include "consolemanager.h"
#endif
#include "vector4.h"
#include "clock.h"
#include "wbactionfactory.h"
#include "wbactionstack.h"
#include "wbevent.h"
#include "iuiinputmap.h"
#include "mathcore.h"
#include "irenderer.h"

UIManager* UIWidget::m_UIManager = NULL;

UIWidget::UIWidget()
:	m_Name()
,	m_Archetype()
,	m_RenderPriority( 0 )
,	m_CanBeFocused( false )
,	m_RenderInWorld( false )
,	m_IsDisabled( false )
,	m_FocusOrder( 0 )
,	m_EventName()
,	m_Command()
,	m_Callback()
,	m_FocusSound()
,	m_OwnerScreen( NULL )
,	m_FocusShiftUp( 0 )
,	m_FocusShiftDown( 0 )
,	m_FocusShiftLeft( 0 )
,	m_FocusShiftRight( 0 )
,	m_TriggerSound()
,	m_Actions()
,	m_OwnsActions( false )
,	m_Hidden( false )
,	m_Hidden_SavedState( false )
,	m_Color( 1.0f, 1.0f, 1.0f, 1.0f )
,	m_Highlight( 1.0f, 1.0f, 1.0f, 1.0f )
,	m_DisabledColor( 1.0f, 1.0f, 1.0f, 1.0f )
,	m_TopLeft()
,	m_Velocity()
,	m_AllowNegativeOrigin( false )
,	m_Origin( EWO_TopLeft )
,	m_OriginParent( NULL )
,	m_UsePulsingHighlight( false )
,	m_PulsingHighlightMul( 0.0f )
,	m_PulsingHighlightAdd( 0.0f )
,	m_PulsingHighlightTimeScalar( 0.0f )
#if BUILD_DEBUG
,	m_LastRenderedTime( 0 )
#endif
{
}

UIWidget::~UIWidget()
{
	ClearActions();
}

void UIWidget::Render( bool HasFocus )
{
	Unused( HasFocus );

#if BUILD_DEBUG
	m_LastRenderedTime = m_UIManager->GetClock()->GetPhysicalCurrentTime();
#endif
}

void UIWidget::UpdateRender()
{
}

void UIWidget::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	STATICHASH( Archetype );
	STATICHASH( RenderPriority );
	STATICHASH( Focus );
	STATICHASH( FocusOrder );
	STATICHASH( Event );
	STATICHASH( Command );
	STATICHASH( FocusSound );
	STATICHASH( TriggerSound );
	STATICHASH( RenderInWorld );
	STATICHASH( ColorR );
	STATICHASH( ColorG );
	STATICHASH( ColorB );
	STATICHASH( ColorA );
	STATICHASH( HighlightR );
	STATICHASH( HighlightG );
	STATICHASH( HighlightB );
	STATICHASH( HighlightA );
	STATICHASH( Origin );
	MAKEHASH( DefinitionName );

	m_Name = DefinitionName;
	m_Archetype = ConfigManager::GetString( sArchetype, "", sDefinitionName );

	MAKEHASH( m_Archetype );

	m_RenderPriority = ConfigManager::GetArchetypeInt( sRenderPriority, sm_Archetype, 0, sDefinitionName );
	m_CanBeFocused = ConfigManager::GetArchetypeBool( sFocus, sm_Archetype, false, sDefinitionName );
	m_RenderInWorld = ConfigManager::GetArchetypeBool( sRenderInWorld, sm_Archetype, false, sDefinitionName );
	m_FocusOrder = ConfigManager::GetArchetypeInt( sFocusOrder, sm_Archetype, 0, sDefinitionName );
	m_EventName = ConfigManager::GetArchetypeString( sEvent, sm_Archetype, "", sDefinitionName );
	m_Command = ConfigManager::GetArchetypeString( sCommand, sm_Archetype, "", sDefinitionName );
	m_FocusSound = ConfigManager::GetArchetypeString( sFocusSound, sm_Archetype, "", sDefinitionName );
	m_TriggerSound = ConfigManager::GetArchetypeString( sTriggerSound, sm_Archetype, "", sDefinitionName );

	STATICHASH( AllowNegativeOrigin );
	m_AllowNegativeOrigin = ConfigManager::GetArchetypeBool( sAllowNegativeOrigin, sm_Archetype, false, sDefinitionName );

	STATICHASH( FocusShiftUp );
	m_FocusShiftUp = ConfigManager::GetArchetypeInt( sFocusShiftUp, sm_Archetype, 0, sDefinitionName );

	STATICHASH( FocusShiftDown );
	m_FocusShiftDown = ConfigManager::GetArchetypeInt( sFocusShiftDown, sm_Archetype, 0, sDefinitionName );

	STATICHASH( FocusShiftLeft );
	m_FocusShiftLeft = ConfigManager::GetArchetypeInt( sFocusShiftLeft, sm_Archetype, 0, sDefinitionName );

	STATICHASH( FocusShiftRight );
	m_FocusShiftRight = ConfigManager::GetArchetypeInt( sFocusShiftRight, sm_Archetype, 0, sDefinitionName );

	m_Color.x = ConfigManager::GetArchetypeFloat( sColorR, sm_Archetype, 1.0f, sDefinitionName );
	m_Color.y = ConfigManager::GetArchetypeFloat( sColorG, sm_Archetype, 1.0f, sDefinitionName );
	m_Color.z = ConfigManager::GetArchetypeFloat( sColorB, sm_Archetype, 1.0f, sDefinitionName );
	m_Color.w = ConfigManager::GetArchetypeFloat( sColorA, sm_Archetype, 1.0f, sDefinitionName );
	m_Highlight.x = ConfigManager::GetArchetypeFloat( sHighlightR, sm_Archetype, 1.0f, sDefinitionName );
	m_Highlight.y = ConfigManager::GetArchetypeFloat( sHighlightG, sm_Archetype, 1.0f, sDefinitionName );
	m_Highlight.z = ConfigManager::GetArchetypeFloat( sHighlightB, sm_Archetype, 1.0f, sDefinitionName );
	m_Highlight.w = ConfigManager::GetArchetypeFloat( sHighlightA, sm_Archetype, 1.0f, sDefinitionName );

	STATICHASH( DisabledColorR );
	m_DisabledColor.r = ConfigManager::GetArchetypeFloat( sDisabledColorR, sm_Archetype, 1.0f, sDefinitionName );

	STATICHASH( DisabledColorG );
	m_DisabledColor.g = ConfigManager::GetArchetypeFloat( sDisabledColorG, sm_Archetype, 1.0f, sDefinitionName );

	STATICHASH( DisabledColorB );
	m_DisabledColor.b = ConfigManager::GetArchetypeFloat( sDisabledColorB, sm_Archetype, 1.0f, sDefinitionName );

	STATICHASH( DisabledColorA );
	m_DisabledColor.a = ConfigManager::GetArchetypeFloat( sDisabledColorA, sm_Archetype, 1.0f, sDefinitionName );

	// Pulsing highlights are currently global
	{
		STATICHASH( UI );

		STATICHASH( UsePulsingHighlight );
		m_UsePulsingHighlight = ConfigManager::GetBool( sUsePulsingHighlight, false, sUI );

		STATICHASH( PulsingHighlightMin );
		const float PulsingHighlightMin = ConfigManager::GetFloat( sPulsingHighlightMin, 0.0f, sUI );

		STATICHASH( PulsingHighlightMax );
		const float PulsingHighlightMax = ConfigManager::GetFloat( sPulsingHighlightMax, 0.0f, sUI );

		m_PulsingHighlightMul = ( PulsingHighlightMax - PulsingHighlightMin ) / 2.0f;
		m_PulsingHighlightAdd = PulsingHighlightMin - ( -1.0f * m_PulsingHighlightMul );

		STATICHASH( PulsingHighlightCycleSeconds );
		const float PulsingHighlightCycleSeconds = ConfigManager::GetFloat( sPulsingHighlightCycleSeconds, 0.0f, sUI );
		m_PulsingHighlightTimeScalar = ( 1.0f / PulsingHighlightCycleSeconds ) * 2.0f * PI;
	}

	STATICHASH( Hidden );
	m_Hidden = ConfigManager::GetArchetypeBool( sHidden, sm_Archetype, false, sDefinitionName );

	STATICHASH( Parent );
	const HashedString ParentWidget = ConfigManager::GetArchetypeHash( sParent, sm_Archetype, HashedString::NullString, sDefinitionName );
	if( ParentWidget != HashedString::NullString )
	{
		ASSERT( m_OwnerScreen );
		m_OriginParent = m_OwnerScreen->GetWidget( ParentWidget );

		ASSERTDESC( m_OriginParent, "Could not find specified parent for widget." );

		// This is important not only for rendering but because reinitialization is based on
		// render order, and we need parents to be in the correct position to update children.
		if( m_OriginParent && m_RenderPriority <= m_OriginParent->m_RenderPriority )
		{
			m_RenderPriority = m_OriginParent->m_RenderPriority + 1;

			STATIC_HASHED_STRING( UI );
			CATPRINTF( sUI, 2, "Promoting %s's render priority to %d to float above parent.\n", m_Name.CStr(), m_RenderPriority );
		}
	}

	STATICHASH( PixelVelocityX );
	m_Velocity.x = ConfigManager::GetArchetypeFloat( sPixelVelocityX, sm_Archetype, 0.0f, sDefinitionName );

	STATICHASH( PixelVelocityY );
	m_Velocity.y = ConfigManager::GetArchetypeFloat( sPixelVelocityY, sm_Archetype, 0.0f, sDefinitionName );

	GetOriginFromString( ConfigManager::GetArchetypeString( sOrigin, sm_Archetype, "", sDefinitionName ) );

	ClearActions();
	WBActionFactory::InitializeActionArray( DefinitionName, m_Actions );
	m_OwnsActions = true;
}

void UIWidget::ClearActions()
{
	if( m_OwnsActions )
	{
		FOR_EACH_ARRAY( ActionIter, m_Actions, WBAction* )
		{
			WBAction* pAction = ActionIter.GetValue();
			SafeDelete( pAction );
		}
	}

	m_Actions.Clear();
}

void UIWidget::GetBounds( SRect& OutBounds )
{
	Unused( OutBounds );
}

void UIWidget::OnTrigger()
{
	if( m_EventName != HashedString::NullString )
	{
		m_UIManager->PostEvent( m_EventName );
	}

#if BUILD_WINDOWS
	if( m_Command != "" )
	{
		ConsoleManager::GetInstance()->PushCommands( m_Command );
	}
#endif

	if( m_Callback.m_Callback )
	{
		m_Callback.m_Callback( this, m_Callback.m_Void );
	}

	if( m_TriggerSound != "" )
	{
		m_UIManager->GetAudioSystem()->Play( m_TriggerSound, Vector() );
	}

	FOR_EACH_ARRAY( ActionIter, m_Actions, WBAction* )
	{
		WBAction* const pAction = ActionIter.GetValue();
		ASSERT( pAction );

		WBActionStack::Push( WBEvent() );
		pAction->Execute();
		WBActionStack::Pop();
	}
}

/*virtual*/ void UIWidget::Released()
{
}

/*virtual*/ void UIWidget::Drag( float X, float Y )
{
	Unused( X );
	Unused( Y );
}

/*virtual*/ bool UIWidget::HandleInput()
{
	IUIInputMap* const pInputMap = m_UIManager->GetUIInputMap();

	if( pInputMap )
	{
		if( pInputMap->OnAccept() )
		{
			OnTrigger();
			return true;
		}
	}

	return false;
}

void UIWidget::GetFocus()
{
	if( m_FocusSound != "" )
	{
		m_UIManager->GetAudioSystem()->Play( m_FocusSound, Vector() );
	}
}

bool UIWidget::HasFocus() const
{
	ASSERT( GetOwnerScreen() );

	return GetOwnerScreen()->GetFocusedWidget() == this;
}

int UIWidget::OverrideFocusUp( int FocusShift )
{
	return Pick( m_FocusShiftUp, FocusShift );
}

int UIWidget::OverrideFocusDown( int FocusShift )
{
	return Pick( m_FocusShiftDown, FocusShift );
}

int UIWidget::OverrideFocusLeft( int FocusShift )
{
	return Pick( m_FocusShiftLeft, FocusShift );
}

int UIWidget::OverrideFocusRight( int FocusShift )
{
	return Pick( m_FocusShiftRight, FocusShift );
}

void UIWidget::Reinitialize()
{
	PushState();
	InitializeFromDefinition( m_Name );
	PullState();
}

void UIWidget::Refresh()
{
}

/*virtual*/ void UIWidget::Tick( const float DeltaTime )
{
	Unused( DeltaTime );
}

void UIWidget::SetHidden( const bool Hidden )
{
	m_Hidden = Hidden;
	Refresh();
}

void UIWidget::Show()
{
	m_Hidden = false;
	Refresh();
}

void UIWidget::SetDisabled( const bool Disabled )
{
	m_IsDisabled = Disabled;

	if( IsDisabled() )
	{
		if( HasFocus() )
		{
			GetOwnerScreen()->FocusNext();
		}
	}
}

// This is really ugly. I should refactor UI system to support dynamic
// positions uniformly for all widgets, and without rebuilding meshes.
void UIWidget::SetLocation( const float X, const float Y )
{
	MAKEHASH( m_Name );

	STATICHASH( ScreenX );
	ConfigManager::SetFloat( sScreenX, X, sm_Name );

	STATICHASH( ScreenY );
	ConfigManager::SetFloat( sScreenY, Y, sm_Name );

	Reinitialize();
}

/*virtual*/ void UIWidget::PushState()
{
	m_Hidden_SavedState = m_Hidden;
}

/*virtual*/ void UIWidget::PullState()
{
	m_Hidden = m_Hidden_SavedState;
}

void UIWidget::SetUIManager( UIManager* pManager )
{
	m_UIManager = pManager;
}

void UIWidget::GetOriginFromString( const SimpleString& Origin )
{
	if( Origin == "TopLeft" )
	{
		m_Origin = EWO_TopLeft;
	}
	else if( Origin == "TopCenter" )
	{
		m_Origin = EWO_TopCenter;
	}
	else if( Origin == "TopRight" )
	{
		m_Origin = EWO_TopRight;
	}
	else if( Origin == "CenterLeft" )
	{
		m_Origin = EWO_CenterLeft;
	}
	else if( Origin == "Center" )
	{
		m_Origin = EWO_Center;
	}
	else if( Origin == "CenterRight" )
	{
		m_Origin = EWO_CenterRight;
	}
	else if( Origin == "BottomLeft" )
	{
		m_Origin = EWO_BottomLeft;
	}
	else if( Origin == "BottomCenter" )
	{
		m_Origin = EWO_BottomCenter;
	}
	else if( Origin == "BottomRight" )
	{
		m_Origin = EWO_BottomRight;
	}
	else
	{
		// Unspecified origin defaults to top-left. That's fine. Some widgets don't need origins.
	}
}

void UIWidget::AdjustDimensionsToParent(
	float& X, float& Y, float& Width, float& Height,
	const float ParentX, const float ParentY, const float ParentWidth, const float ParentHeight )
{
	// Use negative numbers as offsets from the right/bottom of the screen unless negative origin is allowed
	X		= ( !m_AllowNegativeOrigin && X < 0.0f )	? ( X		+ ( ParentX + ParentWidth ) )	: ParentX + X;
	Y		= ( !m_AllowNegativeOrigin && Y < 0.0f )	? ( Y		+ ( ParentY + ParentHeight ) )	: ParentY + Y;
	Width	= ( Width < 0.0f )							? ( Width	+ ParentWidth )					: Width;
	Height	= ( Height < 0.0f )							? ( Height	+ ParentHeight )				: Height;
}

void UIWidget::GetPositionFromOrigin( const float X, const float Y, const float Width, const float Height )
{
	switch( m_Origin )
	{
	case EWO_TopLeft:
		m_TopLeft = Vector2( X, Y );
		break;
	case EWO_TopCenter:
		m_TopLeft = Vector2( X - Width * 0.5f, Y );
		break;
	case EWO_TopRight:
		m_TopLeft = Vector2( X - Width, Y );
		break;
	case EWO_CenterLeft:
		m_TopLeft = Vector2( X, Y - Height * 0.5f );
		break;
	case EWO_Center:
		m_TopLeft = Vector2( X - Width * 0.5f, Y - Height * 0.5f );
		break;
	case EWO_CenterRight:
		m_TopLeft = Vector2( X - Width, Y - Height * 0.5f );
		break;
	case EWO_BottomLeft:
		m_TopLeft = Vector2( X, Y - Height );
		break;
	case EWO_BottomCenter:
		m_TopLeft = Vector2( X - Width * 0.5f, Y - Height );
		break;
	case EWO_BottomRight:
		m_TopLeft = Vector2( X - Width, Y - Height );
		break;
	default:
		WARNDESC( "Bad origin for unknown reason" );
		break;
	}
}

Vector4 UIWidget::GetHighlightColor() const
{
	if( m_UsePulsingHighlight )
	{
		const float		CurrentTime		= m_UIManager->GetClock()->GetMachineCurrentTime();
		const float		TimePulse		= Cos( CurrentTime * m_PulsingHighlightTimeScalar );
		const float		LerpT			= ( TimePulse * m_PulsingHighlightMul ) + m_PulsingHighlightAdd;
		const Vector4	PulseHighlight	= m_Color.LERP( LerpT, m_Highlight );

		return PulseHighlight;
	}
	else
	{
		return m_Highlight;
	}
}

float UIWidget::GetPixelGridOffset() const
{
	ASSERT( m_UIManager );

	IRenderer* const pRenderer = m_UIManager->GetRenderer();
	ASSERT( pRenderer );

	return pRenderer->GetPixelGridOffset();
}
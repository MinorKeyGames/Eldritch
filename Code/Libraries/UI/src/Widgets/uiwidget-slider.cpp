#include "core.h"
#include "3d.h"
#include "uiwidget-slider.h"
#include "uimanager.h"
#include "font.h"
#include "mesh.h"
#include "shadermanager.h"
#include "configmanager.h"
#include "stringmanager.h"
#include "Widgets/uiwidget-text.h"
#include "Widgets/uiwidget-image.h"
#include "mathcore.h"
#include "uifactory.h"
#include "wbeventmanager.h"
#include "wbworld.h"
#include "iuiinputmap.h"

UIWidgetSlider::UIWidgetSlider()
:	m_SliderLabel( NULL )
,	m_SliderBack( NULL )
,	m_Slider( NULL )
,	m_ShiftAmount( 0.0f )
,	m_SlideValue_SavedState( 0.0f )
{
}

UIWidgetSlider::UIWidgetSlider( const SimpleString& DefinitionName )
:	m_SliderLabel( NULL )
,	m_SliderBack( NULL )
,	m_Slider( NULL )
,	m_ShiftAmount( 0.0f )
,	m_SlideValue_SavedState( 0.0f )
{
	InitializeFromDefinition( DefinitionName );
}

UIWidgetSlider::~UIWidgetSlider()
{
	SafeDelete( m_SliderLabel );
	SafeDelete( m_SliderBack );
	SafeDelete( m_Slider );
}

void UIWidgetSlider::UpdateRender()
{
	if( m_SliderLabel )
	{
		m_SliderLabel->UpdateRender();
	}

	m_SliderBack->UpdateRender();
	m_Slider->UpdateRender();
}

void UIWidgetSlider::Render( bool HasFocus )
{
	XTRACE_FUNCTION;

	if( m_SliderLabel )
	{
		m_SliderLabel->Render( HasFocus );
	}

	m_SliderBack->Render( HasFocus );
	m_Slider->Render( HasFocus );
}

void UIWidgetSlider::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	SafeDelete( m_SliderLabel );
	SafeDelete( m_SliderBack );
	SafeDelete( m_Slider );

	UIWidget::InitializeFromDefinition( DefinitionName );

	STATICHASH( SliderLabel );
	STATICHASH( SliderBack );
	STATICHASH( Slider );
	MAKEHASH( m_Archetype );
	MAKEHASH( DefinitionName );

	STATICHASH( ShiftAmount );
	m_ShiftAmount = ConfigManager::GetArchetypeFloat( sShiftAmount, sm_Archetype, 0.1f, sDefinitionName );

	STATICHASH( UI );
	STATICHASH( UIScreenVersion );
	const uint UIScreenVersion = ConfigManager::GetInt( sUIScreenVersion, 0, sUI );

	if( UIScreenVersion < 2 )
	{
		m_SliderLabel = new UIWidgetText( ConfigManager::GetArchetypeString( sSliderLabel, sm_Archetype, "", sDefinitionName ) );
		m_SliderBack = new UIWidgetImage( ConfigManager::GetArchetypeString( sSliderBack, sm_Archetype, "", sDefinitionName ) );
		m_Slider = new UIWidgetImage( ConfigManager::GetArchetypeString( sSlider, sm_Archetype, "", sDefinitionName ) );
	}
	else
	{
		m_SliderLabel	= static_cast<UIWidgetText*>(	UIFactory::CreateWidget( ConfigManager::GetArchetypeString( sSliderLabel, sm_Archetype, "", sDefinitionName ), GetOwnerScreen() ) );
		m_SliderBack	= static_cast<UIWidgetImage*>(	UIFactory::CreateWidget( ConfigManager::GetArchetypeString( sSliderBack, sm_Archetype, "", sDefinitionName ), GetOwnerScreen() ) );
		m_Slider		= static_cast<UIWidgetImage*>(	UIFactory::CreateWidget( ConfigManager::GetArchetypeString( sSlider, sm_Archetype, "", sDefinitionName ), GetOwnerScreen() ) );

		// We don't need a label, but we should have a slider and back.
		ASSERT( m_SliderBack );
		ASSERT( m_Slider );
	}
}

void UIWidgetSlider::GetBounds( SRect& OutBounds )
{
	SRect SliderBackBounds;
	SRect SliderBounds;

	m_SliderBack->GetBounds( SliderBackBounds );
	m_Slider->GetBounds( SliderBounds );

	OutBounds.m_Left	= Min( SliderBackBounds.m_Left,		SliderBounds.m_Left );
	OutBounds.m_Top		= Min( SliderBackBounds.m_Top,		SliderBounds.m_Top );
	OutBounds.m_Right	= Max( SliderBackBounds.m_Right,	SliderBounds.m_Right );
	OutBounds.m_Bottom	= Max( SliderBackBounds.m_Bottom,	SliderBounds.m_Bottom );
	
	if( m_SliderLabel )
	{
		SRect LabelBounds;

		m_SliderLabel->GetBounds( LabelBounds );

		OutBounds.m_Left	= Min( OutBounds.m_Left,	LabelBounds.m_Left );
		OutBounds.m_Top		= Min( OutBounds.m_Top,		LabelBounds.m_Top );
		OutBounds.m_Right	= Max( OutBounds.m_Right,	LabelBounds.m_Right );
		OutBounds.m_Bottom	= Max( OutBounds.m_Bottom,	LabelBounds.m_Bottom );
	}
}

void UIWidgetSlider::Refresh()
{
	UIWidget::Refresh();

	if( m_SliderLabel )
	{
		m_SliderLabel->Refresh();
	}

	m_SliderBack->Refresh();
	m_Slider->Refresh();
}

/*virtual*/ void UIWidgetSlider::Released()
{
	OnTrigger();
}

/*virtual*/ void UIWidgetSlider::OnTrigger()
{
	UIWidget::OnTrigger();

	SendSliderEvent();
}

void UIWidgetSlider::SendSliderEvent() const
{
	WB_MAKE_EVENT( OnSliderChanged, NULL );
	WB_SET_AUTO( OnSliderChanged, Hash, SliderName, m_Name );
	WB_SET_AUTO( OnSliderChanged, Float, SliderValue, GetSliderValue() );
	WB_DISPATCH_EVENT( WBWorld::GetInstance()->GetEventManager(), OnSliderChanged, NULL );
}

/*virtual*/ void UIWidgetSlider::Drag( float X, float Y )
{
	Unused( Y );

	m_Slider->m_TopLeft.x = Clamp( X - ( m_Slider->m_Dimensions.x / 2 ), m_SliderBack->m_TopLeft.x, m_SliderBack->m_TopLeft.x + m_SliderBack->m_Dimensions.x - m_Slider->m_Dimensions.x );
	UpdateRender();

	SendSliderEvent();
}

/*virtual*/ bool UIWidgetSlider::HandleInput()
{
	if( UIWidget::HandleInput() )
	{
		return true;
	}

	IUIInputMap* const pInputMap = m_UIManager->GetUIInputMap();

	if( pInputMap )
	{
		if( pInputMap->OnLeft() )
		{
			SetSliderValue( GetSliderValue() - m_ShiftAmount );
			return true;
		}
		else if( pInputMap->OnRight() )
		{
			SetSliderValue( GetSliderValue() + m_ShiftAmount );
			return true;
		}
	}

	return false;
}

void UIWidgetSlider::SetSliderValue( const float T )
{
	m_Slider->m_TopLeft.x = Lerp( m_SliderBack->m_TopLeft.x, m_SliderBack->m_TopLeft.x + m_SliderBack->m_Dimensions.x - m_Slider->m_Dimensions.x, Saturate( T ) );
	UpdateRender();

	SendSliderEvent();
}

float UIWidgetSlider::GetSliderValue() const
{
	return Saturate( InvLerp( m_Slider->m_TopLeft.x, m_SliderBack->m_TopLeft.x, m_SliderBack->m_TopLeft.x + m_SliderBack->m_Dimensions.x - m_Slider->m_Dimensions.x ) );
}

/*virtual*/ void UIWidgetSlider::PushState()
{
	UIWidget::PushState();

	m_SlideValue_SavedState = GetSliderValue();
}

/*virtual*/ void UIWidgetSlider::PullState()
{
	UIWidget::PullState();

	SetSliderValue( m_SlideValue_SavedState );
}
#include "core.h"
#include "wbactionuishowhidewidget.h"
#include "configmanager.h"
#include "wbeventmanager.h"

WBActionUIShowHideWidget::WBActionUIShowHideWidget()
:	m_ScreenName()
,	m_WidgetName()
,	m_Hidden( false )
,	m_SetDisabled( false )
{
}

WBActionUIShowHideWidget::~WBActionUIShowHideWidget()
{
}

/*virtual*/ void WBActionUIShowHideWidget::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	WBAction::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( Screen );
	m_ScreenName = ConfigManager::GetHash( sScreen, HashedString::NullString, sDefinitionName );

	STATICHASH( Widget );
	m_WidgetName = ConfigManager::GetHash( sWidget, HashedString::NullString, sDefinitionName );

	STATICHASH( Hidden );
	m_Hidden = ConfigManager::GetBool( sHidden, false, sDefinitionName );

	STATICHASH( SetDisabled );
	m_SetDisabled = ConfigManager::GetBool( sSetDisabled, false, sDefinitionName );
}

/*virtual*/ void WBActionUIShowHideWidget::Execute()
{
	{
		WB_MAKE_EVENT( SetWidgetHidden, NULL );
		WB_SET_AUTO( SetWidgetHidden, Hash, Screen, m_ScreenName );
		WB_SET_AUTO( SetWidgetHidden, Hash, Widget, m_WidgetName );
		WB_SET_AUTO( SetWidgetHidden, Bool, Hidden, m_Hidden );
		WB_DISPATCH_EVENT( WBWorld::GetInstance()->GetEventManager(), SetWidgetHidden, NULL );
	}

	if( m_SetDisabled )
	{
		WB_MAKE_EVENT( SetWidgetDisabled, NULL );
		WB_SET_AUTO( SetWidgetDisabled, Hash, Screen, m_ScreenName );
		WB_SET_AUTO( SetWidgetDisabled, Hash, Widget, m_WidgetName );
		WB_SET_AUTO( SetWidgetDisabled, Bool, Disabled, m_Hidden );
		WB_DISPATCH_EVENT( WBWorld::GetInstance()->GetEventManager(), SetWidgetDisabled, NULL );
	}
}
#include "core.h"
#include "wbactionuipushscreen.h"
#include "configmanager.h"
#include "wbeventmanager.h"

WBActionUIPushScreen::WBActionUIPushScreen()
:	m_ScreenName()
{
}

WBActionUIPushScreen::~WBActionUIPushScreen()
{
}

/*virtual*/ void WBActionUIPushScreen::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	WBAction::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( Screen );
	m_ScreenName = ConfigManager::GetHash( sScreen, HashedString::NullString, sDefinitionName );
}

/*virtual*/ void WBActionUIPushScreen::Execute()
{
	// We can't push directly to the UI stack because we don't know anything about Framework3D
	// or whoever else might own a UI manager and stack. Instead, use Workbench events.

	WB_MAKE_EVENT( PushUIScreen, NULL );
	WB_SET_AUTO( PushUIScreen, Hash, Screen, m_ScreenName );
	WB_DISPATCH_EVENT( WBWorld::GetInstance()->GetEventManager(), PushUIScreen, NULL );
}
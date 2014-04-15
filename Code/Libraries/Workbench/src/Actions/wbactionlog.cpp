#include "core.h"
#include "wbactionlog.h"
#include "configmanager.h"
#include "wbactionstack.h"
#include "reversehash.h"
#include "wbevent.h"

WBActionLog::WBActionLog()
:	m_Text( "" )
{
}

WBActionLog::~WBActionLog()
{
}

/*virtual*/ void WBActionLog::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	WBAction::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( Text );
	m_Text = ConfigManager::GetString( sText, "", sDefinitionName );
}

/*virtual*/ void WBActionLog::Execute()
{
	WBAction::Execute();

	const HashedString EventNameHash = WBActionStack::Top().GetEventName();
	const SimpleString EventName = ReverseHash::ReversedHash( EventNameHash );
	PRINTF( "%s\n", EventName.CStr() );
	PRINTF( "%s\n", m_Text.CStr() );
}
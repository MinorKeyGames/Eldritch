#include "core.h"
#include "wbactiontriggerstatmod.h"
#include "configmanager.h"
#include "Components/wbcompstatmod.h"
#include "wbentity.h"

WBActionTriggerStatMod::WBActionTriggerStatMod()
:	m_StatModEvent()
,	m_Trigger( false )
{
}

WBActionTriggerStatMod::~WBActionTriggerStatMod()
{
}

/*virtual*/ void WBActionTriggerStatMod::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	WBAction::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( StatModEvent );
	m_StatModEvent = ConfigManager::GetHash( sStatModEvent, HashedString::NullString, sDefinitionName );

	STATICHASH( Trigger );
	m_Trigger = ConfigManager::GetBool( sTrigger, true, sDefinitionName );
}

/*virtual*/ void WBActionTriggerStatMod::Execute()
{
	WBAction::Execute();

	WBCompStatMod* const pStatMod = GET_WBCOMP( GetEntity(), StatMod );
	ASSERT( pStatMod );

	pStatMod->SetEventActive( m_StatModEvent, m_Trigger );
}
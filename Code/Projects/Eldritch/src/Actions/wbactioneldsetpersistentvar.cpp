#include "core.h"
#include "wbactioneldsetpersistentvar.h"
#include "configmanager.h"
#include "wbeventmanager.h"

WBActionEldSetPersistentVar::WBActionEldSetPersistentVar()
:	m_Key()
,	m_ValuePE()
{
}

WBActionEldSetPersistentVar::~WBActionEldSetPersistentVar()
{
}

/*virtual*/ void WBActionEldSetPersistentVar::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	WBAction::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( Key );
	m_Key = ConfigManager::GetHash( sKey, HashedString::NullString, sDefinitionName );

	STATICHASH( ValuePE );
	const SimpleString ValuePEDef = ConfigManager::GetString( sValuePE, "", sDefinitionName );
	m_ValuePE.InitializeFromDefinition( ValuePEDef );
}

/*virtual*/ void WBActionEldSetPersistentVar::Execute()
{
	WBAction::Execute();

	WBParamEvaluator::SPEContext PEContext;
	PEContext.m_Entity = GetEntity();

	m_ValuePE.Evaluate( PEContext );

	WB_MAKE_EVENT( SetPersistentVar, NULL );
	WB_SET_AUTO( SetPersistentVar, Hash, Name, m_Key );
	WB_SET_AUTO_PE( SetPersistentVar, Value, m_ValuePE );
	WB_DISPATCH_EVENT( WBWorld::GetInstance()->GetEventManager(), SetPersistentVar, NULL );
}
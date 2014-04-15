#include "core.h"
#include "wbactionunqueueevent.h"
#include "configmanager.h"
#include "wbeventmanager.h"
#include "Components/wbcompvariablemap.h"

WBActionUnqueueEvent::WBActionUnqueueEvent()
:	m_EventOwnerPE()
,	m_VariableMapTag()
{
}

WBActionUnqueueEvent::~WBActionUnqueueEvent()
{
}

/*virtual*/ void WBActionUnqueueEvent::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	WBAction::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( EventOwner );
	const SimpleString EventOwnerDef = ConfigManager::GetString( sEventOwner, "", sDefinitionName );
	m_EventOwnerPE.InitializeFromDefinition( EventOwnerDef );

	STATICHASH( VariableMapTag );
	m_VariableMapTag = ConfigManager::GetHash( sVariableMapTag, HashedString::NullString, sDefinitionName );
}

/*virtual*/ void WBActionUnqueueEvent::Execute()
{
	WBAction::Execute();

	WBEntity* const pContextEntity = GetEntity();

	WBParamEvaluator::SPEContext PEContext;
	PEContext.m_Entity = pContextEntity;

	m_EventOwnerPE.Evaluate( PEContext );

	WBEntity* const pEvaluatedEventOwnerEntity	= m_EventOwnerPE.GetEntity();
	WBEntity* const pEventOwnerEntity			= pEvaluatedEventOwnerEntity ? pEvaluatedEventOwnerEntity : pContextEntity;

	// We need an event owner so we can look up the event UID in its var map.
	ASSERT( pEventOwnerEntity );

	WBCompVariableMap* const pVarMap = GET_WBCOMP( pEventOwnerEntity, VariableMap );
	ASSERT( pVarMap );

	WBEvent& VariableMap = pVarMap->GetVariables();
	const TEventUID EventUID = VariableMap.GetInt( m_VariableMapTag );

	WBWorld::GetInstance()->GetEventManager()->UnqueueEvent( EventUID );
}
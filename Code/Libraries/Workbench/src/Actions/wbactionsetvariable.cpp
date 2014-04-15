#include "core.h"
#include "wbactionsetvariable.h"
#include "configmanager.h"
#include "Components/wbcompvariablemap.h"
#include "wbparamevaluatorfactory.h"

WBActionSetVariable::WBActionSetVariable()
:	m_EntityPE()
,	m_VariableName()
,	m_ValuePE()
{
}

WBActionSetVariable::~WBActionSetVariable()
{
}

/*virtual*/ void WBActionSetVariable::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	WBAction::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( EntityPE );
	const SimpleString EntityPE = ConfigManager::GetString( sEntityPE, "", sDefinitionName );
	m_EntityPE.InitializeFromDefinition( EntityPE );

	STATICHASH( VariableName );
	m_VariableName = ConfigManager::GetHash( sVariableName, HashedString::NullString, sDefinitionName );

	STATICHASH( ValuePE );
	const SimpleString ValuePE = ConfigManager::GetString( sValuePE, "", sDefinitionName );
	m_ValuePE.InitializeFromDefinition( ValuePE );
}

/*virtual*/ void WBActionSetVariable::Execute()
{
	WBAction::Execute();

	WBParamEvaluator::SPEContext PEContext;
	PEContext.m_Entity = GetEntity();

	m_EntityPE.Evaluate( PEContext );
	m_ValuePE.Evaluate( PEContext );

	WBEntity* const pEntity = m_EntityPE.GetEntity();
	if( !pEntity )
	{
		return;
	}

	WBCompVariableMap* const pVariableMap = GET_WBCOMP( pEntity, VariableMap );
	if( !pVariableMap )
	{
		return;
	}

	WBEvent& Variables = pVariableMap->GetVariables();
	Variables.Set( m_VariableName, m_ValuePE );
}
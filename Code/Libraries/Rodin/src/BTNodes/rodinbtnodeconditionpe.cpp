#include "core.h"
#include "rodinbtnodeconditionpe.h"
#include "configmanager.h"

RodinBTNodeConditionPE::RodinBTNodeConditionPE()
:	m_ValuePE()
{
}

RodinBTNodeConditionPE::~RodinBTNodeConditionPE()
{
}

void RodinBTNodeConditionPE::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	MAKEHASH( DefinitionName );

	STATICHASH( ValuePE );
	m_ValuePE.InitializeFromDefinition( ConfigManager::GetString( sValuePE, "", sDefinitionName ) );
}

RodinBTNode::ETickStatus RodinBTNodeConditionPE::Tick( float DeltaTime )
{
	Unused( DeltaTime );

	WBParamEvaluator::SPEContext Context;
	Context.m_Entity = GetEntity();

	m_ValuePE.Evaluate( Context );

	return m_ValuePE.GetBool() ? ETS_Success : ETS_Fail;
}
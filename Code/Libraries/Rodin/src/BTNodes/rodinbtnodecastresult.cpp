#include "core.h"
#include "rodinbtnodecastresult.h"
#include "configmanager.h"

RodinBTNodeCastResult::RodinBTNodeCastResult()
:	m_ValuePE()
{
}

RodinBTNodeCastResult::~RodinBTNodeCastResult()
{
}

void RodinBTNodeCastResult::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	RodinBTNodeDecorator::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( ValuePE );
	m_ValuePE.InitializeFromDefinition( ConfigManager::GetString( sValuePE, "", sDefinitionName ) );
}

/*virtual*/ RodinBTNode::ETickStatus RodinBTNodeCastResult::Tick( float DeltaTime )
{
	const ETickStatus ActualResult = RodinBTNodeDecorator::Tick( DeltaTime );

	if( ActualResult == ETS_Fail || ActualResult == ETS_Success )
	{
		WBParamEvaluator::SPEContext Context;
		Context.m_Entity = GetEntity();
		m_ValuePE.Evaluate( Context );
		return m_ValuePE.GetBool() ? ETS_Success : ETS_Fail;
	}
	else
	{
		return ActualResult;
	}
}
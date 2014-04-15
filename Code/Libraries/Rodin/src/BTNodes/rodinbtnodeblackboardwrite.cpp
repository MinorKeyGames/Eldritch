#include "core.h"
#include "rodinbtnodeblackboardwrite.h"
#include "configmanager.h"
#include "Components/wbcomprodinblackboard.h"

RodinBTNodeBlackboardWrite::RodinBTNodeBlackboardWrite()
:	m_BlackboardKey()
,	m_ValuePE()
{
}

RodinBTNodeBlackboardWrite::~RodinBTNodeBlackboardWrite()
{
}

void RodinBTNodeBlackboardWrite::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	MAKEHASH( DefinitionName );

	STATICHASH( BlackboardKey );
	m_BlackboardKey = ConfigManager::GetHash( sBlackboardKey, HashedString::NullString, sDefinitionName );

	STATICHASH( ValuePE );
	m_ValuePE.InitializeFromDefinition( ConfigManager::GetString( sValuePE, "", sDefinitionName ) );
}

RodinBTNode::ETickStatus RodinBTNodeBlackboardWrite::Tick( float DeltaTime )
{
	Unused( DeltaTime );

	WBEntity* const					pEntity			= GetEntity();
	WBCompRodinBlackboard* const	pBlackboard	= GET_WBCOMP( pEntity, RodinBlackboard );
	ASSERT( pBlackboard );

	WBParamEvaluator::SPEContext Context;
	Context.m_Entity = pEntity;

	m_ValuePE.Evaluate( Context );
	pBlackboard->Set( m_BlackboardKey, m_ValuePE );

	return ETS_Success;
}
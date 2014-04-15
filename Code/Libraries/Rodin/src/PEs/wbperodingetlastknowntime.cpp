#include "core.h"
#include "wbperodingetlastknowntime.h"
#include "configmanager.h"
#include "wbparamevaluatorfactory.h"
#include "Components/wbcomprodinknowledge.h"

WBPERodinGetLastKnownTime::WBPERodinGetLastKnownTime()
:	m_EntityPE( NULL )
{
}

WBPERodinGetLastKnownTime::~WBPERodinGetLastKnownTime()
{
	SafeDelete( m_EntityPE );
}

/*virtual*/ void WBPERodinGetLastKnownTime::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	MAKEHASH( DefinitionName );

	STATICHASH( Entity );
	m_EntityPE = WBParamEvaluatorFactory::Create( ConfigManager::GetString( sEntity, "", sDefinitionName ) );
}

/*virtual*/ void WBPERodinGetLastKnownTime::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	ASSERT( Context.m_Entity );

	WBParamEvaluator::SEvaluatedParam Value;
	m_EntityPE->Evaluate( Context, Value );

	WBEntity* const pKnowledgeEntity = Value.GetEntity();

	if( !pKnowledgeEntity )
	{
		return;
	}

	WBCompRodinKnowledge* const pKnowledge = GET_WBCOMP( Context.m_Entity, RodinKnowledge );
	if( !pKnowledge )
	{
		return;
	}

	const WBCompRodinKnowledge::TKnowledge* const pKnowledgeEntry = pKnowledge->GetKnowledge( pKnowledgeEntity );
	if( !pKnowledgeEntry )
	{
		return;
	}

	STATIC_HASHED_STRING( LastKnownTime );
	EvaluatedParam.m_Type	= WBParamEvaluator::EPT_Float;
	EvaluatedParam.m_Float	= pKnowledgeEntry->GetFloat( sLastKnownTime );
}
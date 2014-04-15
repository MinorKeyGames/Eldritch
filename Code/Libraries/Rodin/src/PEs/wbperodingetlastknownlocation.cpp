#include "core.h"
#include "wbperodingetlastknownlocation.h"
#include "configmanager.h"
#include "wbparamevaluatorfactory.h"
#include "Components/wbcomprodinknowledge.h"

WBPERodinGetLastKnownLocation::WBPERodinGetLastKnownLocation()
:	m_EntityPE( NULL )
{
}

WBPERodinGetLastKnownLocation::~WBPERodinGetLastKnownLocation()
{
	SafeDelete( m_EntityPE );
}

/*virtual*/ void WBPERodinGetLastKnownLocation::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	MAKEHASH( DefinitionName );

	STATICHASH( Entity );
	m_EntityPE = WBParamEvaluatorFactory::Create( ConfigManager::GetString( sEntity, "", sDefinitionName ) );
}

/*virtual*/ void WBPERodinGetLastKnownLocation::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
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

	Vector LastKnownLocation;
	if( !pKnowledge->GetLastKnownLocationFor( pKnowledgeEntity, LastKnownLocation ) )
	{
		return;
	}

	EvaluatedParam.m_Type	= WBParamEvaluator::EPT_Vector;
	EvaluatedParam.m_Vector	= LastKnownLocation;
}
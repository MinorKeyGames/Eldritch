#include "core.h"
#include "wbperodinblackboardget.h"
#include "configmanager.h"
#include "Components/wbcomprodinblackboard.h"

WBPERodinBlackboardGet::WBPERodinBlackboardGet()
:	m_BlackboardKey()
{
}

WBPERodinBlackboardGet::~WBPERodinBlackboardGet()
{
}

void WBPERodinBlackboardGet::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	MAKEHASH( DefinitionName );

	STATICHASH( BlackboardKey );
	m_BlackboardKey = ConfigManager::GetHash( sBlackboardKey, HashedString::NullString, sDefinitionName );
}

void WBPERodinBlackboardGet::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	ASSERT( Context.m_Entity );

	WBCompRodinBlackboard* const pBlackboard = GET_WBCOMP( Context.m_Entity, RodinBlackboard );
	ASSERT( pBlackboard );

	// Cases not listed here are either unsupported by WBEvents or by WBParamEvaluators.
	switch( pBlackboard->GetType( m_BlackboardKey ) )
	{
		case WBEvent::EWBEPT_Bool:
			EvaluatedParam.m_Type	= WBParamEvaluator::EPT_Bool;
			EvaluatedParam.m_Bool	= pBlackboard->GetBool( m_BlackboardKey );
			break;

		case WBEvent::EWBEPT_Int:
			EvaluatedParam.m_Type	= WBParamEvaluator::EPT_Int;
			EvaluatedParam.m_Int	= pBlackboard->GetInt( m_BlackboardKey );
			break;

		case WBEvent::EWBEPT_Float:
			EvaluatedParam.m_Type	= WBParamEvaluator::EPT_Float;
			EvaluatedParam.m_Float	= pBlackboard->GetFloat( m_BlackboardKey );
			break;

		case WBEvent::EWBEPT_Vector:
			EvaluatedParam.m_Type	= WBParamEvaluator::EPT_Vector;
			EvaluatedParam.m_Vector	= pBlackboard->GetVector( m_BlackboardKey );
			break;

		case WBEvent::EWBEPT_Entity:
			EvaluatedParam.m_Type	= WBParamEvaluator::EPT_Entity;
			EvaluatedParam.m_Entity	= pBlackboard->GetEntity( m_BlackboardKey );
			break;
	}
}
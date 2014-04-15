#include "core.h"
#include "wbperandombool.h"
#include "configmanager.h"
#include "mathfunc.h"

WBPERandomBool::WBPERandomBool()
:	m_Probability( 0.0f )
{
}

WBPERandomBool::~WBPERandomBool()
{
}

void WBPERandomBool::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	STATICHASH( Probability );
	MAKEHASH( DefinitionName );

	m_Probability = ConfigManager::GetFloat( sProbability, 0.0f, sDefinitionName );
}

void WBPERandomBool::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	Unused( Context );

	EvaluatedParam.m_Type = WBParamEvaluator::EPT_Bool;
	EvaluatedParam.m_Bool = Math::Random( 0.0f, 1.0f ) < m_Probability;
}
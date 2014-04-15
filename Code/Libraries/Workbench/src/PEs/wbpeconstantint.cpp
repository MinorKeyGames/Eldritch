#include "core.h"
#include "wbpeconstantint.h"
#include "configmanager.h"

WBPEConstantInt::WBPEConstantInt()
:	m_Value( 0 )
{
}

WBPEConstantInt::~WBPEConstantInt()
{
}

void WBPEConstantInt::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	STATICHASH( Value );
	MAKEHASH( DefinitionName );

	m_Value = ConfigManager::GetInt( sValue, 0, sDefinitionName );
}

void WBPEConstantInt::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	Unused( Context );

	EvaluatedParam.m_Type = WBParamEvaluator::EPT_Int;
	EvaluatedParam.m_Int = m_Value;
}
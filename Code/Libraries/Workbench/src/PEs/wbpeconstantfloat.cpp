#include "core.h"
#include "wbpeconstantfloat.h"
#include "configmanager.h"

WBPEConstantFloat::WBPEConstantFloat()
:	m_Value( 0.0f )
{
}

WBPEConstantFloat::~WBPEConstantFloat()
{
}

void WBPEConstantFloat::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	STATICHASH( Value );
	MAKEHASH( DefinitionName );

	m_Value = ConfigManager::GetFloat( sValue, 0.0f, sDefinitionName );
}

void WBPEConstantFloat::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	Unused( Context );

	EvaluatedParam.m_Type = WBParamEvaluator::EPT_Float;
	EvaluatedParam.m_Float = m_Value;
}
#include "core.h"
#include "wbpeconstantbool.h"
#include "configmanager.h"

WBPEConstantBool::WBPEConstantBool()
:	m_Value( false )
{
}

WBPEConstantBool::~WBPEConstantBool()
{
}

void WBPEConstantBool::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	STATICHASH( Value );
	MAKEHASH( DefinitionName );

	m_Value = ConfigManager::GetBool( sValue, false, sDefinitionName );
}

void WBPEConstantBool::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	Unused( Context );

	EvaluatedParam.m_Type = WBParamEvaluator::EPT_Bool;
	EvaluatedParam.m_Bool = m_Value;
}
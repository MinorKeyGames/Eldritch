#include "core.h"
#include "wbpeconstantstring.h"
#include "configmanager.h"

WBPEConstantString::WBPEConstantString()
:	m_Value()
{
}

WBPEConstantString::~WBPEConstantString()
{
}

void WBPEConstantString::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	MAKEHASH( DefinitionName );

	STATICHASH( Value );
	m_Value = ConfigManager::GetString( sValue, "", sDefinitionName );
}

void WBPEConstantString::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	Unused( Context );

	EvaluatedParam.m_Type	= WBParamEvaluator::EPT_String;
	EvaluatedParam.m_String	= m_Value;
}
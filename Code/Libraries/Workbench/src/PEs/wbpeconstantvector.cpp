#include "core.h"
#include "wbpeconstantvector.h"
#include "configmanager.h"

WBPEConstantVector::WBPEConstantVector()
:	m_Value()
{
}

WBPEConstantVector::~WBPEConstantVector()
{
}

void WBPEConstantVector::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	MAKEHASH( DefinitionName );

	STATICHASH( ValueX );
	m_Value.x = ConfigManager::GetFloat( sValueX, 0.0f, sDefinitionName );

	STATICHASH( ValueY );
	m_Value.y = ConfigManager::GetFloat( sValueY, 0.0f, sDefinitionName );

	STATICHASH( ValueZ );
	m_Value.z = ConfigManager::GetFloat( sValueZ, 0.0f, sDefinitionName );
}

void WBPEConstantVector::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	Unused( Context );

	EvaluatedParam.m_Type	= WBParamEvaluator::EPT_Vector;
	EvaluatedParam.m_Vector	= m_Value;
}
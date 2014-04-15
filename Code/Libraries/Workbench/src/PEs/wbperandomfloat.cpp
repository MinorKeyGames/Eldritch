#include "core.h"
#include "wbperandomfloat.h"
#include "configmanager.h"
#include "mathcore.h"
#include "mathfunc.h"

WBPERandomFloat::WBPERandomFloat()
:	m_ValueA( 0.0f )
,	m_ValueB( 0.0f )
{
}

WBPERandomFloat::~WBPERandomFloat()
{
}

void WBPERandomFloat::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	STATICHASH( ValueA );
	STATICHASH( ValueB );
	MAKEHASH( DefinitionName );

	m_ValueA = ConfigManager::GetFloat( sValueA, 0.0f, sDefinitionName );
	m_ValueB = ConfigManager::GetFloat( sValueB, 0.0f, sDefinitionName );

	if( m_ValueA > m_ValueB )
	{
		Swap( m_ValueA, m_ValueB );
	}
}

void WBPERandomFloat::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	Unused( Context );

	EvaluatedParam.m_Type = WBParamEvaluator::EPT_Float;
	EvaluatedParam.m_Float = Math::Random( m_ValueA, m_ValueB );
}
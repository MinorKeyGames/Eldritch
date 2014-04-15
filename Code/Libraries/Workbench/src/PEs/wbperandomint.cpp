#include "core.h"
#include "wbperandomint.h"
#include "configmanager.h"
#include "mathcore.h"
#include "mathfunc.h"

WBPERandomInt::WBPERandomInt()
:	m_ValueA( 0 )
,	m_ValueB( 0 )
{
}

WBPERandomInt::~WBPERandomInt()
{
}

void WBPERandomInt::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	STATICHASH( ValueA );
	STATICHASH( ValueB );
	MAKEHASH( DefinitionName );

	m_ValueA = ConfigManager::GetInt( sValueA, 0, sDefinitionName );
	m_ValueB = ConfigManager::GetInt( sValueB, 0, sDefinitionName );

	if( m_ValueA > m_ValueB )
	{
		Swap( m_ValueA, m_ValueB );
	}
}

void WBPERandomInt::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	Unused( Context );

	EvaluatedParam.m_Type = WBParamEvaluator::EPT_Int;
	EvaluatedParam.m_Int = Math::Random( m_ValueA, m_ValueB );
}
#include "core.h"
#include "wbpebinaryop.h"
#include "configmanager.h"
#include "../wbparamevaluatorfactory.h"

WBPEBinaryOp::WBPEBinaryOp()
:	m_InputA( NULL )
,	m_InputB( NULL )
{
}

WBPEBinaryOp::~WBPEBinaryOp()
{
	SafeDelete( m_InputA );
	SafeDelete( m_InputB );
}

void WBPEBinaryOp::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	STATICHASH( InputA );
	STATICHASH( InputB );
	MAKEHASH( DefinitionName );

	m_InputA = WBParamEvaluatorFactory::Create( ConfigManager::GetString( sInputA, "", sDefinitionName ) );
	m_InputB = WBParamEvaluatorFactory::Create( ConfigManager::GetString( sInputB, "", sDefinitionName ) );
}
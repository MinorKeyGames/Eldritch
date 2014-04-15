#include "core.h"
#include "wbpeunaryop.h"
#include "configmanager.h"
#include "../wbparamevaluatorfactory.h"

WBPEUnaryOp::WBPEUnaryOp()
:	m_Input( NULL )
{
}

WBPEUnaryOp::~WBPEUnaryOp()
{
	SafeDelete( m_Input );
}

void WBPEUnaryOp::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	MAKEHASH( DefinitionName );

	STATICHASH( Input );
	m_Input = WBParamEvaluatorFactory::Create( ConfigManager::GetString( sInput, "", sDefinitionName ) );
}
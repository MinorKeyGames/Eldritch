#include "core.h"
#include "wbpepushcontext.h"
#include "configmanager.h"
#include "wbevent.h"
#include "wbparamevaluatorfactory.h"
#include "wbactionstack.h"

WBPEPushContext::WBPEPushContext()
:	m_EntityPE( NULL )
{
}

WBPEPushContext::~WBPEPushContext()
{
	SafeDelete( m_EntityPE );
}

/*virtual*/ void WBPEPushContext::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	WBPEUnaryOp::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( EntityPE );
	m_EntityPE = WBParamEvaluatorFactory::Create( ConfigManager::GetString( sEntityPE, "", sDefinitionName ) );
}

/*virtual*/ void WBPEPushContext::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	if( !Context.m_Entity )
	{
		return;
	}

	WBParamEvaluator::SEvaluatedParam Value;
	m_EntityPE->Evaluate( Context, Value );

	ASSERT( Value.m_Type == WBParamEvaluator::EPT_Entity );

	WBEntity* const pContextEntity = Value.m_Entity.Get();
	if( !pContextEntity )
	{
		return;
	}

	WBEvent ContextEvent;
	pContextEntity->AddContextToEvent( ContextEvent );

	WBParamEvaluator::SPEContext NewContext;
	NewContext.m_Entity = pContextEntity;

	WBActionStack::Push( ContextEvent );
	m_Input->Evaluate( NewContext, EvaluatedParam );
	WBActionStack::Pop();
}
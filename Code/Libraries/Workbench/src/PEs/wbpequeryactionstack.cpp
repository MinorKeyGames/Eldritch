#include "core.h"
#include "wbpequeryactionstack.h"
#include "configmanager.h"
#include "wbactionstack.h"
#include "wbevent.h"

WBPEQueryActionStack::WBPEQueryActionStack()
:	m_Key()
{
}

WBPEQueryActionStack::~WBPEQueryActionStack()
{
}

/*virtual*/ void WBPEQueryActionStack::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	MAKEHASH( DefinitionName );

	STATICHASH( Key );
	m_Key = ConfigManager::GetString( sKey, "", sDefinitionName );
}

/*virtual*/ void WBPEQueryActionStack::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	Unused( Context );

	const WBEvent& Event = WBActionStack::Top();
	const WBEvent::SParameter* pParam = Event.GetParameter( m_Key );

	EvaluatedParam.Set( pParam );
}
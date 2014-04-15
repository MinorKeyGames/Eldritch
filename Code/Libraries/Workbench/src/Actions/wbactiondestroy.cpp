#include "core.h"
#include "wbactiondestroy.h"
#include "wbentity.h"
#include "wbactionstack.h"
#include "wbevent.h"
#include "configmanager.h"

WBActionDestroy::WBActionDestroy()
:	m_DestroyPE()
{
}

WBActionDestroy::~WBActionDestroy()
{
}

/*virtual*/ void WBActionDestroy::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	WBAction::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( DestroyPE );
	const SimpleString DestroyDef = ConfigManager::GetString( sDestroyPE, "", sDefinitionName );
	m_DestroyPE.InitializeFromDefinition( DestroyDef );
}

/*virtual*/ void WBActionDestroy::Execute()
{
	WBAction::Execute();

	STATIC_HASHED_STRING( EventOwner );
	WBEntity* const pEntity = WBActionStack::Top().GetEntity( sEventOwner );

	WBParamEvaluator::SPEContext PEContext;
	PEContext.m_Entity = pEntity;

	m_DestroyPE.Evaluate( PEContext );

	WBEntity* const pDestroyEntity = m_DestroyPE.GetEntity();
	if( pDestroyEntity )
	{
		pDestroyEntity->Destroy();
	}
}
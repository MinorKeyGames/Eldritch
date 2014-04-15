#include "core.h"
#include "wbactioneldsetlight.h"
#include "configmanager.h"
#include "wbactionstack.h"
#include "wbeventmanager.h"

WBActionEldSetLight::WBActionEldSetLight()
:	m_AddLight( false )
{
}

WBActionEldSetLight::~WBActionEldSetLight()
{
}

/*virtual*/ void WBActionEldSetLight::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	WBAction::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( AddLight );
	m_AddLight = ConfigManager::GetBool( sAddLight, false, sDefinitionName );
}

/*virtual*/ void WBActionEldSetLight::Execute()
{
	WBAction::Execute();

	STATIC_HASHED_STRING( EventOwner );
	WBEntity* const pEntity = WBActionStack::Top().GetEntity( sEventOwner );

	if( pEntity )
	{
		if( m_AddLight )
		{
			// Queue because if we do this during spawning, the world won't accept a light yet.
			WB_MAKE_EVENT( AddLight, pEntity );
			WB_QUEUE_EVENT( WBWorld::GetInstance()->GetEventManager(), AddLight, pEntity );
		}
		else
		{
			WB_MAKE_EVENT( RemoveLight, pEntity );
			WB_DISPATCH_EVENT( WBWorld::GetInstance()->GetEventManager(), RemoveLight, pEntity );
		}
	}
}
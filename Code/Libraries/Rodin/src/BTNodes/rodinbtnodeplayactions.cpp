#include "core.h"
#include "rodinbtnodeplayactions.h"
#include "wbaction.h"
#include "wbactionfactory.h"
#include "wbactionstack.h"
#include "wbevent.h"

RodinBTNodePlayActions::RodinBTNodePlayActions()
:	m_Actions()
{
}

RodinBTNodePlayActions::~RodinBTNodePlayActions()
{
	const uint NumActions = m_Actions.Size();
	for( uint ActionsIndex = 0; ActionsIndex < NumActions; ++ActionsIndex )
	{
		SafeDelete( m_Actions[ ActionsIndex ] );
	}
}

void RodinBTNodePlayActions::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	WBActionFactory::InitializeActionArray( DefinitionName, m_Actions );
}

RodinBTNode::ETickStatus RodinBTNodePlayActions::Tick( float DeltaTime )
{
	Unused( DeltaTime );

	WBEntity* const pEntity = GetEntity();

	WBEvent BTNodePlayActionsEvent;
	STATIC_HASHED_STRING( BTNodePlayActionsEvent );
	BTNodePlayActionsEvent.SetEventName( sBTNodePlayActionsEvent );
	pEntity->AddContextToEvent( BTNodePlayActionsEvent );

	const uint NumActions = m_Actions.Size();
	for( uint ActionIndex = 0; ActionIndex < NumActions; ++ActionIndex )
	{
		WBAction* const pAction = m_Actions[ ActionIndex ];
		ASSERT( pAction );

		WBActionStack::Push( BTNodePlayActionsEvent );
		pAction->Execute();
		WBActionStack::Pop();
	}

	return ETS_Success;
}
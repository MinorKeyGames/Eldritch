#include "core.h"
#include "animeventeldexecuteaction.h"
#include "wbactionfactory.h"
#include "configmanager.h"
#include "wbactionstack.h"
#include "wbevent.h"
#include "eldritchmesh.h"
#include "Components/wbcompowner.h"

AnimEventEldExecuteAction::AnimEventEldExecuteAction()
:	m_Actions()
{
}

AnimEventEldExecuteAction::~AnimEventEldExecuteAction()
{
	const uint NumActions = m_Actions.Size();
	for( uint ActionsIndex = 0; ActionsIndex < NumActions; ++ActionsIndex )
	{
		SafeDelete( m_Actions[ ActionsIndex ] );
	}
}

void AnimEventEldExecuteAction::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	WBActionFactory::InitializeActionArray( DefinitionName, m_Actions );
}

void AnimEventEldExecuteAction::Call( Mesh* pMesh, Animation* pAnimation )
{
	Unused( pMesh );
	Unused( pAnimation );

	EldritchMesh* const	pEldritchMesh	= static_cast<EldritchMesh*>( pMesh );
	WBEntity* const		pEntity			= WBCompOwner::GetTopmostOwner( pEldritchMesh->GetEntity() );
	DEVASSERT( pEntity );

	WBEvent OnAnimEventEvent;
	STATIC_HASHED_STRING( OnAnimEvent );
	OnAnimEventEvent.SetEventName( sOnAnimEvent );
	pEntity->AddContextToEvent( OnAnimEventEvent );

	const uint NumActions = m_Actions.Size();
	for( uint ActionIndex = 0; ActionIndex < NumActions; ++ActionIndex )
	{
		WBAction* const pAction = m_Actions[ ActionIndex ];
		ASSERT( pAction );

		WBActionStack::Push( OnAnimEventEvent );
		pAction->Execute();
		WBActionStack::Pop();
	}
}
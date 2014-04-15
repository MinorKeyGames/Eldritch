#include "core.h"
#include "wbactioneldplayhandanim.h"
#include "configmanager.h"
#include "wbactionstack.h"
#include "wbeventmanager.h"
#include "eldritchmesh.h"
#include "Components/wbcompowner.h"

WBActionEldPlayHandAnim::WBActionEldPlayHandAnim()
:	m_AnimationName( "" )
{
}

WBActionEldPlayHandAnim::~WBActionEldPlayHandAnim()
{
}

/*virtual*/ void WBActionEldPlayHandAnim::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	WBAction::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( Animation );
	m_AnimationName = ConfigManager::GetHash( sAnimation, HashedString::NullString, sDefinitionName );
}

/*virtual*/ void WBActionEldPlayHandAnim::Execute()
{
	WBAction::Execute();

	STATIC_HASHED_STRING( EventOwner );
	WBEntity* const pEntity = WBActionStack::Top().GetEntity( sEventOwner );

	if( pEntity )
	{
		WB_MAKE_EVENT( PlayHandAnim, pEntity );
		WB_SET_AUTO( PlayHandAnim, Hash, AnimationName, m_AnimationName );
		WB_SET_AUTO( PlayHandAnim, Entity, AnimatingEntity, pEntity );
		WB_DISPATCH_EVENT( WBWorld::GetInstance()->GetEventManager(), PlayHandAnim, WBCompOwner::GetTopmostOwner( pEntity ) );
	}
}
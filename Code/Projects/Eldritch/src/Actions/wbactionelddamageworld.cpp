#include "core.h"
#include "wbactionelddamageworld.h"
#include "eldritchworld.h"
#include "Components/wbcompeldtransform.h"
#include "configmanager.h"
#include "wbactionstack.h"
#include "eldritchframework.h"

WBActionEldDamageWorld::WBActionEldDamageWorld()
:	m_Radius( 0.0f )
{
}

WBActionEldDamageWorld::~WBActionEldDamageWorld()
{
}

/*virtual*/ void WBActionEldDamageWorld::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	WBAction::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( Radius );
	m_Radius = ConfigManager::GetFloat( sRadius, 0.0f, sDefinitionName );
}

/*virtual*/ void WBActionEldDamageWorld::Execute()
{
	WBAction::Execute();

	STATIC_HASHED_STRING( EventOwner );
	WBEntity* const				pEntity		= WBActionStack::Top().GetEntity( sEventOwner );
	DEVASSERT( pEntity );

	WBCompEldTransform* const	pTransform	= pEntity->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	EldritchWorld* const		pWorld		= EldritchFramework::GetInstance()->GetWorld();
	ASSERT( pWorld );

	pWorld->RemoveVoxelsAt( pTransform->GetLocation(), m_Radius );
}
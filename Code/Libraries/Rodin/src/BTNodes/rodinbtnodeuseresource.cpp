#include "core.h"
#include "rodinbtnodeuseresource.h"
#include "configmanager.h"
#include "Components/wbcomprodinresourcemap.h"
#include "Components/wbcomprodinbehaviortree.h"
#include "wbentity.h"

RodinBTNodeUseResource::RodinBTNodeUseResource()
:	m_Resource()
,	m_ForceClaim( false )
{
}

RodinBTNodeUseResource::~RodinBTNodeUseResource()
{
}

void RodinBTNodeUseResource::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	RodinBTNodeDecorator::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( Resource );
	m_Resource = ConfigManager::GetHash( sResource, HashedString::NullString, sDefinitionName );

	STATICHASH( ForceClaim );
	m_ForceClaim = ConfigManager::GetBool( sForceClaim, false, sDefinitionName );
}

/*virtual*/ RodinBTNode::ETickStatus RodinBTNodeUseResource::Tick( float DeltaTime )
{
	WBCompRodinResourceMap* const pResourceMap = GET_WBCOMP( GetEntity(), RodinResourceMap );
	ASSERT( pResourceMap );

	if( m_ChildStatus != ETS_None )
	{
		// We're just continuing from being woken.
		return RodinBTNodeDecorator::Tick( DeltaTime );

	}

	if( pResourceMap->ClaimResource( this, m_Resource, m_ForceClaim ) )
	{
		// We got the resource, we're all good (or we're just continuing from being woken).
		return RodinBTNodeDecorator::Tick( DeltaTime );
	}
	else
	{
		// We couldn't get the resource.
		return ETS_Fail;
	}
}

/*virtual*/ void RodinBTNodeUseResource::OnFinish()
{
	RodinBTNodeDecorator::OnFinish();

	WBCompRodinResourceMap* const pResourceMap = GET_WBCOMP( GetEntity(), RodinResourceMap );
	ASSERT( pResourceMap );

	pResourceMap->ReleaseResource( this, m_Resource );
}

/*virtual*/ bool RodinBTNodeUseResource::OnResourceStolen( const HashedString& Resource )
{
	Unused( Resource );
	ASSERT( Resource == m_Resource );
	m_BehaviorTree->Stop( this );
	return false;
}
#include "core.h"
#include "wbscene.h"
#include "wbentity.h"
#include "idatastream.h"
#include "wbworld.h"

#if BUILD_DEBUG
#define BEGIN_ITERATING_ENTITIES do{ m_IteratingEntities = true; } while(0)
#define END_ITERATING_ENTITIES do{ m_IteratingEntities = false; } while(0)
#define CHECK_ITERATING_ENTITIES do{ DEBUGASSERT( !m_IteratingEntities ); } while(0)
#else
#define BEGIN_ITERATING_ENTITIES DoNothing
#define END_ITERATING_ENTITIES DoNothing
#define CHECK_ITERATING_ENTITIES DoNothing
#endif

WBScene::WBScene()
:	m_UID( 0 )
,	m_Entities()
,	m_LastEntitySceneHandle( 0 )
,	m_NumValidEntities( 0 )
,	m_DeferredRemoveHandles()
#if BUILD_DEBUG
,	m_IteratingEntities( false )
#endif
{
}

WBScene::~WBScene()
{
	BEGIN_ITERATING_ENTITIES;
	FOR_EACH_MAP( EntityIter, m_Entities, uint, SEntityRef )
	{
		const SEntityRef& EntityRef = EntityIter.GetValue();
		if( !EntityRef.m_Removed )
		{
			SafeDeleteNoNull( EntityRef.m_Entity );
		}
	}
	END_ITERATING_ENTITIES;
}

void WBScene::Initialize()
{
}

void WBScene::Tick()
{
	PROFILE_FUNCTION;

	// Delete any destroyed entities this scene owns
	BEGIN_ITERATING_ENTITIES;
	FOR_EACH_MAP( EntityIter, m_Entities, uint, SEntityRef )
	{
		const SEntityRef& EntityRef = EntityIter.GetValue();
		WBEntity* const pEntity = EntityRef.m_Entity;
		DEVASSERT( pEntity );
		if( !EntityRef.m_Removed && pEntity->IsDestroyed() )
		{
			SafeDeleteNoNull( pEntity );
		}
	}
	END_ITERATING_ENTITIES;

	// Deferred removal of entities
	for( uint HandleIndex = 0; HandleIndex < m_DeferredRemoveHandles.Size(); ++HandleIndex )
	{
		const uint Handle = m_DeferredRemoveHandles[ HandleIndex ];
		RemoveEntity( Handle );
	}
	m_DeferredRemoveHandles.Clear();
}

WBEntity* WBScene::GetEntity( const uint EntitySceneHandle ) const
{
	Map<uint, SEntityRef>::Iterator EntityIter = m_Entities.Search( EntitySceneHandle );
	if( EntityIter.IsValid() )
	{
		return EntityIter.GetValue().m_Entity;
	}
	else
	{
		return NULL;
	}
}

uint WBScene::AddEntity( WBEntity* const Entity )
{
	CHECK_ITERATING_ENTITIES;

	const uint EntitySceneHandle = ++m_LastEntitySceneHandle;

	SEntityRef& EntityRef = m_Entities[ EntitySceneHandle ];
	EntityRef.m_Entity = Entity;

	Entity->SetSceneHandle( EntitySceneHandle );
	Entity->SetScene( this );

	m_NumValidEntities++;

	return EntitySceneHandle;
}

void WBScene::AddEntity( WBEntity* const Entity, const uint EntitySceneHandle )
{
	CHECK_ITERATING_ENTITIES;

	DEBUGASSERT( EntitySceneHandle <= m_LastEntitySceneHandle );
	DEBUGASSERT( m_Entities.Search( EntitySceneHandle ).IsNull() );

	SEntityRef& EntityRef = m_Entities[ EntitySceneHandle ];
	EntityRef.m_Entity = Entity;

	Entity->SetSceneHandle( EntitySceneHandle );
	Entity->SetScene( this );

	m_NumValidEntities++;
}

void WBScene::RemoveEntity( const uint EntitySceneHandle )
{
	CHECK_ITERATING_ENTITIES;

	DEBUGASSERT( m_Entities.Search( EntitySceneHandle ).IsValid() );

	SEntityRef& EntityRef = m_Entities[ EntitySceneHandle ];

	WBEntity* const pEntity = EntityRef.m_Entity;
	pEntity->SetSceneHandle( EntitySceneHandle );
	pEntity->SetScene( this );

	m_Entities.Remove( EntitySceneHandle );
}

void WBScene::DeferredRemoveEntity( const uint EntitySceneHandle )
{
	DEBUGASSERT( m_Entities.Search( EntitySceneHandle ).IsValid() );

	SEntityRef& EntityRef = m_Entities[ EntitySceneHandle ];
	EntityRef.m_Removed = true;

	WBEntity* const pEntity = EntityRef.m_Entity;
	pEntity->SetSceneHandle( EntitySceneHandle );
	pEntity->SetScene( this );

	m_DeferredRemoveHandles.PushBack( EntitySceneHandle );
	m_NumValidEntities--;
}

WBEntity* WBScene::GetFirstEntityByComponent( const HashedString& ComponentName ) const
{
	WBEntity* RetVal = NULL;
	BEGIN_ITERATING_ENTITIES;
	FOR_EACH_MAP( EntityIter, m_Entities, uint, SEntityRef )
	{
		const SEntityRef& EntityRef = EntityIter.GetValue();
		WBEntity* const pEntity = EntityRef.m_Entity;
		DEVASSERT( pEntity );

		if( EntityRef.m_Removed || pEntity->IsDestroyed() )
		{
			continue;
		}

		if( pEntity->GetComponent( ComponentName ) )
		{
			RetVal = pEntity;
			break;
		}
	}
	END_ITERATING_ENTITIES;

	return RetVal;
}

void WBScene::GetEntitiesByComponent( Array<WBEntity*>& OutEntities, const HashedString& ComponentName ) const
{
	BEGIN_ITERATING_ENTITIES;
	FOR_EACH_MAP( EntityIter, m_Entities, uint, SEntityRef )
	{
		const SEntityRef& EntityRef = EntityIter.GetValue();
		WBEntity* const pEntity = EntityRef.m_Entity;
		DEVASSERT( pEntity );

		if( EntityRef.m_Removed || pEntity->IsDestroyed() )
		{
			continue;
		}

		if( pEntity->GetComponent( ComponentName ) )
		{
			OutEntities.PushBack( pEntity );
		}
	}
	END_ITERATING_ENTITIES;
}

void WBScene::Report() const
{
	PRINTF( WBSCENE_REPORT_PREFIX "Scene 0x%08X (*0x%08X) has %d entities. Next entity scene handle is %d.\n", m_UID, this, m_NumValidEntities, m_LastEntitySceneHandle );

	BEGIN_ITERATING_ENTITIES;
	FOR_EACH_MAP( EntityIter, m_Entities, uint, SEntityRef )
	{
		const SEntityRef& EntityRef = EntityIter.GetValue();
		WBEntity* const pEntity = EntityRef.m_Entity;
		DEVASSERT( pEntity );
		if( !EntityRef.m_Removed && !pEntity->IsDestroyed() )
		{
			pEntity->Report();
		}
	}
	END_ITERATING_ENTITIES;
}

#define VERSION_EMPTY					0
#define VERSION_LASTENTITYSCENEHANDLE	1
#define VERSION_UID						2
#define VERSION_CURRENT					2

void WBScene::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_UID )
	{
		m_UID = Stream.ReadUInt32();
	}

	if( Version >= VERSION_LASTENTITYSCENEHANDLE )
	{
		m_LastEntitySceneHandle = Stream.ReadUInt32();
	}

	uint NumEntities = Stream.ReadUInt32();
	for( uint EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex )
	{
		WBEntity* pNewEntity = new WBEntity;
		pNewEntity->Load( Stream );

		AddEntity( pNewEntity, pNewEntity->GetSceneHandle() );
		WBWorld::GetInstance()->AddEntity( pNewEntity, pNewEntity->GetUID() );

		pNewEntity->SendOnInitializedEvent();
	}
}

void WBScene::Save( const IDataStream& Stream ) const
{
	Stream.WriteUInt32( VERSION_CURRENT );

	Stream.WriteUInt32( m_UID );
	Stream.WriteUInt32( m_LastEntitySceneHandle );

	Stream.WriteUInt32( m_NumValidEntities );
	BEGIN_ITERATING_ENTITIES;
	FOR_EACH_MAP( EntityIter, m_Entities, uint, SEntityRef )
	{
		const SEntityRef& EntityRef = EntityIter.GetValue();
		const WBEntity* const pEntity = EntityRef.m_Entity;
		if( !EntityRef.m_Removed )
		{
			pEntity->Save( Stream );
		}
	}
	END_ITERATING_ENTITIES;
}

/*static*/ WBScene* WBScene::GetDefault()
{
	return WBWorld::GetInstance()->GetDefaultScene();
}
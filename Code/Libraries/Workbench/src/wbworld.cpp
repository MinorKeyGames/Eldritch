#include "core.h"
#include "wbworld.h"
#include "wbentity.h"
#include "wbscene.h"
#include "wbeventmanager.h"
#include "idatastream.h"
#include "clock.h"

#if BUILD_DEBUG

#define BEGIN_ITERATING_ENTITIES do{ m_IteratingEntities = true; } while(0)
#define END_ITERATING_ENTITIES do{ m_IteratingEntities = false; } while(0)
#define CHECK_ITERATING_ENTITIES do{ DEBUGASSERT( !m_IteratingEntities ); } while(0)

#define BEGIN_ITERATING_SCENES do{ m_IteratingScenes = true; } while(0)
#define END_ITERATING_SCENES do{ m_IteratingScenes = false; } while(0)
#define CHECK_ITERATING_SCENES do{ DEBUGASSERT( !m_IteratingScenes ); } while(0)

#else

#define BEGIN_ITERATING_ENTITIES DoNothing
#define END_ITERATING_ENTITIES DoNothing
#define CHECK_ITERATING_ENTITIES DoNothing

#define BEGIN_ITERATING_SCENES DoNothing
#define END_ITERATING_SCENES DoNothing
#define CHECK_ITERATING_SCENES DoNothing

#endif

WBWorld* WBWorld::m_Instance = NULL;

/*static*/ void WBWorld::CreateInstance()
{
	ASSERT( !m_Instance );
	if( !m_Instance )
	{
		m_Instance = new WBWorld;
	}
}

/*static*/ WBWorld* WBWorld::GetInstance()
{
	ASSERT( m_Instance );
	return m_Instance;
}

/*static*/ void WBWorld::DeleteInstance()
{
	SafeDelete( m_Instance );
}

WBWorld::WBWorld()
:	m_EventManager( NULL )
,	m_Entities()
,	m_NextEntityUID( 0x1d000000 )
,	m_DefaultScene( NULL )
,	m_Scenes()
,	m_NextSceneUID( 0x5c000000 )
#if BUILD_DEBUG
,	m_IteratingEntities( false )
,	m_IteratingScenes( false )
#endif
{
	m_EventManager = new WBEventManager;

	// Create the default scene, because entities can never not belong to a scene.
	m_DefaultScene = CreateScene();
}

WBWorld::~WBWorld()
{
	m_EventManager->Destroy();
	m_EventManager = NULL;

	ShutDown();
}

void WBWorld::ShutDown()
{
	if( m_EventManager )
	{
		m_EventManager->Flush();
	}

	BEGIN_ITERATING_SCENES;
	FOR_EACH_MAP( SceneIter, m_Scenes, uint, WBScene* )
	{
		WBScene* const pScene = SceneIter.GetValue();
		SafeDeleteNoNull( pScene );
	}
	END_ITERATING_SCENES;

	m_DefaultScene = NULL;
	m_Scenes.Clear();
	m_Entities.Clear();
}

void WBWorld::Tick( const float DeltaTime )
{
	XTRACE_FUNCTION;
	PROFILE_FUNCTION;

	m_EventManager->Tick();

	BEGIN_ITERATING_ENTITIES;
	FOR_EACH_MAP( EntityIter, m_Entities, uint, WBEntity* )
	{
		WBEntity* const pEntity = EntityIter.GetValue();

		if( pEntity->IsDestroyed() )
		{
			continue;
		}

		pEntity->Tick( DeltaTime );
	}
	END_ITERATING_ENTITIES;

	BEGIN_ITERATING_SCENES;
	FOR_EACH_MAP( SceneIter, m_Scenes, uint, WBScene* )
	{
		WBScene* const pScene = SceneIter.GetValue();
		pScene->Tick();
	}
	END_ITERATING_SCENES;
}

void WBWorld::Tick( const float DeltaTime, const bool DoClientTick, const bool DoServerTick )
{
	PROFILE_FUNCTION;

	m_EventManager->Tick();

	if( DoClientTick )
	{
		BEGIN_ITERATING_ENTITIES;
		FOR_EACH_MAP( EntityIter, m_Entities, uint, WBEntity* )
		{
			WBEntity* const pEntity = EntityIter.GetValue();
			pEntity->ClientTick( DeltaTime );
		}
		END_ITERATING_ENTITIES;
	}

	if( DoServerTick )
	{
		BEGIN_ITERATING_ENTITIES;
		FOR_EACH_MAP( EntityIter, m_Entities, uint, WBEntity* )
		{
			WBEntity* const pEntity = EntityIter.GetValue();
			pEntity->ServerTick( DeltaTime );
		}
		END_ITERATING_ENTITIES;
	}

	BEGIN_ITERATING_SCENES;
	FOR_EACH_MAP( SceneIter, m_Scenes, uint, WBScene* )
	{
		WBScene* const pScene = SceneIter.GetValue();
		pScene->Tick();
	}
	END_ITERATING_SCENES;
}

void WBWorld::Render() const
{
	PROFILE_FUNCTION;

	XTRACE_FUNCTION;

	BEGIN_ITERATING_ENTITIES;
	FOR_EACH_MAP( EntityIter, m_Entities, uint, WBEntity* )
	{
		WBEntity* const pEntity = EntityIter.GetValue();
		pEntity->Render();
	}
	END_ITERATING_ENTITIES;
}

WBEntity* WBWorld::GetEntity( const uint EntityUID ) const
{
	Map<uint, WBEntity*>::Iterator EntityIter = m_Entities.Search( EntityUID );
	if( EntityIter.IsValid() )
	{
		return EntityIter.GetValue();
	}
	else
	{
		return NULL;
	}
}

WBEntity* WBWorld::CreateEntity( const SimpleString& Archetype, WBScene* Scene /*= NULL*/ )
{
	if( Archetype == "" )
	{
		return NULL;
	}

	WBEntity* const pNewEntity = new WBEntity;

	// Add it immediately, before initialization, so it has a UID.
	AddEntity( pNewEntity );

	pNewEntity->InitializeFromDefinition( Archetype );
	pNewEntity->SendOnInitializedEvent();

	// OnSpawned only fires when a thing is spawned, not when loading. (For that, use OnLoaded.)
	WB_MAKE_EVENT( OnSpawned, pNewEntity );
	WB_LOG_EVENT( OnSpawned );
	WB_DISPATCH_EVENT( m_EventManager, OnSpawned, pNewEntity );

	// This is useful because transform isn't set up yet.
	// (Also, listen for OnMoved to get immediate transform update.)
	WB_MAKE_EVENT( OnSpawnedQueued, pNewEntity );
	WB_QUEUE_EVENT( m_EventManager, OnSpawnedQueued, pNewEntity );

	if( !Scene )
	{
		Scene = GetDefaultScene();
	}
	Scene->AddEntity( pNewEntity );

	return pNewEntity;
}

uint WBWorld::AddEntity( WBEntity* const Entity )
{
	DEBUGASSERT( m_Entities.SearchValue( Entity ).IsNull() );

	const uint EntityUID = m_NextEntityUID++;
	m_Entities[ EntityUID ] = Entity;
	Entity->SetUID( EntityUID );

	return EntityUID;
}

void WBWorld::AddEntity( WBEntity* const Entity, const uint EntityUID )
{
	DEBUGASSERT( EntityUID < m_NextEntityUID );
	DEBUGASSERT( m_Entities.Search( EntityUID ).IsNull() );
	DEBUGASSERT( m_Entities.SearchValue( Entity ).IsNull() );

	m_Entities[ EntityUID ] = Entity;
}

void WBWorld::RemoveEntity( const uint EntityUID )
{
	CHECK_ITERATING_ENTITIES;
	m_Entities.Remove( EntityUID );
}

WBScene* WBWorld::GetScene( const uint SceneUID ) const
{
	Map<uint, WBScene*>::Iterator SceneIter = m_Scenes.Search( SceneUID );
	if( SceneIter.IsValid() )
	{
		return SceneIter.GetValue();
	}
	else
	{
		return NULL;
	}
}

WBScene* WBWorld::CreateScene()
{
	WBScene* const pNewScene = new WBScene;

	AddScene( pNewScene );

	return pNewScene;
}

uint WBWorld::AddScene( WBScene* const Scene )
{
	CHECK_ITERATING_SCENES;

	DEBUGASSERT( m_Scenes.SearchValue( Scene ).IsNull() );

	const uint SceneUID = m_NextSceneUID++;
	m_Scenes[ SceneUID ] = Scene;
	Scene->SetUID( SceneUID );

	return SceneUID;
}

void WBWorld::AddScene( WBScene* const Scene, const uint SceneUID )
{
	CHECK_ITERATING_SCENES;

	DEBUGASSERT( SceneUID < m_NextSceneUID );
	DEBUGASSERT( m_Scenes.Search( SceneUID ).IsNull() );
	DEBUGASSERT( m_Scenes.SearchValue( Scene ).IsNull() );

	m_Scenes[ SceneUID ] = Scene;
}

void WBWorld::RemoveScene( const uint SceneUID )
{
	CHECK_ITERATING_SCENES;
	m_Scenes.Remove( SceneUID );
}

float WBWorld::GetTime() const
{
	return m_Clock->GetGameCurrentTime();
}

void WBWorld::Report() const
{
	PRINTF( "WORKBENCH WORLD REPORT:\n" );
	PRINTF( "World has %d entities. Next entity UID is 0x%08X.\n", m_Entities.Size(), m_NextEntityUID );
	PRINTF( "World has %d scenes. Next scene UID is 0x%08X.\n", m_Scenes.Size(), m_NextSceneUID );

	BEGIN_ITERATING_SCENES;
	FOR_EACH_MAP( SceneIter, m_Scenes, uint, WBScene* )
	{
		WBScene* const pScene = *SceneIter;
		ASSERT( pScene );
		pScene->Report();
	}
	END_ITERATING_SCENES;
}

#if BUILD_DEV
void WBWorld::DebugRender() const
{
	BEGIN_ITERATING_ENTITIES;
	FOR_EACH_MAP( EntityIter, m_Entities, uint, WBEntity* )
	{
		WBEntity* const pEntity = EntityIter.GetValue();
		if( pEntity->ShouldDebugRender() )
		{
			pEntity->DebugRender();
		}
	}
	END_ITERATING_ENTITIES;
}
#endif

#define VERSION_EMPTY	0
#define VERSION_CURRENT	0

void WBWorld::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	PROFILE_FUNCTION;

	ShutDown();

	const uint Version = Stream.ReadUInt32();
	Unused( Version );

	m_NextEntityUID = Stream.ReadUInt32();
	m_NextSceneUID = Stream.ReadUInt32();

	uint NumScenes = Stream.ReadUInt32();
	for( uint SceneIndex = 0; SceneIndex < NumScenes; ++SceneIndex )
	{
		WBScene* pNewScene = new WBScene;
		pNewScene->Load( Stream );
		AddScene( pNewScene, pNewScene->GetUID() );

		if( Stream.ReadBool() )
		{
			m_DefaultScene = pNewScene;
		}
	}

	// Load the event manager *after* everything else so the recipient UIDs are valid.
	m_EventManager->Load( Stream );

	WB_MAKE_EVENT( OnWorldLoaded, NULL );
	WB_LOG_EVENT( OnWorldLoaded );
	WB_DISPATCH_EVENT( m_EventManager, OnWorldLoaded, NULL );
}

void WBWorld::Save( const IDataStream& Stream ) const
{
	PROFILE_FUNCTION;

	Stream.WriteUInt32( VERSION_CURRENT );

	Stream.WriteUInt32( m_NextEntityUID );
	Stream.WriteUInt32( m_NextSceneUID );

	Stream.WriteUInt32( m_Scenes.Size() );
	BEGIN_ITERATING_SCENES;
	FOR_EACH_MAP( SceneIter, m_Scenes, uint, WBScene* )
	{
		const WBScene* const pScene = SceneIter.GetValue();
		pScene->Save( Stream );

		Stream.WriteBool( pScene == m_DefaultScene );
	}
	END_ITERATING_SCENES;

	m_EventManager->Save( Stream );
}
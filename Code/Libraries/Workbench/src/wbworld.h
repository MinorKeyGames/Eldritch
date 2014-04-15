#ifndef WBWORLD_H
#define WBWORLD_H

#include "map.h"

class WBEventManager;
class WBEntity;
class WBScene;
class Clock;

class WBWorld
{
public:
	static void		CreateInstance();
	static WBWorld*	GetInstance();
	static void		DeleteInstance();

	WBEventManager*	GetEventManager() { return m_EventManager; }

	void			ShutDown();

	void			Tick( const float DeltaTime );
	void			Tick( const float DeltaTime, const bool DoClientTick, const bool DoServerTick );

	// In case game ticks and renders at different rates. We always need to submit a renderable even when we don't tick.
	void			Render() const;

	WBEntity*		GetEntity( const uint EntityUID ) const;
	WBEntity*		CreateEntity( const SimpleString& Archetype, WBScene* Scene = NULL );
	uint			AddEntity( WBEntity* const Entity );
	void			AddEntity( WBEntity* const Entity, const uint EntityUID );
	void			RemoveEntity( const uint EntityUID );

	WBScene*		GetDefaultScene() const { return m_DefaultScene; }
	WBScene*		GetScene( const uint SceneUID ) const;
	WBScene*		CreateScene();
	uint			AddScene( WBScene* const Scene );
	void			AddScene( WBScene* const Scene, const uint SceneUID );
	void			RemoveScene( const uint SceneUID );

	void			SetClock( Clock* const pClock ) { m_Clock = pClock; }
	Clock*			GetClock() const { return m_Clock; }
	float			GetTime() const;

	void			Report() const;

#if BUILD_DEV
	void			DebugRender() const;
#endif

	void			Load( const IDataStream& Stream );
	void			Save( const IDataStream& Stream ) const;

private:
	WBWorld();
	~WBWorld();

	static WBWorld*			m_Instance;

	WBEventManager*			m_EventManager;

	Map<uint, WBEntity*>	m_Entities;
	uint					m_NextEntityUID;

	WBScene*				m_DefaultScene;
	Map<uint, WBScene*>		m_Scenes;
	uint					m_NextSceneUID;

	Clock*					m_Clock;

#if BUILD_DEBUG
	mutable bool			m_IteratingEntities;
	mutable bool			m_IteratingScenes;
#endif
};

#endif // WBWORLD_H
#ifndef WBSCENE_H
#define WBSCENE_H

// The Workbench scene just manages the entities in the scene. It doesn't represent the world,
// which will generally have more game-specific needs than Workbench should know about.

#include "map.h"
#include "array.h"

class WBEntity;
class Vector;

class WBScene
{
public:
	WBScene();
	~WBScene();

	void			Initialize();

	void			Tick();

	uint			GetUID() const { return m_UID; }
	void			SetUID( uint EntityUID ) { m_UID = EntityUID; }

	WBEntity*		GetEntity( const uint EntitySceneHandle ) const;
	uint			AddEntity( WBEntity* const Entity );
	void			AddEntity( WBEntity* const Entity, const uint EntitySceneHandle );
	void			RemoveEntity( const uint EntitySceneHandle );
	void			DeferredRemoveEntity( const uint EntitySceneHandle );

	WBEntity*		GetFirstEntityByComponent( const HashedString& ComponentName ) const;
	void			GetEntitiesByComponent( Array<WBEntity*>& OutEntities, const HashedString& ComponentName ) const;

	// NOTE: This function needs to be implemented per-project using that project's transform component.
	void			GetEntitiesByRadius( Array<WBEntity*>& OutEntities, const Vector& Location, const float Radius ) const;

#define WBSCENE_REPORT_PREFIX "  "
	void			Report() const;

	void			Load( const IDataStream& Stream );
	void			Save( const IDataStream& Stream ) const;

	// Helper shortcut to WBWorld::GetInstance()->GetDefaultScene()
	static WBScene*	GetDefault();

private:
	struct SEntityRef
	{
		SEntityRef()
		:	m_Entity( NULL )
		,	m_Removed( false )
		{
		}

		WBEntity*	m_Entity;
		bool		m_Removed;
	};

	uint					m_UID;

	Map<uint, SEntityRef>	m_Entities;
	uint					m_LastEntitySceneHandle;

	uint					m_NumValidEntities;
	Array<uint>				m_DeferredRemoveHandles;

#if BUILD_DEBUG
	mutable bool			m_IteratingEntities;
#endif
};

#endif // WBSCENE_H
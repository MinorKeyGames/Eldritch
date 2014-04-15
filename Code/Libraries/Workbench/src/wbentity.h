#ifndef WBENTITY_H
#define WBENTITY_H

#include "map.h"
#include "multimap.h"
#include "hashedstring.h"
#include "simplestring.h"
#include "iwbeventobserver.h"

class WBComponent;
class WBScene;

class WBEntity : public IWBEventObserver
{
public:
	WBEntity();
	~WBEntity();	// Not virtual, because WorkbenchEntities aren't designed to be inherited.

	void			InitializeFromDefinition( const SimpleString& DefinitionName );
	void			SendOnInitializedEvent();

	void			Tick( float DeltaTime );
	void			ClientTick( float DeltaTime );
	void			ServerTick( float DeltaTime );

	void			Render() const;

	void			Destroy();
	bool			IsDestroyed() const { return m_Destroyed; }

	uint			GetUID() const { DEVASSERT( m_UID > 0 ); return m_UID; }
	void			SetUID( const uint EntityUID ) { m_UID = EntityUID; }

	uint			GetSceneHandle() const { return m_SceneHandle; }
	void			SetSceneHandle( const uint EntitySceneHandle ) { m_SceneHandle = EntitySceneHandle; }

	WBScene*		GetScene() const { return m_Scene; }
	void			SetScene( WBScene* const Scene ) { m_Scene = Scene; }

	WBComponent*	GetComponent( const HashedString& ComponentName ) const;

	WBComponent*			GetTransformComponent() const { return m_TransformComponent; }
	template<class C> C*	GetTransformComponent() const { return static_cast<C*>( GetTransformComponent() ); }
	void					SetTransformComponent( WBComponent* const pTransformComponent );

#define WBENTITY_REPORT_PREFIX "    "
	void			Report() const;

#if BUILD_DEV
	void			DebugRender() const;
	bool			ShouldDebugRender() const { return m_ShouldDebugRender; }
#endif

	void			Load( const IDataStream& Stream );
	void			Save( const IDataStream& Stream ) const;

	SimpleString	GetName() const { return m_DefinitionName; }
	SimpleString	GetUniqueName() const { return SimpleString::PrintF( "%s_0x%08X", m_DefinitionName.CStr(), m_UID ); }

	// IWBEventObserver
	virtual void	HandleEvent( const WBEvent& Event );
	virtual uint	GetEntityUID() const { return m_UID; }

	void			AddContextToEvent( WBEvent& Event ) const;

private:
	void			AddComponent( const HashedString& ComponentType, const char* ComponentDefinition);

	void			ForwardEventToComponents( const WBEvent& Event ) const;

	Map<HashedString, WBComponent*>	m_Components;			// All components, mapped by their type name
	Multimap<int, WBComponent*>		m_TickComponents;		// Only the tickable components in their tick order
	Array<WBComponent*>				m_TickComponentsFlat;	// Flattened array version of m_TickComponents, for better cache performance
	Array<WBComponent*>				m_RenderComponents;		// Only the renderable components
	WBComponent*					m_TransformComponent;	// For optimization. Not guaranteed to exist. Actual type depends on game.

	SimpleString					m_DefinitionName;
	bool							m_Destroyed;

	uint							m_UID;
	uint							m_SceneHandle;
	WBScene*						m_Scene;

#if BUILD_DEBUG
	mutable bool					m_IteratingComponents;
#endif

#if BUILD_DEV
	bool							m_ShouldDebugRender;
#endif
};

#endif // WBENTITY_H
#ifndef WBCOMPONENT_H
#define WBCOMPONENT_H

// Base class for entity components.

#include "hashedstring.h"
#include "iwbeventobserver.h"
#include "map.h"

class SimpleString;
class WBEntity;
class IDataStream;
class WBEvent;
class WBEventManager;

#define DEFINE_WBCOMP_FACTORY( type ) static class WBComponent* Factory() { return new WBComp##type; }
typedef class WBComponent* ( *WBCompFactoryFunc )( void );

#define DEFINE_WBCOMP_SUPER( parent ) typedef parent Super;

#define DEFINE_WBCOMP_STATICNAME( type ) static const HashedString& GetStaticName() { static const HashedString sName( #type ); return sName; }
#define DEFINE_WBCOMP_NAME virtual HashedString GetName() const { return GetStaticName(); }
#define DEFINE_WBCOMP_STATICREADABLENAME( type ) static const char* GetStaticReadableName() { return #type; }
#define DEFINE_WBCOMP_READABLENAME virtual const char* GetReadableName() const { return GetStaticReadableName(); }

#define DEFINE_WBCOMP( type, parent )			\
	DEFINE_WBCOMP_SUPER( parent )				\
	DEFINE_WBCOMP_FACTORY( type )				\
	DEFINE_WBCOMP_STATICNAME( type )			\
	DEFINE_WBCOMP_NAME							\
	DEFINE_WBCOMP_STATICREADABLENAME( type )	\
	DEFINE_WBCOMP_READABLENAME

// This sort of enforces one component per type, but shouldn't prevent manually adding a component with a unique name if I want to do that
#define GET_WBCOMP( entity, type ) ( static_cast<WBComp##type*>( ( entity )->GetComponent( WBComp##type::GetStaticName() ) ) )
#define SAFE_GET_WBCOMP( entity, type ) ( ( entity ) ? ( static_cast<WBComp##type*>( ( entity )->GetComponent( WBComp##type::GetStaticName() ) ) ) : NULL )

#define CREATE_WBCOMP( type, def ) ( static_cast<WBComp##type##*>( WBComponent::Create( WBComp##type::GetStaticName(), def ) ) )

class WBComponent : public IWBEventObserver
{
public:
	WBComponent();
	virtual ~WBComponent();

	typedef Map<HashedString, WBCompFactoryFunc> FactoryMap;
	static void			RegisterWBCompFactory( const HashedString& TypeName, WBCompFactoryFunc Factory );
	static const FactoryMap& GetWBCompFactories();

	static void			InitializeBaseFactories();
	static void			ShutDownBaseFactories();

	static WBComponent*	Create( const HashedString& TypeName, const SimpleString& DefinitionName, WBEntity* const pEntity );

	// Expand as needed, there's no constraints on these values.
	enum ETickOrder
	{
		ETO_NoTick,
		ETO_TickFirst,
		ETO_TickSecond,
		ETO_TickDefault,
		ETO_TickLast,
	};

	// Constructor and destructor, effectively; but called when object is fully constructed.
	virtual void	Initialize();	// Called just before InitializeFromDefinition
	virtual void	ShutDown();		// Called just before destruction

	virtual void	Tick( float DeltaTime );
	virtual void	ClientTick( float DeltaTime );
	virtual void	ServerTick( float DeltaTime );
	virtual int		GetTickOrder() { return ETO_TickDefault; }

	// For fast component iteration
	virtual bool	BelongsInComponentArray() { return false; }

	virtual bool	IsRenderable() { return false; }
	virtual void	Render();

	WBEntity*		GetEntity() const { DEVASSERT( m_Entity ); return m_Entity; }

	// Implemented by DEFINE_WBCOMP macro
	virtual HashedString	GetName() const = 0;
	virtual const char*		GetReadableName() const = 0;

	virtual uint	GetSerializationSize();
	virtual void	Save( const IDataStream& Stream );
	virtual void	Load( const IDataStream& Stream );

#define WB_REPORT_SPACER "  "
#define WBCOMPONENT_REPORT_PREFIX "      "
#define WBPROPERTY_REPORT_PREFIX "        "
	virtual void	Report() const;

#if BUILD_DEV
	virtual void	DebugRender() const;
#endif

	// IWBEventObserver
	virtual void	HandleEvent( const WBEvent& Event );

	virtual void	AddContextToEvent( WBEvent& Event ) const;

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

	float			GetTime() const;
	WBEventManager*	GetEventManager() const;

	void			SetEntity( WBEntity* pEntity ) { ASSERT( !m_Entity ); DEVASSERT( pEntity ); m_Entity = pEntity; }

private:
	static FactoryMap m_WBCompFactoryMap;

	WBEntity*		m_Entity;
};

#endif // WBCOMPONENT_H
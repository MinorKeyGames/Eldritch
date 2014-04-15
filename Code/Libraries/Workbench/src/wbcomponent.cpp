#include "core.h"
#include "wbcomponent.h"
#include "simplestring.h"
#include "idatastream.h"
#include "wbevent.h"
#include "wbcomponentarrays.h"
#include "wbcomponents.h"

WBComponent::FactoryMap WBComponent::m_WBCompFactoryMap;

/*static*/ void WBComponent::RegisterWBCompFactory( const HashedString& TypeName, WBCompFactoryFunc Factory )
{
	ASSERT( m_WBCompFactoryMap.Search( TypeName ).IsNull() );
	ASSERT( Factory );
	m_WBCompFactoryMap[ TypeName ] = Factory;
}

/*static*/ const WBComponent::FactoryMap& WBComponent::GetWBCompFactories()
{
	return m_WBCompFactoryMap;
}

/*static*/ void WBComponent::InitializeBaseFactories()
{
	// NOTE: This only initializes core Workbench factories. For registering extensions in
	// external libraries, register other component headers wherever this function is called.
#define ADDWBCOMPONENT( type ) WBComponent::RegisterWBCompFactory( #type, WBComp##type::Factory );
#include "wbcomponents.h"
#undef ADDWBCOMPONENT
}

/*static*/ void WBComponent::ShutDownBaseFactories()
{
	m_WBCompFactoryMap.Clear();
}

/*static*/ WBComponent* WBComponent::Create( const HashedString& TypeName, const SimpleString& DefinitionName, WBEntity* const pEntity )
{
	// NOTE: This supports nulling out a component in a mod by using an empty string.
	if( DefinitionName == "" )
	{
		return NULL;
	}

	FactoryMap::Iterator FactoryIter = m_WBCompFactoryMap.Search( TypeName );
	if( FactoryIter.IsNull() )
	{
		PRINTF( "Invalid type requested for WBComponent %s.\n", DefinitionName.CStr() );
		WARNDESC( "Invalid WBComponent type requested." );
		return NULL;
	}

	WBCompFactoryFunc pWBCompFactory = ( *FactoryIter );
	ASSERT( pWBCompFactory );
	WBComponent* pNewComponent = pWBCompFactory();
	ASSERT( pNewComponent );

	pNewComponent->SetEntity( pEntity );
	pNewComponent->Initialize();
	pNewComponent->InitializeFromDefinition( DefinitionName );

	return pNewComponent;
}

WBComponent::WBComponent()
:	m_Entity( NULL )
{
}

WBComponent::~WBComponent()
{
}

/*virtual*/ void WBComponent::Initialize()
{
	if( BelongsInComponentArray() )
	{
		WBComponentArrays::AddComponent( this );
	}
}

/*virtual*/ void WBComponent::ShutDown()
{
	if( BelongsInComponentArray() )
	{
		WBComponentArrays::RemoveComponent( this );
	}
}

/*virtual*/ void WBComponent::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	// Do nothing, WBComponent is abstract and not all components are configurable
	Unused( DefinitionName );
}

/*virtual*/ void WBComponent::ClientTick( float DeltaTime )
{
	// Do nothing
	Unused( DeltaTime );
}

/*virtual*/ void WBComponent::ServerTick( float DeltaTime )
{
	// Do nothing
	Unused( DeltaTime );
}

/*virtual*/ void WBComponent::Tick( float DeltaTime )
{
	// Do nothing
	Unused( DeltaTime );
}

/*virtual*/ void WBComponent::Render()
{
	// Do nothing
}

// As a placeholder for future development, every component at minimum serializes a uint.
// This can be used as a version number later. These functions should not be called by
// derived classes; they are expected to handle the version number themselves.
/*virtual*/ uint WBComponent::GetSerializationSize()
{
	return 4;	// Version number
}

/*virtual*/ void WBComponent::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( 0 );
}

/*virtual*/ void WBComponent::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	Stream.ReadUInt32();
}

/*virtual*/ void WBComponent::Report() const
{
	PRINTF( WBCOMPONENT_REPORT_PREFIX "%s\n", GetReadableName() );
}

#if BUILD_DEV
/*virtual*/ void WBComponent::DebugRender() const
{
	// Do nothing by default
}
#endif

/*virtual*/ void WBComponent::HandleEvent( const WBEvent& Event )
{
	// Do nothing by default
	Unused( Event );
}

/*virtual*/ void WBComponent::AddContextToEvent( WBEvent& Event ) const
{
	// Do nothing by default
	Unused( Event );
}

float WBComponent::GetTime() const
{
	return WBWorld::GetInstance()->GetTime();
}

WBEventManager* WBComponent::GetEventManager() const
{
	return WBWorld::GetInstance()->GetEventManager();
}
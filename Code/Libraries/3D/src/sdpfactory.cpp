#include "core.h"
#include "sdpfactory.h"
#include "configmanager.h"
#include "sdps.h"

static Map<HashedString, SDPFactoryFunc>		sSDPFactoryFuncMap;
static Map<HashedString, ShaderDataProvider*>	sSDPInstanceMap;

void SDPFactory::RegisterSDPFactory( const HashedString& TypeName, SDPFactoryFunc Factory )
{
	ASSERT( sSDPFactoryFuncMap.Search( TypeName ).IsNull() );
	ASSERT( Factory );
	sSDPFactoryFuncMap[ TypeName ] = Factory;
}

void SDPFactory::InitializeBaseFactories()
{
#define ADDSDPFACTORY( type ) SDPFactory::RegisterSDPFactory( #type, SDP##type::Factory );
#include "sdps.h"
#undef ADDSDPFACTORY
}

void SDPFactory::ShutDown()
{
	sSDPFactoryFuncMap.Clear();

	FOR_EACH_MAP( SDPInstanceIter, sSDPInstanceMap, HashedString, ShaderDataProvider* )
	{
		SafeDelete( SDPInstanceIter.GetValue() );
	}
	sSDPInstanceMap.Clear();
}

ShaderDataProvider* SDPFactory::GetSDPInstance( const HashedString& TypeName )
{
	// Return the cached instance, if any
	Map<HashedString, ShaderDataProvider*>::Iterator InstanceIter = sSDPInstanceMap.Search( TypeName );
	if( InstanceIter.IsValid() )
	{
		return InstanceIter.GetValue();
	}

	Map<HashedString, SDPFactoryFunc>::Iterator FactoryIter = sSDPFactoryFuncMap.Search( TypeName );
	if( FactoryIter.IsNull() )
	{
		WARNDESC( "Invalid SDP type requested." );
		return NULL;
	}

	SDPFactoryFunc pFactory = ( *FactoryIter );
	ASSERT( pFactory );

	ShaderDataProvider* const pSDP = pFactory();
	ASSERT( pSDP );

	// Add the new instance to the cache
	sSDPInstanceMap.Insert( TypeName, pSDP );

	return pSDP;
}
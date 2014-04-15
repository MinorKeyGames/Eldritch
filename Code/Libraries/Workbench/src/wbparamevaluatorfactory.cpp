#include "core.h"
#include "wbparamevaluatorfactory.h"
#include "configmanager.h"
#include "wbpe.h"
#include "wbpes.h"

static Map<HashedString, WBPEFactoryFunc>	sFactoryFuncMap;

void WBParamEvaluatorFactory::RegisterFactory( const HashedString& TypeName, WBPEFactoryFunc Factory )
{
	ASSERT( sFactoryFuncMap.Search( TypeName ).IsNull() );
	ASSERT( Factory );
	sFactoryFuncMap[ TypeName ] = Factory;
}

void WBParamEvaluatorFactory::InitializeBaseFactories()
{
#define ADDWBPEFACTORY( type ) WBParamEvaluatorFactory::RegisterFactory( #type, WBPE##type::Factory );
#include "wbpes.h"
#undef ADDWBPEFACTORY
}

void WBParamEvaluatorFactory::ShutDown()
{
	sFactoryFuncMap.Clear();
}

WBPE* WBParamEvaluatorFactory::Create( const SimpleString& DefinitionName )
{
	STATICHASH( PEType );
	MAKEHASH( DefinitionName );
	HashedString PEType = ConfigManager::GetHash( sPEType, "", sDefinitionName );

	Map<HashedString, WBPEFactoryFunc>::Iterator FactoryIter = sFactoryFuncMap.Search( PEType );
	if( FactoryIter.IsNull() )
	{
		// The node type specified by the config file is not valid
		PRINTF( "Invalid type requested for WBPE %s.\n", DefinitionName.CStr() );
		WARNDESC( "Invalid WBPE type requested." );
		return NULL;
	}

	WBPEFactoryFunc pFactory = ( *FactoryIter );
	ASSERT( pFactory );

	WBPE* pNewNode = pFactory();
	ASSERT( pNewNode );

	pNewNode->InitializeFromDefinition( DefinitionName );

	return pNewNode;
}
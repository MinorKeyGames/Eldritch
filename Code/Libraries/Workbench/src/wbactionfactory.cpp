#include "core.h"
#include "wbactionfactory.h"
#include "configmanager.h"
#include "wbaction.h"
#include "wbactions.h"

static Map<HashedString, WBActionFactoryFunc>	sFactoryFuncMap;

void WBActionFactory::RegisterFactory( const HashedString& TypeName, WBActionFactoryFunc Factory )
{
	ASSERT( sFactoryFuncMap.Search( TypeName ).IsNull() );
	ASSERT( Factory );
	sFactoryFuncMap[ TypeName ] = Factory;
}

void WBActionFactory::InitializeBaseFactories()
{
#define ADDWBACTIONFACTORY( type ) WBActionFactory::RegisterFactory( #type, WBAction##type::Factory );
#include "wbactions.h"
#undef ADDWBACTIONFACTORY
}

void WBActionFactory::ShutDown()
{
	sFactoryFuncMap.Clear();
}

WBAction* WBActionFactory::Create( const SimpleString& DefinitionName )
{
	STATICHASH( ActionType );
	MAKEHASH( DefinitionName );
	HashedString ActionType = ConfigManager::GetHash( sActionType, "", sDefinitionName );

	Map<HashedString, WBActionFactoryFunc>::Iterator FactoryIter = sFactoryFuncMap.Search( ActionType );
	if( FactoryIter.IsNull() )
	{
		PRINTF( "Invalid type requested for WBAction %s.\n", DefinitionName.CStr() );
		WARNDESC( "Invalid WBAction type requested." );
		return NULL;
	}

	WBActionFactoryFunc pFactory = ( *FactoryIter );
	ASSERT( pFactory );

	WBAction* pAction = pFactory();
	ASSERT( pAction );

	pAction->InitializeFromDefinition( DefinitionName );

	return pAction;
}

void WBActionFactory::InitializeActionArray( const SimpleString& DefinitionName, Array<WBAction*>& OutActionArray )
{
	MAKEHASH( DefinitionName );

	STATICHASH( NumActions );
	const uint NumActions = ConfigManager::GetInt( sNumActions, 0, sDefinitionName );
	for( uint ActionIndex = 0; ActionIndex < NumActions; ++ActionIndex )
	{
		const SimpleString Action = ConfigManager::GetSequenceString( "Action%d", ActionIndex, "", DefinitionName );
		WBAction* const pAction = WBActionFactory::Create( Action );
		if( pAction )
		{
			OutActionArray.PushBack( pAction );
		}
	}
}
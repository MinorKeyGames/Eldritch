#include "core.h"
#include "rodinbtnodefactory.h"
#include "configmanager.h"
#include "rodinbtnode.h"
#include "rodinbtnodes.h"

static Map<HashedString, RodinBTNodeFactoryFunc>	sFactoryFuncMap;

void RodinBTNodeFactory::RegisterFactory( const HashedString& TypeName, RodinBTNodeFactoryFunc Factory )
{
	ASSERT( sFactoryFuncMap.Search( TypeName ).IsNull() );
	ASSERT( Factory );
	sFactoryFuncMap[ TypeName ] = Factory;
}

void RodinBTNodeFactory::InitializeBaseFactories()
{
#define ADDRODINBTNODEFACTORY( type ) RodinBTNodeFactory::RegisterFactory( #type, RodinBTNode##type::Factory );
#include "rodinbtnodes.h"
#undef ADDRODINBTNODEFACTORY
}

void RodinBTNodeFactory::ShutDown()
{
	sFactoryFuncMap.Clear();
}

RodinBTNode* RodinBTNodeFactory::Create( const SimpleString& DefinitionName, WBCompRodinBehaviorTree* const pBehaviorTree )
{
	STATICHASH( NodeType );
	MAKEHASH( DefinitionName );
	HashedString NodeType = ConfigManager::GetHash( sNodeType, "", sDefinitionName );

	Map<HashedString, RodinBTNodeFactoryFunc>::Iterator FactoryIter = sFactoryFuncMap.Search( NodeType );
	if( FactoryIter.IsNull() )
	{
		PRINTF( "Invalid type requested for RodinBTNode %s.\n", DefinitionName.CStr() );
		WARNDESC( "Invalid RodinBTNode type requested." );
		return NULL;
	}

	RodinBTNodeFactoryFunc pFactory = ( *FactoryIter );
	ASSERT( pFactory );

	RodinBTNode* pNewNode = pFactory();
	ASSERT( pNewNode );

	pNewNode->m_DefinitionName = DefinitionName;
	pNewNode->m_BehaviorTree = pBehaviorTree;
	pNewNode->InitializeFromDefinition( DefinitionName );

	return pNewNode;
}
#include "core.h"
#include "rodinbtnodelookup.h"
#include "rodinbtnodefactory.h"
#include "Components/wbcomprodinbehaviortree.h"
#include "configmanager.h"

RodinBTNodeLookup::RodinBTNodeLookup()
{
}

RodinBTNodeLookup::~RodinBTNodeLookup()
{
}

void RodinBTNodeLookup::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	ASSERT( m_BehaviorTree );

	MAKEHASH( DefinitionName );

	STATICHASH( Key );
	const HashedString Key = ConfigManager::GetHash( sKey, "", sDefinitionName );
	const SimpleString NodeDef = m_BehaviorTree->GetLookupNode( Key );

	m_Child = RodinBTNodeFactory::Create( NodeDef, m_BehaviorTree );
	ASSERT( m_Child );
}
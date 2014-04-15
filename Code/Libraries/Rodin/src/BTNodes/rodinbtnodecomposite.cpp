#include "core.h"
#include "rodinbtnodecomposite.h"
#include "simplestring.h"
#include "configmanager.h"
#include "rodinbtnodefactory.h"

RodinBTNodeComposite::RodinBTNodeComposite()
:	m_Children()
{
}

RodinBTNodeComposite::~RodinBTNodeComposite()
{
	for( uint ChildIndex = 0; ChildIndex < m_Children.Size(); ++ChildIndex )
	{
		SafeDelete( m_Children[ ChildIndex ] );
	}
	m_Children.Clear();
}

void RodinBTNodeComposite::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	STATICHASH( NumChildren );
	MAKEHASH( DefinitionName );

	uint NumChildren = ConfigManager::GetInt( sNumChildren, 0, sDefinitionName );

	for( uint ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex )
	{
		const SimpleString ChildDef = ConfigManager::GetSequenceString( "Child%d", ChildIndex, "", sDefinitionName );
		RodinBTNode* pChildNode = RodinBTNodeFactory::Create( ChildDef, m_BehaviorTree );
		if( pChildNode )
		{
			m_Children.PushBack( pChildNode );
		}
	}
}
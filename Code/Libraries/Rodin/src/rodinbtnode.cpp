#include "core.h"
#include "rodinbtnode.h"
#include "simplestring.h"
#include "Components/wbcomprodinbehaviortree.h"
#include "wbworld.h"

RodinBTNode::RodinBTNode()
:	m_BehaviorTree( NULL )
,	m_IsSleeping( false )
{
}

RodinBTNode::~RodinBTNode()
{
}

void RodinBTNode::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Unused( DefinitionName );
}

RodinBTNode::ETickStatus RodinBTNode::Tick( float DeltaTime )
{
	Unused( DeltaTime );
	return ETS_None;
}

void RodinBTNode::OnStart()
{
}

void RodinBTNode::OnFinish()
{
}

void RodinBTNode::OnChildCompleted( RodinBTNode* pChildNode, ETickStatus TickStatus )
{
	Unused( pChildNode );
	Unused( TickStatus );
}

void RodinBTNode::Report( uint Depth )
{
	PRINTF( WBPROPERTY_REPORT_PREFIX WB_REPORT_SPACER );
	for( uint i = 0; i < Depth; ++i )
	{
		PRINTF( WB_REPORT_SPACER );
	}
	PRINTF( "%s\n", GetName().CStr() );
}

WBEntity* RodinBTNode::GetEntity() const
{
	ASSERT( m_BehaviorTree );
	return m_BehaviorTree->GetEntity();
}

float RodinBTNode::GetTime() const
{
	return WBWorld::GetInstance()->GetTime();
}

WBEventManager* RodinBTNode::GetEventManager() const
{
	return WBWorld::GetInstance()->GetEventManager();
}

SimpleString RodinBTNode::GetName() const
{
	return m_DefinitionName;
}
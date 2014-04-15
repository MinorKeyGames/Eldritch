#include "core.h"
#include "rodinbtnodesleep.h"
#include "Components/wbcomprodinbehaviortree.h"

RodinBTNodeSleep::RodinBTNodeSleep()
{
}

RodinBTNodeSleep::~RodinBTNodeSleep()
{
}

RodinBTNode::ETickStatus RodinBTNodeSleep::Tick( float DeltaTime )
{
	Unused( DeltaTime );
	return ETS_Success;
}

void RodinBTNodeSleep::OnStart()
{
	RodinBTNode::OnStart();

	m_BehaviorTree->Sleep( this );
}

void RodinBTNodeSleep::OnFinish()
{
	RodinBTNode::OnFinish();

	if( m_IsSleeping )
	{
		m_BehaviorTree->Wake( this );
	}
}
#include "core.h"
#include "rodinbtnodeselector.h"
#include "simplestring.h"
#include "Components/wbcomprodinbehaviortree.h"

RodinBTNodeSelector::RodinBTNodeSelector()
{
}

RodinBTNodeSelector::~RodinBTNodeSelector()
{
}

RodinBTNode::ETickStatus RodinBTNodeSelector::Tick( float DeltaTime )
{
	Unused( DeltaTime );

	if( m_ChildStatus == ETS_Running )
	{
		WARN;	// This case shouldn't happen if we put this task to sleep, so make sure!
		return ETS_Running;
	}

	// Succeed if any subtask succeeds
	if( m_ChildStatus == ETS_Success )
	{
		return ETS_Success;
	}

	// If there's any tasks left to run, do that now
	if( m_ChildIndex < m_Children.Size() )
	{
		// Start subtask
		m_BehaviorTree->Start( m_Children[ m_ChildIndex ], this );
		m_ChildStatus = ETS_Running;
		m_BehaviorTree->Sleep( this );
		return ETS_Running;
	}
	else
	{
		// Else we've run out of tasks and none have succeeded, so we fail
		return ETS_Fail;
	}
}
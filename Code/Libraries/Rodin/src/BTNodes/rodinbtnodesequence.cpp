#include "core.h"
#include "rodinbtnodesequence.h"
#include "simplestring.h"
#include "Components/wbcomprodinbehaviortree.h"

RodinBTNodeSequence::RodinBTNodeSequence()
{
}

RodinBTNodeSequence::~RodinBTNodeSequence()
{
}

RodinBTNode::ETickStatus RodinBTNodeSequence::Tick( float DeltaTime )
{
	Unused( DeltaTime );

	if( m_ChildStatus == ETS_Running )
	{
		WARN;	// This case shouldn't happen if we put this task to sleep, so make sure!
		return ETS_Running;
	}

	// Fail if any subtask fails
	if( m_ChildStatus == ETS_Fail )
	{
		return ETS_Fail;
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
		// Else we've run out of tasks and they've all succeeded, so we succeed
		return ETS_Success;
	}
}
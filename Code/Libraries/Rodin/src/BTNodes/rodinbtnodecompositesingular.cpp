#include "core.h"
#include "rodinbtnodecompositesingular.h"
#include "simplestring.h"
#include "Components/wbcomprodinbehaviortree.h"

RodinBTNodeCompositeSingular::RodinBTNodeCompositeSingular()
:	m_ChildIndex( 0 )
,	m_ChildStatus( ETS_None )
{
}

RodinBTNodeCompositeSingular::~RodinBTNodeCompositeSingular()
{
}

void RodinBTNodeCompositeSingular::OnStart()
{
	RodinBTNodeComposite::OnStart();

	m_ChildIndex = 0;
	m_ChildStatus = ETS_None;
}

void RodinBTNodeCompositeSingular::OnFinish()
{
	RodinBTNodeComposite::OnFinish();

	// In case this is interrupted, clean up currently running subtask (if any).
	if( m_ChildIndex < m_Children.Size() && m_ChildStatus == ETS_Running )
	{
		m_BehaviorTree->Stop( m_Children[ m_ChildIndex ] );
	}
}

void RodinBTNodeCompositeSingular::OnChildCompleted( RodinBTNode* pChildNode, ETickStatus TickStatus )
{
	RodinBTNodeComposite::OnChildCompleted( pChildNode, TickStatus );

	ASSERT( pChildNode == m_Children[ m_ChildIndex ] );

	m_ChildStatus = TickStatus;
	m_ChildIndex++;

	m_BehaviorTree->Wake( this );
}

void RodinBTNodeCompositeSingular::Report( uint Depth )
{
	RodinBTNode::Report( Depth );

	for( uint ChildIndex = 0; ChildIndex < m_Children.Size(); ++ChildIndex )
	{
		m_Children[ ChildIndex ]->Report( Depth + 1 );
	}
}
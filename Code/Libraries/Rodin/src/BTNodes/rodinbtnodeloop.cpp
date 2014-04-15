#include "core.h"
#include "rodinbtnodeloop.h"
#include "configmanager.h"
#include "Components/wbcomprodinbehaviortree.h"

RodinBTNodeLoop::RodinBTNodeLoop()
:	m_CanFail( false )
,	m_CanSucceed( false )
,	m_LastTickTime( 0 )
{
}

RodinBTNodeLoop::~RodinBTNodeLoop()
{
}

void RodinBTNodeLoop::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	RodinBTNodeDecorator::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( CanFail );
	m_CanFail = ConfigManager::GetBool( sCanFail, false, sDefinitionName );

	STATICHASH( CanSucceed );
	m_CanSucceed = ConfigManager::GetBool( sCanSucceed, false, sDefinitionName );
}

RodinBTNode::ETickStatus RodinBTNodeLoop::Tick( float DeltaTime )
{
	Unused( DeltaTime );

	if( m_ChildStatus == ETS_Running )
	{
		WARN;	// This case shouldn't happen if we put this task to sleep, so make sure!
		return ETS_Running;
	}

	if( m_CanFail && ( m_ChildStatus == ETS_Fail ) )
	{
		return ETS_Fail;
	}

	if( m_CanSucceed && ( m_ChildStatus == ETS_Success ) )
	{
		return ETS_Success;
	}

	if( GetTime() > m_LastTickTime )
	{
		m_LastTickTime = GetTime();

		// (Re)start child node.
		m_BehaviorTree->Start( m_Child, this );
		m_ChildStatus = ETS_Running;
		m_BehaviorTree->Sleep( this );
	}

	return ETS_Running;
}
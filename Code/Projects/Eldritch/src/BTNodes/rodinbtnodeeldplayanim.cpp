#include "core.h"
#include "rodinbtnodeeldplayanim.h"
#include "configmanager.h"
#include "Components/wbcomprodinbehaviortree.h"
#include "wbeventmanager.h"
#include "animation.h"

RodinBTNodeEldPlayAnim::RodinBTNodeEldPlayAnim()
:	m_AnimationName( "" )
,	m_Loop( false )
{
}

RodinBTNodeEldPlayAnim::~RodinBTNodeEldPlayAnim()
{
}

void RodinBTNodeEldPlayAnim::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	MAKEHASH( DefinitionName );

	STATICHASH( Animation );
	m_AnimationName = ConfigManager::GetHash( sAnimation, HashedString::NullString, sDefinitionName );

	STATICHASH( Loop );
	m_Loop = ConfigManager::GetBool( sLoop, false, sDefinitionName );
}

RodinBTNode::ETickStatus RodinBTNodeEldPlayAnim::Tick( float DeltaTime )
{
	Unused( DeltaTime );

	if( m_HasRequestedAnim )
	{
		return ETS_Success;
	}
	else
	{
		WB_MAKE_EVENT( PlayAnim, GetEntity() );
		WB_SET_AUTO( PlayAnim, Hash, AnimationName, m_AnimationName );
		WB_SET_AUTO( PlayAnim, Bool, Loop, m_Loop );
		WB_DISPATCH_EVENT( GetEventManager(), PlayAnim, GetEntity() );

		// NOTE: We just have to trust that the animation will actually start. No return value from event.
		m_HasRequestedAnim = true;

		// Go to sleep and wait for animation to complete.
		m_BehaviorTree->Sleep( this );
		return ETS_Running;
	}
}

/*virtual*/ void RodinBTNodeEldPlayAnim::OnStart()
{
	RodinBTNode::OnStart();

	m_HasRequestedAnim = false;

	STATIC_HASHED_STRING( OnAnimationFinished );
	GetEventManager()->AddObserver( sOnAnimationFinished, this, GetEntity() );
}

/*virtual*/ void RodinBTNodeEldPlayAnim::OnFinish()
{
	RodinBTNode::OnFinish();

	STATIC_HASHED_STRING( OnAnimationFinished );
	GetEventManager()->RemoveObserver( sOnAnimationFinished, this, GetEntity() );

	if( m_IsSleeping )
	{
		m_BehaviorTree->Wake( this );
	}
}

/*virtual*/ void RodinBTNodeEldPlayAnim::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	STATIC_HASHED_STRING( OnAnimationFinished );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sOnAnimationFinished )
	{
		STATIC_HASHED_STRING( AnimationName );
		const HashedString AnimationName = Event.GetHash( sAnimationName );

		if( AnimationName == m_AnimationName )
		{
			if( m_IsSleeping )
			{
				m_BehaviorTree->Wake( this );
			}
		}
	}
}
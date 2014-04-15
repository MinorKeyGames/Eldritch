#include "core.h"
#include "rodinbtnodetimeout.h"
#include "configmanager.h"

RodinBTNodeTimeout::RodinBTNodeTimeout()
:	m_TimeoutPE()
,	m_NextCanExecuteTime( 0.0f )
{
}

RodinBTNodeTimeout::~RodinBTNodeTimeout()
{
}

void RodinBTNodeTimeout::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	RodinBTNodeDecorator::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( TimeoutPE );
	m_TimeoutPE.InitializeFromDefinition( ConfigManager::GetString( sTimeoutPE, "", sDefinitionName ) );
}

/*virtual*/ RodinBTNode::ETickStatus RodinBTNodeTimeout::Tick( float DeltaTime )
{
	const float Time = GetTime();

	// We only need to apply the timeout when the child is starting.
	if( m_ChildStatus != ETS_None )
	{
		return RodinBTNodeDecorator::Tick( DeltaTime );
	}

	if( Time < m_NextCanExecuteTime )
	{
		// In all my use cases so far, success has been the expected behavior here.
		// If that ever changes, I can return failure and override the result with another decorator.
		return ETS_Success;
	}

	// NOTE: In the old Couriers implementation, the timeout was only set if the child ran successfully.
	// I can't find if there was a use case for that or if it was just a theoretical benefit, but I'm
	// not doing it here. I can always bring it back if desired.
	WBParamEvaluator::SPEContext Context;
	Context.m_Entity = GetEntity();
	m_TimeoutPE.Evaluate( Context );
	m_NextCanExecuteTime = Time + m_TimeoutPE.GetFloat();

	return RodinBTNodeDecorator::Tick( DeltaTime );
}
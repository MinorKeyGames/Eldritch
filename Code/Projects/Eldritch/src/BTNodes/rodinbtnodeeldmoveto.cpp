#include "core.h"
#include "rodinbtnodeeldmoveto.h"
#include "configmanager.h"
#include "Components/wbcompeldaimotion.h"
#include "Components/wbcomprodinblackboard.h"

RodinBTNodeEldMoveTo::RodinBTNodeEldMoveTo()
:	m_MoveTargetBlackboardKey()
,	m_Wander( false )
,	m_WanderTargetDistance( 0.0f )
,	m_ReachedThresholdMin( 0.0f )
,	m_ReachedThresholdMax( 0.0f )
,	m_FlyingDeflectionRadius( 0.0f )
,	m_MoveState( EMTS_Begin )
{
}

RodinBTNodeEldMoveTo::~RodinBTNodeEldMoveTo()
{
}

void RodinBTNodeEldMoveTo::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	MAKEHASH( DefinitionName );

	STATICHASH( MoveTargetBlackboardKey );
	m_MoveTargetBlackboardKey = ConfigManager::GetHash( sMoveTargetBlackboardKey, HashedString::NullString, sDefinitionName );

	STATICHASH( Wander );
	m_Wander = ConfigManager::GetBool( sWander, false, sDefinitionName );

	STATICHASH( WanderTargetDistance );
	m_WanderTargetDistance = ConfigManager::GetFloat( sWanderTargetDistance, false, sDefinitionName );

	STATICHASH( ReachedThresholdMin );
	m_ReachedThresholdMin = ConfigManager::GetFloat( sReachedThresholdMin, 0.0f, sDefinitionName );

	STATICHASH( ReachedThresholdMax );
	m_ReachedThresholdMax = ConfigManager::GetFloat( sReachedThresholdMax, 0.0f, sDefinitionName );

	STATICHASH( DeflectionRadius );
	m_FlyingDeflectionRadius = ConfigManager::GetFloat( sDeflectionRadius, 0.0f, sDefinitionName );
}

void RodinBTNodeEldMoveTo::OnStart()
{
	m_MoveState = EMTS_Begin;
}

void RodinBTNodeEldMoveTo::OnFinish()
{
	WBCompEldAIMotion* pAIMotion = GET_WBCOMP( GetEntity(), EldAIMotion );
	ASSERT( pAIMotion );

	if( m_MoveState == EMTS_StartedMove )
	{
		pAIMotion->StopMove();
	}
}

RodinBTNode::ETickStatus RodinBTNodeEldMoveTo::Tick( float DeltaTime )
{
	Unused( DeltaTime );

	WBEntity* const		pEntity		= GetEntity();
	WBCompEldAIMotion*	pAIMotion	= GET_WBCOMP( pEntity, EldAIMotion );
	ASSERT( pAIMotion );

	if( m_MoveState == EMTS_Begin )
	{
		if( m_Wander )
		{
			pAIMotion->SetReachedThreshold( m_ReachedThresholdMin, m_ReachedThresholdMax );
			pAIMotion->StartWander( m_WanderTargetDistance );
			m_MoveState = EMTS_StartedMove;

			return ETS_Running;
		}
		else
		{
			WBCompRodinBlackboard* const	pAIBlackboard	= GET_WBCOMP( pEntity, RodinBlackboard );
			ASSERT( pAIBlackboard );

			const WBEvent::EType			TargetType		= pAIBlackboard->GetType( m_MoveTargetBlackboardKey );

			if( TargetType == WBEvent::EWBEPT_Vector )
			{
				const Vector MoveTarget = pAIBlackboard->GetVector( m_MoveTargetBlackboardKey );

				pAIMotion->SetReachedThreshold( m_ReachedThresholdMin, m_ReachedThresholdMax );
				pAIMotion->SetDeflectionRadius( m_FlyingDeflectionRadius );
				pAIMotion->StartMove( MoveTarget );
				m_MoveState = EMTS_StartedMove;

				return ETS_Running;
			}
			else if( TargetType == WBEvent::EWBEPT_Entity )
			{
				WBEntity* const pMoveTargetEntity = pAIBlackboard->GetEntity( m_MoveTargetBlackboardKey );
				if( !pMoveTargetEntity )
				{
					return ETS_Fail;
				}

//#if BUILD_DEV
//				PRINTF( "RodinBTNodeEldMoveTo: %s: %s moving to entity %s\n",
//					GetName().CStr(),
//					pEntity->GetUniqueName().CStr(),
//					pMoveTargetEntity->GetUniqueName().CStr() );
//#endif

				pAIMotion->SetReachedThreshold( m_ReachedThresholdMin, m_ReachedThresholdMax );
				pAIMotion->SetDeflectionRadius( m_FlyingDeflectionRadius );
				pAIMotion->StartFollow( pMoveTargetEntity );
				m_MoveState = EMTS_StartedMove;

				return ETS_Running;
			}
		}
	}
	else
	{
		ASSERT( m_MoveState == EMTS_StartedMove );

		if( pAIMotion->IsMoving() )
		{
			return ETS_Running;
		}
		else if( pAIMotion->DidMoveSucceed() )
		{
			return ETS_Success;
		}
	}

	return ETS_Fail;
}
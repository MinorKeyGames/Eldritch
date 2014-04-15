#include "core.h"
#include "rodinbtnodeeldturntoward.h"
#include "configmanager.h"
#include "Components/wbcompeldtransform.h"
#include "Components/wbcompeldaimotion.h"
#include "Components/wbcomprodinblackboard.h"

RodinBTNodeEldTurnToward::RodinBTNodeEldTurnToward()
:	m_TurnTargetBlackboardKey()
,	m_TurnState( ETTS_Begin )
{
}

RodinBTNodeEldTurnToward::~RodinBTNodeEldTurnToward()
{
}

void RodinBTNodeEldTurnToward::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	MAKEHASH( DefinitionName );

	STATICHASH( TurnTargetBlackboardKey );
	m_TurnTargetBlackboardKey = ConfigManager::GetHash( sTurnTargetBlackboardKey, HashedString::NullString, sDefinitionName );
}

void RodinBTNodeEldTurnToward::OnStart()
{
	m_TurnState = ETTS_Begin;
}

void RodinBTNodeEldTurnToward::OnFinish()
{
	WBCompEldAIMotion* pAIMotion = GET_WBCOMP( GetEntity(), EldAIMotion );
	ASSERT( pAIMotion );

	if( m_TurnState == ETTS_StartedTurn )
	{
		pAIMotion->StopMove();
	}
}

RodinBTNode::ETickStatus RodinBTNodeEldTurnToward::Tick( float DeltaTime )
{
	Unused( DeltaTime );

	WBEntity* const		pEntity		= GetEntity();
	WBCompEldAIMotion*	pAIMotion	= GET_WBCOMP( pEntity, EldAIMotion );
	ASSERT( pAIMotion );

	if( m_TurnState == ETTS_Begin )
	{
		WBCompRodinBlackboard* const	pAIBlackboard	= GET_WBCOMP( pEntity, RodinBlackboard );
		ASSERT( pAIBlackboard );

		const WBEvent::EType			TargetType		= pAIBlackboard->GetType( m_TurnTargetBlackboardKey );

		if( TargetType == WBEvent::EWBEPT_Vector )
		{
			const Vector TurnTarget = pAIBlackboard->GetVector( m_TurnTargetBlackboardKey );

			pAIMotion->StartTurn( TurnTarget );
			m_TurnState = ETTS_StartedTurn;

			return ETS_Running;
		}
		else if( TargetType == WBEvent::EWBEPT_Entity )
		{
			WBEntity* const pTurnTargetEntity = pAIBlackboard->GetEntity( m_TurnTargetBlackboardKey );
			if( !pTurnTargetEntity )
			{
				return ETS_Fail;
			}

			WBCompEldTransform* const pTransform = pTurnTargetEntity->GetTransformComponent<WBCompEldTransform>();
			if( !pTransform )
			{
				return ETS_Fail;
			}

			const Vector TurnTarget = pTransform->GetLocation();

			pAIMotion->StartTurn( TurnTarget );
			m_TurnState = ETTS_StartedTurn;

			return ETS_Running;
		}
	}
	else
	{
		ASSERT( m_TurnState == ETTS_StartedTurn );

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
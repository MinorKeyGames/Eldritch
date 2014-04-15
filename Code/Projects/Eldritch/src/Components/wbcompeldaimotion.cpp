#include "core.h"
#include "wbcompeldaimotion.h"
#include "wbcompeldtransform.h"
#include "wbcompeldcollision.h"
#include "wbcompelddoor.h"
#include "Components/wbcompeldhealth.h"
#include "Components/wbcomprodinknowledge.h"
#include "Components/wbcomprodinresourcemap.h"
#include "Components/wbcompstatmod.h"
#include "configmanager.h"
#include "mathcore.h"
#include "eldritchnav.h"
#include "wbeventmanager.h"
#include "eldritchframework.h"
#include "irenderer.h"
#include "segment.h"
#include "collisioninfo.h"
#include "idatastream.h"

// NOTE: This *never* releases its animation resource. That's ok.

WBCompEldAIMotion::WBCompEldAIMotion()
:	m_LandAcceleration( 0.0f )
,	m_AirAcceleration( 0.0f )
,	m_JumpImpulse( 0.0f )
,	m_JumpOnNextTick( false )
,	m_TurnSpeed( 0.0f )
,	m_FollowValidateTime( 0.0f )
,	m_NextFollowValidateTime( 0.0f )
,	m_StepReachedThresholdSq( 0.0f )
,	m_TurnReachedThreshold( 0.0f )
,	m_RepathOnNextTick( false )
,	m_Paused( false )
,	m_IsFlying( false )
,	m_CanJump( false )
,	m_CanOpenDoors( false )
,	m_CanUnlockDoors( false )
,	m_MaxPathSteps( 0 )
,	m_IdleAnimationName()
,	m_WalkAnimationName()
,	m_JumpAnimationName()
,	m_AnimationResource()
,	m_Path()
,	m_PathIndex( 0 )
,	m_PathBound()
,	m_MotionStatus( EMS_Still )
,	m_Wander( false )
,	m_WanderTargetDistance( 0.0f )
,	m_ReachedThresholdMinSq( 0.0f )
,	m_ReachedThresholdMaxSq( 0.0f )
,	m_FlyingDeflectionRadiusSq( 0.0f )
,	m_LastDestination()
,	m_LastDestinationIndex( 0 )
,	m_LastDestinationEntity()
{
	STATIC_HASHED_STRING( OnWorldChanged );
	GetEventManager()->AddObserver( sOnWorldChanged, this );
}

WBCompEldAIMotion::~WBCompEldAIMotion()
{
	WBEventManager* const pEventManager = GetEventManager();
	if( pEventManager )
	{
		STATIC_HASHED_STRING( OnWorldChanged );
		pEventManager->RemoveObserver( sOnWorldChanged, this );
	}
}

void WBCompEldAIMotion::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( LandAcceleration );
	m_LandAcceleration = ConfigManager::GetInheritedFloat( sLandAcceleration, 0.0f, sDefinitionName );

	STATICHASH( AirAcceleration );
	m_AirAcceleration = ConfigManager::GetInheritedFloat( sAirAcceleration, 0.0f, sDefinitionName );

	STATICHASH( JumpImpulse );
	m_JumpImpulse = ConfigManager::GetInheritedFloat( sJumpImpulse, 0.0f, sDefinitionName );

	STATICHASH( TurnSpeed );
	m_TurnSpeed = DEGREES_TO_RADIANS( ConfigManager::GetInheritedFloat( sTurnSpeed, 0.0f, sDefinitionName ) );

	STATICHASH( FollowValidateTime );
	m_FollowValidateTime = ConfigManager::GetInheritedFloat( sFollowValidateTime, 0.0f, sDefinitionName );

	STATICHASH( StepReachedThreshold );
	m_StepReachedThresholdSq = Square( ConfigManager::GetInheritedFloat( sStepReachedThreshold, 0.0f, sDefinitionName ) );

	STATICHASH( TurnReachedThreshold );
	const float TurnReachedComplementDegrees = 90.0f - ConfigManager::GetInheritedFloat( sTurnReachedThreshold, 0.0f, sDefinitionName );
	m_TurnReachedThreshold = Cos( DEGREES_TO_RADIANS( TurnReachedComplementDegrees ) );

	STATICHASH( IsFlying );
	m_IsFlying = ConfigManager::GetInheritedBool( sIsFlying, false, sDefinitionName );

	STATICHASH( CanJump );
	m_CanJump = ConfigManager::GetInheritedBool( sCanJump, false, sDefinitionName );

	STATICHASH( CanUnlockDoors );
	m_CanUnlockDoors = ConfigManager::GetInheritedBool( sCanUnlockDoors, false, sDefinitionName );

	STATICHASH( CanOpenDoors );
	m_CanOpenDoors = m_CanUnlockDoors || ConfigManager::GetInheritedBool( sCanOpenDoors, false, sDefinitionName );

	STATICHASH( MaxPathSteps );
	m_MaxPathSteps = ConfigManager::GetInheritedInt( sMaxPathSteps, 0, sDefinitionName );

	STATICHASH( IdleAnimation );
	m_IdleAnimationName = ConfigManager::GetInheritedHash( sIdleAnimation, HashedString::NullString, sDefinitionName );

	STATICHASH( WalkAnimation );
	m_WalkAnimationName = ConfigManager::GetInheritedHash( sWalkAnimation, HashedString::NullString, sDefinitionName );

	STATICHASH( JumpAnimation );
	m_JumpAnimationName = ConfigManager::GetInheritedHash( sJumpAnimation, HashedString::NullString, sDefinitionName );

	STATICHASH( AnimationResource );
	m_AnimationResource = ConfigManager::GetInheritedHash( sAnimationResource, HashedString::NullString, sDefinitionName );
}

/*virtual*/ void WBCompEldAIMotion::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( OnCollided );
	STATIC_HASHED_STRING( OnWorldChanged );
	STATIC_HASHED_STRING( OnLanded );
	STATIC_HASHED_STRING( OnInitialized );
	STATIC_HASHED_STRING( PauseAIMotion );
	STATIC_HASHED_STRING( UnpauseAIMotion );
	STATIC_HASHED_STRING( Follow );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sOnCollided )
	{
		STATIC_HASHED_STRING( CollidedEntity );
		WBEntity* const				pCollidedEntity		= Event.GetEntity( sCollidedEntity );
		WBCompEldDoor* const		pCollidedDoor		= SAFE_GET_WBCOMP( pCollidedEntity, EldDoor );

		if( pCollidedDoor && m_CanOpenDoors )
		{
			// This was intended to be more general purpose, but doors are the only thing that can be pushed.
			WB_MAKE_EVENT( OnPushed, pCollidedEntity );
			WB_SET_AUTO( OnPushed, Entity, Pusher, GetEntity() );
			WB_DISPATCH_EVENT( GetEventManager(), OnPushed, pCollidedEntity );
		}
		else
		{
			// If we collided with anything except a door, or if we can't open doors, jump!
			TryJump();
		}
	}
	else if( EventName == sOnWorldChanged )
	{
		if( IsLocomoting() )
		{
			STATIC_HASHED_STRING( BoxMin );
			const Vector BoxMin = Event.GetVector( sBoxMin );

			STATIC_HASHED_STRING( BoxMax );
			const Vector BoxMax = Event.GetVector( sBoxMax );

			const AABB ChangedBox( BoxMin, BoxMax );
			m_RepathOnNextTick = ChangedBox.Intersects( m_PathBound );
		}
	}
	else if( EventName == sOnInitialized )
	{
		PlayAnimation( m_IdleAnimationName, true, true, 1.0f );
	}
	else if( EventName == sOnLanded )
	{
		if( IsLocomoting() )
		{
			PlayAnimation( m_WalkAnimationName, true, true, GetWalkAnimationPlayRate() );
		}
		else
		{
			PlayAnimation( m_IdleAnimationName, true, true, 1.0f );
		}
	}
	else if( EventName == sPauseAIMotion )
	{
		StopMove();
		m_Paused = true;
	}
	else if( EventName == sUnpauseAIMotion )
	{
		m_Paused = false;
	}
	else if( EventName == sFollow )
	{
		// HACKHACK: This is mostly for Yog in the expansion. Normally, AIs should
		// invoke motion through their behavior trees (i.e., RodinBTNodeEldMoveTo).

		STATIC_HASHED_STRING( FollowEntity );
		WBEntity* const	pFollowEntity		= Event.GetEntity( sFollowEntity );

		STATIC_HASHED_STRING( ReachedThresholdMin );
		const float		ReachedThresholdMin	= Event.GetFloat( sReachedThresholdMin );

		STATIC_HASHED_STRING( ReachedThresholdMax );
		const float		ReachedThresholdMax	= Event.GetFloat( sReachedThresholdMax );

		STATIC_HASHED_STRING( DeflectionRadius );
		const float		DeflectionRadius	= Event.GetFloat( sDeflectionRadius );

		SetReachedThreshold( ReachedThresholdMin, ReachedThresholdMax );
		SetDeflectionRadius( DeflectionRadius );
		StartFollow( pFollowEntity );
	}
}

/*virtual*/ void WBCompEldAIMotion::AddContextToEvent( WBEvent& Event ) const
{
	Super::AddContextToEvent( Event );

	WB_SET_CONTEXT( Event, Bool, CanOpenDoors, m_CanOpenDoors );
	WB_SET_CONTEXT( Event, Bool, CanUnlockDoors, m_CanUnlockDoors );
}

/*virtual*/ void WBCompEldAIMotion::Tick( float DeltaTime )
{
	XTRACE_FUNCTION;

	Unused( DeltaTime );

	if( m_RepathOnNextTick )
	{
		m_RepathOnNextTick = false;
		Repath();
	}

	if( m_MotionStatus == EMS_Following && GetTime() > m_NextFollowValidateTime )
	{
		m_NextFollowValidateTime = GetTime() + m_FollowValidateTime;
		ValidateFollow();
	}

	TickMove();

	if( m_MotionStatus == EMS_TurningToFace )
	{
		TickTurn();
	}
}

bool WBCompEldAIMotion::GetNextStep( Vector& OutStep )
{
	WBCompEldTransform* const pTransform = GetEntity()->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	const Vector CurrentLocation	= pTransform->GetLocation();
	const Vector CurrentFacing		= pTransform->GetOrientation().ToVector();

	for( ; m_PathIndex < m_Path.Size(); ++m_PathIndex )
	{
		const Vector&	Step		= m_Path[ m_PathIndex ];

		const float		StepDistSq	= ( CurrentLocation - Step ).LengthSquared2D();
		if( StepDistSq < m_StepReachedThresholdSq )
		{
			// We're within a reasonable tolerance, go to the next step.
			continue;
		}

		// We're still on our way to this step.
		OutStep = Step;

		return true;
	}

	return false;
}

// Bastardized from TickMove. I should refactor to handle this case properly within that function.
void WBCompEldAIMotion::TickTurn()
{
	ASSERT( m_MotionStatus == EMS_TurningToFace );

	WBEntity* const pEntity = GetEntity();
	DEVASSERT( pEntity );

	WBCompEldTransform* const pTransform = pEntity->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	const Vector		MovementOffset		= m_LastDestination - pTransform->GetLocation();
	const Vector		MovementDirection	= MovementOffset.Get2D().GetNormalized();

	if( MovementDirection.IsZero() )
	{
		// We're at the target location, nowhere to turn.
		// TODO: Face a target direction that's *not* toward the target point?
		m_MotionStatus = EMS_MoveSucceeded;
	}

	const Angles		CurrentOrientation	= pTransform->GetOrientation();
	const Vector		CurrentFacing		= CurrentOrientation.ToVector();
	static const Vector	Up					= Vector( 0.0f, 0.0f, 1.0f );
	const Vector		CurrentRight		= CurrentFacing.Cross( Up );
	const bool			FacingMovement		= MovementDirection.Dot( CurrentFacing ) > 0.0f;
	const float			CosTurnAngle		= MovementDirection.Dot( CurrentRight );
	const float			TurnEpsilon			= FacingMovement ? m_TurnReachedThreshold : 0.0f;
	const bool			NeedsRightTurn		= CosTurnAngle >= TurnEpsilon;
	const bool			NeedsLeftTurn		= CosTurnAngle <= -TurnEpsilon;

	if( NeedsLeftTurn || NeedsRightTurn )
	{
		const float		TurnYaw				= NeedsRightTurn ? -1.0f : ( NeedsLeftTurn ? 1.0f : 0.0f );
		const Angles	TurnVelocity		= Angles( 0.0f, 0.0f, TurnYaw * m_TurnSpeed );
		pTransform->SetRotationalVelocity( TurnVelocity );
	}
	else
	{
		// We've finished turning to face. Movement is all done.
		m_MotionStatus = EMS_MoveSucceeded;
	}
}

void WBCompEldAIMotion::TickMove()
{
	m_IsFlying ? TickFlying() : TickWalking();
}

void WBCompEldAIMotion::TickWalking()
{
	WBEntity* const pEntity = GetEntity();
	DEVASSERT( pEntity );

	WBCompEldTransform* const	pTransform = pEntity->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	WBCompEldCollision* const	pCollision = GET_WBCOMP( pEntity, EldCollision );
	ASSERT( pCollision );

	pTransform->SetAcceleration( Vector() );
	pTransform->SetRotationalVelocity( Angles() );

	if( !IsLocomoting() )
	{
		return;
	}

	const Vector CurrentLocation = pTransform->GetLocation();

	const Vector RemainingOffset	= CurrentLocation - m_LastDestination;
	const float DistanceRemainingSq	= RemainingOffset.LengthSquared2D();
	const float DistanceRemainingZ	= Abs( RemainingOffset.z );
	if( DistanceRemainingSq <= m_ReachedThresholdMinSq && DistanceRemainingZ <= pCollision->GetExtents().z )
	{
		// We're within our reached threshold, we're done!
		m_MotionStatus			= EMS_TurningToFace;
		return;
	}

	Vector NextStep;
	if( !GetNextStep( NextStep ) )
	{
		// Path has run out, we're done!
		m_MotionStatus			= EMS_TurningToFace;
		return;
	}

	const float			AccelerationSize	= pCollision->IsRecentlyLanded( 0.0f ) ? GetLandAcceleration() : m_AirAcceleration;
	const Vector		MovementOffset		= NextStep - CurrentLocation;
	const Vector		MovementDirection	= MovementOffset.Get2D().GetNormalized();
	const Vector		Acceleration		= MovementDirection * AccelerationSize;

	static const Vector	kUp					= Vector( 0.0f, 0.0f, 1.0f );
	const Angles		CurrentOrientation	= pTransform->GetOrientation();
	const Vector		CurrentFacing		= CurrentOrientation.ToVector();
	const Vector		CurrentRight		= CurrentFacing.Cross( kUp );
	const bool			FacingMovement		= MovementDirection.Dot( CurrentFacing ) > 0.0f;
	const float			CosTurnAngle		= MovementDirection.Dot( CurrentRight );
	const float			TurnEpsilon			= FacingMovement ? m_TurnReachedThreshold : 0.0f;
	const bool			NeedsRightTurn		= CosTurnAngle >= TurnEpsilon;
	const bool			NeedsLeftTurn		= CosTurnAngle <= -TurnEpsilon;
	const float			TurnYaw				= NeedsRightTurn ? -1.0f : ( NeedsLeftTurn ? 1.0f : 0.0f );
	const Angles		TurnVelocity		= Angles( 0.0f, 0.0f, TurnYaw * m_TurnSpeed );

	pTransform->SetAcceleration( Acceleration );
	pTransform->SetRotationalVelocity( TurnVelocity );

	if( m_JumpOnNextTick )
	{
		m_JumpOnNextTick = false;

		WB_MAKE_EVENT( OnJumped, pEntity );
		WB_DISPATCH_EVENT( GetEventManager(), OnJumped, pEntity );

		pTransform->ApplyImpulse( Vector( 0.0f, 0.0f, m_JumpImpulse ) );

		PlayAnimation( m_JumpAnimationName, false, false, 1.0f );
	}
}

void WBCompEldAIMotion::TickFlying()
{
	WBEntity* const pEntity = GetEntity();
	DEVASSERT( pEntity );

	WBCompEldTransform* const	pTransform = pEntity->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	WBCompEldCollision* const	pCollision = GET_WBCOMP( pEntity, EldCollision );

	pTransform->SetAcceleration( Vector() );
	pTransform->SetRotationalVelocity( Angles() );

	if( m_Paused )
	{
		return;
	}

	const Vector CurrentLocation = pTransform->GetLocation();

	const Vector RemainingOffset	= CurrentLocation - m_LastDestination;
	const float DistanceRemainingSq	= RemainingOffset.LengthSquared2D();
	const float DistanceRemainingZ	= Abs( RemainingOffset.z );
	if( IsLocomoting() && DistanceRemainingSq <= m_ReachedThresholdMinSq && ( !pCollision || DistanceRemainingZ <= pCollision->GetExtents().z ) )
	{
		// We're within our reached threshold.
		// Signal that we're done, though we'll actually continue circling target.
		m_MotionStatus			= EMS_MoveSucceeded;
	}

	Vector NextStep = m_LastDestination;
	if( IsLocomoting() )
	{
		GetNextStep( NextStep );
	}

	Vector				CombinedAcceleration;

	// Compensate for current velocity
	static const float	kCompensateTime			= 0.5f;
	static const float	kCompensateReciprocal	= 1.0f / kCompensateTime;
	const Vector		CompensateAcceleration	= -pTransform->GetVelocity() * kCompensateReciprocal;
	CombinedAcceleration						+= CompensateAcceleration;

	// And accelerate toward destination
	const Vector		MovementOffset			= NextStep - CurrentLocation;
	const Vector		TargetDirection			= MovementOffset.GetNormalized();
	const Vector		PrimaryAcceleration		= TargetDirection * m_AirAcceleration;
	CombinedAcceleration						+= PrimaryAcceleration;

	// Figure out which way we need to turned
	static const Vector	kUp						= Vector( 0.0f, 0.0f, 1.0f );
	const Angles		CurrentOrientation		= pTransform->GetOrientation();
	const Vector		CurrentFacing			= CurrentOrientation.ToVector();
	const Vector		CurrentRight			= CurrentFacing.Cross( kUp );
	const bool			FacingTarget			= TargetDirection.Dot( CurrentFacing ) > 0.0f;
	const float			CosTurnAngle			= TargetDirection.Dot( CurrentRight );
	const float			TurnEpsilon				= FacingTarget ? m_TurnReachedThreshold : 0.0f;
	const bool			NeedsRightTurn			= CosTurnAngle >= TurnEpsilon;
	const bool			NeedsLeftTurn			= CosTurnAngle <= -TurnEpsilon;

	// Add deflection acceleration to circle the target, if desired
	if( m_MotionStatus == EMS_Following && DistanceRemainingSq < m_FlyingDeflectionRadiusSq )
	{
		const Vector	TargetRight				= TargetDirection.Cross( kUp );
		const bool		DeflectRight			= CosTurnAngle <= 0.0f;
		const Vector	DeflectionNormal		= DeflectRight ? TargetRight : -TargetRight;
		const Vector	DeflectionAcceleration	= DeflectionNormal * m_AirAcceleration;
		CombinedAcceleration					+= DeflectionAcceleration;
	}

	// Combine all accelerations
	Vector				CombinedNormal;
	float				CombinedSize;
	float				CombinedSizeReciprocal;
	CombinedAcceleration.GetNormalized( CombinedNormal, CombinedSize, CombinedSizeReciprocal );
	const float			AccelerationSize		= Min( CombinedSize, m_AirAcceleration );
	const Vector		Acceleration			= CombinedNormal * AccelerationSize;

	const float			TurnYaw					= NeedsRightTurn ? -1.0f : ( NeedsLeftTurn ? 1.0f : 0.0f );
	const Angles		TurnVelocity			= Angles( 0.0f, 0.0f, TurnYaw * m_TurnSpeed );

	pTransform->SetAcceleration( Acceleration );
	pTransform->SetRotationalVelocity( TurnVelocity );
}

float WBCompEldAIMotion::GetLandAcceleration() const
{
	WBCompStatMod* const pStatMod = GET_WBCOMP( GetEntity(), StatMod );

	WB_MODIFY_FLOAT_SAFE( LandAcceleration, m_LandAcceleration, pStatMod );
	return WB_MODDED( LandAcceleration );
}

float WBCompEldAIMotion::GetWalkAnimationPlayRate() const
{
	WBCompStatMod* const pStatMod = GET_WBCOMP( GetEntity(), StatMod );
	
	WB_MODIFY_FLOAT_SAFE( WalkAnimRate, 1.0f, pStatMod );
	return WB_MODDED( WalkAnimRate );
}

void WBCompEldAIMotion::TryJump()
{
	if( !IsLocomoting() )
	{
		return;
	}

	WBCompEldCollision* const pCollision = GET_WBCOMP( GetEntity(), EldCollision );
	ASSERT( pCollision );

	if( pCollision->IsRecentlyLanded( 0.0f ) )
	{
		m_JumpOnNextTick = true;
	}
}

void WBCompEldAIMotion::ValidateFollow()
{
	WBEntity* const pDestinationEntity = m_LastDestinationEntity();

	if( !pDestinationEntity )
	{
		// We no longer have a target to follow.
		m_MotionStatus			= EMS_MoveFailed;
		return;
	}

	WBCompEldHealth* const pHealth = GET_WBCOMP( pDestinationEntity, EldHealth );
	if( pHealth && pHealth->IsDead() )
	{
		// Target is a living creature that has died. Stop following.
		m_MotionStatus			= EMS_MoveFailed;
		return;
	}

	Vector LastKnownLocation;
	if( !GetLastKnownLocationFor( pDestinationEntity, LastKnownLocation ) )
	{
		// We no longer have a last known location for this target.
		m_MotionStatus			= EMS_MoveFailed;
		return;
	}

	const vidx_t VoxelIndex = GetWorld()->GetIndex( LastKnownLocation );
	if( VoxelIndex == m_LastDestinationIndex )
	{
		// We're still on track.
		return;
	}

	// Repath because the follow target has moved.
	RepathFollow();
}

void WBCompEldAIMotion::SetReachedThreshold( const float ReachedThresholdMin, const float ReachedThresholdMax )
{
	m_ReachedThresholdMinSq		= Square( ReachedThresholdMin );
	m_ReachedThresholdMaxSq		= Square( ReachedThresholdMax );
}

void WBCompEldAIMotion::SetDeflectionRadius( const float DeflectionRadius )
{
	m_FlyingDeflectionRadiusSq	= Square( DeflectionRadius );
}

void WBCompEldAIMotion::StartMove( const Vector& Destination )
{
	m_MotionStatus				= EMS_MoveFailed;

	InternalStartMove( Destination, NULL );
}

void WBCompEldAIMotion::StartTurn( const Vector& TurnTarget )
{
	m_MotionStatus			= EMS_TurningToFace;
	m_LastDestination		= TurnTarget;
	m_LastDestinationIndex	= GetWorld()->GetIndex( TurnTarget );
	m_LastDestinationEntity	= NULL;

	PlayAnimation( m_WalkAnimationName, true, true, GetWalkAnimationPlayRate() );
}

void WBCompEldAIMotion::StartFollow( const WBEntity* const pDestinationEntity )
{
	m_MotionStatus				= EMS_MoveFailed;
	m_Wander					= false;

	if( !pDestinationEntity )
	{
		return;
	}

	InternalStartMove( Vector(), pDestinationEntity );
}

void WBCompEldAIMotion::StartWander( const float WanderTargetDistance )
{
	m_MotionStatus				= EMS_MoveFailed;
	m_Wander					= true;
	m_WanderTargetDistance		= WanderTargetDistance;

	InternalStartMove( Vector(), NULL );
}

void WBCompEldAIMotion::RepathFollow()
{
	m_MotionStatus = EMS_MoveFailed;

	WBEntity* const pDestinationEntity = m_LastDestinationEntity();
	if( !pDestinationEntity )
	{
		return;
	}

	InternalStartMove( Vector(), pDestinationEntity );
}

// Simply repath with the same inputs as the previous request.
void WBCompEldAIMotion::Repath()
{
	m_MotionStatus = EMS_MoveFailed;

	InternalStartMove( m_LastDestination, m_LastDestinationEntity() );
}

bool WBCompEldAIMotion::GetLastKnownLocationFor( const WBEntity* const pFollowEntity, Vector& OutLocation ) const
{
	ASSERT( pFollowEntity );

	WBCompRodinKnowledge* const pKnowledge = GET_WBCOMP( GetEntity(), RodinKnowledge );
	if( pKnowledge )
	{
		return pKnowledge->GetLastKnownLocationFor( pFollowEntity, OutLocation );
	}
	else
	{
		// HACKHACK: For Yog, who doesn't need to actually acquire knowledge of the player.
		WBCompEldTransform* const pFollowTransform = pFollowEntity->GetTransformComponent<WBCompEldTransform>();
		ASSERT( pFollowTransform );

		OutLocation = pFollowTransform->GetLocation();
		return true;
	}
}

void WBCompEldAIMotion::InternalStartMove( const Vector& Destination, const WBEntity* const pDestinationEntity )
{
	// Just make sure we're calling this from a clean slate.
	ASSERT( m_MotionStatus == EMS_MoveFailed );

	EldritchWorld* const		pWorld				= GetWorld();
	ASSERT( pWorld );

	EldritchNav* const			pNav				= EldritchNav::GetInstance();
	ASSERT( pNav );

	WBEntity* const				pEntity				= GetEntity();
	DEVASSERT( pEntity );

	WBCompEldTransform* const	pTransform			= pEntity->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	WBCompEldCollision* const	pCollision			= GET_WBCOMP( pEntity, EldCollision );
	const bool					BypassPathing		= ( m_IsFlying && pCollision == NULL );
	ASSERT( pCollision || BypassPathing );

	const Vector				CurrentLocation		= pTransform->GetLocation();
	Vector						ActualDestination	= Destination;
	if( pDestinationEntity )
	{
		if( GetLastKnownLocationFor( pDestinationEntity, ActualDestination ) )
		{
			// We're good.
		}
		else
		{
			// No last known position for this entity.
			// Call this a success. It's not really, but it's not a pathing failure; it's more like
			// the request wasn't valid in the first place. This prevents stalling in BT loops that
			// wait for the move to be successful.
			m_MotionStatus = EMS_MoveSucceeded;
			return;
		}
	}

	if( !m_Wander )
	{
		// If we're already at our destination, exit early.
		const float DistanceRemainingSq = ( CurrentLocation - ActualDestination ).LengthSquared();
		if( DistanceRemainingSq <= m_ReachedThresholdMaxSq )
		{
			// We're within our reached threshold, we're done!
			m_MotionStatus			= EMS_TurningToFace;
			m_LastDestination		= ActualDestination;
			m_LastDestinationIndex	= pWorld->GetIndex( ActualDestination );
			m_LastDestinationEntity	= pDestinationEntity;
			return;
		}
	}

	const bool	IsFollowing	= ( pDestinationEntity != NULL );
	Vector		ActualStart	= CurrentLocation;

	if( !m_IsFlying )
	{
		// Get endpoints on ground.
		CollisionInfo Info;
		Info.m_CollideWorld		= true;
		Info.m_CollideEntities	= true;
		Info.m_CollidingEntity	= pEntity;
		Info.m_UserFlags		= EECF_EntityCollision;

		// This is sort of arbitrary. I'm solving a case where the destination is about 1.4m off the ground.
		static const float	kTraceDistance			= 2.0f;
		const Vector		Extents					= pCollision->GetExtents();

		// Sweep down to get actual start.
		const Segment		SweepStartSegment		= Segment( CurrentLocation, CurrentLocation + Vector( 0.0f, 0.0f, -kTraceDistance ) );
		pWorld->Sweep( SweepStartSegment, Extents, Info );
		if( Info.m_Collision )
		{
			ActualStart								= Info.m_Intersection;
			ActualStart.z							+= 0.5f - Extents.z;	// Start in the center of the voxel touching the ground; fixes tall AIs pathing funny
		}

		// Sweep down to get actual destination.
		// Don't assume extents actually fits at ActualDestination yet!
		const Segment		SweepDestinationSegment	= Segment( ActualDestination, ActualDestination + Vector( 0.0f, 0.0f, -kTraceDistance ) );
		pWorld->Trace( SweepDestinationSegment, Info );
		if( Info.m_Collision )
		{
			ActualDestination						= Info.m_Intersection;
			ActualDestination.z						+= + 0.5f;
		}
	}

	// Do pathfinding and smoothing
	if( BypassPathing )
	{
		m_Path.Clear();
		m_Path.PushBack( ActualStart );
		m_Path.PushBack( ActualDestination );
	}
	else
	{
		EldritchNav::SPathfindingParams Params;
		Params.m_Start				= ActualStart;
		if( m_Wander )
		{
			Params.m_TargetDistance	= m_WanderTargetDistance;
		}
		else
		{
			Params.m_Destination	= ActualDestination;
		}
		//Params.m_PursuingEntity	= IsFollowing;
		Params.m_Height				= static_cast<uint>( Ceiling( pCollision->GetExtents().z * 2.0f ) );
		Params.m_MotionType			= m_IsFlying ? EldritchNav::EMT_Flying : EldritchNav::EMT_Walking;
		Params.m_CanJump			= m_CanJump;
		Params.m_CanOpenDoors		= m_CanOpenDoors;
		Params.m_CanUnlockDoors		= m_CanUnlockDoors;
		Params.m_MaxSteps			= m_MaxPathSteps;
		Params.m_UsePartialPath		= true;

		const EldritchNav::EPathStatus Status = m_Wander ? pNav->Wander( Params, m_Path ) : pNav->FindPath( Params, m_Path );
		if( Status == EldritchNav::EPS_NoPathFound )
		{
			return;
		}

		// If we're already at our (path found) destination, exit early.
		const float DistanceRemainingSq = ( CurrentLocation - m_Path.Last() ).LengthSquared();
		if( DistanceRemainingSq <= m_ReachedThresholdMaxSq )
		{
			// We're within our reached threshold, we're done!
			m_MotionStatus			= EMS_TurningToFace;
			m_LastDestination		= ActualDestination;
			m_LastDestinationIndex	= pWorld->GetIndex( ActualDestination );
			m_LastDestinationEntity	= pDestinationEntity;
			return;
		}

		pNav->SmoothPath( pCollision->GetExtents(), m_Path );
	}

	// Cache the bound of our path so we can easily invalidate the path when the world changes.
	if( pCollision )
	{
		const uint PathSize = m_Path.Size();
		ASSERT( PathSize >= 2 );
		m_PathBound = AABB( m_Path[0], m_Path[1] );
		for( uint PathIndex = 2; PathIndex < PathSize; ++PathIndex )
		{
			m_PathBound.ExpandTo( m_Path[ PathIndex ] );
		}
		m_PathBound.ExpandBy( pCollision->GetExtents() );	// Expand by collision extents so we get all possible changes.
		m_PathBound.m_Min.z -= 1.0f;						// Also expand down by one unit so we detect changes to the floor.
	}

	// Set parameters and do side effects
	{
		m_MotionStatus				= IsFollowing ? EMS_Following : EMS_Moving;
		m_PathIndex					= 1;	// The first node is the start; no need to move to it.
		m_LastDestination			= m_Path.Last();
		m_LastDestinationIndex		= pWorld->GetIndex( m_LastDestination );
		m_LastDestinationEntity		= pDestinationEntity;
		m_NextFollowValidateTime	= GetTime() + m_FollowValidateTime;

		PlayAnimation( m_WalkAnimationName, true, true, GetWalkAnimationPlayRate() );
	}
}

void WBCompEldAIMotion::PlayAnimation( const HashedString& AnimationName, const bool Loop, const bool IgnoreIfAlreadyPlaying, const float PlayRate )
{
	if( m_Paused )
	{
		return;
	}

	WBCompRodinResourceMap* const pResourceMap = GET_WBCOMP( GetEntity(), RodinResourceMap );
	if( pResourceMap && pResourceMap->ClaimResource( this, m_AnimationResource, false ) )
	{
		WB_MAKE_EVENT( PlayAnim, GetEntity() );
		WB_SET_AUTO( PlayAnim, Hash, AnimationName, AnimationName );
		WB_SET_AUTO( PlayAnim, Bool, Loop, Loop );
		WB_SET_AUTO( PlayAnim, Bool, IgnoreIfAlreadyPlaying, IgnoreIfAlreadyPlaying );
		WB_SET_AUTO( PlayAnim, Float, PlayRate, PlayRate );
		WB_DISPATCH_EVENT( GetEventManager(), PlayAnim, GetEntity() );
	}
}

/*virtual*/ bool WBCompEldAIMotion::OnResourceStolen( const HashedString& Resource )
{
	Unused( Resource );
	ASSERT( Resource == m_AnimationResource );
	return true;
}

/*virtual*/ void WBCompEldAIMotion::OnResourceReturned( const HashedString& Resource )
{
	Unused( Resource );
	ASSERT( Resource == m_AnimationResource );

	if( IsLocomoting() )
	{
		PlayAnimation( m_WalkAnimationName, true, true, GetWalkAnimationPlayRate() );
	}
	else
	{
		PlayAnimation( m_IdleAnimationName, true, true, 1.0f );
	}
}

void WBCompEldAIMotion::StopMove()
{
	m_MotionStatus = EMS_Still;
	PlayAnimation( m_IdleAnimationName, true, true, 1.0f );
}

#if BUILD_DEV
/*virtual*/ void WBCompEldAIMotion::DebugRender() const
{
	if( !IsMoving() )
	{
		return;
	}

	IRenderer* const pRenderer = GetFramework()->GetRenderer();

	const uint NumPathSteps = m_Path.Size();
	for( uint PathIndex = 1; PathIndex < NumPathSteps; ++PathIndex )
	{
		const Vector& PrevStep = m_Path[ PathIndex - 1 ];
		const Vector& NextStep = m_Path[ PathIndex ];

		pRenderer->DEBUGDrawLine( PrevStep, NextStep, ARGB_TO_COLOR( 255, 0, 255, 255 ) );
	}

	if( m_PathIndex < NumPathSteps )
	{
		pRenderer->DEBUGDrawCross( m_Path[ m_PathIndex ], 0.5f, ARGB_TO_COLOR( 255, 255, 128, 0 ) );
	}
}
#endif

#define VERSION_EMPTY	0
#define VERSION_PAUSED	1
#define VERSION_CURRENT	1

uint WBCompEldAIMotion::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;					// Version
	Size += 1;					// m_Paused

	return Size;
}

void WBCompEldAIMotion::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );
	Stream.WriteBool( m_Paused );
}

void WBCompEldAIMotion::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_PAUSED )
	{
		m_Paused = Stream.ReadBool();
	}
}
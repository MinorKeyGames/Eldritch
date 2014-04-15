#include "core.h"
#include "wbcompeldplayer.h"
#include "keyboard.h"
#include "eldritchworld.h"
#include "eldritchframework.h"
#include "wbeventmanager.h"
#include "wbcompeldtransform.h"
#include "wbcompeldcollision.h"
#include "wbcompeldcamera.h"
#include "Components/wbcompstatmod.h"
#include "Components/wbcompelddamager.h"
#include "configmanager.h"
#include "idatastream.h"
#include "ray.h"
#include "collisioninfo.h"
#include "wbscene.h"
#include "inputsystem.h"
#include "configmanager.h"
#include "mathcore.h"
#include "eldritchgame.h"
#include "eldritchsaveload.h"
#include "reversehash.h"
#include "matrix.h"
#include "wbcompeldhealth.h"
#include "irenderer.h"
#include "Common/uimanagercommon.h"

#if BUILD_DEV
// TEMPHACK stuff
#if BUILD_WINDOWS
#include "console.h"
#endif
#include "wbcompeldhealth.h"
#include "wbcompeldinventory.h"
#include "wbcompeldrope.h"
#include "wbcompeldvisible.h"
#include "wbcompeldfootsteps.h"
#include "wbcompeldwallet.h"
#include "wbcompeldkeyring.h"
#include "wbcompeldhard.h"
#include "eldritchnav.h"
#endif

#define CAMERA_RELATIVE_CLIMBING		1
#define CAMERA_RELATIVE_CLIMBING_BIAS	1

WBCompEldPlayer::WBCompEldPlayer()
:	m_UnlandedJumpWindow( 0.0f )
,	m_UnlandedLeanWindow( 0.0f )
,	m_LandAcceleration( 0.0f )
,	m_AirAcceleration( 0.0f )
,	m_TurnSpeed( 0.0f )
,	m_JumpImpulse( 0.0f )
,	m_UncrouchOnSprint( false )
,	m_IsCrouched( false )
,	m_IsUncrouching( false )
,	m_StandExtentsZ( 0.0f )
,	m_CrouchExtentsZ( 0.0f )
,	m_StandViewOffsetZ( 0.0f )
,	m_CrouchViewOffsetZ( 0.0f )
,	m_CrouchViewInterpTime( 0.0f )
,	m_ViewOffsetZInterpolator()
,	m_ViewOffsetAngleRollInterpolator()
,	m_IsPowerSliding( false )
,	m_PowerSlideDuration( 0.0f )
,	m_PowerSlideEndTime( 0.0f )
,	m_PowerSlideY()
,	m_PowerSlideInputContext()
,	m_PowerSlideReqVelocitySq( 0.0f )
,	m_PowerSlideRoll( 0.0f )
,	m_PowerSlideRollInterpTime( 0.0f )
,	m_ClimbRefs( 0 )
,	m_Climbing( false )
,	m_ClimbInputContext()
,	m_UnclimbingGravity( 0.0f )
,	m_ClimbOffImpulse( 0.0f )
,	m_ClimbFacingBiasAngle( 0.0f )
,	m_ClimbFacingBiasScale( 0.0f )
,	m_IsMantling( false )
,	m_MantleInputContext()
,	m_MantleVelocity( 0.0f )
,	m_MantleVector()
,	m_MantleDestination()
,	m_CanMantle( false )
,	m_IsDisablingPause( false )
,	m_HasSetSpawnPoint( false )
,	m_SpawnLocation()
,	m_SpawnOrientation()
,	m_LastDamageDesc()
,	m_DefaultDamageDesc()
#if BUILD_DEV
,	m_TEMPHACKPath()
,	m_TEMPHACKCamActive( false )
,	m_TEMPHACKCamVelocity( 1.0f )
,	m_TEMPHACKCamGravity( 0.0f )
,	m_TEMPHACKCamLocation()
,	m_TEMPHACKCamOrientation()
,	m_TEMPHACKCamStartLocation()
,	m_TEMPHACKCamEndLocation()
,	m_TEMPHACKCamStartOrientation()
,	m_TEMPHACKCamEndOrientation()
#endif
{
	STATIC_HASHED_STRING( PreLevelTransition );
	GetEventManager()->AddObserver( sPreLevelTransition, this );

	STATIC_HASHED_STRING( PostLevelTransition );
	GetEventManager()->AddObserver( sPostLevelTransition, this );

	GetFramework()->GetInputSystem()->AddInputSystemObserver( this );
}

WBCompEldPlayer::~WBCompEldPlayer()
{
	WBEventManager* const pEventManager = GetEventManager();
	if( pEventManager )
	{
		STATIC_HASHED_STRING( PreLevelTransition );
		pEventManager->RemoveObserver( sPreLevelTransition, this );

		STATIC_HASHED_STRING( PostLevelTransition );
		pEventManager->RemoveObserver( sPostLevelTransition, this );
	}

	InputSystem* const pInputSystem = GetFramework()->GetInputSystem();
	if( pInputSystem )
	{
		pInputSystem->RemoveInputSystemObserver( this );
	}
}

/*virtual*/ void WBCompEldPlayer::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( UnlandedJumpWindow );
	m_UnlandedJumpWindow = ConfigManager::GetInheritedFloat( sUnlandedJumpWindow, 0.0f, sDefinitionName );

	STATICHASH( UnlandedLeanWindow );
	m_UnlandedLeanWindow = ConfigManager::GetInheritedFloat( sUnlandedLeanWindow, 0.0f, sDefinitionName );

	STATICHASH( LandAcceleration );
	m_LandAcceleration = ConfigManager::GetInheritedFloat( sLandAcceleration, 0.0f, sDefinitionName );

	STATICHASH( AirAcceleration );
	m_AirAcceleration = ConfigManager::GetInheritedFloat( sAirAcceleration, 0.0f, sDefinitionName );

	STATICHASH( TurnSpeed );
	m_TurnSpeed = ConfigManager::GetInheritedFloat( sTurnSpeed, 0.0f, sDefinitionName );

	STATICHASH( JumpImpulse );
	m_JumpImpulse = ConfigManager::GetInheritedFloat( sJumpImpulse, 0.0f, sDefinitionName );

	STATICHASH( UncrouchOnSprint );
	m_UncrouchOnSprint = ConfigManager::GetInheritedBool( sUncrouchOnSprint, false, sDefinitionName );

	STATICHASH( StandExtentsZ );
	m_StandExtentsZ = ConfigManager::GetInheritedFloat( sStandExtentsZ, 0.0f, sDefinitionName );

	STATICHASH( CrouchExtentsZ );
	m_CrouchExtentsZ = ConfigManager::GetInheritedFloat( sCrouchExtentsZ, 0.0f, sDefinitionName );

	STATICHASH( StandViewOffsetZ );
	m_StandViewOffsetZ = ConfigManager::GetInheritedFloat( sStandViewOffsetZ, 0.0f, sDefinitionName );
	m_ViewOffsetZInterpolator.Reset( Interpolator<float>::EIT_Linear, m_StandViewOffsetZ, m_StandViewOffsetZ, 0.0f );

	STATICHASH( CrouchViewOffsetZ );
	m_CrouchViewOffsetZ = ConfigManager::GetInheritedFloat( sCrouchViewOffsetZ, 0.0f, sDefinitionName );

	STATICHASH( CrouchViewInterpTime );
	m_CrouchViewInterpTime = ConfigManager::GetInheritedFloat( sCrouchViewInterpTime, 0.0f, sDefinitionName );

	STATICHASH( PowerSlideDuration );
	m_PowerSlideDuration = ConfigManager::GetInheritedFloat( sPowerSlideDuration, 0.0f, sDefinitionName );

	STATICHASH( PowerSlideInputContext );
	m_PowerSlideInputContext = ConfigManager::GetInheritedHash( sPowerSlideInputContext, HashedString::NullString, sDefinitionName );

	STATICHASH( PowerSlideReqVelocity );
	m_PowerSlideReqVelocitySq = Square( ConfigManager::GetInheritedFloat( sPowerSlideReqVelocity, 0.0f, sDefinitionName ) );

	STATICHASH( PowerSlideRoll );
	m_PowerSlideRoll = DEGREES_TO_RADIANS( ConfigManager::GetInheritedFloat( sPowerSlideRoll, 0.0f, sDefinitionName ) );

	STATICHASH( PowerSlideRollInterpTime );
	m_PowerSlideRollInterpTime = ConfigManager::GetInheritedFloat( sPowerSlideRollInterpTime, 0.0f, sDefinitionName );

	STATICHASH( ClimbInputContext );
	m_ClimbInputContext = ConfigManager::GetInheritedHash( sClimbInputContext, HashedString::NullString, sDefinitionName );

	STATICHASH( ClimbOffImpulse );
	m_ClimbOffImpulse = ConfigManager::GetInheritedFloat( sClimbOffImpulse, 0.0f, sDefinitionName );

	STATICHASH( ClimbFacingBiasAngle );
	m_ClimbFacingBiasAngle = DEGREES_TO_RADIANS( ConfigManager::GetInheritedFloat( sClimbFacingBiasAngle, 0.0f, sDefinitionName ) );

	STATICHASH( ClimbFacingBiasScale );
	m_ClimbFacingBiasScale = ConfigManager::GetInheritedFloat( sClimbFacingBiasScale, 0.0f, sDefinitionName );

	STATICHASH( MantleInputContext );
	m_MantleInputContext = ConfigManager::GetInheritedHash( sMantleInputContext, HashedString::NullString, sDefinitionName );

	STATICHASH( MantleVelocity );
	m_MantleVelocity = ConfigManager::GetInheritedFloat( sMantleVelocity, 0.0f, sDefinitionName );

	STATICHASH( DefaultDamageDesc );
	m_DefaultDamageDesc = ConfigManager::GetInheritedString( sDefaultDamageDesc, "", sDefinitionName );

	// This isn't really the player component's domain, it's just a convenient place to validate this.
	ASSERT( GetFramework()->GetInputSystem()->GetNumActiveContexts() == 0 );
}

/*virtual*/ void WBCompEldPlayer::Tick( float DeltaTime )
{
	XTRACE_FUNCTION;

	WBEntity* const				pEntity		= GetEntity();
	WBCompEldCamera* const		pCamera		= GET_WBCOMP( pEntity, EldCamera );
	WBCompEldHealth* const		pHealth		= GET_WBCOMP( pEntity, EldHealth );

#if BUILD_DEV
	if( m_TEMPHACKCamActive )
	{
		m_TEMPHACKCamLocation.Tick( DeltaTime );
		m_TEMPHACKCamOrientation.Tick( DeltaTime );

		WBCompEldTransform* const	pTransform	= pEntity->GetTransformComponent<WBCompEldTransform>();
		WBCompEldCollision* const	pCollision	= GET_WBCOMP( pEntity, EldCollision );

		pTransform->SetLocation( m_TEMPHACKCamLocation.GetValue() );
		pTransform->SetOrientation( m_TEMPHACKCamOrientation.GetValue() );

		if( m_TEMPHACKCamLocation.GetT() == 1.0f )
		{
			m_TEMPHACKCamActive = false;

			pCollision->SetCollisionFlags( pCollision->GetDefaultCollisionFlags() );
			pTransform->SetGravity( m_TEMPHACKCamGravity );

			WB_MAKE_EVENT( ShowHands, GetEntity() );
			WB_DISPATCH_EVENT( GetEventManager(), ShowHands, GetEntity() );
		}
	}
#endif	// BUILD_DEV

	if( pHealth->IsAlive() )
	{
		m_ViewOffsetZInterpolator.Tick( DeltaTime );
		pCamera->SetViewOffsetZ( m_ViewOffsetZInterpolator.GetValue() );

		m_ViewOffsetAngleRollInterpolator.Tick( DeltaTime );
		pCamera->SetViewAngleOffsetRoll( m_ViewOffsetAngleRollInterpolator.GetValue() );
	}

	if( m_IsUncrouching )
	{
		TryUncrouch();
	}

	if( m_IsPowerSliding )
	{
		if( GetTime() >= m_PowerSlideEndTime )
		{
			EndPowerSlide();
		}
	}

	// Everything before this point is independent of current input and must be done every tick.
	// Everything after this point is dependent on input and should be done only when we have focus.

	if( !GetFramework()->HasFocus() )
	{
		return;
	}

	InputSystem* const pInputSystem = GetFramework()->GetInputSystem();

	WBCompEldTransform* const	pTransform	= pEntity->GetTransformComponent<WBCompEldTransform>();
	WBCompEldCollision* const	pCollision	= GET_WBCOMP( pEntity, EldCollision );
	WBCompStatMod* const		pStatMod	= GET_WBCOMP( pEntity, StatMod );

	DEVASSERT( pTransform );
	ASSERT( pCollision );
	ASSERT( pCamera );
	ASSERT( pStatMod );

	// Get 2D orientation; we don't want to move as if we're flying.
	Angles PlayerOrientation;
	Vector X, Y, Z;
	const Vector UpVector		= Vector( 0.0f, 0.0f, 1.0f );
	const Vector PlayerFacing	= pTransform->GetOrientation().ToVector();
	const Vector PlayerRight	= PlayerFacing.Cross( UpVector );
	PlayerOrientation.Yaw		= pTransform->GetOrientation().Yaw;
	PlayerOrientation.GetAxes( X, Y, Z );

	Vector MovementVector;
	Angles TurnAngles;
	Vector ImpulseVector;

	STATIC_HASHED_STRING( Right );
	STATIC_HASHED_STRING( Left );
	STATIC_HASHED_STRING( Forward );
	STATIC_HASHED_STRING( Back );
	STATIC_HASHED_STRING( MoveX );
	STATIC_HASHED_STRING( MoveY );
	STATIC_HASHED_STRING( Jump );
	STATIC_HASHED_STRING( UseWeapon );
	STATIC_HASHED_STRING( UsePower );
	STATIC_HASHED_STRING( DropWeapon );
	STATIC_HASHED_STRING( CycleUp );
	STATIC_HASHED_STRING( CycleDown );
	STATIC_HASHED_STRING( Frob );
	STATIC_HASHED_STRING( LeanLeft );
	STATIC_HASHED_STRING( LeanRight );
	STATIC_HASHED_STRING( TurnX );
	STATIC_HASHED_STRING( TurnY );
	STATIC_HASHED_STRING( Run );
	STATIC_HASHED_STRING( Crouch );
	STATIC_HASHED_STRING( ClimbForward );
	STATIC_HASHED_STRING( ClimbBack );
	STATIC_HASHED_STRING( ClimbDown );
	STATIC_HASHED_STRING( ClimbY );
	STATIC_HASHED_STRING( ClimbJump );
	STATIC_HASHED_STRING( Mantle );

	if( pInputSystem->IsHigh( sRight ) )	{ MovementVector += X; }
	if( pInputSystem->IsHigh( sLeft ) )		{ MovementVector -= X; }
	if( pInputSystem->IsHigh( sForward ) )	{ MovementVector += Y; }
	if( pInputSystem->IsHigh( sBack ) )		{ MovementVector -= Y; }

	MovementVector += X * pInputSystem->GetPosition( sMoveX );
	MovementVector += Y * pInputSystem->GetPosition( sMoveY );

	// If we're crouched and we start sprinting, even if we don't have any movement input, then uncrouch.
	// (Surprisingly, this feels more correct than requiring movement input. Feels weird to be crouched,
	// press shift, start moving, and not be sprinting.)
	if( m_UncrouchOnSprint && m_IsCrouched && pInputSystem->OnRise( sRun ) )
	{
		BeginUncrouch();
	}

	if( m_IsPowerSliding )
	{
		MovementVector += m_PowerSlideY;
	}

	float LeanTarget = 0.0f;
	const bool CanLean = pCollision->IsRecentlyLanded( m_UnlandedLeanWindow );
	if( CanLean )
	{
		if( pInputSystem->IsHigh( sLeanLeft ) )		{ LeanTarget -= 1.0f; }
		if( pInputSystem->IsHigh( sLeanRight ) )	{ LeanTarget += 1.0f; }
	}

	const int UseWeaponInput = pInputSystem->OnEdge( sUseWeapon );
	if( UseWeaponInput )
	{
		WB_MAKE_EVENT( UseRightHand, pEntity );
		WB_SET_AUTO( UseRightHand, Int, InputEdge, UseWeaponInput );
		WB_DISPATCH_EVENT( GetEventManager(), UseRightHand, pEntity );
	}

	const int UsePowerInput = pInputSystem->OnEdge( sUsePower );
	if( UsePowerInput )
	{
		WB_MAKE_EVENT( UseLeftHand, pEntity );
		WB_SET_AUTO( UseLeftHand, Int, InputEdge, UsePowerInput );
		WB_DISPATCH_EVENT( GetEventManager(), UseLeftHand, pEntity );
	}

	if( pInputSystem->OnRise( sDropWeapon ) )
	{
		STATIC_HASHED_STRING( Weapon );
		WB_MAKE_EVENT( DropItem, pEntity );
		WB_SET_AUTO( DropItem, Hash, Slot, sWeapon );
		WB_DISPATCH_EVENT( GetEventManager(), DropItem, pEntity );
	}

	if( pInputSystem->OnRise( sCycleUp ) || pInputSystem->OnRise( sCycleDown ) )
	{
		STATIC_HASHED_STRING( Weapon );
		STATIC_HASHED_STRING( WeaponAlt );
		WB_MAKE_EVENT( SwapItems, pEntity );
		WB_SET_AUTO( SwapItems, Hash, SlotA, sWeapon );
		WB_SET_AUTO( SwapItems, Hash, SlotB, sWeaponAlt );
		WB_DISPATCH_EVENT( GetEventManager(), SwapItems, pEntity );
	}

	const int FrobInput = pInputSystem->OnEdge( sFrob );
	if( FrobInput )
	{
		WB_MAKE_EVENT( OnFrob, pEntity );
		WB_SET_AUTO( OnFrob, Int, InputEdge, FrobInput );
		WB_DISPATCH_EVENT( GetEventManager(), OnFrob, pEntity );
	}

	if( pInputSystem->OnRise( sJump ) && pCollision->IsRecentlyLanded( m_UnlandedJumpWindow ) )
	{
		WB_MAKE_EVENT( OnJumped, pEntity );
		WB_DISPATCH_EVENT( GetEventManager(), OnJumped, pEntity );

		ImpulseVector.z += 1.0f;

		if( m_IsPowerSliding )
		{
			EndPowerSlide();
		}
	}

	if( m_IsMantling && pInputSystem->IsLow( sMantle ) )
	{
		EndMantle();
	}

	if( pInputSystem->OnRise( sCrouch ) )
	{
		if( m_IsUncrouching )
		{
			CancelUncrouch();
		}
		else if( m_IsCrouched )
		{
			BeginUncrouch();
		}
		else
		{
			Crouch();

			if( pInputSystem->IsHigh( sRun ) &&
				pTransform->GetVelocity().LengthSquared2D() > m_PowerSlideReqVelocitySq &&
				( pInputSystem->IsHigh( sForward ) || pInputSystem->GetPosition( sMoveY ) > 0.0f ) )
			{
				BeginPowerSlide( Y );
			}
		}
	}

	const bool	IsClimbForward	= pInputSystem->IsHigh( sClimbForward );
	const bool	IsClimbBack		= pInputSystem->IsHigh( sClimbBack );
	const bool	IsClimbDown		= pInputSystem->IsHigh( sClimbDown );
	const float	ClimbY			= pInputSystem->GetPosition( sClimbY );
	const bool	IsClimbY		= Abs( ClimbY ) > EPSILON;

#if CAMERA_RELATIVE_CLIMBING
	// Camera-relative climbing motion (how I prefer).
	if( IsClimbForward || IsClimbBack || IsClimbDown || IsClimbY )
	{
#if CAMERA_RELATIVE_CLIMBING_BIAS
		const Matrix		ClimbFacingMatrix		= Matrix::CreateRotation( PlayerRight, m_ClimbFacingBiasAngle );
		const Vector		ClimbFacing				= PlayerFacing * ClimbFacingMatrix;
		const float			ClimbMagnitude			= ClimbFacing.z;
		const float			ScaledClimbMagnitude	= Clamp( ClimbMagnitude * m_ClimbFacingBiasScale, -1.0f, 1.0f );
		const Vector		ClimbVector				= UpVector * ScaledClimbMagnitude;
#else
		const Vector		ClimbVector				= PlayerFacing.ProjectionOnto( UpVector );
#endif
		ASSERT( ClimbVector.x == 0.0f && ClimbVector.y == 0.0f );

		if( IsClimbForward )	{ MovementVector += ClimbVector; }
		if( IsClimbBack )		{ MovementVector -= ClimbVector; }
		if( IsClimbDown )		{ MovementVector -= UpVector; }
		if( IsClimbY )			{ MovementVector += ClimbVector * ClimbY; }
	}
#else
	if( IsClimbForward || IsClimbBack || IsClimbDown || IsClimbY )
	{
		if( IsClimbForward )	{ MovementVector += UpVector; }
		if( IsClimbBack )		{ MovementVector -= UpVector; }
		if( IsClimbDown )		{ MovementVector -= UpVector; }
		if( IsClimbY )			{ MovementVector += UpVector * ClimbY; }
	}
#endif

	if( pInputSystem->OnRise( sClimbJump ) &&
		!pInputSystem->OnFall( sJump ) )		// This prevents jumping off when we've just switched to the climb context
	{
		ZeroClimbRefs();

		WB_MAKE_EVENT( OnJumped, pEntity );
		WB_DISPATCH_EVENT( GetEventManager(), OnJumped, pEntity );

		ImpulseVector.z += 1.0f;
		MovementVector = Vector();

		if( IsClimbForward )	{ ImpulseVector += Y; }
		if( IsClimbBack )		{ ImpulseVector -= Y; }
		if( IsClimbY )			{ ImpulseVector += Y * ClimbY; }
	}

	ConditionalApplyRunningStatMods();

	TurnAngles.Yaw -= pInputSystem->GetPosition( sTurnX );
	TurnAngles.Pitch -= pInputSystem->GetPosition( sTurnY );

	float MoveSpeed = 0.0f;
	if( m_Climbing || pCollision->IsLanded() )
	{
		WB_MODIFY_FLOAT( LandAcceleration, m_LandAcceleration, pStatMod );
		MoveSpeed = WB_MODDED( LandAcceleration );
	}
	else
	{
		MoveSpeed = m_AirAcceleration;
	}

	if( m_Climbing )
	{
		const float Mu = pCollision->GetFrictionCoefficient();
		pTransform->SetVelocity( pTransform->GetVelocity() * Mu );
	}

	// Don't let movement input exceed 1.0 (run + strafe doesn't double speed!),
	// but do allow it to be less than 1.0 for camera-relative climbing or analog input.
	Vector MovementVectorDirection;
	float MovementVectorLength;
	float MovementVectorLengthOverOne;
	MovementVector.GetNormalized( MovementVectorDirection, MovementVectorLength, MovementVectorLengthOverOne );
	const float AppliedMovementLength = Min( MovementVectorLength, 1.0f );

	MovementVector	= MovementVectorDirection * AppliedMovementLength * MoveSpeed;
	TurnAngles		*= m_TurnSpeed;

	WB_MODIFY_FLOAT( JumpImpulse, m_JumpImpulse, pStatMod );
	ImpulseVector	= ImpulseVector.GetFastNormalized() * WB_MODDED( JumpImpulse );

	if( m_IsMantling )
	{
		// Check if we're past the destination and end the mantle if so
		const Vector ToDestination = ( m_MantleDestination - pTransform->GetLocation() );
		if( ToDestination.Dot( m_MantleVector ) < 0.0f )
		{
			EndMantle();
		}
		else
		{
			const Vector MantleVelocity = m_MantleVector * m_MantleVelocity;
			pTransform->SetVelocity( MantleVelocity );
		}
	}

	pTransform->SetAcceleration( MovementVector );
	pTransform->ApplyImpulse( ImpulseVector );
	pTransform->SetRotationalVelocity( TurnAngles );

	pCamera->SetLeanPosition( LeanTarget );

#if BUILD_DEV
	TEMPHACKInput();
#endif
}

void WBCompEldPlayer::UseJumpPower( const float JumpPowerImpulse )
{
	WBEntity* const				pEntity		= GetEntity();
	WBCompEldTransform* const	pTransform	= pEntity->GetTransformComponent<WBCompEldTransform>();

	// Make sure we handle this like a normal jump.

	WB_MAKE_EVENT( OnJumped, pEntity );
	WB_DISPATCH_EVENT( GetEventManager(), OnJumped, pEntity );

	if( m_IsPowerSliding )
	{
		EndPowerSlide();
	}

	// Use max(j, v.z+j) so that jump power is effective even when moving very fast downward.
	Vector NewVelocity = pTransform->GetVelocity();
	NewVelocity.z = Max( JumpPowerImpulse, NewVelocity.z + JumpPowerImpulse );
	pTransform->SetVelocity( NewVelocity );
}

void WBCompEldPlayer::SetCrouchOverlayHidden( const bool Hidden )
{
	STATIC_HASHED_STRING( HUD );
	STATIC_HASHED_STRING( CrouchOverlay );

	WB_MAKE_EVENT( SetWidgetHidden, GetEntity() );
	WB_SET_AUTO( SetWidgetHidden, Hash, Screen, sHUD );
	WB_SET_AUTO( SetWidgetHidden, Hash, Widget, sCrouchOverlay );
	WB_SET_AUTO( SetWidgetHidden, Bool, Hidden, Hidden );
	WB_DISPATCH_EVENT( GetEventManager(), SetWidgetHidden, GetFramework()->GetUIManager() );
}

void WBCompEldPlayer::Crouch()
{
	ASSERT( !m_IsCrouched );
	ASSERT( !m_IsUncrouching );

	m_IsCrouched = true;

	WBEntity* const				pEntity		= GetEntity();
	DEVASSERT( pEntity );

	WBCompEldCollision* const	pCollision	= GET_WBCOMP( pEntity, EldCollision );
	ASSERT( pCollision );

	WBCompEldCamera* const		pCamera		= GET_WBCOMP( pEntity, EldCamera );
	ASSERT( pCamera );

	WBCompStatMod* const		pStatMod	= GET_WBCOMP( pEntity, StatMod );
	ASSERT( pStatMod );

	Vector Extents = pCollision->GetExtents();
	ASSERT( m_CrouchExtentsZ < Extents.z );
	Extents.z = m_CrouchExtentsZ;
	pCollision->SetExtents( Extents );

	m_ViewOffsetZInterpolator.Reset( Interpolator<float>::EIT_EaseOut, m_StandViewOffsetZ, m_CrouchViewOffsetZ, m_CrouchViewInterpTime );

	SetCrouchOverlayHidden( false );

	STATIC_HASHED_STRING( Crouching );
	pStatMod->TriggerEvent( sCrouching );
}

void WBCompEldPlayer::BeginUncrouch()
{
	ASSERT( m_IsCrouched );

	m_IsUncrouching = true;

	if( m_IsPowerSliding )
	{
		EndPowerSlide();
	}
}

void WBCompEldPlayer::CancelUncrouch()
{
	ASSERT( m_IsCrouched );
	ASSERT( m_IsUncrouching );

	m_IsUncrouching = false;
}

void WBCompEldPlayer::TryUncrouch()
{
	ASSERT( m_IsCrouched );
	ASSERT( m_IsUncrouching );

	if( CanUncrouch() )
	{
		Uncrouch();
	}
}

bool WBCompEldPlayer::CanUncrouch()
{
	ASSERT( m_IsCrouched );

	WBEntity* const				pEntity		= GetEntity();
	DEVASSERT( pEntity );

	WBCompEldTransform* const	pTransform	= pEntity->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	WBCompEldCollision* const	pCollision	= GET_WBCOMP( pEntity, EldCollision );
	ASSERT( pCollision );

	EldritchWorld* const pWorld = GetWorld();
	ASSERT( pWorld );

	Vector StandLocation = pTransform->GetLocation();
	StandLocation.z = StandLocation.z + m_StandExtentsZ - m_CrouchExtentsZ;

	Vector CheckExtents = pCollision->GetExtents();
	CheckExtents.z = m_StandExtentsZ;

	CollisionInfo Info;
	Info.m_CollideWorld		= true;
	Info.m_CollideEntities	= true;
	Info.m_CollidingEntity	= GetEntity();
	Info.m_UserFlags		= EECF_EntityCollision;

	if( pWorld->CheckClearance( StandLocation, CheckExtents, Info ) )
	{
		// Something is blocking the uncrouch
		return false;
	}
	else
	{
		return true;
	}
}

void WBCompEldPlayer::Uncrouch()
{
	ASSERT( m_IsCrouched );
	ASSERT( m_IsUncrouching );

	DEBUGASSERT( CanUncrouch() );

	m_IsUncrouching = false;
	m_IsCrouched = false;

	WBEntity* const				pEntity		= GetEntity();
	DEVASSERT( pEntity );

	WBCompEldTransform* const	pTransform	= pEntity->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	WBCompEldCollision* const	pCollision	= GET_WBCOMP( pEntity, EldCollision );
	ASSERT( pCollision );

	WBCompStatMod* const		pStatMod	= GET_WBCOMP( pEntity, StatMod );
	ASSERT( pStatMod );

	Vector StandLocation = pTransform->GetLocation();
	StandLocation.z = StandLocation.z + m_StandExtentsZ - m_CrouchExtentsZ;
	pTransform->SetLocation( StandLocation );

	Vector Extents = pCollision->GetExtents();
	ASSERT( m_StandExtentsZ > Extents.z );
	Extents.z = m_StandExtentsZ;
	pCollision->SetExtents( Extents );

	m_ViewOffsetZInterpolator.Reset( Interpolator<float>::EIT_EaseOut, m_CrouchViewOffsetZ, m_StandViewOffsetZ, m_CrouchViewInterpTime );

	SetCrouchOverlayHidden( true );

	STATIC_HASHED_STRING( Crouching );
	pStatMod->UnTriggerEvent( sCrouching );
}

void WBCompEldPlayer::RestoreCrouch()
{
	// Fix up view offset and stat mod from state.
	// Other crouch properties (collision extents, transform location) are serialized.

	WBEntity* const			pEntity		= GetEntity();
	DEVASSERT( pEntity );

	WBCompEldCamera* const	pCamera		= GET_WBCOMP( pEntity, EldCamera );
	ASSERT( pCamera );

	WBCompStatMod* const	pStatMod	= GET_WBCOMP( pEntity, StatMod );
	ASSERT( pStatMod );

	const float ViewOffsetZ = m_IsCrouched ? m_CrouchViewOffsetZ : m_StandViewOffsetZ;
	m_ViewOffsetZInterpolator.Reset( Interpolator<float>::EIT_Linear, ViewOffsetZ, ViewOffsetZ, 0.0f );
	pCamera->SetViewOffsetZ( ViewOffsetZ );

	SetCrouchOverlayHidden( !m_IsCrouched );

	STATIC_HASHED_STRING( Crouching );
	pStatMod->SetEventActive( sCrouching, m_IsCrouched );
}

void WBCompEldPlayer::BeginPowerSlide( const Vector& PowerSlideY )
{
	ASSERT( !m_IsPowerSliding );

	m_IsPowerSliding	= true;
	m_PowerSlideEndTime	= GetTime() + m_PowerSlideDuration;
	m_PowerSlideY		= PowerSlideY;

	WBEntity* const				pEntity		= GetEntity();
	DEVASSERT( pEntity );

	WBCompStatMod* const		pStatMod	= GET_WBCOMP( pEntity, StatMod );
	ASSERT( pStatMod );

	m_ViewOffsetAngleRollInterpolator.Reset( Interpolator<float>::EIT_Linear, 0.0f, m_PowerSlideRoll, m_PowerSlideRollInterpTime );

	STATIC_HASHED_STRING( PowerSliding );
	pStatMod->TriggerEvent( sPowerSliding );

	GetFramework()->GetInputSystem()->PushContext( m_PowerSlideInputContext );
}

void WBCompEldPlayer::EndPowerSlide()
{
	ASSERT( m_IsPowerSliding );

	m_IsPowerSliding = false;

	WBEntity* const				pEntity		= GetEntity();
	DEVASSERT( pEntity );

	WBCompStatMod* const		pStatMod	= GET_WBCOMP( pEntity, StatMod );
	ASSERT( pStatMod );

	m_ViewOffsetAngleRollInterpolator.Reset( Interpolator<float>::EIT_Linear, m_PowerSlideRoll, 0.0f, m_PowerSlideRollInterpTime );

	STATIC_HASHED_STRING( PowerSliding );
	pStatMod->UnTriggerEvent( sPowerSliding );

	GetFramework()->GetInputSystem()->PopContext( m_PowerSlideInputContext );
}

void WBCompEldPlayer::RestorePowerSlide()
{
	WBEntity* const			pEntity		= GetEntity();
	DEVASSERT( pEntity );

	WBCompStatMod* const	pStatMod	= GET_WBCOMP( pEntity, StatMod );
	ASSERT( pStatMod );

	const float ViewAngleOffsetRoll = m_IsPowerSliding ? m_PowerSlideRoll : 0.0f;
	m_ViewOffsetAngleRollInterpolator.Reset( Interpolator<float>::EIT_Linear, ViewAngleOffsetRoll, ViewAngleOffsetRoll, 0.0f );

	STATIC_HASHED_STRING( PowerSliding );
	pStatMod->SetEventActive( sPowerSliding, m_IsPowerSliding );

	if( m_IsPowerSliding )
	{
		GetFramework()->GetInputSystem()->PushContext( m_PowerSlideInputContext );
	}
}

bool WBCompEldPlayer::ShouldAttachToClimbable( const WBEvent& ClimbEvent )
{
	if( m_ClimbRefs > 0 )
	{
		// If we're already climbing, always attach to any further climbables.
		return true;
	}

	STATIC_HASHED_STRING( UseSnapPlane );
	const bool			UseSnapPlane				= ClimbEvent.GetBool( sUseSnapPlane );

	if( !UseSnapPlane )
	{
		// We have no snap plane, always attach.
		return true;
	}

	WBEntity* const				pEntity			= GetEntity();

	WBCompEldCollision* const	pCollision		= GET_WBCOMP( pEntity, EldCollision );
	ASSERT( pCollision );

	if( !pCollision->IsLanded() )
	{
		// Always attach if jumping or falling.
		return true;
	}

	WBCompEldTransform* const	pTransform		= pEntity->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	const Vector	Orientation					= pTransform->GetOrientation().ToVector();
	const Vector	VelocityNormal				= pTransform->GetVelocity().GetFastNormalized();

	STATIC_HASHED_STRING( SnapPlaneNormal );
	const Vector	SnapPlaneNormal				= ClimbEvent.GetVector( sSnapPlaneNormal );

	// Cheap 90 degree rotation to get facing plane from snap plane
	ASSERT( SnapPlaneNormal.z == 0.0f );
	const Vector	FacingPlaneNormal			= Vector( -SnapPlaneNormal.y, SnapPlaneNormal.x, 0.0f );

	// TODO: Configurate if needed.
	static const float	kCos90	= 0.0f;	// Climbing angle
	const float			FacingDot = Orientation.Dot( FacingPlaneNormal );
	const float			MovingDot = VelocityNormal.Dot( FacingPlaneNormal );
	if( FacingDot < kCos90 &&
		MovingDot < kCos90 )
	{
		// We're facing the snap plane and moving toward the snap plane, so we should attach.
		return true;
	}

	// All cases failed, don't attach.
	return false;
}

void WBCompEldPlayer::IncrementClimbRefs( const WBEvent& ClimbEvent )
{
	if( ++m_ClimbRefs == 1 )
	{
		if( !m_IsMantling )
		{
			BeginClimb( ClimbEvent );
		}
	}
}

void WBCompEldPlayer::DecrementClimbRefs( const bool AddClimbOffImpulse )
{
	if( m_ClimbRefs > 0 )
	{
		if( --m_ClimbRefs == 0 )
		{
			if( m_Climbing )
			{
				EndClimb( AddClimbOffImpulse );
			}
		}
	}
}

void WBCompEldPlayer::ZeroClimbRefs()
{
	if( m_ClimbRefs > 0 )
	{
		m_ClimbRefs = 0;

		if( m_Climbing )
		{
			EndClimb( false );
		}
	}
}

void WBCompEldPlayer::BeginClimb( const WBEvent& ClimbEvent )
{
	if( m_IsPowerSliding )
	{
		EndPowerSlide();
	}

	if( m_IsCrouched )
	{
		BeginUncrouch();
	}

	ASSERT( !m_Climbing );

	m_Climbing = true;

	WBEntity* const				pEntity			= GetEntity();
	DEVASSERT( pEntity );

	WBCompEldTransform* const	pTransform		= pEntity->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	WBCompStatMod* const		pStatMod		= GET_WBCOMP( pEntity, StatMod );
	ASSERT( pStatMod );

	STATIC_HASHED_STRING( UseSnapPlane );
	const bool					UseSnapPlane	= ClimbEvent.GetBool( sUseSnapPlane );

	if( UseSnapPlane )
	{
		STATIC_HASHED_STRING( SnapPlaneDistance );
		const float		SnapPlaneDistance	= ClimbEvent.GetFloat( sSnapPlaneDistance );

		STATIC_HASHED_STRING( SnapPlaneNormal );
		const Vector	SnapPlaneNormal		= ClimbEvent.GetVector( sSnapPlaneNormal );

		const Plane		SnapPlane			= Plane( SnapPlaneNormal, SnapPlaneDistance );
		const Vector	Location			= pTransform->GetLocation();
		const Vector	SnappedLocation		= SnapPlane.ProjectPoint( Location );

		pTransform->SetLocation( SnappedLocation );
	}

	m_UnclimbingGravity = pTransform->GetGravity();
	pTransform->SetGravity( 0.0f );

	pTransform->SetVelocity( Vector() );
	pTransform->SetAcceleration( Vector() );
	pTransform->SetRotationalVelocity( Angles() );

	STATIC_HASHED_STRING( Climbing );
	pStatMod->TriggerEvent( sClimbing );

	GetFramework()->GetInputSystem()->PushContext( m_ClimbInputContext );

	WB_MAKE_EVENT( HideHands, pEntity );
	WB_DISPATCH_EVENT( GetEventManager(), HideHands, pEntity );
}

void WBCompEldPlayer::EndClimb( const bool AddClimbOffImpulse )
{
	ASSERT( m_Climbing );

	m_Climbing = false;

	GetFramework()->GetInputSystem()->PopContext( m_ClimbInputContext );

	WBEntity* const				pEntity		= GetEntity();
	DEVASSERT( pEntity );

	WBCompEldTransform* const	pTransform	= pEntity->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	WBCompStatMod* const		pStatMod	= GET_WBCOMP( pEntity, StatMod );
	ASSERT( pStatMod );

	pTransform->SetGravity( m_UnclimbingGravity );

	if( AddClimbOffImpulse )
	{
		pTransform->ApplyImpulse( Vector( 0.0f, 0.0f, m_ClimbOffImpulse ) );
	}

	STATIC_HASHED_STRING( Climbing );
	pStatMod->UnTriggerEvent( sClimbing );

	WB_MAKE_EVENT( ShowHands, pEntity );
	WB_DISPATCH_EVENT( GetEventManager(), ShowHands, pEntity );
}

void WBCompEldPlayer::RestoreClimb()
{
	m_Climbing = ( m_ClimbRefs > 0 );

	WBEntity* const				pEntity		= GetEntity();
	DEVASSERT( pEntity );

	WBCompStatMod* const		pStatMod	= GET_WBCOMP( pEntity, StatMod );
	ASSERT( pStatMod );

	STATIC_HASHED_STRING( Climbing );
	pStatMod->SetEventActive( sClimbing, m_Climbing );

	if( m_Climbing )
	{
		GetFramework()->GetInputSystem()->PushContext( m_ClimbInputContext );
	}
}

void WBCompEldPlayer::TryBeginMantle( const Vector& CollisionNormal )
{
	if( m_IsMantling || !m_CanMantle || m_ClimbRefs > 0 )
	{
		return;
	}

	WBEntity* const				pEntity			= GetEntity();
	DEVASSERT( pEntity );

	WBCompEldTransform* const	pTransform		= pEntity->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	WBCompEldCollision* const	pCollision		= GET_WBCOMP( pEntity, EldCollision );
	ASSERT( pCollision );

	InputSystem* const			pInputSystem	= GetFramework()->GetInputSystem();
	ASSERT( pInputSystem );

	STATIC_HASHED_STRING( Jump );

	// TODO: Configurate if needed.
	static const float kCos45	= 0.707107f;	// Mantle angle
	const Vector Facing			= pTransform->GetOrientation().ToVector2D();

	// If we're facing the collision surface and falling and holding the jump button, try to mantle.
	if( !pCollision->IsRecentlyLanded( 0.0f ) &&
		pTransform->GetVelocity().z < 0.0f &&
		Facing.Dot( CollisionNormal ) < -kCos45 &&
		pInputSystem->IsHigh( sJump ) )
	{
		EldritchWorld* const	pWorld				= GetWorld();
		ASSERT( pWorld );

		// TODO: GrabLocation feels a bit high when making long jumps. Maybe bias it a bit lower so jumps are more reliable?
		// Right now, test at the feet means that you can only mantle in places where you would be able to land one block below.
		// And that makes sense, it just doesn't always feel right.

		const Vector			UpVector			= Vector( 0.0f, 0.0f, 1.0f );
		const Vector			Extents				= pCollision->GetExtents();
		const Vector			ExtentsZ			= Vector( 0.0f, 0.0f, Extents.z );
		const Vector			CurrentLocation		= pTransform->GetLocation();
		const Vector			GrabLocation		= CurrentLocation - ExtentsZ - CollisionNormal + UpVector;		// Check one block in front of player, one block above feet.
		const float				MantleDestinationZ	= pWorld->GetVoxelBase( GrabLocation ).z + Extents.z + EPSILON;	// Add an epsilon so we'll have room in the clearance test
		const Vector			MantleDestinationXY	= CurrentLocation - ( CollisionNormal * Extents.x );
		const Vector			MantleDestination	= Vector( MantleDestinationXY.x, MantleDestinationXY.y, MantleDestinationZ );

		CollisionInfo			ClearanceInfo;
		ClearanceInfo.m_CollideWorld	= true;
		ClearanceInfo.m_CollideEntities	= true;
		ClearanceInfo.m_CollidingEntity	= pEntity;
		ClearanceInfo.m_UserFlags		= EECF_EntityCollision;

		// Check that we still have room at our destination.
		if( pWorld->CheckClearance( MantleDestination, Extents, ClearanceInfo ) )
		{
			// Something is blocking the mantle
		}
		else
		{
			BeginMantle( MantleDestination );
		}
	}
}

void WBCompEldPlayer::BeginMantle( const Vector& MantleDestination )
{
	ASSERT( !m_IsMantling );
	m_IsMantling = true;

	WBEntity* const				pEntity			= GetEntity();
	DEVASSERT( pEntity );

	WBCompEldTransform* const	pTransform		= pEntity->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	GetFramework()->GetInputSystem()->PushContext( m_MantleInputContext );

	m_MantleDestination	= MantleDestination;

	// Make sure m_MantleVector has a significant component in both X/Y and Z.
	// As such, it does *not* indicate the actual direction to the mantle point.
	m_MantleVector		= ( MantleDestination - pTransform->GetLocation() ).GetNormalized().Get2D();
	m_MantleVector.z	= 1.0f;
	m_MantleVector.FastNormalize();

	m_CanMantle = false;
}

void WBCompEldPlayer::EndMantle()
{
	ASSERT( m_IsMantling );
	m_IsMantling = false;

	GetFramework()->GetInputSystem()->PopContext( m_MantleInputContext );
}

void WBCompEldPlayer::SetSpawnPoint()
{
	ASSERT( !m_HasSetSpawnPoint );

	WBCompEldTransform* const pTransform = GetEntity()->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	m_SpawnLocation		= pTransform->GetLocation();
	m_SpawnOrientation	= pTransform->GetOrientation();
	m_HasSetSpawnPoint	= true;
}

void WBCompEldPlayer::RestoreSpawnPoint()
{
	ASSERT( m_HasSetSpawnPoint );

	WBEntity* const				pEntity		= GetEntity();
	ASSERT( pEntity );

	WBCompEldTransform* const	pTransform	= pEntity->GetTransformComponent<WBCompEldTransform>();
	ASSERT( pTransform );

	WBCompEldCollision* const	pCollision	= GET_WBCOMP( pEntity, EldCollision );
	ASSERT( pCollision );

	EldritchWorld* const pWorld = GetWorld();
	DEVASSERT( pWorld );

	CollisionInfo Info;
	Info.m_CollideWorld		= true;
	Info.m_CollideEntities	= true;
	Info.m_CollidingEntity	= pEntity;
	Info.m_UserFlags		= EECF_EntityCollision;
	Vector SpawnLocation	= m_SpawnLocation;
	const bool FoundSpot	= pWorld->FindSpot( SpawnLocation, pCollision->GetExtents(), Info );
	ASSERT( FoundSpot );

	pTransform->SetLocation(		SpawnLocation );
	pTransform->SetVelocity(		Vector() );
	pTransform->SetAcceleration(	Vector() );
	pTransform->SetOrientation(		m_SpawnOrientation );

	if( m_IsCrouched )
	{
		BeginUncrouch();
	}
}

#if BUILD_DEV
/*virtual*/ void WBCompEldPlayer::DebugRender() const
{
	IRenderer* const pRenderer = GetFramework()->GetRenderer();

	const uint NumPathSteps = m_TEMPHACKPath.Size();
	for( uint PathIndex = 1; PathIndex < NumPathSteps; ++PathIndex )
	{
		const Vector& PrevStep = m_TEMPHACKPath[ PathIndex - 1 ];
		const Vector& NextStep = m_TEMPHACKPath[ PathIndex ];

		pRenderer->DEBUGDrawLine( PrevStep, NextStep, ARGB_TO_COLOR( 255, 255, 0, 0 ) );
	}
}
#endif	// BUILD_DEV

#if BUILD_DEV
void WBCompEldPlayer::TEMPHACKInput()
{
	// TEMPHACK!!

	const char*			kEntities[] = {	"Rock",	"RevolverPickup",	"RevolverPickupForSale",	"SnaregunPickup",	"RopegunPickup",	"Artifact",	"Artifact5",	"Key",	"DaggerPickup",	"HatchetPickup",	"PickaxePickup",	"CompassPickup",	"AmuletPickup",	"TalismanPickup",	"CharmPickup",	"HolySymbolPickup",	"MedicalKitPickup",	"TinningKitPickup",	"LocksmithKitPickup",	"CampingKitPickup",	"ConsecrationKitPickup",	"JumpBootsPickup",	"SpeedBootsPickup",	"QuietBootsPickup",	"ClimbingBootsPickup",	"SandalsPickup",	"Candle",	"RitualBook",	"Dynamite",	"SmallTorch",	"Fishman",	"FrogmanShopkeeper",	"Cultist",	"LizardmanShopkeeper",	"LizardWatson",	"Mummy",	"FlyingNPCProxy",	"Polyp",	"Shoggoth",	"ShoggothShopkeeper",	"HearWorm",	"Bug",	"Yog",	"WaterSoulPickup",	"SandSoulPickup",	"SwampSoulPickup",	"StatueTeleport",	"StatueBuild",	"StatueCloak",	"StatueHypnotize",	"StatueKnock",	"StatueLift",	"StatueSummon",	"StatueFlare" };
	float				kOffsetX[] = {	0.0f,	0.0f,				0.0f,						0.0f,				0.0f,				0.0f,		0.0f,			0.0f,	0.0f,			0.0f,				0.0f,				0.0f,				0.0f,			0.0f,				0.0f,			0.0f,				0.0f,				0.0f,				0.0f,					0.0f,				0.0f,						0.0f,				0.0f,				0.0f,				0.0f,					0.0f,				0.0f,		0.0f,			0.0f,		0.0f,			0.0f,		0.0f,					0.0f,		0.0f,					0.0f,			0.0f,		0.0f,				0.0f,		0.0f,		0.0f,					0.0f,		0.0f,	0.0f,	0.0f,				0.0f,				0.0f,				0.5f,				0.5f,			0.5f,			0.5f,				0.5f,			0.5f,			0.5f,			0.5f };
	float				kOffsetY[] = {	0.0f,	0.0f,				0.0f,						0.0f,				0.0f,				0.0f,		0.0f,			0.0f,	0.0f,			0.0f,				0.0f,				0.0f,				0.0f,			0.0f,				0.0f,			0.0f,				0.0f,				0.0f,				0.0f,					0.0f,				0.0f,						0.0f,				0.0f,				0.0f,				0.0f,					0.0f,				0.0f,		0.0f,			0.0f,		0.0f,			0.0f,		0.0f,					0.0f,		0.0f,					0.0f,			0.0f,		0.0f,				0.0f,		0.0f,		0.0f,					0.0f,		0.0f,	0.0f,	0.0f,				0.0f,				0.0f,				0.5f,				0.5f,			0.5f,			0.5f,				0.5f,			0.5f,			0.5f,			0.5f };
	float				kOffsetZ[] = {	-0.2f,	0.0f,				0.0f,						0.0f,				0.0f,				-0.45f,		-0.45f,			0.0f,	0.0f,			0.0f,				0.0f,				0.0f,				0.0f,			0.0f,				0.0f,			0.0f,				0.0f,				0.0f,				0.0f,					0.0f,				0.0f,						0.0f,				0.0f,				0.0f,				0.0f,					0.0f,				0.0f,		0.0f,			0.0f,		0.0f,			0.5f,		0.5f,					0.5f,		0.5f,					0.5f,			1.0f,		0.0f,				0.5f,		0.5f,		0.0f,					0.0f,		0.0f,	0.0f,	0.0f,				0.0f,				0.0f,				1.0f,				1.0f,			1.0f,			1.0f,				1.0f,			1.0f,			1.0f,			1.0f };

	static const uint	kEntityCount = sizeof( kEntities ) / sizeof( const char* );
	static uint			NextEntity = 0;

	WBCompEldCamera* const pCamera = GET_WBCOMP( GetEntity(), EldCamera );
	WBCompEldTransform* const pTransform = GetEntity()->GetTransformComponent<WBCompEldTransform>();
	EldritchWorld* const pWorld = GetWorld();
	Keyboard* const pKeyboard = GetFramework()->GetKeyboard();

	if( pKeyboard->OnRise( Keyboard::EB_H ) )
	{
		WBCompEldHard* const pHard = GET_WBCOMP( GetEntity(), EldHard );
		ASSERT( pHard );
		PRINTF( pHard->IsHard() ? "HARD\n" : "NORMAL\n" );
	}

	if( pKeyboard->OnRise( Keyboard::EB_G ) )
	{
		WB_MAKE_EVENT( HideHands, GetEntity() );
		WB_DISPATCH_EVENT( GetEventManager(), HideHands, GetEntity() );

		STATIC_HASHED_STRING( HUD );
		WB_MAKE_EVENT( RemoveUIScreen, NULL );
		WB_SET_AUTO( RemoveUIScreen, Hash, Screen, sHUD );
		WB_DISPATCH_EVENT( GetEventManager(), RemoveUIScreen, NULL );
	}

#if BUILD_WINDOWS
	if( pKeyboard->OnRise( Keyboard::EB_Tilde ) )
	{
		Console::Toggle();
	}
#endif

	if( pKeyboard->OnRise( Keyboard::EB_LeftBrace ) )
	{
		NextEntity = ( NextEntity + kEntityCount - 1 ) % kEntityCount;
		PRINTF( "Next entity: %s\n", kEntities[ NextEntity ] );
	}

	if( pKeyboard->OnRise( Keyboard::EB_RightBrace ) )
	{
		NextEntity = ( NextEntity + 1 ) % kEntityCount;
		PRINTF( "Next entity: %s\n", kEntities[ NextEntity ] );
	}

	if( pKeyboard->OnRise( Keyboard::EB_N ) )
	{
		EldritchNav::GetInstance()->PrintSlice( pTransform->GetLocation() );
	}

	if( pKeyboard->OnRise( Keyboard::EB_Y ) )
	{
		WBCompEldInventory* const pInventory = GET_WBCOMP( GetEntity(), EldInventory );
		STATIC_HASHED_STRING( Power );
		pInventory->DropItem( sPower, false, NULL );
	}

	if( pKeyboard->OnRise( Keyboard::EB_Mouse_Left ) )
	{
		const Vector EyeLoc = pTransform->GetLocation() + pCamera->GetViewTranslationOffset( WBCompEldCamera::EVM_All );
		const Vector EyeDir = ( pTransform->GetOrientation() + pCamera->GetViewOrientationOffset( WBCompEldCamera::EVM_All ) ).ToVector();
		const Ray TraceRay( EyeLoc, EyeDir );

		CollisionInfo Info;
		Info.m_CollideWorld = true;

		if( pWorld->Trace( TraceRay, Info ) )
		{
			const Vector HitLoc = Info.m_Intersection + Info.m_Plane.m_Normal * 0.1f;

			// TEMP, just validating this function
			CollisionInfo Info;
			Info.m_CollideWorld = true;
			ASSERT( !pWorld->CheckClearance( HitLoc, Vector(), Info ) );

			if( pKeyboard->IsHigh( Keyboard::EB_LeftControl ) )
			{
				//pWorld->RemoveLightAt( HitLoc );
				//pWorld->AddLightAt( HitLoc, 8.0f, Vector4( 0.8f, 0.6f, 0.2f, 1.0f ) );
			}
			else if( pKeyboard->IsHigh( Keyboard::EB_LeftShift ) )
			{
				const SimpleString EntityDef = kEntities[ NextEntity ];
				WBEntity* pSphereEntity = WBWorld::GetInstance()->CreateEntity( EntityDef );
				WBCompEldTransform* pSphereTransform = pSphereEntity->GetTransformComponent<WBCompEldTransform>();
				WBCompEldCollision* pSphereCollision = GET_WBCOMP( pSphereEntity, EldCollision );
				CollisionInfo Info;
				Info.m_CollideWorld		= true;
				Info.m_CollideEntities	= true;
				Info.m_CollidingEntity	= pSphereEntity;
				Info.m_UserFlags		= EECF_EntityCollision;
				const Vector VoxelLoc = pWorld->GetVoxelCenter( HitLoc );
				Vector SpawnLocation = VoxelLoc + Vector( kOffsetX[ NextEntity ], kOffsetY[ NextEntity ], kOffsetZ[ NextEntity ] );
				if( pSphereCollision )
				{
					pWorld->FindSpot( SpawnLocation, pSphereCollision->GetExtents(), Info );
				}
				pSphereTransform->SetLocation( SpawnLocation );
			}
			else if( pKeyboard->IsHigh( Keyboard::EB_LeftAlt ) )
			{
				// Test pathfinding
				EldritchNav::SPathfindingParams Params;
				Params.m_Start			= pTransform->GetLocation();
				Params.m_Destination	= HitLoc;
				Params.m_Height			= m_IsCrouched ? 1 : 2;
				Params.m_MotionType		= EldritchNav::EMT_Walking;
				Params.m_CanJump		= true;
				Params.m_CanOpenDoors	= true;
				Params.m_MaxSteps		= 1000;
				Params.m_UsePartialPath	= true;
#if BUILD_DEBUG
				Params.m_Verbose		= true;
#endif

				EldritchNav::GetInstance()->FindPath( Params, m_TEMPHACKPath );

				WBCompEldCollision* const pCollision = GET_WBCOMP( GetEntity(), EldCollision );
				EldritchNav::GetInstance()->SmoothPath( pCollision->GetExtents(), m_TEMPHACKPath );
			}
			else
			{
				//pWorld->AddVoxelAt( HitLoc );
			}
		}
	}

	if( pKeyboard->OnRise( Keyboard::EB_Mouse_Right ) )
	{
		const Vector EyeLoc = pTransform->GetLocation() + pCamera->GetViewTranslationOffset( WBCompEldCamera::EVM_All );
		const Vector EyeDir = ( pTransform->GetOrientation() + pCamera->GetViewOrientationOffset( WBCompEldCamera::EVM_All ) ).ToVector();
		const Ray TraceRay( EyeLoc, EyeDir );

		CollisionInfo Info;
		Info.m_CollideWorld = true;

		if( pWorld->Trace( TraceRay, Info ) )
		{
			if( pKeyboard->IsHigh( Keyboard::EB_LeftControl ) )
			{
				//const Vector HitLoc = Info.m_Intersection + Info.m_Plane.m_Normal * 0.1f;
				//pWorld->RemoveLightAt( HitLoc );
			}
			else if( pKeyboard->IsHigh( Keyboard::EB_LeftShift ) )
			{
				const Vector HitLoc = Info.m_Intersection + Info.m_Plane.m_Normal * 0.1f;
				const Vector VoxelLoc = pWorld->GetVoxelCenter( HitLoc );
				Array<WBEntity*> Entities;
				WBScene::GetDefault()->GetEntitiesByRadius( Entities, VoxelLoc, 0.0f );
				for( uint EntityIndex = 0; EntityIndex < Entities.Size(); ++EntityIndex )
				{
					WBEntity* pEntity = Entities[ EntityIndex ];
					pEntity->Destroy();
				}
			}
			else
			{
				//const Vector HitLoc = Info.m_Intersection - Info.m_Plane.m_Normal * 0.1f;

				//// TEMP, just validating this function
				//CollisionInfo Info;
				//Info.m_CollideWorld = true;
				//ASSERT( pWorld->CheckClearance( HitLoc, Vector(), Info ) );

				//pWorld->RemoveVoxelAt( HitLoc );
			}
		}
	}

	// Camera hacks: Shift + C + ...
	if( pKeyboard->IsHigh( Keyboard::EB_LeftShift ) && pKeyboard->IsHigh( Keyboard::EB_C ) )
	{
		WBCompEldTransform* const	pTransform	= GetEntity()->GetTransformComponent<WBCompEldTransform>();
		WBCompEldCollision* const	pCollision	= GET_WBCOMP( GetEntity(), EldCollision );
		WBCompEldHealth* const		pHealth		= GET_WBCOMP( GetEntity(), EldHealth );
		WBCompEldWallet* const		pWallet		= GET_WBCOMP( GetEntity(), EldWallet );
		WBCompEldKeyRing* const		pKeyRing	= GET_WBCOMP( GetEntity(), EldKeyRing );
		WBCompEldVisible* const		pVisible	= GET_WBCOMP( GetEntity(), EldVisible );
		WBCompEldFootsteps* const	pFootsteps	= GET_WBCOMP( GetEntity(), EldFootsteps );

		if( pKeyboard->OnRise( Keyboard::EB_I ) )
		{
			pHealth->GiveMaxHealth( 99999, 99999 );
			pWallet->AddMoney( 99999, false );
			pKeyRing->AddKeys( 99999, false );

			STATIC_HASHED_STRING( HUD );
			WB_MAKE_EVENT( RemoveUIScreen, NULL );
			WB_SET_AUTO( RemoveUIScreen, Hash, Screen, sHUD );
			WB_DISPATCH_EVENT( GetEventManager(), RemoveUIScreen, NULL );
		}

		if( pKeyboard->OnRise( Keyboard::EB_O ) )
		{
			pVisible->SetVisible( false );
			pFootsteps->SetFootstepsDisabled( true );
		}

		if( pKeyboard->OnRise( Keyboard::EB_U ) )
		{
			m_TEMPHACKCamVelocity *= 0.8f;
		}

		if( pKeyboard->OnRise( Keyboard::EB_P ) )
		{
			m_TEMPHACKCamVelocity *= 1.25f;
		}

		if( pKeyboard->OnRise( Keyboard::EB_J ) )
		{
			m_TEMPHACKCamStartLocation		= pTransform->GetLocation();
			m_TEMPHACKCamStartOrientation	= pTransform->GetOrientation();
		}

		if( pKeyboard->OnRise( Keyboard::EB_K ) )
		{
			m_TEMPHACKCamEndLocation	= pTransform->GetLocation();
			m_TEMPHACKCamEndOrientation	= pTransform->GetOrientation();
		}

		if( pKeyboard->OnRise( Keyboard::EB_L ) )
		{
			m_TEMPHACKCamActive = !m_TEMPHACKCamActive;

			if( m_TEMPHACKCamActive )
			{
				if( m_TEMPHACKCamStartLocation.IsZero() || m_TEMPHACKCamEndLocation.IsZero() )
				{
					// Don't interpolate, it will cause problems.
					m_TEMPHACKCamActive = false;
				}
				else
				{
					m_TEMPHACKCamGravity = pTransform->GetGravity();

					pCollision->SetCollisionFlags( 0 );
					pTransform->SetGravity( 0.0f );

					WB_MAKE_EVENT( HideHands, GetEntity() );
					WB_DISPATCH_EVENT( GetEventManager(), HideHands, GetEntity() );

					const float Distance = ( m_TEMPHACKCamStartLocation - m_TEMPHACKCamEndLocation ).Length();
					const float Duration = Distance / m_TEMPHACKCamVelocity;
					m_TEMPHACKCamLocation.Reset( Interpolator<Vector>::EIT_Linear, m_TEMPHACKCamStartLocation, m_TEMPHACKCamEndLocation, Duration );
					m_TEMPHACKCamOrientation.Reset( Interpolator<Angles>::EIT_Linear, m_TEMPHACKCamStartOrientation, m_TEMPHACKCamEndOrientation, Duration );
				}
			}
			else
			{
				pCollision->SetCollisionFlags( pCollision->GetDefaultCollisionFlags() );
				pTransform->SetGravity( m_TEMPHACKCamGravity );

				WB_MAKE_EVENT( ShowHands, GetEntity() );
				WB_DISPATCH_EVENT( GetEventManager(), ShowHands, GetEntity() );
			}
		}
	}
}
#endif // BUILD_DEV

/*virtual*/ void WBCompEldPlayer::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( OnSpawned );
	STATIC_HASHED_STRING( OnLoaded );
	STATIC_HASHED_STRING( OnInitialized );
	STATIC_HASHED_STRING( OnTouchedClimbable );
	STATIC_HASHED_STRING( OnUntouchedClimbable );
	STATIC_HASHED_STRING( OnLanded );
	STATIC_HASHED_STRING( OnCollided );
	STATIC_HASHED_STRING( PreLevelTransition );
	STATIC_HASHED_STRING( PostLevelTransition );
	STATIC_HASHED_STRING( PushInputContext );
	STATIC_HASHED_STRING( PopInputContext );
	STATIC_HASHED_STRING( OnDamaged );
	STATIC_HASHED_STRING( SetDamageDesc );
	STATIC_HASHED_STRING( UseJumpPower );
	STATIC_HASHED_STRING( DisablePause );
	STATIC_HASHED_STRING( EnablePause );
	STATIC_HASHED_STRING( PlayMusic );
	STATIC_HASHED_STRING( StopMusic );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sOnSpawned )
	{
		SetCrouchOverlayHidden( true );
	}
	else if( EventName == sOnLoaded )
	{
		RestoreCrouch();
		RestorePowerSlide();
		RestoreClimb();
	}
	else if( EventName == sOnInitialized )
	{
		static const float kPlayMusicDelay = 0.01f;
		WB_MAKE_EVENT( PlayMusic, NULL );
		WB_QUEUE_EVENT_DELAY( GetEventManager(), PlayMusic, GetGame(), kPlayMusicDelay );
	}
	else if( EventName == sOnTouchedClimbable )
	{
		if( ShouldAttachToClimbable( Event ) )
		{
			IncrementClimbRefs( Event );
		}
	}
	else if( EventName == sOnUntouchedClimbable )
	{
		const bool AddClimbOffImpulse = true;
		DecrementClimbRefs( AddClimbOffImpulse );
	}
	else if( EventName == sOnLanded )
	{
		if( !m_HasSetSpawnPoint )
		{
			SetSpawnPoint();
		}

		m_CanMantle = true;
		ZeroClimbRefs();
	}
	else if( EventName == sOnCollided )
	{
		if( m_IsPowerSliding )
		{
			EndPowerSlide();
		}

		STATIC_HASHED_STRING( CollisionNormal );
		const Vector CollisionNormal = Event.GetVector( sCollisionNormal );
		TryBeginMantle( CollisionNormal );
	}
	else if( EventName == sPreLevelTransition )
	{
		WB_MAKE_EVENT( PushPersistence, GetEntity() );
		WB_DISPATCH_EVENT( GetEventManager(), PushPersistence, GetEntity() );
	}
	else if( EventName == sPostLevelTransition )
	{
		const TPersistence& Persistence = EldritchGame::StaticGetTravelPersistence();
		if( Persistence.Size() )
		{
			WB_MAKE_EVENT( PullPersistence, GetEntity() );
			WB_DISPATCH_EVENT( GetEventManager(), PullPersistence, GetEntity() );
		}

		STATIC_HASHED_STRING( RestoreSpawnPoint );
		const bool ShouldRestoreSpawnPoint = Event.GetBool( sRestoreSpawnPoint );

		if( ShouldRestoreSpawnPoint && m_HasSetSpawnPoint )
		{
			RestoreSpawnPoint();
		}
	}
	else if( EventName == sPushInputContext )
	{
		STATIC_HASHED_STRING( InputContext );
		const HashedString InputContext = Event.GetHash( sInputContext );
		GetFramework()->GetInputSystem()->PushContext( InputContext );
	}
	else if( EventName == sPopInputContext )
	{
		STATIC_HASHED_STRING( InputContext );
		const HashedString InputContext = Event.GetHash( sInputContext );
		GetFramework()->GetInputSystem()->PopContext( InputContext );
	}
	else if( EventName == sOnDamaged )
	{
		if( EldritchGame::IsPlayerAlive() )
		{
			STATIC_HASHED_STRING( Damager );
			WBEntity* const pDamager = Event.GetEntity( sDamager );

			STATIC_HASHED_STRING( DamageType );
			const HashedString DamageType = Event.GetHash( sDamageType );

			m_LastDamageDesc = GetDamageDesc( pDamager, DamageType );
		}
	}
	else if( EventName == sSetDamageDesc )
	{
		STATIC_HASHED_STRING( DamageDesc );
		m_LastDamageDesc = Event.GetString( sDamageDesc );
	}
	else if( EventName == sUseJumpPower )
	{
		STATIC_HASHED_STRING( JumpPowerImpulse );
		const float JumpPowerImpulse = Event.GetFloat( sJumpPowerImpulse );

		UseJumpPower( JumpPowerImpulse );
	}
	else if( EventName == sDisablePause )
	{
		m_IsDisablingPause = true;
	}
	else if( EventName == sEnablePause )
	{
		m_IsDisablingPause = false;
	}
	else if( EventName == sPlayMusic || EventName == sStopMusic )
	{
		// Route through player to game
		GetGame()->HandleEvent( Event );
	}
}

/*virtual*/ void WBCompEldPlayer::AddContextToEvent( WBEvent& Event ) const
{
	Super::AddContextToEvent( Event );

	WBCompStatMod* const pStatMod = GET_WBCOMP( GetEntity(), StatMod );
	WB_MODIFY_FLOAT( CanUnlockDoors, 0.0f, pStatMod );
	const bool CanUnlockDoors = ( WB_MODDED( CanUnlockDoors ) != 0.0f );

	{
		WB_SET_CONTEXT( Event, Bool, CanOpenDoors, true );
		WB_SET_CONTEXT( Event, Bool, CanUnlockDoors, CanUnlockDoors );
	}
}

/*virtual*/ void WBCompEldPlayer::OnInputContextsChanged()
{
	ConditionalApplyRunningStatMods();
}

void WBCompEldPlayer::ConditionalApplyRunningStatMods()
{
	InputSystem* const			pInputSystem	= GetFramework()->GetInputSystem();
	WBEntity* const				pEntity			= GetEntity();
	WBCompStatMod* const		pStatMod		= GET_WBCOMP( pEntity, StatMod );

	STATIC_HASHED_STRING( Run );
	STATIC_HASHED_STRING( Running );
	if( pInputSystem->IsHigh( sRun ) && !m_IsCrouched )
	{
		pStatMod->SetEventActive( sRunning, true );
	}
	else
	{
		pStatMod->SetEventActive( sRunning, false );
	}
}

SimpleString WBCompEldPlayer::GetDamageDesc( WBEntity* const pDamager, const HashedString& DamageType ) const
{
	// Get the damage description components.
	const WBCompEldDamager* const pDamagerComp	= SAFE_GET_WBCOMP( pDamager, EldDamager );
	const SimpleString EntityName				= pDamager ? pDamager->GetName() : "";
	const SimpleString DamagerName				= pDamagerComp ? pDamagerComp->GetDamagerName() : EntityName;
	const SimpleString DamageTypeName			= ReverseHash::ReversedHash( DamageType );

	// Look up config names.
	STATICHASH( EldPlayer_DamageDesc );

	// First, try the most specific, fully qualified name.
	if( DamagerName != "" && DamageTypeName != "" )
	{
		const SimpleString DamageName = SimpleString::PrintF( "%s_%s", DamagerName.CStr(), DamageTypeName.CStr() );
		MAKEHASH( DamageName );

		const SimpleString DamageDesc = ConfigManager::GetString( sDamageName, "", sEldPlayer_DamageDesc );
		if( DamageDesc != "" )
		{
			return DamageDesc;
		}
	}

	// Then try the damager name alone.
	if( DamagerName != "" )
	{
		MAKEHASH( DamagerName );

		const SimpleString DamageDesc = ConfigManager::GetString( sDamagerName, "", sEldPlayer_DamageDesc );
		if( DamageDesc != "" )
		{
			return DamageDesc;
		}
	}

	// Finally, try just the damage type.
	if( DamageTypeName != "" )
	{
		MAKEHASH( DamageTypeName );

		const SimpleString DamageDesc = ConfigManager::GetString( sDamageTypeName, "", sEldPlayer_DamageDesc );
		if( DamageDesc != "" )
		{
			return DamageDesc;
		}
	}

	// Couldn't find anything, use the default.
	DEVWARNDESC( "Using default damage description!" );
	return m_DefaultDamageDesc;
}

SimpleString WBCompEldPlayer::GetLastDamageDesc() const
{
	if( m_LastDamageDesc == "" )
	{
		DEVWARNDESC( "Using default damage description because there is no last damage." );
		return m_DefaultDamageDesc;
	}
	else
	{
		return m_LastDamageDesc;
	}
}

#define VERSION_EMPTY				0
#define VERSION_CROUCHING			1
#define VERSION_CLIMBING			2
#define VERSION_UNCLIMBINGGRAVITY	3
#define VERSION_SPAWNPOINT			4
#define VERSION_POWERSLIDE			5
#define VERSION_POWERSLIDEY			6
#define VERSION_ISDISABLINGPAUSE	7
#define VERSION_CURRENT				7

uint WBCompEldPlayer::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;					// Version
	Size += 1;					// m_IsCrouched
	Size += 1;					// m_IsUncrouching
	Size += 4;					// m_ClimbRefs
	Size += 4;					// m_UnclimbingGravity
	Size += 1;					// m_HasSetSpawnPoint
	Size += sizeof( Vector );	// m_SpawnLocation
	Size += sizeof( Angles );	// m_SpawnOrientation
	Size += 1;					// m_IsPowerSliding
	Size += 4;					// Power slide time remaining
	Size += sizeof( Vector );	// m_PowerSlideY
	Size += 1;					// m_IsDisablingPause

	return Size;
}

void WBCompEldPlayer::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );

	Stream.WriteBool( m_IsCrouched );
	Stream.WriteBool( m_IsUncrouching );

	Stream.WriteInt32( m_ClimbRefs );

	Stream.WriteFloat( m_UnclimbingGravity );

	Stream.WriteBool( m_HasSetSpawnPoint );
	Stream.Write( sizeof( Vector ), &m_SpawnLocation );
	Stream.Write( sizeof( Angles ), &m_SpawnOrientation );

	Stream.WriteBool( m_IsPowerSliding );
	Stream.WriteFloat( m_PowerSlideEndTime - GetTime() );
	Stream.Write( sizeof( Vector ), &m_PowerSlideY );

	Stream.WriteBool( m_IsDisablingPause );
}

void WBCompEldPlayer::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_CROUCHING )
	{
		m_IsCrouched = Stream.ReadBool();
		m_IsUncrouching = Stream.ReadBool();
	}

	if( Version >= VERSION_CLIMBING )
	{
		m_ClimbRefs = Stream.ReadInt32();
	}

	if( Version >= VERSION_UNCLIMBINGGRAVITY )
	{
		m_UnclimbingGravity = Stream.ReadFloat();
	}

	if( Version >= VERSION_SPAWNPOINT )
	{
		m_HasSetSpawnPoint = Stream.ReadBool();
		Stream.Read( sizeof( Vector ), &m_SpawnLocation );
		Stream.Read( sizeof( Angles ), &m_SpawnOrientation );
	}

	if( Version >= VERSION_POWERSLIDE )
	{
		m_IsPowerSliding = Stream.ReadBool();
		m_PowerSlideEndTime = GetTime() + Stream.ReadFloat();
	}

	if( Version >= VERSION_POWERSLIDEY )
	{
		Stream.Read( sizeof( Vector ), &m_PowerSlideY );
	}

	if( Version >= VERSION_ISDISABLINGPAUSE )
	{
		m_IsDisablingPause = Stream.ReadBool();
	}
}
#ifndef WBCOMPELDPLAYER_H
#define WBCOMPELDPLAYER_H

#include "wbeldritchcomponent.h"
#include "wbevent.h"
#include "vector.h"
#include "iinputsystemobserver.h"
#include "interpolator.h"

class WBEvent;

class WBCompEldPlayer : public WBEldritchComponent, public IInputSystemObserver
{
public:
	WBCompEldPlayer();
	virtual ~WBCompEldPlayer();

	DEFINE_WBCOMP( EldPlayer, WBEldritchComponent );

	virtual bool	BelongsInComponentArray() { return true; }

	virtual void	Tick( float DeltaTime );
	virtual int		GetTickOrder() { return ETO_TickFirst; }	// Should tick before motion is integrated so input is applied ASAP.

	virtual void	HandleEvent( const WBEvent& Event );
	virtual void	AddContextToEvent( WBEvent& Event ) const;

#if BUILD_DEV
	virtual void	DebugRender() const;
#endif

	virtual uint	GetSerializationSize();
	virtual void	Save( const IDataStream& Stream );
	virtual void	Load( const IDataStream& Stream );

	// IInputSystemObserver
	virtual void	OnInputContextsChanged();

	SimpleString	GetLastDamageDesc() const;

	bool			IsDisablingPause() const { return m_IsDisablingPause; }

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
#if BUILD_DEV
	void			TEMPHACKInput();
#endif

	void			ConditionalApplyRunningStatMods();

	void			UseJumpPower( const float JumpPowerImpulse );

	void			Crouch();
	void			BeginUncrouch();
	void			CancelUncrouch();
	void			TryUncrouch();
	bool			CanUncrouch();
	void			Uncrouch();
	void			RestoreCrouch();
	void			SetCrouchOverlayHidden( const bool Hidden );

	void			BeginPowerSlide( const Vector& PowerSlideY );
	void			EndPowerSlide();
	void			RestorePowerSlide();

	bool			ShouldAttachToClimbable( const WBEvent& ClimbEvent );
	void			IncrementClimbRefs( const WBEvent& ClimbEvent );
	void			DecrementClimbRefs( const bool AddClimbOffImpulse );
	void			ZeroClimbRefs();		// Immediately end climb
	void			BeginClimb( const WBEvent& ClimbEvent );
	void			EndClimb( const bool AddClimbOffImpulse );
	void			RestoreClimb();

	void			TryBeginMantle( const Vector& CollisionNormal );
	void			BeginMantle( const Vector& MantleDestination );
	void			EndMantle();

	void			SetSpawnPoint();
	void			RestoreSpawnPoint();

	SimpleString	GetDamageDesc( WBEntity* const pDamager, const HashedString& DamageType ) const;

	float	m_UnlandedJumpWindow;
	float	m_UnlandedLeanWindow;
	float	m_LandAcceleration;
	float	m_AirAcceleration;
	float	m_TurnSpeed;
	float	m_JumpImpulse;

	bool	m_UncrouchOnSprint;					// Config

	bool	m_IsCrouched;
	bool	m_IsUncrouching;
	float	m_StandExtentsZ;
	float	m_CrouchExtentsZ;
	float	m_StandViewOffsetZ;
	float	m_CrouchViewOffsetZ;
	float	m_CrouchViewInterpTime;							// Config
	Interpolator<float>	m_ViewOffsetZInterpolator;			// Transient
	Interpolator<float>	m_ViewOffsetAngleRollInterpolator;	// Transient

	bool			m_IsPowerSliding;			// Serialized
	float			m_PowerSlideDuration;		// Config
	float			m_PowerSlideEndTime;		// Serialized (as time remaining)
	Vector			m_PowerSlideY;				// Serialized
	HashedString	m_PowerSlideInputContext;	// Config
	float			m_PowerSlideReqVelocitySq;	// Config
	float			m_PowerSlideRoll;			// Config
	float			m_PowerSlideRollInterpTime;	// Config

	int				m_ClimbRefs;				// Serialized; refcount climbing so we can transfer climbables without issue
	bool			m_Climbing;					// Transient
	HashedString	m_ClimbInputContext;
	float			m_UnclimbingGravity;		// Serialized
	float			m_ClimbOffImpulse;			// Config
	float			m_ClimbFacingBiasAngle;		// Config
	float			m_ClimbFacingBiasScale;		// Config

	bool			m_IsMantling;				// Transient
	HashedString	m_MantleInputContext;		// Config
	float			m_MantleVelocity;			// Config
	Vector			m_MantleVector;				// Transient
	Vector			m_MantleDestination;		// Transient
	bool			m_CanMantle;				// Transient

	bool			m_IsDisablingPause;			// Serialized

	bool			m_HasSetSpawnPoint;			// Serialized
	Vector			m_SpawnLocation;			// Serialized
	Angles			m_SpawnOrientation;			// Serialized

	SimpleString	m_LastDamageDesc;			// Transient
	SimpleString	m_DefaultDamageDesc;		// Config

#if BUILD_DEV
	Array<Vector>	m_TEMPHACKPath;

	bool					m_TEMPHACKCamActive;
	float					m_TEMPHACKCamVelocity;
	float					m_TEMPHACKCamGravity;
	Interpolator<Vector>	m_TEMPHACKCamLocation;
	Interpolator<Angles>	m_TEMPHACKCamOrientation;
	Vector					m_TEMPHACKCamStartLocation;
	Vector					m_TEMPHACKCamEndLocation;
	Angles					m_TEMPHACKCamStartOrientation;
	Angles					m_TEMPHACKCamEndOrientation;
#endif
};

#endif // WBCOMPELDPLAYER_H
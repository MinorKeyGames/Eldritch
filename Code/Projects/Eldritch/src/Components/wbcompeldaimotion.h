#ifndef WBCOMPELDAIMOTION_H
#define WBCOMPELDAIMOTION_H

#include "wbeldritchcomponent.h"
#include "eldritchworld.h"
#include "wbentityref.h"
#include "vector.h"
#include "aabb.h"
#include "irodinresourceuser.h"

class WBCompEldAIMotion : public WBEldritchComponent, public IRodinResourceUser
{
public:
	WBCompEldAIMotion();
	virtual ~WBCompEldAIMotion();

	DEFINE_WBCOMP( EldAIMotion, WBEldritchComponent );

	virtual void	Tick( float DeltaTime );
	virtual int		GetTickOrder() { return ETO_TickFirst; }	// Should tick before motion is integrated so input is applied ASAP.

	virtual void	HandleEvent( const WBEvent& Event );
	virtual void	AddContextToEvent( WBEvent& Event ) const;

#if BUILD_DEV
	virtual void	DebugRender() const;
#endif

	void			StartMove( const Vector& Destination );
	void			StartFollow( const WBEntity* const pDestinationEntity );
	void			StartWander( const float WanderTargetDistance );
	void			StartTurn( const Vector& TurnTarget );
	void			StopMove();

	void			SetReachedThreshold( const float ReachedThresholdMin, const float ReachedThresholdMax );
	void			SetDeflectionRadius( const float DeflectionRadius );

	bool			IsMoving() const		{ return ( m_MotionStatus == EMS_Moving || m_MotionStatus == EMS_Following || m_MotionStatus == EMS_TurningToFace ); }
	bool			IsLocomoting() const	{ return ( m_MotionStatus == EMS_Moving || m_MotionStatus == EMS_Following ); }
	bool			DidMoveSucceed() const	{ return m_MotionStatus == EMS_MoveSucceeded; }

	// IRodinResourceUser
	virtual bool	OnResourceStolen( const HashedString& Resource );
	virtual void	OnResourceReturned( const HashedString& Resource );

	virtual uint	GetSerializationSize();
	virtual void	Save( const IDataStream& Stream );
	virtual void	Load( const IDataStream& Stream );

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	void			TickMove();
	void			TickWalking();
	void			TickFlying();
	void			TickTurn();
	void			TryJump();

	void			ValidateFollow();
	void			RepathFollow();
	void			Repath();

	bool			GetNextStep( Vector& OutStep );

	float			GetLandAcceleration() const;
	float			GetWalkAnimationPlayRate() const;

	// Common code for StartMove, StartFollow, and RepathFollow.
	void			InternalStartMove( const Vector& Destination, const WBEntity* const pDestinationEntity );
	void			PlayAnimation( const HashedString& AnimationName, const bool Loop, const bool IgnoreIfAlreadyPlaying, const float PlayRate );
	bool			GetLastKnownLocationFor( const WBEntity* const pFollowEntity, Vector& OutLocation ) const;

	enum EMotionStatus
	{
		EMS_Still,
		EMS_Moving,
		EMS_Following,
		EMS_TurningToFace,
		EMS_MoveSucceeded,
		EMS_MoveFailed,
	};

	float			m_LandAcceleration;			// Config
	float			m_AirAcceleration;			// Config
	float			m_JumpImpulse;				// Config
	bool			m_JumpOnNextTick;			// Transient
	float			m_TurnSpeed;				// Config
	float			m_FollowValidateTime;		// Config
	float			m_NextFollowValidateTime;	// Transient
	float			m_StepReachedThresholdSq;	// Config
	float			m_TurnReachedThreshold;		// Config
	bool			m_RepathOnNextTick;			// Transient
	bool			m_Paused;					// Serialized
	bool			m_IsFlying;					// Config
	bool			m_CanJump;					// Config
	bool			m_CanOpenDoors;				// Config
	bool			m_CanUnlockDoors;			// Config
	uint			m_MaxPathSteps;				// Config
	HashedString	m_IdleAnimationName;		// Config
	HashedString	m_WalkAnimationName;		// Config
	HashedString	m_JumpAnimationName;		// Config
	HashedString	m_AnimationResource;		// Config
	Array<Vector>	m_Path;						// Transient
	uint			m_PathIndex;				// Transient
	AABB			m_PathBound;				// Transient
	EMotionStatus	m_MotionStatus;				// Transient
	bool			m_Wander;					// Transient
	float			m_WanderTargetDistance;		// Transient
	float			m_ReachedThresholdMinSq;	// Transient
	float			m_ReachedThresholdMaxSq;	// Transient
	float			m_FlyingDeflectionRadiusSq;	// Transient
	Vector			m_LastDestination;			// Transient
	vidx_t			m_LastDestinationIndex;		// Transient
	WBEntityRef		m_LastDestinationEntity;	// Transient
};

#endif // WBCOMPELDAIMOTION_H
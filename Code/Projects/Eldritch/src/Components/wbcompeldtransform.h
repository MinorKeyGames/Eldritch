#ifndef WBCOMPELDTRANSFORM_H
#define WBCOMPELDTRANSFORM_H

#include "wbeldritchcomponent.h"
#include "vector.h"
#include "angles.h"
#include "array.h"

class WBCompEldTransform : public WBEldritchComponent
{
public:
	WBCompEldTransform();
	virtual ~WBCompEldTransform();

	DEFINE_WBCOMP( EldTransform, WBEldritchComponent );

	virtual void	Initialize();

	virtual void	Tick( float DeltaTime );
	virtual int		GetTickOrder() { return ETO_TickSecond; }

	Vector	GetLocation() const { return m_Location; }
	void	SetLocation( const Vector& NewLocation );
	void	MoveBy( const Vector& Offset );

	Vector	GetVelocity() const { return m_Velocity; }
	void	SetVelocity( const Vector& NewVelocity ) { m_Velocity = NewVelocity; }
	void	ApplyImpulse( const Vector& Impulse ) { m_Velocity += Impulse; }

	Vector	GetAcceleration() const { return m_Acceleration; }
	void	SetAcceleration( const Vector& NewAcceleration ) { m_Acceleration = NewAcceleration; }

	Angles	GetOrientation() const { return m_Orientation; }
	void	SetOrientation( const Angles& NewOrientation );

	Angles	GetRotationalVelocity() const { return m_RotationalVelocity; }
	void	SetRotationalVelocity( const Angles& NewRotationalVelocity ) { m_RotationalVelocity = NewRotationalVelocity; }
	void	ApplyRotationalImpulse( const Angles& RotationalImpulse ) { m_RotationalVelocity += RotationalImpulse; }

	float	GetGravity() const { return m_Gravity; }
	void	SetGravity( const float Gravity ) { m_Gravity = Gravity; }

	bool	GetCanMove() const { return m_CanMove; }
	void	SetCanMove( const bool CanMove ) { m_CanMove = CanMove; }

	virtual void	HandleEvent( const WBEvent& Event );
	virtual void	AddContextToEvent( WBEvent& Event ) const;

	virtual uint	GetSerializationSize();
	virtual void	Save( const IDataStream& Stream );
	virtual void	Load( const IDataStream& Stream );

	virtual void	Report() const;

#if BUILD_DEV
	virtual void	DebugRender() const;
#endif

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	void	TickMotion( float DeltaTime );
	void	TickAcceleration( float DeltaTime );
	void	MoveWithOwner();

	Vector		m_Location;
	Vector		m_Velocity;
	Vector		m_Acceleration;

	float		m_Gravity;				// Config/serialized

	bool		m_UseSpeedLimit;		// Transient
	float		m_SpeedLimit;			// Config, applied only to acceleration

	bool		m_AllowImpulses;		// Config

	Angles		m_Orientation;
	Angles		m_RotationalVelocity;

	bool		m_CanMove;				// Config/serialized

	bool		m_IsAttachedToOwner;	// Config
	Vector		m_OwnerOffset;			// Config

	bool		m_IsSettled;			// Serialized
};

#endif // WBCOMPELDTRANSFORM_H
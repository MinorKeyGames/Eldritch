#ifndef WBCOMPELDDOOR_H
#define WBCOMPELDDOOR_H

#include "wbeldritchcomponent.h"
#include "interpolator.h"
#include "vector.h"
#include "angles.h"
#include "simplestring.h"

class WBCompEldDoor : public WBEldritchComponent
{
public:
	WBCompEldDoor();
	virtual ~WBCompEldDoor();

	DEFINE_WBCOMP( EldDoor, WBEldritchComponent );

	virtual void	Tick( float DeltaTime );
	virtual int		GetTickOrder() { return ETO_TickSecond; }	// Tick at the same priority as Transform

	virtual void	HandleEvent( const WBEvent& Event );
	virtual void	AddContextToEvent( WBEvent& Event ) const;

	virtual uint	GetSerializationSize();
	virtual void	Save( const IDataStream& Stream );
	virtual void	Load( const IDataStream& Stream );

	bool			IsLocked() const { return m_Locked; }

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	enum EDoorRotation
	{
		EDR_0,
		EDR_90,
		EDR_180,
		EDR_270,
	};

	void			TryToggle( WBEntity* const pFrobber );
	void			Toggle();
	void			Lock();
	void			Unlock();
	void			UpdateFromOpenState( const bool InitialSetup, const bool SuppressWorldChange );
	bool			CanOpenDoor( WBEntity* const pFrobber );

	EDoorRotation	GetRotation() const;
	void			AdjustForFacing();
	Vector			RotateExtents( const Vector& Extents, const EDoorRotation Rotation ) const;
	Vector			RotateOffset( const Vector& Offset, const EDoorRotation Rotation ) const;
	float			RotateYaw( const float Yaw, const EDoorRotation Rotation ) const;

	bool		m_Open;
	bool		m_Locked;					// Config/serialized

	float		m_InterpTime;				// Config
	Vector		m_ClosedOffset;				// Config
	Angles		m_ClosedOrientation;		// Config
	Vector		m_ClosedFrobOffset;			// Config
	Vector		m_ClosedFrobExtents;		// Config
	Vector		m_ClosedIrradianceOffset;	// Config
	Vector		m_OpenOffset;				// Config
	Angles		m_OpenOrientation;			// Config
	Vector		m_OpenFrobOffset;			// Config
	Vector		m_OpenFrobExtents;			// Config
	Vector		m_OpenIrradianceOffset;		// Config

	Interpolator<Vector>	m_OffsetInterpolator;		// Transient
	Interpolator<Angles>	m_OrientationInterpolator;	// Transient

	SimpleString	m_UnlockedMesh;			// Config
	SimpleString	m_LockedMesh;			// Config
	SimpleString	m_UnlockedTexture;		// Config
	SimpleString	m_LockedTexture;		// Config
	SimpleString	m_UnlockedFriendlyName;	// Config
	SimpleString	m_LockedFriendlyName;	// Config
};

#endif // WBCOMPELDDOOR_H
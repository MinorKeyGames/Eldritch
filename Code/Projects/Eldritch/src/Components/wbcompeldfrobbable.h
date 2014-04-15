#ifndef WBCOMPELDFROBBABLE_H
#define WBCOMPELDFROBBABLE_H

#include "wbeldritchcomponent.h"
#include "vector.h"
#include "simplestring.h"

class WBEvent;

class WBCompEldFrobbable : public WBEldritchComponent
{
public:
	WBCompEldFrobbable();
	virtual ~WBCompEldFrobbable();

	DEFINE_WBCOMP( EldFrobbable, WBEldritchComponent );

	virtual bool	BelongsInComponentArray() { return true; }

	virtual int		GetTickOrder() { return ETO_NoTick; }

	virtual void	HandleEvent( const WBEvent& Event );
	virtual void	AddContextToEvent( WBEvent& Event ) const;

#if BUILD_DEV
	virtual void	DebugRender() const;
#endif

	virtual uint	GetSerializationSize();
	virtual void	Save( const IDataStream& Stream );
	virtual void	Load( const IDataStream& Stream );

	bool	IsFrobbable() const { return m_IsFrobbable; }

	void	SetBoundOffset( const Vector& Offset ) { m_BoundOffset = Offset; }
	Vector	GetBoundOffset() const { return m_BoundOffset; }

	void	SetBoundExtents( const Vector& Extents ) { m_BoundExtents = Extents; }
	Vector	GetBoundExtents() const { return m_BoundExtents; }

	AABB	GetBound() const;

	void	SetIsFrobTarget( const bool IsFrobTarget, WBEntity* const pFrobber );
	bool	GetIsFrobTarget() const { return m_IsProbableFrobbable; }

	bool	GetUseMeshExtents() { return m_UseMeshExtents; }

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	void	MarshalFrob( WBEntity* const pFrobber, const int InputEdge );
	void	SendOnFrobbedEvent( WBEntity* const pFrobber ) const;
	void	SendOnFrobbedHeldEvent( WBEntity* const pFrobber ) const;

	void	PublishToHUD() const;
	void	SetHUDHidden( const bool Hidden ) const;

	bool	m_IsFrobbable;			// Serialized

	bool	m_IsProbableFrobbable;	// Transient; are we the focus of a frobber?

	bool	m_HoldReleaseMode;		// Config/serialized: supports two frob reactions, one for held input and one for released
	bool	m_HandleHoldRelease;	// Transient, state that says we've received an OnRise event and will handle a hold or release

	bool	m_UseCollisionExtents;	// Config: if true, uses collisions's extents (else, uses configured extents)
	bool	m_UseMeshExtents;		// Config: if true, uses mesh's AABB
	float	m_ExtentsFatten;		// If using collision or mesh extents, optionally fatten up the bounds

	Vector	m_BoundOffset;			// Config/serialized
	Vector	m_BoundExtents;			// Config/serialized

	Vector	m_Highlight;			// Config

	SimpleString	m_FriendlyName;	// Config/serialized
	SimpleString	m_FrobVerb;		// Config/serialized
	SimpleString	m_HoldVerb;		// Config
};

#endif // WBCOMPELDFROBBABLE_H
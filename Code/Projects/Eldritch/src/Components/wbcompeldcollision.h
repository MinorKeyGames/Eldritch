#ifndef WBCOMPELDCOLLISION_H
#define WBCOMPELDCOLLISION_H

#include "wbeldritchcomponent.h"
#include "vector.h"
#include "array.h"
#include "wbentityref.h"
#include "aabb.h"
#include "map.h"

class WBCompEldCollision : public WBEldritchComponent
{
public:
	WBCompEldCollision();
	virtual ~WBCompEldCollision();

	DEFINE_WBCOMP( EldCollision, WBEldritchComponent );

	virtual bool	BelongsInComponentArray() { return true; }

	void	Collide( const Vector& StartLocation, Vector& InOutMovement );

	void	OnLanded( const float LandedMagnitude, WBEntity* const pCollidedEntity );
	void	OnCollided( const Vector& CollisionNormal, WBEntity* const pCollidedEntity );

	void	Jump();
	void	Fall();
	bool	IsLanded() const { return m_Landed; }
	bool	IsRecentlyLanded( const float TimeThreshold ) const;	// Returns landed status within a threshold; never returns true if jumping.

	const AABB&	GetBounds() const { /*ASSERT( m_Bounds.Equals( GetCurrentBounds() ) );*/ return m_Bounds; }
	Vector		GetExtents() const { return m_HalfExtents; }
	void		SetExtents( const Vector& HalfExtents );

	float		GetFrictionCoefficient() const;

	// Uses EECF flags defined in eldritchworld.h
	inline uint	GetCollisionFlags() const { return m_CollisionFlags; }
	inline uint	GetDefaultCollisionFlags() const { return m_DefaultCollisionFlags; }
	void		SetCollisionFlags( const uint Flags, const uint Mask = 0xffffffff, const bool SendEvents = false, const bool UpdateCollisionMap = true );
	bool		MatchesAllCollisionFlags( const uint Flags ) const { return ( m_CollisionFlags & Flags ) == Flags; }
	bool		MatchesAnyCollisionFlags( const uint Flags ) const { return ( m_CollisionFlags & Flags ) > 0; }

	void		GetTouchingEntities( Array<WBEntity*>& OutTouchingEntities ) const;

	virtual void	HandleEvent( const WBEvent& Event );
	virtual void	AddContextToEvent( WBEvent& Event ) const;

#if BUILD_DEV
	virtual void	DebugRender() const;
#endif

	virtual uint	GetSerializationSize();
	virtual void	Save( const IDataStream& Stream );
	virtual void	Load( const IDataStream& Stream );

	typedef Array<WBCompEldCollision*>	TCollisionArray;
	typedef Map<uint, TCollisionArray>	TCollisionMap;

	static const TCollisionArray*	GetCollisionArray( const uint Flags );
	static const TCollisionArray&	GetTouchingArray() { return sm_TouchingArray; }

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	void			GatherTouching( Array<WBEntityRef>& OutTouching ) const;
	void			UpdateTouching();	// This also sends touch events
	void			SendTouchEvent( const WBEntityRef& TouchingEntity );
	void			SendUntouchEvent( const WBEntityRef& TouchingEntity );

	void			AddTouching( const WBEntityRef& Entity ) { m_Touching.PushBack( Entity ); }
	void			RemoveTouching( const WBEntityRef& Entity ) { m_Touching.Remove( Entity ); }

	void			UpdateBounds() { m_Bounds = GetCurrentBounds(); }
	AABB			GetCurrentBounds() const;

	void			AddToCollisionMap();
	void			AddToCollisionArray( const uint Flags );
	void			AddToTouchingArray();

	void			RemoveFromCollisionMap();
	void			RemoveFromCollisionArray( const uint Flags );
	void			RemoveFromTouchingArray();

	void			ConditionalSetNavBlocking( const bool NavBlocking );
	void			ConditionalSendStaticCollisionChangedEvent();

	Vector				m_HalfExtents;						// Config/serialized
	AABB				m_Bounds;							// Transient, constructed from location and half extents

	float				m_Elasticity;						// Config
	float				m_FrictionTargetTime;				// Config
	float				m_FrictionTargetPercentVelocity;	// Config

	bool				m_IsNavBlocking;					// Transient

	bool				m_Landed;							// Transient
	float				m_UnlandedTime;						// Transient

	// m_Touching used to be a set for quicker Removes; I'm trying an Array for quicker UpdateTouching.
	Array<WBEntityRef>	m_Touching;							// Serialized
	bool				m_CanTouch;							// Config/serialized

	uint				m_CollisionFlags;					// Config/serialized, see EldritchWorld::ECollisionFlags
	uint				m_DefaultCollisionFlags;			// Config

	// Map from collision flags to collidables that match those flags
	static TCollisionMap	sm_CollisionMap;
	static TCollisionArray	sm_TouchingArray;
};

#endif // WBCOMPELDCOLLISION_H
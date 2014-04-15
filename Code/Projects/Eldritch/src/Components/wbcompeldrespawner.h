#ifndef WBCOMPELDRESPAWNER_H
#define WBCOMPELDRESPAWNER_H

#include "wbeldritchcomponent.h"
#include "vector.h"
#include "angles.h"

class WBCompEldRespawner : public WBEldritchComponent
{
public:
	WBCompEldRespawner();
	virtual ~WBCompEldRespawner();

	DEFINE_WBCOMP( EldRespawner, WBEldritchComponent );

	virtual int		GetTickOrder() { return ETO_NoTick; }

	virtual void	HandleEvent( const WBEvent& Event );

	virtual uint	GetSerializationSize();
	virtual void	Save( const IDataStream& Stream );
	virtual void	Load( const IDataStream& Stream );

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	void	TryRespawn();
	void	Respawn();

	bool	CanRespawn();
	bool	IsOriginNearPlayer();
	bool	CanOriginBeSeenByPlayer();

	bool	m_OriginSet;			// Serialized
	Vector	m_OriginLocation;		// Serialized
	Angles	m_OriginOrientation;	// Serialized

	float	m_RetryRespawnTime;				// Config
	float	m_RespawnMinPlayerDistanceSq;	// Config
};

#endif // WBCOMPELDRESPAWNER_H
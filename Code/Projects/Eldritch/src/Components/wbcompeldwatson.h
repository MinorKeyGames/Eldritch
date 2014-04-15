#ifndef WBCOMPELDWATSON_H
#define WBCOMPELDWATSON_H

#include "wbeldritchcomponent.h"
#include "vector.h"

class WBCompEldWatson : public WBEldritchComponent
{
public:
	WBCompEldWatson();
	virtual ~WBCompEldWatson();

	DEFINE_WBCOMP( EldWatson, WBEldritchComponent );

	virtual int		GetTickOrder() { return ETO_TickDefault; }
	virtual void	Tick( float DeltaTime );

	virtual uint	GetSerializationSize();
	virtual void	Save( const IDataStream& Stream );
	virtual void	Load( const IDataStream& Stream );

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	void			TickWatson();
	void			TickUnactivated();
	void			TickActivated();
	void			TickPrimed();

	bool			IsPlayerVulnerable( const bool CheckMinDistance ) const;

	bool	m_Activated;		// Serialized
	bool	m_Primed;			// Serialized
	float	m_MinAttackDistSq;	// Config
	float	m_MaxAttackDistSq;	// Config
	float	m_TeleportDist;		// Config
	float	m_EyeOffsetZ;		// Config
};

#endif // WBCOMPELDWATSON_H
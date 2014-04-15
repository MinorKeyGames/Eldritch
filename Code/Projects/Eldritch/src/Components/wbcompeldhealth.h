#ifndef WBCOMPELDHEALTH_H
#define WBCOMPELDHEALTH_H

#include "wbeldritchcomponent.h"
#include "wbeventmanager.h"
#include "map.h"

class WBCompEldHealth : public WBEldritchComponent
{
public:
	WBCompEldHealth();
	virtual ~WBCompEldHealth();

	DEFINE_WBCOMP( EldHealth, WBEldritchComponent );

	virtual int		GetTickOrder() { return ETO_NoTick; }

	virtual void	HandleEvent( const WBEvent& Event );
	virtual void	AddContextToEvent( WBEvent& Event ) const;

	virtual uint	GetSerializationSize();
	virtual void	Save( const IDataStream& Stream );
	virtual void	Load( const IDataStream& Stream );

	int				GetHealth() const { return m_Health; }
	bool			IsAlive() const { return m_Health > 0; }
	bool			IsDead() const { return m_Health <= 0; }
	void			TakeDamage( const int DamageAmount, WBEntity* const pDamager, const HashedString& DamageType );
	int				ModifyDamageAmount( const int DamageAmount, const HashedString& DamageType );
	void			GiveHealth( const int HealthAmount );
	void			GiveMaxHealth( const int MaxHealthAmount, const int HealthAmount );
	void			RestoreHealth( const int TargetHealth );	// Top off health at this value.

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	void			PublishToHUD() const;
	void			PushPersistence() const;
	void			PullPersistence();

	int		m_Health;			// Serialized
	int		m_MaxHealth;		// Serialized
	int		m_InitialHealth;	// Config
	bool	m_PublishToHUD;		// Config

	float	m_DamageTimeout;	// Config
	float	m_NextDamageTime;	// Serialized (as time remaining)

	bool	m_Invulnerable;		// Serialized

	// Pickup UI management
	float		m_HidePickupScreenDelay;
	TEventUID	m_HidePickupScreenUID;

	struct SDamageTypeMod
	{
		SDamageTypeMod()
		:	m_Multiply( 0.0f )
		,	m_Add( 0.0f )
		{
		}

		float	m_Multiply;
		float	m_Add;
	};

	SDamageTypeMod						m_DefaultDamageMod;
	Map<HashedString, SDamageTypeMod>	m_DamageTypeMods;
};

#endif // WBCOMPELDHEALTH_H
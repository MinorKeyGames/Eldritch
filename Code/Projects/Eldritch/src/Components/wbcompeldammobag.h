#ifndef WBCOMPELDAMMOBAG_H
#define WBCOMPELDAMMOBAG_H

#include "wbeldritchcomponent.h"
#include "map.h"
#include "wbeventmanager.h"

class WBCompEldAmmoBag : public WBEldritchComponent
{
public:
	WBCompEldAmmoBag();
	virtual ~WBCompEldAmmoBag();

	DEFINE_WBCOMP( EldAmmoBag, WBEldritchComponent );

	virtual int		GetTickOrder() { return ETO_NoTick; }

	virtual void	HandleEvent( const WBEvent& Event );
	virtual void	AddContextToEvent( WBEvent& Event ) const;

	virtual uint	GetSerializationSize();
	virtual void	Save( const IDataStream& Stream );
	virtual void	Load( const IDataStream& Stream );

	uint			GetAmmo( const HashedString& AmmoType ) const;
	bool			HasAmmo( const HashedString& AmmoType ) const { return GetAmmo( AmmoType ) > 0; }
	void			AddAmmo( const HashedString& AmmoType, uint Ammo );
	void			RemoveAmmo( const HashedString& AmmoType );

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	void			PublishToHUD() const;
	void			ConditionalHideHUD();
	void			SetHUDHidden( const bool Hidden ) const;

	void			PushPersistence() const;
	void			PullPersistence();

	// Map from ammo type to amount
	Map<HashedString, uint>	m_Ammo;
	HashedString			m_CurrentAmmo;	// Transient, cache so we don't need to look it up on the current weapon

	// Turning this off only works because I ended up using universal ammo!
	bool					m_HideAmmoHUD;	// Config, hides ammo HUD when no weapon is equipped.
	HashedString			m_DefaultAmmo;	// Config, the universal ammo type when no weapon is equipped.

	// Pickup UI management
	float					m_HidePickupScreenDelay;
	TEventUID				m_HidePickupScreenUID;
};

#endif // WBCOMPELDAMMOBAG_H
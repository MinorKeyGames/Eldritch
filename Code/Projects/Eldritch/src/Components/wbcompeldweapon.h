#ifndef WBCOMPELDWEAPON_H
#define WBCOMPELDWEAPON_H

#include "wbeldritchcomponent.h"
#include "simplestring.h"

class WBCompEldWeapon : public WBEldritchComponent
{
public:
	WBCompEldWeapon();
	virtual ~WBCompEldWeapon();

	DEFINE_WBCOMP( EldWeapon, WBEldritchComponent );

	virtual int		GetTickOrder() { return ETO_NoTick; }

	virtual void	AddContextToEvent( WBEvent& Event ) const;

	HashedString	GetAmmo() const { return m_Ammo; }
	SimpleString	GetWeaponIcon() const { return m_WeaponIcon; }

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	HashedString	m_Ammo;

	// HACK
	SimpleString	m_WeaponIcon;
};

#endif // WBCOMPELDWEAPON_H
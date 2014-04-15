#ifndef WBCOMPELDPICKUP_H
#define WBCOMPELDPICKUP_H

// This has morphed into a purchaseable component as well as a pickup. Oh well.

#include "wbeldritchcomponent.h"
#include "simplestring.h"

class WBCompEldPickup : public WBEldritchComponent
{
public:
	WBCompEldPickup();
	virtual ~WBCompEldPickup();

	DEFINE_WBCOMP( EldPickup, WBEldritchComponent );

	virtual int		GetTickOrder() { return ETO_NoTick; }

	virtual void	HandleEvent( const WBEvent& Event );

	virtual uint	GetSerializationSize();
	virtual void	Save( const IDataStream& Stream );
	virtual void	Load( const IDataStream& Stream );

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	void			GiveItemTo( WBEntity* const pEntity ) const;
	void			SellItemTo( WBEntity* const pEntity );
	void			AcceptPaymentFrom( WBEntity* const pEntity ) const;

	void			NotifyPurchasedBy( WBEntity* const pEntity ) const;
	void			NotifyTheftBy( WBEntity* const pEntity ) const;

	void			RegisterForPurchaseEvents();
	void			UnregisterForPurchaseEvents();

	SimpleString	m_GiveItemDef;
	uint			m_Price;		// Serialized
	SimpleString	m_FriendlyName;	// TODO: Unify with frobbable friendly name?
	SimpleString	m_FriendlyDesc;

	// HACK: Keep track of which pickup is being considered when purchase screen is open.
	static WBCompEldPickup*	sm_PurchasePickup;
	static WBEntity*		sm_Purchaser;
};

#endif // WBCOMPELDPICKUP_H
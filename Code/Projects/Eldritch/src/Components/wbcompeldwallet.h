#ifndef WBCOMPELDWALLET_H
#define WBCOMPELDWALLET_H

#include "wbeldritchcomponent.h"
#include "wbeventmanager.h"

class WBCompEldWallet : public WBEldritchComponent
{
public:
	WBCompEldWallet();
	virtual ~WBCompEldWallet();

	DEFINE_WBCOMP( EldWallet, WBEldritchComponent );

	virtual int		GetTickOrder() { return ETO_NoTick; }

	virtual void	HandleEvent( const WBEvent& Event );
	virtual void	AddContextToEvent( WBEvent& Event ) const;

	virtual uint	GetSerializationSize();
	virtual void	Save( const IDataStream& Stream );
	virtual void	Load( const IDataStream& Stream );

	uint			GetLimit() const { return m_Limit; }
	uint			GetMoney() const { return m_Money; }
	bool			HasMoney( const uint Money ) { return m_Money >= Money; }
	void			AddMoney( const uint Money, const bool ShowPickupScreen );
	void			RemoveMoney( const uint Money );

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	void			PublishToHUD() const;
	void			PushPersistence() const;
	void			PullPersistence();

	uint	m_Money;
	uint	m_Limit;

	// Pickup UI management
	float		m_HidePickupScreenDelay;
	TEventUID	m_HidePickupScreenUID;
};

#endif // WBCOMPELDWALLET_H
#ifndef WBCOMPELDKEYRING_H
#define WBCOMPELDKEYRING_H

#include "wbeldritchcomponent.h"
#include "wbeventmanager.h"

class WBCompEldKeyRing : public WBEldritchComponent
{
public:
	WBCompEldKeyRing();
	virtual ~WBCompEldKeyRing();

	DEFINE_WBCOMP( EldKeyRing, WBEldritchComponent );

	virtual int		GetTickOrder() { return ETO_NoTick; }

	virtual void	HandleEvent( const WBEvent& Event );
	virtual void	AddContextToEvent( WBEvent& Event ) const;

	virtual uint	GetSerializationSize();
	virtual void	Save( const IDataStream& Stream );
	virtual void	Load( const IDataStream& Stream );

	uint			GetKeys() const { return m_Keys; }
	bool			HasKeys() { return m_Keys > 0; }
	void			AddKeys( const uint Keys, const bool ShowPickupScreen );
	void			RemoveKey();

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	void			PublishToHUD() const;
	void			PushPersistence() const;
	void			PullPersistence();

	uint	m_Keys;

	// Pickup UI management
	float		m_HidePickupScreenDelay;
	TEventUID	m_HidePickupScreenUID;
};

#endif // WBCOMPELDKEYRING_H
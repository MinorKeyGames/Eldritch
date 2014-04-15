#ifndef WBCOMPELDENDGAMECOUNTER_H
#define WBCOMPELDENDGAMECOUNTER_H

#include "wbeldritchcomponent.h"
#include "wbeventmanager.h"

class WBCompEldEndgameCounter : public WBEldritchComponent
{
public:
	WBCompEldEndgameCounter();
	virtual ~WBCompEldEndgameCounter();

	DEFINE_WBCOMP( EldEndgameCounter, WBEldritchComponent );

	virtual int		GetTickOrder() { return ETO_NoTick; }

	virtual void	HandleEvent( const WBEvent& Event );
	virtual void	AddContextToEvent( WBEvent& Event ) const;

	virtual uint	GetSerializationSize();
	virtual void	Save( const IDataStream& Stream );
	virtual void	Load( const IDataStream& Stream );

	uint			GetCount() const { return m_Count; }

private:
private:
	void			PublishToHUD() const;
	void			SetHUDHidden( const bool Hidden ) const;

	void			PushPersistence() const;
	void			PullPersistence();

	uint			m_Count;
};

#endif // WBCOMPELDENDGAMECOUNTER_H
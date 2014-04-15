#ifndef WBCOMPELDHARDLISTENER_H
#define WBCOMPELDHARDLISTENER_H

#include "wbeldritchcomponent.h"

class WBCompEldHardListener : public WBEldritchComponent
{
public:
	WBCompEldHardListener();
	virtual ~WBCompEldHardListener();

	DEFINE_WBCOMP( EldHardListener, WBEldritchComponent );

	virtual int		GetTickOrder() { return ETO_NoTick; }

	virtual void	HandleEvent( const WBEvent& Event );

private:
	void			RegisterForEvents();
	void			UnregisterForEvents();
};

#endif // WBCOMPELDHARDLISTENER_H
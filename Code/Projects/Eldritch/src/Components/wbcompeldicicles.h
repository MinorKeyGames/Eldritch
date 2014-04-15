#ifndef WBCOMPELDICICLES_H
#define WBCOMPELDICICLES_H

#include "wbeldritchcomponent.h"

class WBCompEldIcicles : public WBEldritchComponent
{
public:
	WBCompEldIcicles();
	virtual ~WBCompEldIcicles();

	DEFINE_WBCOMP( EldIcicles, WBEldritchComponent );

	virtual int		GetTickOrder() { return ETO_TickDefault; }
	virtual void	Tick( float DeltaTime );

	virtual void	HandleEvent( const WBEvent& Event );

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	float	m_CheckDistance;	// Config
};

#endif // WBCOMPELDICICLES_H
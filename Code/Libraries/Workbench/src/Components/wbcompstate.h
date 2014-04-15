#ifndef WBCOMPSTATE_H
#define WBCOMPSTATE_H

#include "wbcomponent.h"

class WBCompState : public WBComponent
{
public:
	WBCompState();
	virtual ~WBCompState();

	DEFINE_WBCOMP( State, WBComponent );

	virtual int		GetTickOrder() { return ETO_NoTick; }

	virtual void	HandleEvent( const WBEvent& Event );
	virtual void	AddContextToEvent( WBEvent& Event ) const;

	virtual uint	GetSerializationSize();
	virtual void	Save( const IDataStream& Stream );
	virtual void	Load( const IDataStream& Stream );

	virtual void	Report() const;

	HashedString	GetState() const { return m_State; }
	void			SetState( const HashedString& NewState ) { m_State = NewState; }

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	HashedString	m_State;
};

#endif // WBCOMPSTATE_H
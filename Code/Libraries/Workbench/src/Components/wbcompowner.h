#ifndef WBCOMPOWNER_H
#define WBCOMPOWNER_H

#include "wbcomponent.h"
#include "wbentityref.h"

class WBCompOwner : public WBComponent
{
public:
	WBCompOwner();
	virtual ~WBCompOwner();

	DEFINE_WBCOMP( Owner, WBComponent );

	virtual int			GetTickOrder() { return ETO_NoTick; }

	virtual void		HandleEvent( const WBEvent& Event );

	virtual uint		GetSerializationSize();
	virtual void		Save( const IDataStream& Stream );
	virtual void		Load( const IDataStream& Stream );

	virtual void		Report() const;

	void				SetOwner( WBEntity* const pNewOwner );
	WBEntity*			GetOwner() const;

	static WBEntity*	GetTopmostOwner( WBEntity* pEntity );

private:
	WBEntityRef			m_Owner;
};

#endif // WBCOMPOWNER_H
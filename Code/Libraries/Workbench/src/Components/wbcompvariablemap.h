#ifndef WBCOMPVARIABLEMAP_H
#define WBCOMPVARIABLEMAP_H

#include "wbcomponent.h"
#include "wbevent.h"

class WBCompVariableMap : public WBComponent
{
public:
	WBCompVariableMap();
	virtual ~WBCompVariableMap();

	DEFINE_WBCOMP( VariableMap, WBComponent );

	virtual int		GetTickOrder() { return ETO_NoTick; }

	virtual void	HandleEvent( const WBEvent& Event );

	virtual uint	GetSerializationSize();
	virtual void	Save( const IDataStream& Stream );
	virtual void	Load( const IDataStream& Stream );

	virtual void	Report() const;

	WBEvent&		GetVariables() { return m_Variables; }
	const WBEvent&	GetVariables() const { return m_Variables; }

private:
	WBEvent	m_Variables;	// Serialized
};

#endif // WBCOMPVARIABLEMAP_H
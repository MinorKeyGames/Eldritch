#ifndef WBCOMPELDFROBBER_H
#define WBCOMPELDFROBBER_H

#include "wbeldritchcomponent.h"
#include "wbentityref.h"

class WBCompEldFrobber : public WBEldritchComponent
{
public:
	WBCompEldFrobber();
	virtual ~WBCompEldFrobber();

	DEFINE_WBCOMP( EldFrobber, WBEldritchComponent );

	virtual void	Tick( float DeltaTime );
	virtual int		GetTickOrder() { return ETO_TickDefault; }	// Needs to tick after transform.

	virtual void	HandleEvent( const WBEvent& Event );
	virtual void	AddContextToEvent( WBEvent& Event ) const;

	virtual uint	GetSerializationSize();
	virtual void	Save( const IDataStream& Stream );
	virtual void	Load( const IDataStream& Stream );

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	void		TryFrob( const int InputEdge );
	WBEntity*	FindTargetFrobbable() const;

	void		OnSetFrobTarget( WBEntity* const pFrobTarget );
	void		OnUnsetFrobTarget( WBEntity* const pFrobTarget );

	float		m_FrobDistance;	// Config
	WBEntityRef	m_FrobTarget;	// Transient
	bool		m_FrobDisabled;	// Serialized
};

#endif // WBCOMPELDFROBBER_H
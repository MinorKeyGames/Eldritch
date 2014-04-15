#ifndef WBCOMPELDCLIMBABLE_H
#define WBCOMPELDCLIMBABLE_H

#include "wbeldritchcomponent.h"
#include "plane.h"

class WBCompEldClimbable : public WBEldritchComponent
{
public:
	WBCompEldClimbable();
	virtual ~WBCompEldClimbable();

	DEFINE_WBCOMP( EldClimbable, WBEldritchComponent );

	virtual int		GetTickOrder() { return ETO_NoTick; }

	virtual void	HandleEvent( const WBEvent& Event );
	virtual void	AddContextToEvent( WBEvent& Event ) const;

	virtual uint	GetSerializationSize();
	virtual void	Save( const IDataStream& Stream );
	virtual void	Load( const IDataStream& Stream );

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	void			InitializeSnapPlane();

	bool			m_UseSnapPlane;	// Config
	Plane			m_SnapPlane;	// Serialized
};

#endif // WBCOMPELDCLIMBABLE_H
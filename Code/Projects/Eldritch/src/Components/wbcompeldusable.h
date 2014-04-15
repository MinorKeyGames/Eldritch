#ifndef WBCOMPELDUSABLE_H
#define WBCOMPELDUSABLE_H

#include "wbeldritchcomponent.h"

class WBCompEldUsable : public WBEldritchComponent
{
public:
	WBCompEldUsable();
	virtual ~WBCompEldUsable();

	DEFINE_WBCOMP( EldUsable, WBEldritchComponent );

	virtual int		GetTickOrder() { return ETO_NoTick; }

	virtual void	HandleEvent( const WBEvent& Event );

	virtual uint	GetSerializationSize();
	virtual void	Save( const IDataStream& Stream );
	virtual void	Load( const IDataStream& Stream );

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	void			MarshalUsedEvent( const int InputEdge );
	void			TryUse();
	void			SendOnUsedEvent() const;
	void			SendOnUsedHeldEvent() const;

	bool	m_HoldReleaseMode;	// Config: supports two used reactions, one for held input and one for released

	float	m_RefireRate;		// Config
	float	m_RefireTime;		// Serialized (as time remaining)
};

#endif // WBCOMPELDUSABLE_H